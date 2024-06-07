#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_LEN 1024
int bdf_fix(FILE *fp_in, FILE *fp_out)
{
    char strbuf[MAX_LEN];
    while(1){
        long char_idx = 0;
        char *ptr = NULL;
        while((ptr = fgets(strbuf, MAX_LEN, fp_in)) != NULL){
            fwrite(strbuf, 1, strnlen(strbuf, MAX_LEN), fp_out);
            if(strstr(strbuf, "ENCODING")){
                char *idx_p = &(strbuf[9]);
                char_idx = strtol(idx_p, NULL, 10);
                if(char_idx >= 0x8140 && char_idx < 0xF9D5){
                    break;
                }
            }
        }

        if(ptr == NULL){
            break;
        } else {
            int w = 16, h = 16;
            //printf("parsing char - %lX\n", char_idx);
            while((ptr = fgets(strbuf, MAX_LEN, fp_in)) != NULL){
                int dummy0, dummy1;
                if(strstr(strbuf, "BBX")){
                    sscanf(strbuf, "BBX %d %d %d %d\n", &w, &h, &dummy0, &dummy1);
                    if(h > 16){
                        printf("[%lX] H:%d > 16\n", char_idx, h);
                    }
                    fprintf(fp_out, "BBX 16 16 %d %d\n", dummy0, dummy1);
                    break;
                }
                fwrite(strbuf, 1, strnlen(strbuf, MAX_LEN), fp_out);
            }
            while((ptr = fgets(strbuf, MAX_LEN, fp_in)) != NULL){
                fwrite(strbuf, 1, strnlen(strbuf, MAX_LEN), fp_out);
                if(strstr(strbuf, "BITMAP"))
                    break;
            }
            int pre_y = (16 - h) /2;
            int post_y = pre_y;
            post_y += 16 - (h + pre_y + post_y);

            int shift_R = w <= 8 ? 8 : 0;
            int shift_L = (16 - w) / 2;

            for(int y = 0; y < pre_y; y++)
                fprintf(fp_out, "0000\n");

            for(int y = 0; y < h; y++){
                fgets(strbuf, MAX_LEN, fp_in);
                int char_line = strtol(strbuf, NULL, 16);
                char_line <<= shift_R;
                char_line >>= shift_L;
                fprintf(fp_out, "%04X\n", char_line);
            }

            for(int y = 0; y < post_y; y++)
                fprintf(fp_out, "0000\n");
            /*
            for(int y = 0; y < 11; y++){
                fgets(strbuf, MAX_LEN, fp_in);
                int char_line = strtol(strbuf, NULL, 16);
                table[char_idx - 0x4E00].bitmap[y*2] = (char_line >> 8) & 0xFF;
                table[char_idx - 0x4E00].bitmap[y*2+1] = char_line & 0xFF;
            }
            */
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    FILE *bdf_in = fopen(argv[1], "r");
    FILE *bdf_out = fopen(argv[2], "w");
    if(bdf_in == NULL || bdf_out == NULL){
        printf("file opened failed");
    }
    bdf_fix(bdf_in, bdf_out);

    fclose(bdf_in);
    fclose(bdf_out);

    return 0;
}
