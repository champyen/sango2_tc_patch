CC=gcc

all: font_view font16_ext rom_patch bdf_fix

font_view: font_view.c
	$(CC) -o $@ $^

font16_ext: font16_ext.c
	$(CC) -o $@ $^

rom_patch: rom_patch.c
	$(CC) -o $@ $^

bdf_fix: bdf_fix.c
	$(CC) -o $@ $^

clean:
	rm -f font_view font16_ext rom_patch bdf_fix
