#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pnmrdr.h>

#include "assert.h"
#include "except.h"

const char *STDIN = "-";

typedef Pnmrdr_T Pnm;
typedef Pnmrdr_mapdata Pnmdata;

/* Error-Check Functions */
void usage();

/* Process-File Functions */
Pnm  process_file(FILE* fp, char* filename);
Pnm  open_file(FILE* stream);
void close_files(FILE* fp1, FILE* fp2);

/* Comparison Functions */
void compare(FILE* fp1, FILE* fp2, Pnm ppm1, Pnm ppm2);
void compare_dimensions(FILE* fp1, FILE* fp2, Pnm ppm1, Pnm ppm2);
void compute_E(Pnm ppm1, Pnm ppm2);
void print_E(float E);

/* Free Functions */
void free_ppm(Pnm ppm1, Pnm ppm2);

int main (int argc, char* argv[])
{
        char *filename1 = NULL;
        char *filename2 = NULL;
        FILE *fp1 = NULL;
        FILE *fp2 = NULL;
        
        if (argc != 3) {
                usage();
        }
        
        if ((strcmp(argv[1], STDIN) == 0) && (strcmp(argv[2], STDIN) == 0)) {
                usage();
        }
                
        filename1 = argv[1];
        filename2 = argv[2];
        
        Pnm ppm1 = process_file(fp1, filename1);
        Pnm ppm2 = process_file(fp2, filename2);
        
        compare(fp1, fp2, ppm1, ppm2);

        free_ppm(ppm1, ppm2);
        close_files(fp1, fp2);
        
        return 0;
}

void free_ppm(Pnm ppm1, Pnm ppm2)
{
        Pnmrdr_free(&ppm1);
        Pnmrdr_free(&ppm2);
}

void close_files(FILE* fp1, FILE* fp2)
{
        if (fp1 != NULL) {
                fclose(fp1);
        }

        if (fp2 != NULL) {
                fclose(fp2);
        }
}

void usage() 
{
        fprintf(stderr, "ERRRRROR\n");
        exit(EXIT_FAILURE);
}

Pnm process_file(FILE *fp, char* filename)
{
        if (strcmp(filename, STDIN) == 0) {
                return open_file(stdin);
        } else {
                fp = fopen(filename, "r");
                assert(fp != NULL);
                
                Pnm ppm = open_file(fp);
                
                return ppm;
        }
}

Pnm open_file(FILE* stream)
{
        Pnm ppm = Pnmrdr_new(stream);
        
        Pnmdata ppmdata = Pnmrdr_data(ppm);
        assert(ppmdata.type == Pnmrdr_rgb);
        
        return ppm;
}

void compare(FILE* fp1, FILE* fp2, Pnm ppm1, Pnm ppm2)
{
        compare_dimensions(fp1, fp2, ppm1, ppm2);
        compute_E(ppm1, ppm2);
}

void compare_dimensions(FILE* fp1, FILE* fp2, Pnm ppm1, Pnm ppm2)
{
        Pnmdata ppmdata1 = Pnmrdr_data(ppm1);
        Pnmdata ppmdata2 = Pnmrdr_data(ppm2);
        
        if (abs(ppmdata1.width  - ppmdata2.width)  > 1 ||
            abs(ppmdata1.height - ppmdata2.height) > 1) {
                printf("1.0\n");
                free_ppm(ppm1, ppm2);
                close_files(fp1, fp2);
                usage();
        }
}

void compute_E(Pnm ppm1, Pnm ppm2)
{
        Pnmdata ppmdata1 = Pnmrdr_data(ppm1);
        Pnmdata ppmdata2 = Pnmrdr_data(ppm2);
        unsigned width  = ppmdata1.width;
        unsigned height = ppmdata1.height;
        float sum = 0.0;
        float E;

        if (width > ppmdata2.width) {
                width = ppmdata2.width;
        }

        if (height > ppmdata2.height) {
                height = ppmdata2.height;
        }
        
        for (unsigned i = 0; i < width * height; i++) {
                unsigned red1 = Pnmrdr_get(ppm1) / ppmdata1.denominator;
                unsigned blue1 = Pnmrdr_get(ppm1) / ppmdata1.denominator;
                unsigned green1 = Pnmrdr_get(ppm1) / ppmdata1.denominator;         
                
                unsigned red2 = Pnmrdr_get(ppm2) / ppmdata2.denominator;
                unsigned blue2 = Pnmrdr_get(ppm2) / ppmdata2.denominator;
                unsigned green2 = Pnmrdr_get(ppm2) / ppmdata2.denominator;     

                sum += ((red1 - red2) * (red1 - red2));
                sum += ((green1 - green2) * (green1 - green2));
                sum += ((blue1 - blue2) * (blue1 - blue2));
        }

        E = sum / ((float)(3 * width * height));
        E = sqrt(E);
        print_E(E);
}

void print_E(float E)
{
        printf("%.4f\n", E);
}