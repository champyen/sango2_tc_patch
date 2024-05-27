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

void disp_CCHAR(unsigned char *font_buf, long addr)
{
    printf("ADDR - %lX\n", addr);
    unsigned char *font = font_buf + addr;
    for(int y = 0 ; y < 11; y++){
        for(int x = 0; x < 11; x++){
            int bit_idx = x + y*11;
            int fidx = bit_idx >> 3;
            //fidx = (fidx >= 8) ? fidx - 8 : fidx + 8;
            int fofs = bit_idx & 0x7;

            int bit = (font[fidx] >> (fofs)) & 0x1;
            char outchar = bit > 0 ? '#' : ' ';
            printf("%c", outchar);
        }
        printf("\n");
    }
}

void disp_CCHARx8_line(unsigned char *font_buf, long addr, int line)
{
    //for(int y = 0 ; y < 11; y++)
    for(int cidx = 0; cidx <8; cidx++)
    {
        unsigned char *font = font_buf + addr + 0x10*cidx;
        int y = line;
        for(int x = 0; x < 11; x++){
            int bit_idx = x + y*11;
            int fidx = bit_idx >> 3;
            //fidx = (fidx >= 8) ? fidx - 8 : fidx + 8;
            int fofs = bit_idx & 0x7;

            int bit = (font[fidx] >> (fofs)) & 0x1;
            char outchar = bit > 0 ? '#' : ' ';
            printf("%c", outchar);
        }
        printf("     ");
    }
    printf("\n");
}

void disp_CCHARx8(unsigned char *font_buf, long addr){
    printf("ADDR - %lX\n", addr);
    for(int y = 0; y < 11; y++){
        disp_CCHARx8_line(font_buf, addr, y);
    }
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
        FILE *rom = NULL;
        if((rom = fopen(argv[1], "rb")) == NULL) {
            printf("ROM File - %s failed to open.\n", argv[1]);
            exit(0);
        }
        unsigned char *buf = malloc(fsize);
        fread(buf, fsize, 1, rom);
        fclose(rom);
        printf("rom size: %lX\n", fsize);

        char addr_buf[ADDR_LEN];
        printf("Enter the offset:\n");
        fgets(addr_buf, ADDR_LEN, stdin);
        long addr = strtol(addr_buf, NULL, 16);
        char c = 0;
        while(1){
            if(addr >= fsize || c == 'q') {
                printf("addr(%lX) > fsize(%lX), quit", addr, fsize);
                break;
            }
            disp_CCHARx8(buf, addr);
            addr += 8*0x10;
            printf("enter to next (q to quit):\n");
            //fgets(addr_buf, ADDR_LEN, stdin);
            c = fgetc(stdin);
        }
    }
    return 0;
}
