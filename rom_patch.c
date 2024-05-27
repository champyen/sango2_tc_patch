#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    unsigned char bitmap[22];
} bdf_char;

long get_file_size(char *filename) {
    struct stat file_status;
    if (stat(filename, &file_status) < 0) {
        return -1;
    }
    return file_status.st_size;
}

void disp_CCHAR(bdf_char *table, int char_idx)
{
    unsigned char* font = (unsigned char*)(table[char_idx - 0x4E00].bitmap);
    for(int y = 0 ; y < 11; y++){
        for(int x = 0; x < 11; x++){

            int fidx = y*2 + (x/8);
            int fofs = x & 0x7;

            int bit = (font[fidx] >> (7 - fofs)) & 0x1;
            char outchar = bit > 0 ? '#' : ' ';
            printf("%c", outchar);
        }
        printf("\n");
    }
}

void bdf_to_rom(bdf_char *table, int char_idx, unsigned char *dst)
{
    unsigned char* font = (unsigned char*)(table[char_idx - 0x4E00].bitmap);
    for(int y = 0 ; y < 11; y++){
        for(int x = 0; x < 11; x++){

            int s_fidx = y*2 + (x/8);
            int s_fofs = x & 0x7;

            int bit = (font[s_fidx] >> (7 - s_fofs)) & 0x1;

            int d_bitidx = x + y*11;
            int d_fidx = d_bitidx >> 3;
            int d_fofs = d_bitidx & 0x7;
            dst[d_fidx] = dst[d_fidx] | (bit << d_fofs);
        }
    }
}

#define MAX_LEN 1024
int parse_bdf(FILE *fp, bdf_char *table)
{
    char strbuf[MAX_LEN];
    while(1){
        long char_idx = 0;
        char *ptr = NULL;
        while((ptr = fgets(strbuf, MAX_LEN, fp)) != NULL){
            if(strstr(strbuf, "STARTCHAR")){
                char *idx_p = &(strbuf[10]);
                char_idx = strtol(idx_p, NULL, 16);
                if(char_idx >= 0x4E00 && char_idx < 0xA000){
                    break;
                }
            }
        }

        if(ptr == NULL){
            break;
        } else {
            //printf("parsing char - %lX\n", char_idx);
            while((ptr = fgets(strbuf, MAX_LEN, fp)) != NULL){
                if(strstr(strbuf, "BITMAP")){
                    break;
                }
            }
            for(int y = 0; y < 11; y++){
                fgets(strbuf, MAX_LEN, fp);
                int char_line = strtol(strbuf, NULL, 16);
                table[char_idx - 0x4E00].bitmap[y*2] = (char_line >> 8) & 0xFF;
                table[char_idx - 0x4E00].bitmap[y*2+1] = char_line & 0xFF;
            }
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    char *bdf_fn = NULL;
    char *charlist_fn = NULL;
    char *srom_fn = NULL;
    char *f16_fn = NULL;
    char *out_fn = "sango2_cht_gen.nes";

    int opt = -1;
    while ((opt = getopt(argc, argv, "b:c:s:f:o:")) != -1) {
        switch (opt) {
        case 'b':
            bdf_fn = optarg;
            break;
        case 'c':
            charlist_fn = optarg;
            break;
        case 's':
            srom_fn = optarg;
            break;
        case 'f':
            f16_fn = optarg;
            break;
        case 'o':
            out_fn = optarg;
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s -b BDF_FONT -c CHAR_LIST_U16LE -s SC_ROM -f TC_16_FONT [-o OUT_FILE]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if(bdf_fn == NULL || charlist_fn == NULL || srom_fn == NULL || f16_fn == NULL){
        printf("Usage: %s -b BDF_FONT -c CHAR_LIST_U16LE -s SC_ROM -f TC_16_FONT [-o OUT_FILE]\n", argv[0]);
        exit(0);
    }

    bdf_char *bdf_table = NULL;
    bdf_table = (bdf_char*)calloc(1, sizeof(bdf_char)*(0xA000 - 0x4E00));
    FILE *bdf_fp = fopen(bdf_fn, "r");
    if(bdf_fp == NULL){
        printf("open %s failed\n", bdf_fn);
        exit(0);
    }
    printf("parsing %s\n", bdf_fn);
    parse_bdf(bdf_fp, bdf_table);
    fclose(bdf_fp);

    // Test
    disp_CCHAR(bdf_table, 0x749D);

    long rom_size = get_file_size(srom_fn);
    unsigned char *rom_data = malloc(rom_size);
    FILE *romfp = fopen(srom_fn, "rb");
    if(romfp == NULL){
        printf("open %s failed\n", srom_fn);
        exit(0);
    }
    fread(rom_data, 1, rom_size, romfp);
    fclose(romfp);

    // change mapper
    rom_data[6] = 0x63;
    rom_data[7] = 0xC0;

    // patch 16x16 font
    romfp = fopen(f16_fn, "rb");
    if(romfp == NULL){
        printf("open %s failed\n", f16_fn);
        exit(0);
    }
    fread(rom_data+0x1D810, 1, 0x2800, romfp);
    fclose(romfp);

    // generate fonts and patch to rom
    FILE *charlist_fp = fopen(charlist_fn, "r");
    if(charlist_fp == NULL){
        printf("open %s failed\n", charlist_fn);
        exit(0);
    }
    char strbuf[MAX_LEN];
    while(fread(strbuf, 1, 16, charlist_fp) == 16){
        //printf("%ls\n",strbuf);
        for(int i =0; i < 7; i++){
            strbuf[i] = strbuf[2*i];
        }
        strbuf[8] = 0;
        long offset = strtol(strbuf, NULL, 16);
        long num = strtol(strchr(strbuf, ',')+1, NULL, 16)+1;
        //printf("offset=%lX, num=%ld\n", offset, num);
        for(int i = 0; i < num; i++, offset+=0x10){
            unsigned short charenc = 0;
            unsigned char bitmap[16] = {0};
            fread(&charenc, 1, 2, charlist_fp);
            bdf_to_rom(bdf_table, charenc, bitmap);
            memcpy(rom_data + offset, bitmap, 16);
            //printf("%X ", charenc);
        }
        fread(strbuf, 1, 2, charlist_fp);
        //printf("\n");
    }
    fclose(charlist_fp);

    FILE *out_romfp = fopen(out_fn, "wb");
    fwrite(rom_data, 1, rom_size, out_romfp);
    fclose(out_romfp);

    return 0;
}
