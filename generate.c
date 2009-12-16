//
// generate.c - written by Ted Burke - last updated 8-12-2009
// 
// This program generates a PBM image of random black or white pixels.
// Usage:	generate OUTPUT_FILENAME WIDTH HEIGHT
//

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main(int argc, char *argv[])
{
	int w, h, x, y;
	int black_count = 0, white_count = 0;
	
	// Reseed random number generation
	srand(GetTickCount());
	
	if (argc != 4)
	{
		printf("Usage: generate.exe OUTPUT_FILENAME WIDTH HEIGHT\n");
		exit(1);
	}
	
	w = atoi(argv[2]);
	h = atoi(argv[3]);

	FILE* image_file = fopen(argv[1], "w");
	
	fprintf(image_file, "P1\n");
	fprintf(image_file, "# Created by Ted Burke's noise stimulus generate program\n");
	fprintf(image_file, "%d %d\n", w, h);
	
	for (y = 0 ; y < h ; ++y)
	{
		for (x = 0 ; x < w ; ++x)
		{
			if (rand() > RAND_MAX / 2)
			{
				fprintf(image_file, "1 ");
				white_count++;
			}
			else
			{
				fprintf(image_file, "0 ");
				black_count++;
			}
		}
		fprintf(image_file, "\n");
	}
	
	fclose(image_file);
	
	printf("%d black squares, %d white squares\n", black_count, white_count);
	
	return 0;
}
