#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    uint8_t bitmap[32];
} bdf_char;

typedef enum {
    FONT_UTF16 = 0,
    FONT_BIG5 = 1,
} font_encoding;

typedef struct {
    bdf_char *table;
    int encoding;
    int size;
    int start;
    int end;
} bdf_font;

void font_init(bdf_font *fnt, int enc, int size)
{
    fnt->encoding = enc;
    fnt->size = size;
    fnt->table = (bdf_char*)calloc(1, sizeof(bdf_char)*(0x8000));
    switch(enc){
        case FONT_BIG5:
            fnt->start = 0x8140;
            fnt->end = 0xF9E0;
            break;
        case FONT_UTF16:
        default:
            fnt->start = 0x4E00;
            fnt->end = 0x9FFF;
    }
}

void font_release(bdf_font fnt)
{
    if(fnt.table)
        free(fnt.table);
}

long get_file_size(char *filename) {
    struct stat file_status;
    if (stat(filename, &file_status) < 0) {
        return -1;
    }
    return file_status.st_size;
}

void disp_CCHAR(bdf_font fnt, int char_idx)
{
    bdf_char *table = fnt.table;
    uint8_t* font = (uint8_t*)(table[char_idx - fnt.start].bitmap);
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

void disp_C16CHAR(bdf_font fnt, int char_idx)
{
    bdf_char *table = fnt.table;
    uint8_t* font = (uint8_t*)(table[char_idx - fnt.start].bitmap);
    for(int y = 0 ; y < 16; y++){
        for(int x = 0; x < 16; x++){
            int fidx = y*2 + x/8;
            int fofs = x & 0x7;
            int bit = (font[fidx] >> (7 - fofs)) & 0x1;
            char outchar = bit > 0 ? '#' : ' ';
            printf("%c", outchar);
        }
        printf("\n");
    }
}

void bdf_to_rom(bdf_font fnt, int char_idx, uint8_t *dst)
{
    bdf_char *table = fnt.table;
    uint8_t* font = (uint8_t*)(table[char_idx - fnt.start].bitmap);
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

void bdf16_to_rom(bdf_font fnt, int char_idx, uint8_t *dst)
{
    bdf_char *table = fnt.table;
    uint8_t* font = (uint8_t*)(table[char_idx - fnt.start].bitmap);
    for(int y = 0 ; y < 16; y++){
        int didx_0 = y;
        int didx_1 = didx_0 + 16;
        dst[didx_0] = font[y*2];
        dst[didx_1] = font[y*2+1];
    }
}

#define MAX_LEN 1024
bdf_font parse_bdf(FILE *fp, int enc, int size)
{
    bdf_font fnt;
    font_init(&fnt, enc, size);
    char strbuf[MAX_LEN];
    while(1){
        long char_idx = 0;
        char *ptr = NULL;
        while((ptr = fgets(strbuf, MAX_LEN, fp)) != NULL){
            if(strstr(strbuf, "ENCODING")){
                char *idx_p = &(strbuf[9]);
                char_idx = strtol(idx_p, NULL, 10);
                if(char_idx >= fnt.start && char_idx <= fnt.end){
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
            uint32_t tab_idx = char_idx - fnt.start;
            bdf_char *table = fnt.table;
            if(tab_idx < 0x8000){
                for(int y = 0; y < size; y++){
                    fgets(strbuf, MAX_LEN, fp);
                    int char_line = strtol(strbuf, NULL, 16);
                    table[tab_idx].bitmap[y*2] = (char_line >> 8) & 0xFF;
                    table[tab_idx].bitmap[y*2+1] = char_line & 0xFF;
                }
            }else{
                printf("strange idx %X [%lX]\n", tab_idx, char_idx);
            }
        }
    }

    return fnt;
}

#define SEG_CHK(s, e, l) (ofs >= s && ofs < e - l)
int addr_san(int ofs, int len){
    if(
         SEG_CHK(0x54410, 0x56C10, len) ||
         SEG_CHK(0x58410, 0x592D8, len) ||
         SEG_CHK(0x5C410, 0x5D51B, len) ||
         SEG_CHK(0x60410, 0x61BF6, len) ||
         SEG_CHK(0x80410, 0x81185, len) ||
         SEG_CHK(0x84410, 0x86E1B, len) ||
         SEG_CHK(0x88410, 0x8A0D5, len) ||
         SEG_CHK(0x8C410, 0x8D779, len)
    ){
        return 1;
    }

    return 0;
}

void replace_words(FILE *fp, uint16_t *u2r, uint8_t *rom_data, int rom_size)
{
    int lidx = 1;
    while(1){
        uint16_t read_u16;
        unsigned char orig_str[MAX_LEN];
        unsigned char rep_str[MAX_LEN];
        int read_cnt = 0;

        int orig_len = 0;
        while((read_cnt = fread(&read_u16, 1, sizeof(read_u16), fp)) > 0){
            if(read_u16 == 0x3A)
                break;
            uint16_t rom_char = u2r[read_u16];
            memcpy(&(orig_str[orig_len]), &rom_char, 2);
            orig_len += (rom_char < 0xE0 ? 1 : 2);
        }

        int rep_len = 0;
        while((read_cnt = fread(&read_u16, 1, sizeof(read_u16), fp)) > 0){
            if(read_u16 == 0x0A)
                break;
            uint16_t rom_char = u2r[read_u16];
            memcpy(&(rep_str[rep_len]), &rom_char, 2);
            rep_len += (rom_char < 0xE0 ? 1 : 2);
        }

        int rep_cnt = 0;
        if( orig_len == rep_len ) {
            for(int i = 0; i < rom_size; i++){
                if((memcmp(rom_data + i, orig_str, orig_len) == 0)){
                    printf("0x%05X:%d ", i, addr_san(i, rep_len));
                    memcpy(rom_data + i, rep_str, rep_len);
                    i += orig_len;
                    rep_cnt++;
                }
            }
            printf("\n");
        } else {
            printf("str[%d] orig_len[%d] != rep_len[%d]\n", lidx, orig_len, rep_len);
        }
        printf("line[%d] replaces %d\n", lidx, rep_cnt);
        if(rep_cnt == 0){
            printf("ORIG:");
            for(int i = 0; i < orig_len; i++){
                printf("%2X ", orig_str[i]);
            }
            printf("\n");
            printf("REPL:");
            for(int i = 0; i < rep_len; i++){
                printf("%2X ", rep_str[i]);
            }
            printf("\n");
        }

        lidx++;
        if(read_cnt == 0)
            break;
    }
}

int main(int argc, char **argv)
{
    char *bdf_fn = NULL;
    char *bdf16_fn = NULL;
    char *large_charlist_fn = NULL;
    char *charlist_fn = NULL;
    char *srom_fn = NULL;
    char *jrom_fn = NULL;
    char *rep_fn = NULL;
    char *out_fn = "sango2_cht_gen.nes";
    char *num_fn = NULL;
    font_encoding f16_enc = FONT_UTF16;

    int opt = -1;
    while ((opt = getopt(argc, argv, "b:f:c:l:s:t:j:r:o:n:5")) != -1) {
        switch (opt) {
        case 'b':
            bdf_fn = optarg;
            break;
        case 'f':
            bdf16_fn = optarg;
            break;
        case 'c':
            charlist_fn = optarg;
            break;
        case 'l':
            large_charlist_fn = optarg;
            break;
        case 's':
            srom_fn = optarg;
            break;
        case 'j':
            jrom_fn = optarg;
            break;
        case 'r':
            rep_fn = optarg;
            break;
        case 'o':
            out_fn = optarg;
            break;
        case 'n':
            num_fn = optarg;
            break;
        case '5':
            f16_enc = FONT_BIG5;
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s -b BDF_FONT -c CHAR_LIST_U16LE -s SC_ROM -t TC_ROM [ -j JP_ROM -r REPLACE_LIST -n CHINESE_NUM -o OUT_FILE]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if(bdf_fn == NULL || charlist_fn == NULL || srom_fn == NULL){
        printf("Usage: %s -b BDF_FONT -c CHAR_LIST_U16LE -s SC_ROM -t TC_ROM [-j JP_ROM -r REPLACE_LIST -n CHINESE_NUM -o OUT_FILE]\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    //bdf_char *bdf_table = NULL;
    //bdf_table = (bdf_char*)calloc(1, sizeof(bdf_char)*(0xA000 - 0x4E00));
    FILE *bdf_fp = fopen(bdf_fn, "r");
    if(bdf_fp == NULL){
        printf("open %s failed\n", bdf_fn);
        exit(0);
    }
    printf("parsing %s\n", bdf_fn);
    bdf_font fnt_11 = parse_bdf(bdf_fp, FONT_UTF16, 11);
    fclose(bdf_fp);

    //bdf_char *bdf16_table = NULL;
    //bdf16_table = (bdf_char*)calloc(1, sizeof(bdf_char)*(0xFA00 - 0x8140));
    FILE *bdf16_fp = fopen(bdf16_fn, "r");
    if(bdf16_fp == NULL){
        printf("open %s failed\n", bdf16_fn);
        exit(0);
    }
    printf("parsing %s\n", bdf16_fn);

    bdf_font fnt_16 = parse_bdf(bdf16_fp, f16_enc, 16);
    fclose(bdf16_fp);

    // Test
    disp_CCHAR(fnt_11, 0x5F67);
    disp_CCHAR(fnt_11, 0x5DB2);


    if(f16_enc == FONT_BIG5) { 
        disp_C16CHAR(fnt_16, 0xF9DA);
    } else {
        disp_C16CHAR(fnt_16, 0x5F67);
        disp_C16CHAR(fnt_16, 0x8A61);
    }

    long rom_size = get_file_size(srom_fn);
    uint8_t *rom_data = malloc(rom_size);
    FILE *romfp = fopen(srom_fn, "rb");
    if(romfp == NULL){
        printf("open %s failed\n", srom_fn);
        exit(EXIT_FAILURE);
    }
    fread(rom_data, 1, rom_size, romfp);
    fclose(romfp);

    // change mapper
    rom_data[6] = 0x63;
    rom_data[7] = 0xC0;

    // fix Lu-Mon's name
    rom_data[0x6FAAE] = 0xA2;

    // patch CAPCOM title
    if(jrom_fn != NULL){
        romfp = fopen(jrom_fn, "rb");
        if(romfp == NULL){
            printf("open %s failed\n", jrom_fn);
            exit(EXIT_FAILURE);
        }
        fseek(romfp, 0x37BB0, SEEK_SET);
        fread(rom_data+0x37BB0, 1, 0x1A0, romfp);
        fclose(romfp);
    }

    // generate fonts and patch to rom
    FILE *charlist_fp = fopen(charlist_fn, "r");
    if(charlist_fp == NULL){
        printf("open %s failed\n", charlist_fn);
        exit(0);
    }
    uint8_t strbuf[MAX_LEN];
    uint16_t u16_to_rom[0x10000] = {0};
    while(fread(strbuf, 1, 16, charlist_fp) == 16){
        for(int i =0; i < 7; i++){
            strbuf[i] = strbuf[2*i];
        }
        strbuf[8] = 0;
        long offset = strtol((char*)strbuf, NULL, 16);
        long num = strtol(strchr((char*)strbuf, ',')+1, NULL, 16)+1;
        for(int i = 0; i < num; i++, offset+=0x10){
            uint16_t charenc = 0;
            fread(&charenc, 1, 2, charlist_fp);
            if(offset >= 0x90910){
                uint8_t bitmap[16] = {0};
                bdf_to_rom(fnt_11, charenc, bitmap);
                memcpy(rom_data + offset, bitmap, 16);
            }
            // build rom / utf-16 mapping table
            uint32_t rom_idx = (offset - 0x90010) >> 4;
            u16_to_rom[charenc] = rom_idx < 0x100 ? rom_idx : ((rom_idx & 0xFF) << 8) | ((rom_idx >> 8) + 0xDF );
        }
        fread(strbuf, 1, 2, charlist_fp);
    }
    fclose(charlist_fp);

    // patch 16x16 font
    if(large_charlist_fn != NULL) {
        charlist_fp = fopen(large_charlist_fn, "r");
        if(charlist_fp == NULL){
            printf("open %s failed\n", charlist_fn);
            exit(0);
        }
	    if(f16_enc == FONT_BIG5) { 
	        char *ptr = NULL;
	        while((ptr = fgets((char*)strbuf, MAX_LEN, charlist_fp)) != NULL){
	            //printf("%ls\n",strbuf);
	            long offset = strtol((char*)strbuf, NULL, 16);
	            long num = strtol(strchr((char*)strbuf, ',')+1, NULL, 16)+1;
	            //printf("offset=%lX, num=%ld\n", offset, num);
	            fgets((char*)strbuf, MAX_LEN, charlist_fp);
	            for(int i = 0; i < num; i++, offset+=0x20){
	                uint16_t charenc = 0;
	                charenc = strbuf[i*2] << 8 | strbuf[i*2+1];
	                //printf("enc:%X: %lX\n", charenc, offset);
	                uint8_t bitmap[32] = {0};
	                bdf16_to_rom(fnt_16, charenc, bitmap);
	                memcpy(rom_data + offset, bitmap, 32);
	            }
	            //printf("\n");
	        }
	     } else {
	        while(fread(strbuf, 1, 16, charlist_fp) == 16){
	            for(int i =0; i < 7; i++){
	                strbuf[i] = strbuf[2*i];
	            }
	            strbuf[8] = 0;
	            long offset = strtol((char*)strbuf, NULL, 16);
	            long num = strtol(strchr((char*)strbuf, ',')+1, NULL, 16)+1;
	            printf("offset=%lX, num=%ld\n", offset, num);
	            for(int i = 0; i < num; i++, offset+=0x20){
	                uint16_t charenc = 0;
	                fread(&charenc, 1, 2, charlist_fp);

	                printf("enc:%X: %lX\n", charenc, offset);
	                uint8_t bitmap[32] = {0};
	                bdf16_to_rom(fnt_16, charenc, bitmap);
	                memcpy(rom_data + offset, bitmap, 32);
	            }
	            fread(strbuf, 1, 2, charlist_fp);
	            //printf("\n");
	        }
	    }
        fclose(charlist_fp);
    }

    if(rep_fn != NULL) {
        FILE *rep_fp = fopen(rep_fn, "r");
        if(rep_fp == NULL) {
            printf("open %s failed\n", rep_fn);
            exit(0);
        }
        replace_words(rep_fp, u16_to_rom, rom_data, rom_size);
        fclose(rep_fp);
    }

    if(num_fn != NULL) {
        FILE *num_fp = fopen(num_fn, "r");
        if(num_fp == NULL) {
            printf("open %s failed\n", rep_fn);
            exit(0);
        }
        fread(rom_data + 0x1D410, 1, 80, num_fp);
        fclose(num_fp);
    }

    FILE *out_romfp = fopen(out_fn, "wb");
    fwrite(rom_data, 1, rom_size, out_romfp);
    fclose(out_romfp);

    free(rom_data);
    font_release(fnt_11);
    font_release(fnt_16);
    return 0;
}
