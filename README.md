# sango2_tc_patch
任天堂 吞食天地2 - 諸葛孔明傳 同能網版 正體中文化專案

##正體化流程
1. 建立 ROM 內用字列表 \
透過 font_view.c 觀察 0x90910 ~ 0x99010 間存放的字 \
存檔為 char_list.txt \
 \
檔案格式為: \
```
起始位置(16進位),個數-1
依照個數指定的文字
```
 \
然而要將檔案轉存為 UTF-16LE 才可以使用 \

2. 產生 11x11 字型檔案 \
目前找到的點陣字型有兩個 \
俐方體 - https://github.com/ACh-K/Cubic-11 \
螢火飛新宋 - http://www.study-area.org/apt/firefly-font/ \
字型必須要轉換為 bdf 這個 bitmap 格式來使用 \
linux 上可以透過 otf2bdf 這個程式, \
 \
預先處理的 BDF 字型生成方式: \
otf2bdf Cubic_11_1.100_R.ttf -p 11 -r 80 -o cubic11_11_80.bdf \
otf2bdf fireflysung.ttf -p 11 -r 80 -o firefly_11_80.bdf \
pcf2bdf -o wqysong9.bdf wqy-bitmapsong/wenquanyi_9pt.pcf \
(from https://github.com/ganaware/pcf2bdf) \

3. 從著名的 "豪華中文版" 擷取 16x16 中文字型, 位置在 0x1D810 ~ 0x20010 之間 \
透過 font16_ext.c 來預先處理, 也就是附上的 font16.bin \

4. 轉換 2. 當中的 bdf 字型寫到 ROM 對應的位置 \
這主要透過 rom_patch.c 這支程式 \

5. 關於 ROM 當中 11x11 的存放方式 \
基本上文字的點是以 raster scan 的次序連續存放在 16 bytes (128b) 中, 比較不直覺的是每個 byte 是從 low bit 到 high bit 的次序存放, 而非 NES 1BPP/2BPP 常見的 high bit to low bit 的次序

##檔案說明:
1. source code - Makefile, font_view.c, font16_ext.c, rom_patch.c \

2. 吞食天地2 ROM 檔案
* 同能網 - sango2_chs.nes
* 豪華中文版 - sango2_cht_pal.nes

3. 原始字型檔案與其授權
* [螢火飛點陣新宋](http://www.study-area.org/apt/firefly-font/) - fireflysung.ttf, ARPHICPL.TXT
* [俐方體](https://github.com/ACh-K/Cubic-11) - Cubic_11_1.100_R.ttf, OFL.txt
* [文泉驛點陣宋體](http://wenq.org/wqy2/index.cgi?BitmapSong) - wqy-bitmapsong/wenquanyi_9pt.pcf, wqy-bitmapsong/COPYING, wqy-bitmapsong/README

4. 預先處理字型檔案
* font16.bin - 自 2.1 擷取出的 16x16 字型
* fireflysung_11_80.bdf - fireflysung.ttf 轉換編修而成的 BDF 字型
* cubic11_11_80.bdf - Cubic_11_1.100_R.ttf 轉換編修而成的 BDF 字型
* wqysong9.bdf - wqy-bitmapsong/wenquanyi_9pt.pcf 轉換編修而成的 BDF 字型
