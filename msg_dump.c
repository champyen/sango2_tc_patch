#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

long get_file_size(char *filename) {
    struct stat file_status;
    if (stat(filename, &file_status) < 0) {
        return -1;
    }
    return file_status.st_size;
}

#define MAX_LEN 1024
int main(int argc, char ** argv)
{
    unsigned short char_table[0xC00] = {0};

    char *charlist_fn = argv[1];
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
            fread(&charenc, 1, 2, charlist_fp);
            char_table[(offset - 0x90010) >> 4] = charenc;
            //printf("TAB[%lX] %X\n", (offset - 0x90010) >> 4, charenc);
        }
        fread(strbuf, 1, 2, charlist_fp);
        //printf("\n");
    }
    fclose(charlist_fp);

    char *srom_fn = argv[2];
    long rom_size = get_file_size(srom_fn);
    unsigned char *rom_data = malloc(rom_size);
    FILE *romfp = fopen(srom_fn, "rb");
    if(romfp == NULL){
        printf("open %s failed\n", srom_fn);
        exit(EXIT_FAILURE);
    }
    fread(rom_data, 1, rom_size, romfp);
    fclose(romfp);

    FILE *msg_fp = fopen("msg1.txt", "wb");
    int msg_offset = 0x54410;
    for(int i = msg_offset; i < 0x56C10; i++)
    {
        unsigned char msg = rom_data[i];
        if(msg < 0xE0) {
            fwrite(&(char_table[msg]), 1, sizeof(short), msg_fp);
        } else if(msg < 0xF0){
            int tab_ofs = ((msg - 0xDF) << 8) | rom_data[++i];
            fwrite(&(char_table[tab_ofs]), 1, sizeof(short), msg_fp);
        } else {
            // CMD
        }
    }
    fclose(msg_fp);

    msg_fp = fopen("msg2.txt", "wb");
    msg_offset = 0x58410;
    for(int i = msg_offset; i < 0x592D8; i++)
    {
        unsigned char msg = rom_data[i];

        if(msg < 0xE0) {
            fwrite(&(char_table[msg]), 1, sizeof(short), msg_fp);
        } else if(msg < 0xF0){
            int tab_ofs = ((msg - 0xDF) << 8) | rom_data[++i];
            fwrite(&(char_table[tab_ofs]), 1, sizeof(short), msg_fp);
        } else {
            // CMD
        }
    }
    fclose(msg_fp);

    msg_fp = fopen("msg3.txt", "wb");
    msg_offset = 0x5C410;
    for(int i = msg_offset; i < 0x5D51B; i++)
    {
        unsigned char msg = rom_data[i];

        if(msg < 0xE0) {
            fwrite(&(char_table[msg]), 1, sizeof(short), msg_fp);
        } else if(msg < 0xF0){
            int tab_ofs = ((msg - 0xDF) << 8) | rom_data[++i];
            fwrite(&(char_table[tab_ofs]), 1, sizeof(short), msg_fp);
        } else {
            // CMD
        }
    }
    fclose(msg_fp);

    msg_fp = fopen("msg4.txt", "wb");
    msg_offset = 0x60410;
    for(int i = msg_offset; i < 0x61BF6; i++)
    {
        unsigned char msg = rom_data[i];

        if(msg < 0xE0) {
            fwrite(&(char_table[msg]), 1, sizeof(short), msg_fp);
        } else if(msg < 0xF0){
            int tab_ofs = ((msg - 0xDF) << 8) | rom_data[++i];
            fwrite(&(char_table[tab_ofs]), 1, sizeof(short), msg_fp);
        } else {
            // CMD
        }
    }
    fclose(msg_fp);

    free(rom_data);
    return 0;
}
