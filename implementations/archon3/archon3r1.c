/*
*	Archon 3-rel-1	(C) kvark, 2006
*		Archon project
*	Burrows-Wheeler Transformation algoritm
*
*		Anchors + DeepRay + Lazy
*		Direct sorting (a<b>=c)
*		Units: dword, no bswap
*/

//#define SUF_CHECK
#define NDEBUG
#define VERBLEV	0

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>
#include <limits.h>
#include <assert.h>

#define INSERT	5
#define DEEP	100
#define OVER	1000
#define TERM	((1+OVER)*sizeof(uint))
#define LCPART	7
#define ABIT	7
const int AMASK = (1<<ABIT)-1;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef int trax2[0x10001];

int BS,n,*p;
int lim,baza,ch;
uchar *s_bin,sym;
uchar *sfin, *bin;
int rb[0x100];
trax2 r,r2a;

int call_indiana, call_fw_anch, call_bk_anch;
int call_digger, call_deep_ray, call_lazy;

uchar *mak;
int numb,ndis,nlcp;
uchar *offs;
int **anch;
short *lcp;

clock_t t0;
FILE *fi,*fo;
int lucky;
int fsize,memory;

void ray(int *, int *, uchar *);
void encode();
void decode();
void init_hardcore()	{
	mak = malloc(numb);
	memset(mak, 0, numb);
	memory += numb*sizeof(uchar);
	offs = malloc(ndis * sizeof(uchar));
	anch = malloc(ndis * sizeof(int*));
	memset(offs, 0, ndis*sizeof(uchar));
	memset(anch, 0, ndis*sizeof(int*));
	memory += ndis*(sizeof(int*)+sizeof(uchar));
	lcp = malloc(nlcp*sizeof(short));
	memory += nlcp*sizeof(short);
}
void exit_hardcore()	{
	free(mak); free(lcp);
	free(offs); free(anch);
}

uint w;
int s,v;

int main(int argc,char *argv[])	{
	int t0 = clock();
	printf("\nArchon3r1\t(C)kvark, 2006");
	if(argc < 4)	{
		printf("\nCall format:");
		printf("\narchon3 [e|d] <source> <dest>");
		printf("\nBlock size = size of file");
		getchar(); return -1;
	}//files
	fi = fopen(argv[2],"rb");
	fo = fopen(argv[3],"wb");
	if(!fi || !fo)	{
		if(!fi) printf("\nCan't open input file");
		if(!fo) printf("\nCan't create out file");
		getchar(); return -2;
	}//offs
	fseek(fi,0,SEEK_END);
	fsize = ftell(fi);
	fseek(fi,0,SEEK_SET);
	memory = sizeof(r) + sizeof(rb) + sizeof(r2a);
	p = malloc(fsize*sizeof(int));
	s_bin = malloc(fsize+TERM+2);
	bin = s_bin+TERM;
	memory += fsize*(sizeof(int)+1)+TERM;
	printf("\n<%s> progress: ",argv[2]);
	if(argv[1][0] == 'e')	{
		n = fsize; fread(bin,1,n,fi);
		numb = (n+7)>>3;
		ndis = (n+AMASK)>>ABIT;
		nlcp = n >> LCPART;
		if(fsize < TERM) memset(bin-n,0,n);
		else memset(s_bin, 0x00, TERM); 
		
		encode(); // HERE !!
		
		if(lucky) exit_hardcore();
		#if VERBLEV >= 2
		printf("\nRoutine calls:");
		printf("\n\tDigger(%d), Deep_ray(%d)",call_digger,call_deep_ray);
		printf("\n\tLazy(%d), Indiana(%d+%d)",call_lazy,call_fw_anch,call_bk_anch);
		#endif
	}else
	if(argv[1][0] == 'd')	{
		n = fsize-4-1;
		fread(bin,1,n+1,fi);
		fread(&baza,4,1,fi);
		decode();
	}//exit
	free(p); free(s_bin);
	t0 = 1000*(clock() - t0)/CLOCKS_PER_SEC;
	#if VERBLEV >= 1
	printf("\nUsed memory: %lu kb",memory>>10);
	printf("\nExecution time: %lums",t0);
	#endif
	fclose(fi); fclose(fo);
	return 0;
}

