CC=gcc
CHS_ROM=sango2_chs.nes

# UTF-16 16x16 FONTS
FONT16=fonts_16x16/unifont/unifont-16.0.04.bdf
L_CHAR_LIST=large_char_list_u16.txt
BIG5_FLAG=

# BIG5 16x16 FONTS
#FONT16=fonts_16x16/taipei16/taipei16_fix.bdf
#FONT16=fonts_16x16/kcfonts/kc15f_fix.bdf
#L_CHAR_LIST=large_char_list_big5.txt
#BIG5_FLAG=-5

CH_NUM=chinese_num.bin


all: font_view font16_ext rom_patch bdf_fix msg_dump bdf_fix bdf16_fix

roms: sango2_cht_zpix.nes sango2_cht_cubic.nes sango2_cht_fusion.nes sango2_cht_fireflyR12.nes sango2_cht_wqy.nes

tabs: char_list_u16.txt replace_u16.txt large_char_list_big5.txt

font_view: font_view.c
	$(CC) -o $@ $^

font16_ext: font16_ext.c
	$(CC) -o $@ $^

rom_patch: rom_patch.c
	$(CC) -g -o $@ $^

bdf_fix: bdf_fix.c
	$(CC) -o $@ $^

bdf16_fix: bdf16_fix.c
	$(CC) -o $@ $^

msg_dump: msg_dump.c
	$(CC) -o $@ $^


sango2_cht_zpix.nes: rom_patch char_list_u16.txt replace_u16.txt $(L_CHAR_LIST)
	./rom_patch -b fonts_11x11/zpix/zpix.bdf -f $(FONT16) -c char_list_u16.txt -l $(L_CHAR_LIST) $(BIG5_FLAG) -s $(CHS_ROM) -r replace_u16.txt -n $(CH_NUM) -o $@

sango2_cht_cubic.nes: rom_patch char_list_u16.txt replace_u16.txt $(L_CHAR_LIST)
	./rom_patch -b fonts_11x11/cubic/cubic11_11_80.bdf -f $(FONT16)  -c char_list_u16.txt -l $(L_CHAR_LIST) $(BIG5_FLAG) -s $(CHS_ROM) -r replace_u16.txt -n $(CH_NUM) -o $@

sango2_cht_fusion.nes: rom_patch char_list_u16.txt replace_u16.txt $(L_CHAR_LIST)
	./rom_patch -b fonts_11x11/fusion/fusion-pixel-11px-monospaced-zh_hant.bdf -f $(FONT16) -c char_list_u16.txt -l $(L_CHAR_LIST) $(BIG5_FLAG) -s $(CHS_ROM) -r replace_u16.txt -n $(CH_NUM) -o $@

sango2_cht_fireflyR12.nes: rom_patch char_list_u16.txt replace_u16.txt $(L_CHAR_LIST)
	./rom_patch -b fonts_11x11/firefly/fireflyR12.bdf -f $(FONT16) -c char_list_u16.txt -l $(L_CHAR_LIST) $(BIG5_FLAG) -s $(CHS_ROM) -r replace_u16.txt -n $(CH_NUM) -o $@

sango2_cht_wqy.nes: rom_patch char_list_u16.txt replace_u16.txt $(L_CHAR_LIST)
	./rom_patch -b fonts_11x11/wqy/wqysong9.bdf -f $(FONT16) -c char_list_u16.txt -l $(L_CHAR_LIST) $(BIG5_FLAG) -s $(CHS_ROM) -r replace_u16.txt -n $(CH_NUM) -o $@

char_list_u16.txt: char_list.txt
	iconv -f UTF-8 -t UTF-16LE $< > $@

replace_u16.txt: replace.txt
	iconv -f UTF-8 -t UTF-16LE $< > $@

large_char_list_big5.txt: large_char_list.txt
	iconv -f UTF-8 -t BIG5 $< > $@

large_char_list_u16.txt: large_char_list.txt
	iconv -f UTF-8 -t UTF-16LE $< > $@

clean:
	rm -f font_view font16_ext rom_patch bdf_fix msg_dump

clean_roms:
	rm -rf sango2_cht_cubic* sango2_cht_firefly* sango2_cht_fusion* sango2_cht_zpix* sango2_cht_fireflyR12.nes sango2_cht_wqy.nes

clean_tabs:
	rm -f char_list_u16.txt replace_u16.txt large_char_list_big5.txt
