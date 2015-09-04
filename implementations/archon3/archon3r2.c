/*
*	Archon 3-rel-2	(C) kvark, 2006
*		Archon project
*	Burrows-Wheeler Transformation algoritm
*
*		Anchors + DeepRay + Isab
*		Direct sorting (a<b>=c)
*		Units: dword, no bswap
*/

//#define SUFCHECK
#define NDEBUG
#define VERBLEV	0

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>
#include <limits.h>
#include <assert.h>

#define INSERT	10
#define MAXST	64
#define DEEP	150
#define OVER	1000
#define TERM	((1+OVER)*sizeof(ulong))
#define ABIT	7
const int AMASK = (1<<ABIT)-1;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef int trax2[0x10001];

int BS,n,*p;
int baza,ch;
uchar *s_bin,sym;
uchar *sfin, *bin;
int rb[0x100];
trax2 r,r2a;

int call_indiana, call_fw_anch, call_bk_anch;
int call_fw_buck, call_split, call_pseudo;
int call_digger, call_deep_ray, call_smart_ins;

int ndis,numst;
uchar *offs;
int **anch;
short *lcp;

clock_t t0;
FILE *fi,*fo;
int lucky;
int fsize,memory;

#define p2b(pt) (*(ushort *)(pt))
#define p4b(pt) (*(ulong *)(pt))
#define BADSUF(pid) (pid[0]>pid[1] && pid[0]>=pid[-1])

void ray(int *, int *, uchar *);
void encode();
void decode();

struct TLonStr	{
	uchar *sb;
	int len,off;
	char up;
}lst[MAXST+1],
*milen,*mac,*lc;

void init_hardcore()	{ lucky++;
	offs = malloc(ndis * sizeof(uchar));
	anch = malloc(ndis * sizeof(int*));
	memset(offs, 0, ndis*sizeof(uchar));
	memset(anch, 0, ndis*sizeof(int*));
	memory += ndis*(sizeof(int*)+sizeof(uchar));
}
void exit_hardcore()	{
	lucky=0; free(offs); free(anch);
}

#ifdef SUFCHECK
int sufcheck(int A[], int B[])	{
	int *x;
	for(x=A; x<B-1; x++)	{
		uchar *a = bin-3+x[0];
		uchar *b = bin-3+x[1];
		while(p4b(a)==p4b(b)) a-=4,b-=4;
		if(p4b(a) > p4b(b)) return x-A;
	}return -1;
}
#endif