#define nextp(id) p[id] = rb[bin[id]]++;
void decode()	{
	register int i,pos;
	register uchar cl;
	memset(rb, 0, 0x100*sizeof(int));
	for(i=0; i<=n; i++) rb[bin[i]]++;
	rb[bin[baza]]--;
	i=n+1; cl=0; do	{ cl--;
		i -= rb[cl], rb[cl] = i;
	}while(cl);
	putchar('H'); //fflush(stdout);
	p[baza]=0;
	for(i=0; i<baza; i++)		nextp(i);
	for(i=baza+1; i<=n; i++)	nextp(i);
	for(pos=baza,i=0; i<n; i++)
		pos=p[pos],putc(bin[pos],fo);
}
#undef nextp

#define p2b(pt) (*(ushort *)(pt))
#define p4b(pt) (*(uint *)(pt))

int isab(uchar*,uchar *b);

void encode()	{
	register uchar cl,*fly;
	register int i,pos;
	putchar('D'); //fflush(stdout);
	baza=-1; // Radix sort
	memset(r, 0, sizeof(trax2));
	sfin = bin+n; //scan
	for(fly=bin; fly<sfin-1; fly++)
		r[p2b(fly)]++;
	r[ch=0x10000] = w = n;
	i = 256; do	{ i--;
		cl=0; do	{ w -= r[--ch];
			r2a[ch] = r[ch] = w;
		}while(--cl);
		rb[i] = w;
		if((uchar)i == *bin)	{
			p[--w] = 0; r[ch]--;
		}//for start
	}while(i);
	sfin[0] = 0xFF; sfin[1] = 0;
	for(fly = bin; ; fly++)	{
		if(fly[1] < fly[0])	{
			if(fly[0] >= fly[-1])	{
				if(fly >= sfin) break;
				p[r2a[p2b(fly)]++] = fly+1-bin;
			}fly++;
		}
	}// Direct sort
	for(ch=0; ch<0x10000; ch++)	{
		//printf("%4x\b\b\b\b",ch);
		//fflush(stdout);
		ray(p+r[ch], p+r2a[ch], bin-5);
	}//Right2Left wave
	memcpy(r2a,r+1,sizeof(trax2)-sizeof(int));
	putchar('\b'); putchar('W');
	*sfin=0xFF; //fflush(stdout);
	cl=0; do	{ cl--;
		lim = r2a[(cl<<8)+cl];
		for(i=r[(uint)(cl+1)<<8]-1; i>=lim; i--)	{
			unsigned char cc = bin[pos = p[i]+1];
			if(cc <= cl) p[--r2a[(cc<<8)+cl]] = pos;
		}
		for(lim = r2a[(cl<<8)+cl]; i>=lim; i--)
			if(bin[pos = p[i]+1] == cl)
				p[--lim] = pos;
	}while(cl);
	//Left2Right wave
	*sfin=0x00; putc(*bin, fo);
	cl=0; i=0; do	{
		ch = r[(uint)(cl+1)<<8]-r[cl<<8];
		while(ch--)	{
			if((pos = 1+p[i++]) == n)	{
				baza = i; putc(*bin,fo);
				continue;
			}//finish
			sym = bin[pos]; putc(sym,fo);
			if(sym >= cl) p[rb[sym]++] = pos;
		}
	}while(++cl);
	#ifdef SUF_CHECK
	for(w=0,i=0; i<n-1; i++)
		if(isab(bin-3+p[i],bin-3+p[i+1]))	{
			w=1; break;
		}
	if(w) printf("\nError in p[%d]",i);
	#endif
	fwrite(&baza,4,1,fo);
}

