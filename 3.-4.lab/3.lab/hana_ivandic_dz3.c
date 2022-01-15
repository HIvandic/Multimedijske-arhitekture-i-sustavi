#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#define block_size 8
#define PI 3.14159265358979323846

typedef struct
{
    unsigned char r, g, b;
} pixel;

typedef struct
{
    float y, cb, cr;
} ycbcr;

typedef struct
{
    int y, cb, cr;
} quant_data;

float quant_K1[8][8] = {{16, 11, 10, 16, 24, 40, 51, 61},
                        {12, 12, 14, 19, 26, 58, 60, 55},
                        {14, 13, 16, 24, 40, 57, 69, 56},
                        {14, 17, 22, 29, 51, 87, 80, 62},
                        {18, 22, 37, 56, 68, 109, 103, 77},
                        {24, 35, 55, 64, 81, 104, 113, 92},
                        {49, 64, 78, 87, 103, 121, 120, 101},
                        {72, 92, 95, 98, 112, 100, 103, 99}};

float quant_K2[8][8] = {{17, 18, 24, 47, 99, 99, 99, 99},
                        {18, 21, 26, 66, 99, 99, 99, 99},
                        {24, 26, 56, 99, 99, 99, 99, 99},
                        {47, 66, 99, 99, 99, 99, 99, 99},
                        {99, 99, 99, 99, 99, 99, 99, 99},
                        {99, 99, 99, 99, 99, 99, 99, 99},
                        {99, 99, 99, 99, 99, 99, 99, 99},
                        {99, 99, 99, 99, 99, 99, 99, 99}};

pixel *load_image(char *input, int *width, int *height)
{
    FILE *fp;
    pixel *image;
    int max, one;
    int w, h;

    // open file
    if ((fp = fopen(input, "rb")) == NULL)
    {
        printf("Error while opening the file...\n");
        exit(1);
    }

    // ignore comments (start with #), return last one because it is not a comment
    while ((one = getc(fp)) == '#')
        while (getc(fp) != '\n')
            ;
    ungetc(one, fp);

    // check if marked as P6
    if (!(getc(fp) == 'P' && getc(fp) == '6'))
    {
        printf("Wrong image format, PPM required...\n");
        exit(1);
    }

    // ignore comments
    while ((one = getc(fp)) == '#')
        while (getc(fp) != '\n')
            ;
    ungetc(one, fp);

    // get width and height
    fscanf(fp, "%d %d", &w, &h);
    *width = w;
    *height = h;

    // allocate memory
    image = (pixel *)malloc(w * h * sizeof(pixel));
    if (image == NULL)
    {
        printf("Error while allocating memory...\n");
        exit(1);
    }

    // ignore comments
    while ((one = getc(fp)) == '#')
        while (getc(fp) != '\n')
            ;
    ungetc(one, fp);

    // get max value and check it
    fscanf(fp, "%d", &max);
    if (max > 65536 || max < 0)
    {
        printf("Invalid Maxval...\n");
        exit(1);
    }

    // ignore comments
    while ((one = getc(fp)) == '#')
        while (getc(fp) != '\n')
            ;
    ungetc(one, fp);

    // wait for a newline
    while (getc(fp) != '\n')
        ;

    // read the data
    fread(image, 3 * w, h, fp);

    fclose(fp);
    return image;
}

float check_value(float value)
{
    if (value > 127)
        value = 127.0;
    else if (value < -128)
        value = -128.0;

    return value;
}

ycbcr *rgb2ycbcr(pixel *image, int width, int height, int block)
{
    int r, g, b;
    ycbcr *new_image = (ycbcr *)malloc(block_size * block_size * sizeof(ycbcr));
    int index = (block_size * block / width) * width * block_size + block_size * (block % (width / block_size));

    for (int i = 0; i < block_size; ++i)
    {
        for (int j = 0; j < block_size; ++j)
        {

            r = image[index].r;
            g = image[index].g;
            b = image[index].b;

            // convert to y cb cr and check if in range [-128, 127]
            new_image[i * block_size + j].y = check_value(0.299 * r + 0.587 * g + 0.114 * b - 128);
            new_image[i * block_size + j].cb = check_value(-0.1687 * r - 0.3313 * g + 0.5 * b);
            new_image[i * block_size + j].cr = check_value(0.5 * r - 0.4187 * g - 0.0813 * b);

            ++index;
        }

        index += width - block_size;
    }

    return new_image;
}

