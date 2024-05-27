#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

long get_file_size(char *filename) {
    struct stat file_status;
    if (stat(filename, &file_status) < 0) {
        return -1;
    }

    return file_status.st_size;
}

#define ADDR_LEN 10
int main(int argc, char **argv)
{
    if(argc != 2){
        printf("usage: %s ROM_FILE\n", argv[0]);
        exit(0);
    }
    long fsize = get_file_size(argv[1]);
    if(fsize > 0) {
        unsigned char *buf = malloc(fsize);
        FILE *rom = fopen(argv[1], "rb");
        if(rom == NULL)
            exit(0);
        fread(buf, 1, fsize, rom);
        printf("rom size: %lX\n", fsize);
        fclose(rom);

        FILE *font16_fp = fopen("font16.bin", "wb");
        fwrite(buf+0x1D810, 1, 0x2800, font16_fp);
        fclose(font16_fp);
    }
    return 0;
}