int *gA;
/*
*	mysort - quicksort for lcp-key
*/
void mysort(int a, int b)	{
	do	{
		int x=a,y=a,z=b;
		short wk = lcp[(a+b)>>1];
		do	{ short qk = lcp[y];
			s = gA[y];
			if(qk > wk)	{ z--;
				gA[y]  =  gA[z]; gA[z]  =  s;
				lcp[y] = lcp[z]; lcp[z] = qk;
			}else
			if(qk < wk)	{
				gA[y]  =  gA[x]; gA[x]  =  s;
				lcp[y] = lcp[x]; lcp[x] = qk;
				y++; x++;
			}else y++;
		}while(y<z);
		if(x-a > b-z)	{
			if(z+1 < b) mysort(z,b);
			b = x;
		}else	{
			if(a+1 < x) mysort(a,x);
			a = z;
		}
	}while(b-a > 1);
}


#define LENMAX	32000
inline int lcp_isab(register uchar *a, register uchar *b)	{
	register short last;
	for(last=0; p4b(a)==p4b(b) && last<LENMAX; last++)
		a -= 4,b -= 4;
	return p4b(a) > p4b(b) ? LENMAX-last : last-LENMAX;
}
void deep_ray(int*,int*,uchar*);
/*
*	lazy - my invented lcp multi sort
*/
inline void lazy(int A[], const int num, uchar bof[])	{
	int i,j,cur;
	call_lazy++;
	lcp[0] = 0; s = A[0];
	for(i=1; i<num; i++)
		lcp[i] = lcp_isab(A[i]+bof,s+bof);
	gA = A; mysort(0,num);
	for(cur=lcp[j=0],i=0; i<num;)	{
		while(++i<num && lcp[i]==cur);
		if(i-j > 1)	{
			if(cur<=0) cur += LENMAX;
			else cur = LENMAX - cur;
			deep_ray(A+j,A+i,bof-(cur<<2));
		}cur = lcp[j = i];
	}
}

inline int isab(register uchar *a,register uchar *b)	{
	while(p4b(a) == p4b(b))	{a-=4; b-=4;}
	return p4b(a) > p4b(b);
}
inline int median(int a,int b,int c,uchar bof[])	{
	uint qa = p4b(a+bof), qb = p4b(b+bof), qc = p4b(c+bof);
	if(qa > qb)	return (qa<qc ? a : (qb>qc?b:c));
	else		return (qb<qc ? b : (qa>qc?a:c));
}
/*
*	deep_ray - the deep BeSe implementation
*	key is uint64 instead of uint32
*/
void deep_ray(int *A, int *B, uchar *boff)	{
	register int *x,*y,*z; uint w2;
	call_deep_ray++;
	while(B-A > INSERT)	{
		if(B-A < nlcp)	{
			lazy(A,B-A,boff); return;
		}//pivot
		s = median(A[0],A[B-A>>1],B[-1],boff);
		x=A; y=A; z=B; w = p4b(s+boff);
		w2 = p4b(s-4+boff);
		while(y<z)	{
			register uint q;
			s = *y; q = p4b(s+boff);
			if(q == w)	{
				q = p4b(s-4+boff);
				if(q == w2) y++;
				else if(q > w2)	{
					*y = *(--z);
					*z = s; 
				}else	{ //q < w2
					*(y++) = *x;
					*(x++) = s;
				}
			}else if(q > w)	{
				*y = *(--z); *z = s; 
			}else	{ // q < w
				*(y++) = *x; *(x++) = s;
			}
		}//recurse
		if(A+1 < x) ray(A,x,boff);
		if(z+1 < B) ray(z,B,boff);
		A = x; B = z; boff -= 8;
	}//insertion
	for(x=A+1; x<B; x++)	{
		s = *x; z = x;
		//TODO: LCP Insertion
		while(--z>=A && isab(*z+boff, s+boff))
			z[1] = z[0];
		z[1] = s; //put in place
	}
}
#undef LENMAX