int main(int argc,char *argv[])	{
	int t0 = clock();
	setbuf(stdout,NULL);
	printf("\nArchon3r2\t(C)kvark, 2006");
	if(argc < 4)	{
		printf("\nCall format:");
		printf("\narchon3r2 [e|d] <source> <dest>");
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
	s_bin = malloc(fsize+TERM+1);
	bin = s_bin+TERM;
	memory += fsize*(sizeof(int)+1)+TERM;
	printf("\n<%s> progress: ",argv[2]);
	if(argv[1][0] == 'e')	{
		n = fsize; fread(bin,1,n,fi);
		ndis = (n+AMASK)>>ABIT;
		if(fsize < TERM) memset(bin-n,0,n);
		else memset(s_bin, 0x00, TERM); 
		
		encode(); // HERE !!
		
		#if VERBLEV >= 2
		printf("\nRoutine calls:");
		printf("\n\tDigger(%d), NumSt(%d)",call_digger,numst);
		printf("\n\tIndiana(%d+%d+%d)",call_fw_anch,call_bk_anch,call_fw_buck);
		printf("\n\tDeepRay(%d), SmartIns(%d)",call_deep_ray,call_smart_ins);
		printf("\n\tSplit(%d), Pseudo(%d)",call_split,call_pseudo);
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
	putchar('H'); p[baza]=0;
	for(i=0; i<baza; i++)		nextp(i);
	for(i=baza+1; i<=n; i++)	nextp(i);
	for(pos=baza,i=0; i<n; i++)
		pos=p[pos],putc(bin[pos],fo);
}
#undef nextp

void encode()	{
	register uchar cl,*fly;
	register int i,pos,lim;
	putchar('D'); baza=-1; // Radix sort
	memset(r, 0, sizeof(trax2));
	sfin = bin+n; //scan
	for(fly=bin; fly<sfin-1; fly++)
		r[p2b(fly)]++;
	r[ch=0x10000] = pos = n;
	i = 256; do	{ i--;
		cl=0; do	{
			pos -= r[--ch];
			r2a[ch] = r[ch] = pos;
		}while(--cl);
		rb[i] = pos;
		if((uchar)i == *bin)	{
			p[--pos] = 0; r[ch]--;
		}//for start
	}while(i);
	sfin[0] = 0xFF; fly=bin; //border
	do if(BADSUF(fly)) p[r2a[p2b(fly)]++] = fly+1-bin, fly++;
	while(++fly<sfin);
	// Direct sort
	lst[0].len = n; numst=0;
	lst[0].sb = bin-5;
	for(lucky=0,ch=0; ch<0x10000; ch++)	{
		//printf("%4x\b\b\b\b",ch);
		ray(p+r[ch], p+r2a[ch], bin-5);
	}//Right2Left wave
	if(lucky) exit_hardcore();
	memcpy(r2a,r+1,sizeof(trax2)-sizeof(int));
	putchar('\b'); putchar('W'); *sfin=0xFF;
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
	#ifdef SUFCHECK
	printf("\nChecking SA...");
	i = sufcheck(p,p+n);
	if(i>=0) printf("err in p[%d]",i);
	else printf("OK");
	#endif
	fwrite(&baza,4,1,fo);
}

/*
*	isab - smart string compare routine
*/
int isab(register uchar *a,register uchar *b)	{
	int cof,i; uchar *bx;
	cof = (a>b ? (bx=a)-b : (bx=b)-a);
	//if(!cof) return -1;
	mac = milen = lst; //choose period
	for(lc=lst+1; lc <= lst+numst; lc++)	{
		if(lc->len < milen->len) milen=lc;
		if(lc->off == cof  &&  lc->sb - lc->len < bx)
			if(lc->sb > mac->sb) mac=lc;
	}//continue until border
	for(i = bx-mac->sb; i>=0; i-=4)	{
		if(p4b(a) != p4b(b)) break;
		a-=4; b-=4;
	}//replace old bound
	if(i>0 || mac==lst)	{
		int rez = p4b(a)>p4b(b) || b<=lst[0].sb;
		int clen = bx-(a>b?a:b)-4;
		if(numst < MAXST)	{
			milen = lst+(++numst);
			milen->len = 0;
		}//replace-add
		if(clen > milen->len)	{
			milen->sb = bx;
			milen->len = clen;
			milen->off = cof;
			milen->up = (rez == (a>b));
		}return rez;
	}//update bound
	if(bx > mac->sb)	{
		mac->len += bx-mac->sb;
		mac->sb = bx;
	}return (mac->up == (a>b));
}

int median(int a,int b,int c,uchar bof[])	{
	uint qa = p4b(a+bof), qb = p4b(b+bof), qc = p4b(c+bof);
	if(qa > qb)	return (qa<qc ? a : (qb>qc?b:c));
	else		return (qb<qc ? b : (qa>qc?a:c));
}
/*
*	deep_ray - the deep BeSe implementation
*	key is uint64 instead of uint32
*/
void deep_ray(int *A, int *B, uchar *boff)	{
	register int *x,*y,*z; ulong w,w2;
	call_deep_ray++;
	while(B-A > INSERT)	{
		int s = median(A[0],A[B-A>>1],B[-1],boff);
		x=A; y=A; z=B; w = p4b(s+boff);
		w2 = p4b(s-4+boff);
		while(y<z)	{
			register uint q;
			s = *y; q = p4b(s+boff);
			if(q == w)	{
				q = p4b(s-4+boff);
				if(q == w2) y++;
				else if(q > w2)	{
					*y = *--z; *z = s; 
				}else	{ //q < w2
					*y++ = *x; *x++ = s;
				}
			}else if(q > w)	{
				*y = *--z; *z = s; 
			}else	{ // q < w
				*y++ = *x; *x++ = s;
			}
		}//recurse
		if(A+1 < x) ray(A,x,boff);
		if(z+1 < B) ray(z,B,boff);
		A = x; B = z; boff -= 8;
	}//insertion
	for(x=A+1; x<B; x++)	{
		int s = (z=x)[0];
		while(--z>=A && isab(boff+z[0],boff+s))
			z[1] = z[0];
		z[1] = s; //in place
	}
}

int icmp(const void *v0, const void *v1)	{
	return *(int*)v0 - *(int*)v1;
}
/*
*	indiana - general anchor sort
*	uses bit array 'mak' for speed
*/
void indiana(int A[], int B[], const int dif, int an[])	{
	int *x,*z,*hi,*lo; int pre,num=B-A;
	call_indiana++; //assert(lucky);
	qsort(A,num,sizeof(int),icmp);
	pre = p2b(bin+A[0]-dif-1);
	hi = p+r2a[pre]-1; lo = p+r[pre];
	an[0]=~an[0]; //now pre = suffixes left
	for(x=z=an, pre=num-1;;)	{ int id;
		while(x > lo && (id=dif+*--x,bsearch(&id,A,num,sizeof(int),icmp)))
			if(x[0]=~x[0],!--pre) goto allok;
		while(z < hi && (id=dif+*++z,bsearch(&id,A,num,sizeof(int),icmp)))
			if(z[0]=~z[0],!--pre) goto allok;
		assert(x>lo || z<hi);
	}allok:
	//if(z-x > (B-A<<2)) fprintf(fd,"\n%d/%d %d",B-A,z-x,dif);
	for(z=A,x--; z<B; z++)	{
		do x++; while(x[0]>=0);
		*z = (x[0]=~x[0]) + dif;
	}
}

/*
*	split - splits suffixes into 3 groups according
*	to the first 'ofs' characters from 'bof'
*/
int * split(int *A, int *B,uchar *bof,int ofs,int piv,int **end)	{
	call_split++;
	piv+=ofs; do	{ int *x,*y,*z;
		ulong w = p4b(bof + piv);
		x=y=A,z=B; do	{
			int s = *y;
			ulong q = p4b(bof + s);
			if(q == w) y++;
			else if(q<w)	{
				*y++ = *x; *x++ = s;
			}else	{ // q>w
				*y = *--z; *z = s;
			}
		}while(y<z);
		A = x; B = y; bof -= 4;
	}while(A<B && (ofs-=4) > 0);
	end[0] = B; return A;
}

#define MAXDIFF		32
#define MAXLINDIFF	0
/*
*	digger - deep routines guider
*	chooses anchor or calls deep_ray
*/
void digger(int A[], int B[], uchar bof[])	{
	int min_fw,min_bk,min_cr,diff;
	int *x,*afw=0,*abk=0,*acr=0;
	if(B-A <= 1) return;
	if(!lucky) init_hardcore();
	call_digger++;
	min_fw = min_cr = INT_MAX; min_bk = INT_MIN;
	for(x=A; x<B; x++)	{
		uchar *bp; int *an;
		int tj = x[0]>>ABIT;
		if(!(an = anch[tj])) continue;
		bp = bin+(tj<<ABIT)+offs[tj]-1;
		if(bp[-1] > bp[0]) continue;
		diff = bin+x[0]-bp-1;
		if(p2b(bp) == ch)	{
			if(diff>0 && diff<min_cr)
				min_cr=diff,acr=an;
		}else
		if(diff > 0)	{
			if(diff < min_fw) min_fw=diff,afw=an;
		}else	if(diff > min_bk) min_bk=diff,abk=an;
	}diff = 0;
	//if forward anchor sort
	if(afw && min_fw < MAXDIFF)	{
		call_fw_anch++;
		indiana(A,B,diff=min_fw,afw);
	}else //if backward sort
	if(abk && min_bk > -MAXDIFF)	{ int i=0;
		for(x=A+1; x<B && !i; x++)
			for(i=-min_bk; i>0 && bin[A[0]+i] == bin[x[0]+i]; i--);
			if(!i)	{ call_bk_anch++;
				indiana(A,B,diff=min_bk,abk);
			}
	}else //same bucket
	if(acr && min_cr < MAXDIFF)	{ int *z;
		x = split(A,B,bof,min_cr,*acr,&z);
		if(x+1 < z)	{ call_fw_buck++;
			indiana(x,z,diff=min_cr,acr);
		}else diff = -1;
		if(A+1 < x) deep_ray(A,x,bof);
		if(z+1 < B) deep_ray(z,B,bof);
	}//pseudo or deep
	if(!diff)	{ int pre,s,tj;
		uchar *spy = bin+A[0]-1;
		for(diff=0; diff<MAXLINDIFF; diff++,spy--)
			if(BADSUF(spy) && (pre=p2b(spy))<ch)
				if(r[pre+1]-r[pre] > ((B-A)<<4)) break;
		if(diff < MAXLINDIFF)	{ call_pseudo++;
			s = A[0]-diff; tj = s>>ABIT;
			assert(s>=0 && s<n);
			for(acr=p+r[pre]; acr[0]!=s; acr++);
			assert(acr < p+r2a[pre]);
			anch[tj] = acr; offs[tj] = s&AMASK;
			indiana(A,B,diff,acr);
		}else deep_ray(A,B,bof);
	}//update anchors
	for(x=A; x<B; x++)	{
		int tj = x[0] >> ABIT;
		diff = x[0] & AMASK;
		if(!anch[tj] || offs[tj]>diff)	{
			anch[tj] = x; offs[tj] = diff;
		}
	}
}
#undef MAXDIFF
#undef MAXLINDIFF

/*
*	smart_ins - insertion sort limited with DEEP
*	calls 'digger' to sort bad substrings
*/
void smart_ins(int A[], int B[], uchar *bof)	{
	register int *x=A+1,*z;
	int badcase = 0;
	do	{ int s = *x; z=x-1;
		do	{
			int limit = (DEEP+3)>>2;
			register uchar *a,*b;
			a = bof + z[0]; b = bof+s;
			while(p4b(a)==p4b(b) && --limit) a-=4,b-=4;
			if(p4b(a) < p4b(b)) break;
			if(!limit)	{
				z[0]=~z[0];
				badcase++; break;
			}//shift
			do z[1] = z[0];
			while(--z>=A && z[0]<0);
		}while(z>=A);	
		z[1] = s; //put in place
	}while(++x<B);
	if(badcase)	{ bof -= DEEP;
		call_smart_ins++;
		x=A; do	{ //skip bad
			for(z=x; z[0]<0; z++) z[0]=~z[0];
			if(x+1 < ++z) digger(x,z,bof);
		}while((x=z)<B);
	}
}

/* 
*	ray - the modified mkqsort
*	deep = bin-boff, dword comparsions
*/
void ray(int *A, int *B, register uchar *boff)	{
	register int *x,*y,*z;
	while(B-A > INSERT)	{
		int s = median(A[0],A[B-A>>1],B[-1],boff);
		ulong w = p4b(s+boff);
		x=y=A; z=B; do	{
			ulong q = p4b((s=*y)+boff);
			if(q < w)	{
				*y++ = *x; *x++ = s;
			}else
			if(q > w)	{
				*y = *--z; *z = s; 
			}else y++;
		}while(y<z);
		if(A+1 < x) ray(A,x,boff);
		if(z+1 < B) ray(z,B,boff);
		A = x; B = z; boff-=4;
		if(bin-boff > DEEP)	{
			digger(A,B,boff);
			return;
		}
	}//insertion
	if(A+1 < B) smart_ins(A,B,boff);
}
#undef p2b
#undef p4b

#undef INSERT
#undef MAXST
#undef DEEP
#undef OVER
#undef TERM
#undef ABIT
#undef VERBLEV