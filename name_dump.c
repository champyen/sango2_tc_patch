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

#define MAX_LEN 1024
int main(int argc, char ** argv)
{
    uint16_t char_table[320] = {0};
    char_table[319] = 0x20;

    char *charlist_fn = argv[1];
    FILE *charlist_fp = fopen(charlist_fn, "r");
    if(charlist_fp == NULL){
        printf("open %s failed\n", charlist_fn);
        exit(0);
    }
    char strbuf[MAX_LEN];
    int cidx = 0;
    while(fread(strbuf, 1, 16, charlist_fp) == 16){
        for(int i =0; i < 7; i++){
            strbuf[i] = strbuf[2*i];
        }
        strbuf[8] = 0;
        long offset = strtol(strbuf, NULL, 16);
        long num = strtol(strchr(strbuf, ',')+1, NULL, 16)+1;
        //printf("offset=%lX, num=%ld\n", offset, num);
        for(int i = 0; i < num; i++, cidx++){
            uint16_t charenc = 0;
            fread(&charenc, 1, 2, charlist_fp);
            char_table[cidx] = charenc;
        }
        fread(strbuf, 1, 2, charlist_fp);
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

    FILE *name_fp = fopen("name_u16.txt", "w");
    for(int i = 0; i < 0x100; i++){
        int name_addr = 0x64010 + ((rom_data[0x6DF10 + i] << 8) | rom_data[0x6DE10 + i]);
        int tab_ofs = rom_data[name_addr + 0x15];

        for(int j = 0; j < 3; j++){
            int char_idx = rom_data[name_addr + 0x16 + j];
            if(char_idx == 0xFF)
                break;

            char_idx += ((tab_ofs >> (2-j)) & 0x1) * 0x100;
            if(char_idx < 320){
                fwrite(&char_table[char_idx], 1, sizeof(uint16_t), name_fp);
            }else{
                printf("entry:[%X] - invalid char idx %d\n", i, char_idx);
            }
        }
        uint16_t new_line = 0x0A;
        fwrite(&new_line, 1, sizeof(uint16_t), name_fp);
    }
    fclose(name_fp);

    free(rom_data);
    return 0;
}
