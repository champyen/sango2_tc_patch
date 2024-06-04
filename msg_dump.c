#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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

void dump_msg(char *msg_fn, uint8_t *rom_data, uint16_t *char_table, int start_ofs, int end_ofs)
{
    FILE *msg_fp = fopen(msg_fn, "w");
    for(int i = start_ofs; i < end_ofs; i++)
    {
        uint8_t msg = rom_data[i];
        if(msg < 0xE0) {
            fwrite(&(char_table[msg]), 1, sizeof(short), msg_fp);
        } else if(msg < 0xF0){
            int tab_ofs = ((msg - 0xDF) << 8) | rom_data[++i];
            fwrite(&(char_table[tab_ofs]), 1, sizeof(short), msg_fp);
        } else {
            uint16_t new_line;
            if(msg == 0xFF) {
                new_line = 0x0A;
                fwrite(&new_line, 1, sizeof(short), msg_fp);
            } else {
                new_line = 0x20;
                fwrite(&new_line, 1, sizeof(short), msg_fp);
            }
        }
    }
    fclose(msg_fp);
}

#define MAX_LEN 1024
int main(int argc, char ** argv)
{
    uint16_t char_table[0xC00] = {0};

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
            uint16_t charenc = 0;
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
    uint8_t *rom_data = malloc(rom_size);
    FILE *romfp = fopen(srom_fn, "rb");
    if(romfp == NULL){
        printf("open %s failed\n", srom_fn);
        exit(EXIT_FAILURE);
    }
    fread(rom_data, 1, rom_size, romfp);
    fclose(romfp);

    dump_msg("msg1.txt", rom_data, char_table, 0x54410, 0x56C10);
    dump_msg("msg2.txt", rom_data, char_table, 0x58410, 0x592D8);
    dump_msg("msg3.txt", rom_data, char_table, 0x5C410, 0x5D51B);
    dump_msg("msg4.txt", rom_data, char_table, 0x60410, 0x61BF6);
    dump_msg("msg5.txt", rom_data, char_table, 0x80410, 0x81185);
    dump_msg("msg6.txt", rom_data, char_table, 0x84410, 0x86E1B);
    dump_msg("msg7.txt", rom_data, char_table, 0x88410, 0x8A0D5);
    dump_msg("msg8.txt", rom_data, char_table, 0x8C410, 0x8D779);

    free(rom_data);
    return 0;
}
