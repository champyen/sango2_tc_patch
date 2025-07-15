CC=gcc
CHS_ROM=sango2_chs.nes

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


sango2_cht_zpix.nes: rom_patch char_list_u16.txt replace_u16.txt large_char_list_big5.txt
	./rom_patch -b zpix.bdf -f taipei16_fix.bdf -c char_list_u16.txt -l large_char_list_big5.txt -s $(CHS_ROM) -r replace_u16.txt -o $@
	mkdir sango2_cht_zpix; cp $@ sango2_cht_zpix/; zip -r sango2_cht_zpix.zip sango2_cht_zpix

sango2_cht_cubic.nes: rom_patch char_list_u16.txt replace_u16.txt large_char_list_big5.txt
	./rom_patch -b cubic11_11_80.bdf -f taipei16_fix.bdf  -c char_list_u16.txt -l large_char_list_big5.txt -s $(CHS_ROM) -r replace_u16.txt -o $@
	mkdir sango2_cht_cubic; cp $@ sango2_cht_cubic/; cp OFL.txt sango2_cht_cubic/; zip -r sango2_cht_cubic.zip sango2_cht_cubic

sango2_cht_fusion.nes: rom_patch char_list_u16.txt replace_u16.txt large_char_list_big5.txt
	./rom_patch -b fusion-pixel-11px-monospaced-zh_hant.bdf -f taipei16_fix.bdf -c char_list_u16.txt -l large_char_list_big5.txt -s $(CHS_ROM) -r replace_u16.txt -o $@
	mkdir sango2_cht_fusion; cp $@ sango2_cht_fusion/; cp LICENSE-OFL sango2_cht_fusion/; zip -r sango2_cht_fusion.zip sango2_cht_fusion

sango2_cht_fireflyR12.nes: rom_patch char_list_u16.txt replace_u16.txt large_char_list_big5.txt
	./rom_patch -b fireflyR12.bdf -f taipei16_fix.bdf -c char_list_u16.txt -l large_char_list_big5.txt -s $(CHS_ROM) -r replace_u16.txt -o $@
	mkdir sango2_cht_firefly; cp $@ sango2_cht_firefly/; cp ARPHICPL.TXT sango2_cht_firefly/; zip -r sango2_cht_firefly.zip sango2_cht_firefly

sango2_cht_wqy.nes: rom_patch char_list_u16.txt replace_u16.txt large_char_list_big5.txt
	./rom_patch -b wqysong9.bdf -f taipei16_fix.bdf -c char_list_u16.txt -l large_char_list_big5.txt -s $(CHS_ROM) -r replace_u16.txt -o $@

char_list_u16.txt: char_list.txt
	iconv -f UTF-8 -t UTF-16LE $< > $@

replace_u16.txt: replace.txt
	iconv -f UTF-8 -t UTF-16LE $< > $@

large_char_list_big5.txt: large_char_list.txt
	iconv -f UTF-8 -t BIG5 $< > $@
	iconv -f UTF-8 -t UTF-16LE $< > large_char_list_u16.txt

clean:
	rm -f font_view font16_ext rom_patch bdf_fix msg_dump

clean_roms:
	rm -rf sango2_cht_cubic* sango2_cht_firefly* sango2_cht_fusion* sango2_cht_zpix* sango2_cht_fireflyR12.nes sango2_cht_wqy.nes

clean_tabs:
	rm -f char_list_u16.txt replace_u16.txt large_char_list_big5.txt
