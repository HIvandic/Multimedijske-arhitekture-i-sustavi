#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define block_size 16
#define max_int_value 2147483647

typedef struct {
    int x;
    int y;
} vector;

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

int get_start(int index) {
    if (index != 0) return (index - block_size);
    else return 0;
}

int get_end(int index, int limit) {
    if ((index + 2 * block_size) >= limit) return (limit - block_size);
    else return (index + block_size);
}

void get_block_and_search(unsigned char *image, unsigned char *image_1, int width, int height, int block_index) {
    int block[16][16] = {0};
    int x_index, y_index, index;
    int x_start, y_start, x_end, y_end;
    float mad, min_mad = max_int_value;
    vector shift;

    // get the index of the current block
    x_index = (block_index % (width/block_size)) * block_size;
    y_index = (block_index / (height/block_size)) * block_size;
    index = y_index * width + x_index;

    // get the block
    for (int i = 0; i < block_size; ++i) {
        for (int j = 0; j < block_size; ++j) block[i][j] = image_1[index++];
        index += width - block_size;
    }

    // get start and end coordinates
    x_start = get_start(x_index);
    y_start = get_start(y_index);

    x_end = get_end(x_index, width);
    y_end = get_end(y_index, height);

    // search
    for (int i = y_start; i <= y_end; ++i) {
        for (int j = x_start; j <= x_end; ++j) {
            mad = 0;
            for (int m = 0; m < block_size; ++m) {
                for (int n = 0; n < block_size; ++n) mad += (float)abs(block[m][n] - image[(i + m) * width + j + n]);
            }
            mad /= (float)(block_size * block_size);

            if (mad < min_mad) {
                min_mad = mad;
                shift.x = j - x_index;
                shift.y = i - y_index;
            }
        }
    }

    printf("%d,%d", shift.x, shift.y);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Invalid number of arguments, enter block\n");
        exit(1);
    }

    int block_index = atoi(argv[1]);
    int width, height, width_1, height_1;
    unsigned char *image = load_image("lenna.pgm", &width, &height);
    unsigned char *image_1 = load_image("lenna1.pgm", &width_1, &height_1);

    get_block_and_search(image, image_1, width, height, block_index);
    return 0;
}