#define SETMARK(pos) mak[pos>>3] ^= 1<<(pos&7)
#define MARKED(pos) (mak[pos>>3] & (1<<(pos&7)))
/*
*	indiana - general anchor sort
*	uses bit array 'mak' for speed
*/
void indiana(int A[], int B[], const int dif, int an[])	{
	int *x,*z,*hi,*lo; int pre,id;
	call_indiana++;
	for(x=A; x<B; x++)	{
		id = x[0] - dif; SETMARK(id);
	}//small bucket
	pre = p2b(bin+A[0]-dif-1);
	hi = p+r2a[pre]-1; lo = p+r[pre];
	//now pre = suffixes left
	for(x=z=an, pre=B-A-1;;)	{
		while(x > lo && (id=*--x,MARKED(id)))
			if(!--pre) goto allok;
		while(z < hi && (id=*++z,MARKED(id)))
			if(!--pre) goto allok;
		assert(x>lo || z<hi);
	}allok:
	//if(z-x > (B-A<<2)) fprintf(fd,"\n%d/%d %d",B-A,z-x,dif);
	for(z=A; z<B; z++)	{
		while(id=x[0],!MARKED(id)) x++;
		*z = id + dif; SETMARK(id);
	}
}
#undef SETMARK
#undef MARKED

#define MAXDIFF	32
/*
*	digger - deep routines guider
*	chooses anchor or calls deep_ray
*/
inline void digger(int A[], int B[], uchar bof[])	{
	int min_fw,min_bk,diff;
	int *x,*afw=0,*abk=0;
	if(B-A <= 1) return;
	if(!lucky) init_hardcore(),lucky++;
	call_digger++;
	min_fw = INT_MAX; min_bk = INT_MIN;
	for(x=A; x<B; x++)	{
		uchar *bp; int *an;
		int tj = x[0]>>ABIT;
		if(!(an = anch[tj])) continue;
		bp = bin+(tj<<ABIT)+offs[tj]-1;
		if(bp[-1] > bp[0]) continue;
		if(p2b(bp) >= ch) continue;
		diff = bin+x[0]-bp-1;
		if(diff > 0 && diff < min_fw)	{
			min_fw=diff; afw=an;
		}else
		if(diff < 0 && diff > min_bk)	{
			min_bk=diff; abk=an;
		}
	}diff = 0;
	//if forward anchor sort
	if(afw && min_fw < MAXDIFF)	{
		call_fw_anch++;
		indiana(A, B, diff=min_fw, afw);
	}else //if backward sort
	if(abk && min_bk > -MAXDIFF)	{ int i=0;
		for(x=A+1; x<B && !i; x++)
			for(i=-min_bk; i>0 && bin[A[0]+i] == bin[x[0]+i]; i--);
			if(!i)	{ call_bk_anch++;
				indiana(A, B, diff=min_bk, abk);
			}
	}//failed
	if(!diff) deep_ray(A,B,bof);
	//update anchors
	for(x=A; x<B; x++)	{
		int tj = x[0] >> ABIT;
		diff = x[0] & AMASK;
		if(!anch[tj] || offs[tj]>diff)	{
			anch[tj] = x; offs[tj] = diff;
		}
	}
}
#undef MAXDIFF

/* 
*	ray - the modified mkqsort
*	deep = bin-boff, dword comparsions
*/
void ray(int *A, int *B, register uchar *boff)	{
	register int *x,*y,*z;
	while(B-A > INSERT)	{
		s = median(A[0],A[B-A>>1],B[-1],boff);
		x=A; y=A; z=B; w = p4b(s+boff);
		while(y<z)	{
			register uint q;
			s = *y; q = p4b(s+boff);
			if(q < w)	{
				*(y++) = *x;
				*(x++) = s;
			}else
			if(q > w)	{
				*y = *(--z);
				*z = s; 
			}else y++;
		}//recurse
		if(A+1 < x) ray(A,x,boff);
		if(z+1 < B) ray(z,B,boff);
		A = x; B = z; boff-=4;
		if(bin-boff > DEEP)	{
			digger(A,B,boff);
			return;
		}
	}//insertion
	for(x=A+1; x<B; x++)	{
		s = *x; z = x;
		//TODO: limited sort with digger call
		while(--z>=A && isab(*z+boff, s+boff))
			z[1] = z[0];
		z[1] = s; //put in place
	}
}
#undef p2b
#undef p4b
#undef ABIT

#undef INSERT
#undef LINGO
#undef LINSIZE
#undef TERM