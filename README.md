
#吞食天地2 同能網版本 正體中文化

##正體化流程
1. 建立 ROM 內用字列表 \
透過 font_view.c 觀察 0x90910 ~ 0x99010 間存放的字 \
存檔為 char_list.txt \
 \
檔案格式為:
```
起始位置(16進位),個數-1
依照個數指定的文字
```
 \
然而要將檔案轉存為 UTF-16LE 才可以使用

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
[pcf2bdf](https://github.com/ganaware/pcf2bdf) -o wqysong9.bdf wqy-bitmapsong/wenquanyi_9pt.pcf

3. 從著名的 "豪華中文版" 擷取 16x16 中文字型, 位置在 0x1D810 ~ 0x20010 之間

4. (optional) 從日版擷取 CAPCOM 起始畫面

4. 轉換 2. 當中的 bdf 字型寫到 ROM 對應的位置 \
這主要透過 rom_patch.c 這支程式

5. 關於 ROM 當中 11x11 的存放方式 \
基本上文字的點是以 raster scan 的次序連續存放在 16 bytes (128b) 中, 比較不直覺的是每個 byte 是從 low bit 到 high bit 的次序存放, 而非 NES 1BPP/2BPP 常見的 high bit to low bit 的次序

6. 關於 ROM 當中對白的存放方式 \
對白的存放位置請參考 [Zeven 大的筆記](https://tiebac.baidu.com/p/4579995248?pn=1) \
對白的上下位指向的位置存放著對白的資訊, 由文章中可以得知 0xFx 使用為對白命令 \
在 5. 當中提到的 11x11 文字存放的起始位置為 0x90010 \
在 0x90010 ~ 0x90E00 對白中是直接透過 1 BYTE (0x00 ~ 0xDF) 來直接取用 \
在 0x91010 ~ 0x9xxxx 區間的部份, 會透過 2 BYTEs 來取得位值 \
BYTE 0: 數值範圍 0xE0 ~ 0xEx - 來選擇 0x910F0 ~ 0x9F010 的區間 \
BYTE 1: 數值範圍 0x00 ~ 0xDF - 來選擇 0x9x010 ~ 0x9xE00 存放字 \
那麼如果每個區間的 index 為 0xE0~0xFF(對白中的指令/控制碼), 有可能造成對白上的解讀問題 \
所以可以觀察到在每段 0x1000 的區間中 0xE00~0xFFF 都不使用, 就不會產生 00

7. 對白修正 \
透過的檔案是 replace.txt \
檔案格式為:
```
原始字串:替換字串
```
 \
然而要將檔案轉存為 UTF-16LE 才可以使用


##檔案說明:
1. source code
* Makefile - make project file
* font_view.c - font viewer
* font16_ext.c - 16x16 Trad. Chinese font extractor
* rom_patch.c - ROM patcher
* bdf_fix.c - BDF font center / fixed size BITMAP
* msg_dump.c - message dumper

2. 吞食天地2 ROM 檔案
* 同能網 - sango2_chs.nes
* 豪華中文版 - sango2_cht_pal.nes

3. 原始字型檔案與其授權
* [俐方體](https://github.com/ACh-K/Cubic-11) - Cubic_11_1.100_R.ttf, OFL.txt
* [螢火飛點陣新宋](http://www.study-area.org/apt/firefly-font/) - fireflysung.ttf, ARPHICPL.TXT
* [縫合像素字體](https://github.com/TakWolf/fusion-pixel-font) - fusion-pixel-11px-monospaced-zh_hant.bdf, LICENSE-OFL
* [Zpix 最像素](https://github.com/SolidZORO/zpix-pixel-font) - zpix.bdf, 非商用免費
* [文泉驛點陣宋體(版權問題, 請自行產生 ROM)](http://wenq.org/wqy2/index.cgi?BitmapSong) - wqy-bitmapsong/wenquanyi_9pt.pcf, wqy-bitmapsong/COPYING, wqy-bitmapsong/README

4. 預先處理字型檔案
* cubic11_11_80.bdf - Cubic_11_1.100_R.ttf 轉換編修而成的 BDF 字型
* fusion-pixel-11px-monospaced-zh_hant.bdf - 編修自 12px BDF 版本
* fireflyR12.bdf - 原始 fireflyR12.bdf 編修並轉換而成的 BDF 字型
* zpix.bdf - 由 zpix.bdf 轉換而成的 BDF 字型
* wqysong9.bdf - wqy-bitmapsong/wenquanyi_9pt.pcf 轉換編修而成的 BDF 字型

##模擬器推荐
建議使用 FCEUX or FCEUmm 來遊玩, 主要是人物/物品一多還是會有閃爍的情況\
使用 FCEUX/FCEUmm 能夠使用 PPU 超頻功能, 開啟的方式為:\
選單 Options > Timing Configurations > Overclocking 請勾選, 並在 Vblank Scanlines 數值填入 2
