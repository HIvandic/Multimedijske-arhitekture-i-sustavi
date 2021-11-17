#include <stdio.h>
#include <stdlib.h>
#define group_num 16

unsigned char *load_image(char *input, int *width, int *height) {
    FILE *fp;
    unsigned char *image;
    int max, one;
    int w, h;

    // open file
    if ((fp = fopen(input, "rb")) == NULL) {
        printf("Error while opening the file...\n");
        exit(1);
    }

    // ignore comments (start with #), return last one because it is not a comment
    while ((one = getc(fp)) == '#') while (getc(fp) != '\n');
    ungetc(one, fp);

    // check if marked as P6
    if (!(getc(fp) == 'P' && getc(fp) == '5')) {
        printf("Wrong image format, PPM required...\n");
        exit(1);
    }

    // ignore comments
    while ((one = getc(fp)) == '#') while (getc(fp) != '\n');
    ungetc(one, fp);

    // get width and height
    fscanf(fp, "%d %d", &w, &h);
    *width = w;
    *height = h;

    // allocate memory
    image = (unsigned char *)malloc(w * h * sizeof(unsigned char));
    if (image == NULL) {
        printf("Error while allocating memory...\n");
        exit(1);
    }

    // ignore comments
    while ((one = getc(fp)) == '#') while (getc(fp) != '\n');
    ungetc(one, fp);
    
    // get max value and check it
    fscanf(fp, "%d", &max);
    if (max > 65536 || max < 0) {
        printf("Invalid Maxval...\n");
        exit(1);
    }

    // ignore comments
    while ((one = getc(fp)) == '#') while (getc(fp) != '\n');
    ungetc(one, fp);
    
    // wait for a newline
    while (getc(fp) != '\n');

    // read the data
    fread(image, w, h, fp);
    
    fclose(fp);
    return image;
}

int main(int argc, char *argv[]) {
    char *input = "lenna.pgm";
    //char *output = "./hana_ivandic_dz2_freq.txt";
    //FILE *out;
    int i;
    int width, height;
    int freq[group_num] = {0};
    unsigned char *image = load_image(input, &width, &height);

    /*if ((out = fopen(output, "wb")) == NULL) {
        printf("Error while opening the file...\n");
        exit(1);
    }*/

    int size = width * height;
    for (i = 0; i < size; ++i) freq[image[i] >> 4]++;
    for (i = 0; i < group_num; ++i) {
        printf("%d %f\n", i, freq[i]/(float)size);
        //fprintf(out, "%d %f\n", i, freq[i]/(float)size);
    }
}