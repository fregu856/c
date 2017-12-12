// Create 4MB raw data file as a saw tooth signal
#include <stdio.h>

int main(int argc, char **argv)
{
  FILE *fp = fopen("data_saw.raw","wb");
  int i, j, k;

  char buffer[4096];

  for (k=0; k<4; k++) {
    for (i=0; i<256; i++) {
      for (j=0; j<4096; j++) {
	buffer[j]=(char)i;
      }
      fwrite(buffer,sizeof(buffer),1,fp);
    }
  }

  fclose(fp);
}
