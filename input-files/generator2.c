#include <stdio.h>
#include <stdlib.h>

int
main(int argc, const char *argv[]) {
  FILE *fp;
  unsigned char *fibstr;
  int size;
  int i, j, k;

  fp = fopen("test1", "wb");
  for(i = 0; i < (1 << 12); ++i) {
    /* 0x00, 0x00, 0x00, 0x01, ..., 0x00, 0xfe, 0x00, 0xff */
    for(j = 0x00; j <= 0xff; ++j) {
      fputc(0x00, fp);
      fputc(j, fp);
    }
  }
  fclose(fp);

  fp = fopen("test2", "wb");
  for(i = 0; i < (1 << 12); ++i) {
    /* 0x00, 0xff, 0x00, 0xfe, ..., 0x00, 0x01, 0x00, 0x00 */
    for(j = 0xff; 0x00 <= j; --j) {
      fputc(0x00, fp);
      fputc(j, fp);
    }
  }
  fclose(fp);


  fp = fopen("test3", "wb");
  for(i = 0, k = 0; i < (1 << 5); ++i) {
    /* 0x00, 0x00, 0x01, 0x00, 0x02, ..., 0x00, 0xfe, 0x00, 0xff,
       0x01, 0x01, 0x02, 0x01, 0x03, ..., 0x01, 0xfe, 0x01, 0xff,
       ...,
       0xfd, 0xfd, 0xfe, 0xfd, 0xff,
       0xfe, 0xfe, 0xff,
       0xff */
    for(j = 0x00; j <= 0xff; ++j) {
      fputc(j, fp);
      for(k = j + 1; k <= 0xff; ++k) {
        fputc(j, fp);
        fputc(k, fp);
      }
    }
  }
  fclose(fp);

  fp = fopen("test4", "wb");
  for(i = 0, k = 0; i < (1 << 5); ++i) {
    /* 0xff, 0xff, 0xfe, 0xff, 0xfd, ..., 0xff, 0x01, 0xff, 0x00,
       0xfe, 0xfe, 0xfd, 0xfe, 0xfc, ..., 0xfe, 0x01, 0xfe, 0x00,
       ...,
       0x02, 0x02, 0x01, 0x02, 0x00,
       0x01, 0x01, 0x00,
       0x00 */
    for(j = 0xff; 0x00 <= j; --j) {
      fputc(j, fp);
      for(k = j - 1; 0x00 <= k; --k) {
        fputc(j, fp);
        fputc(k, fp);
      }
    }
  }
  fclose(fp);

  size = 1 << 8;
  fibstr = malloc(size * sizeof(unsigned char));
  fibstr[0] = 0;
  fibstr[1] = 1;
  for(i = 0, j = 0, k = 2; k < size; ++i, ++k) {
    fibstr[k] = fibstr[i];
    if(i == j) {
      i = -1;
      j = k - 1;
    }
  }

  fp = fopen("test5", "wb");
  for(i = 0; i < ((1 << 21) / size); ++i) {
    for(j = 0; j < size; ++j) {
      fputc((fibstr[j] + i) % 256, fp);
    }
  }
  fclose(fp);

  free(fibstr);

  return 0;
}
