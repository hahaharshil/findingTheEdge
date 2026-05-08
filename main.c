#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct {
    int width;
    int height;
    unsigned char *data;
} Image;

Image* create_image(int height, int width){
    Image *img = malloc(sizeof(Image));
    img->width = width;
    img->height = height;

    img->data = calloc(width * height, sizeof(unsigned char));

    return img;
}

void free_image(Image *img){
    if(img != NULL){
        free(img->data);
        free(img);
    }
}

void skip_comments(FILE *fp){
    int ch;
    char line[256];

    while((ch = fgetc(fp)) != EOF && isspace(ch));

    if(ch == '#'){
        fgets(line, sizeof(line), fp);
        skip_comments(fp);
    }else{
        fseek(fp, -1, SEEK_CUR);
    }
}

Image* read_pgm(const char *filename){
    FILE *fp = fopen(filename, "rb");

    if(fp == NULL){
        printf("%s Is not a valid file or does not exist", filename);
        return NULL;
    }

    char header[3];

    int w, h, max_val;

    fscanf(fp, "%2s", header);

    if(header[0] != 'P' || header[1] != '5'){
        printf("No p5 header");
        fclose(fp);
        return NULL;
    }


    skip_comments(fp);

    fscanf(fp, "%d %d", &w, &h);
    skip_comments(fp);

    fscanf(fp, "%d", &max_val);

    fgetc(fp);

    Image *img = create_image(h, w);

    fread(img->data, sizeof(unsigned char), w * h, fp);

    fclose(fp);

    return img;
}

void write_pgm(const char *filename, Image *img){
    FILE *fp = fopen(filename, "wb");

    if(!fp){
        printf("Error");
        return;
    }

    fprintf(fp, "P5\n%d %d\n255\n", img->width, img->height);

    fwrite(img->data, sizeof(unsigned char), img->width * img->height, fp);

    fclose(fp);

}

//crazy func
Image* apply_laplacian(Image *input) {

    Image *output = create_image(input->height, input->width);

    int w = input->width;
    int h = input->height;

    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {

            // Find our 1D array index using 2D coordinates
            int center = y * w + x;

            // 2nd Derivative
            int derivative = input->data[(y+1)*w + x] +
                             input->data[(y-1)*w + x] +
                             input->data[y*w + (x+1)] +
                             input->data[y*w + (x-1)] -
                             (4 * input->data[center]);


            if (derivative < 0) {
                derivative = -derivative;
            }


            if (derivative > 30) {
                output->data[center] = 255;
            }
        }
    }
    return output;
}



int main(int argc, char **argv){
    if(argc < 3){
        printf("too few args");
        return 1;
    }

    printf("Loading %s \n", argv[1]);

    Image *my_image = read_pgm(argv[1]);

        if(my_image == NULL){
            return 1;
        }

        printf("Success! Dimensions: %d x %d\n", my_image->width, my_image->height);

        // Apply the differential equation math!
        printf("Applying Laplacian Edge Detection...\n");
        Image *edges = apply_laplacian(my_image);

        printf("Saving edges to %s...\n", argv[2]);
        write_pgm(argv[2], edges);

        // Clean up both images from RAM
        free_image(my_image);
        free_image(edges);

        printf("PBL complete!\n");

        return 0;
}