float get_C(int x)
{
    if (x == 0)
        return pow(1 / 2.0, 1 / 2.0);
    else
        return 1;
}

ycbcr *dtc(ycbcr *image)
{
    float cu, cv;
    double y_sum, cb_sum, cr_sum;
    ycbcr *new_image = (ycbcr *)malloc(block_size * block_size * sizeof(ycbcr));

    for (int u = 0; u < block_size; ++u)
    {
        for (int v = 0; v < block_size; ++v)
        {

            float factor;
            y_sum = cb_sum = cr_sum = 0;

            cu = get_C(u);
            cv = get_C(v);

            for (int i = 0; i < block_size; ++i)
            {
                for (int j = 0; j < block_size; ++j)
                {

                    double cos_part = cos((2 * i + 1) * u * PI / 16.0) * cos((2 * j + 1) * v * PI / 16.0);

                    y_sum += image[i * block_size + j].y * cos_part;
                    cb_sum += image[i * block_size + j].cb * cos_part;
                    cr_sum += image[i * block_size + j].cr * cos_part;
                }
            }

            factor = 0.25 * cu * cv;

            new_image[u * block_size + v].y = factor * y_sum;
            new_image[u * block_size + v].cb = factor * cb_sum;
            new_image[u * block_size + v].cr = factor * cr_sum;
        }
    }

    free(image);
    return new_image;
}

quant_data *quantization(ycbcr *image)
{
    quant_data *new_image = (quant_data *)malloc(block_size * block_size * sizeof(quant_data));

    for (int u = 0; u < block_size; ++u)
    {
        for (int v = 0; v < block_size; ++v)
        {

            new_image[u * block_size + v].y = round((float)image[u * block_size + v].y / quant_K1[u][v]);
            new_image[u * block_size + v].cb = round((float)image[u * block_size + v].cb / quant_K2[u][v]);
            new_image[u * block_size + v].cr = round((float)image[u * block_size + v].cr / quant_K2[u][v]);
        }
    }

    return new_image;
}

void store_image(quant_data *quant, FILE *fp)
{
    int i, j, val;

    // print y quantized coefficients
    for (i = 0; i < block_size; ++i)
    {
        for (j = 0; j < block_size; ++j)
        {

            val = quant[i * block_size + j].y;
            fprintf(fp, "%d ", quant[i * block_size + j].y);
        }

        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");

    // print cb quantized coefficients
    for (i = 0; i < block_size; ++i)
    {
        for (j = 0; j < block_size; ++j)
        {

            val = quant[i * block_size + j].cb;
            fprintf(fp, "%d ", quant[i * block_size + j].cb);
        }

        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");

    // print cr quantized coefficients
    for (i = 0; i < block_size; ++i)
    {
        for (j = 0; j < block_size; ++j)
        {

            val = quant[i * block_size + j].cr;
            fprintf(fp, "%d ", quant[i * block_size + j].cr);
        }

        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
}

int main(int argc, char *argv[])
{
    char *output;
    if (argc == 3)
        output = argv[2];
    else if (argc == 4)
        output = argv[3];
    else
    {
        printf("Invalid number of arguments, enter input and output file\n");
        exit(1);
    }

    char *input = argv[1];
    int width, height;
    FILE *fp;

    if ((fp = fopen(output, "w")) == NULL)
    {
        printf("Error while opening the file...\n");
        exit(1);
    }

    pixel *image = load_image(input, &width, &height);
    for (int block = 0; block < 4096; ++block)
    {
        ycbcr *ycbcr_form = rgb2ycbcr(image, width, height, block);
        ycbcr *ycbcr_dct = dtc(ycbcr_form);
        quant_data *quant = quantization(ycbcr_dct);

        store_image(quant, fp);
    }
    fclose(fp);

    return 0;
}