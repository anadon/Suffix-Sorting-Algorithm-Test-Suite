#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
	
	FILE *fp = fopen(argv[1], "r");
	fseek(fp, 0, SEEK_END);
	size_t length = ftell(fp);
	rewind(fp);
	
	void *sequence = malloc(length);
	fread(sequence, 1, length, fp);
	fclose(fp);
	
	free(sequence);	
	
	return 0;
}