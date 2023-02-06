# MicroPython for X680x0

MicroPython のシャープ X680x0 向け移植です。

[MicroPython](https://micropython.org/) v1.19.1 をベースにしています。

## ビルド方法

ビルドにはPC環境のクロスツールチェイン [elf2x68k](https://github.com/yunkya2/elf2x68k) が必要です。
ツールチェインにパスが通って m68k-elf-gcc 等が実行できる状態で、以下の手順でビルドします。

```
$ cd ports/x68k
$ make
```

ビルドに成功すると実行ファイル `build/micropython.x` が出来るので、X680x0 の環境上にコピーしてください。

## X680x0固有ライブラリ

MicroPython 自体の使い方は [公式ドキュメント](https://micropython-docs-ja.readthedocs.io/ja/v1.19.1ja/index.html) を参照してください。

X680x0版では、加えて以下のライブラリをサポートしています。

### `machine` --- ハードウェア関連の関数

X680x0版では以下のクラスのみをサポートしています。

#### クラス `Pin` -- I/O ピンの制御(を模したキーボードLEDの制御)

通常の MicroPython では `Pin` はMCUのI/O ピン (GPIO - 汎用入出力)の制御のために使用されますが、X680x0版ではすべて出力ピンとして、ピン#0～#6がキーボードの各LEDに対応します。
MicroPython向けLチカのコードをそのままX680x0版で動かすためだけに存在します。:-)

### `x68k` --- X680x0 固有ハードウェア関連の関数

* `x68k.crtmod(mode [,gon])`
  * 画面の表示モードを設定する IOCS CRTMOD を実行します。mode には設定する画面モードを指定します。
  * gon を `True` にすると、画面モード設定後にグラフィック画面をクリアして表示モードにします(IOCS G_CLR_ON の実行)。

* `x68k.gvram()`
* `x68k.tvram()`
  * グラフィックVRAMおよびテキストVRAMのフレームバッファとなる `memoryview` オブジェクトを返します。
  * `framebuf.FrameBuffer` オブジェクトのコンストラクタに与えることで、`framebuffer`クラスによる画面描画が可能になります。
* `x68k.fontrom()`
  * フォントROM領域の `memoryview` オブジェクトを返します。読み出し専用です。

* `x68k.i.<IOCSコール名>`
  * IOCSコール番号を定数で定義しています。
    * 例: `x68k.i.B_PRINT` = 0x21
* `x68k.iocs(d0, d1, d2, d3, d4, d5, a1[w], a2[w], rd, ra)`
  * IOCSコールを実行します。`d0`～`d5`,`a1`,`a2` は呼び出しの際のCPUレジスタ値を指定します。`d0`以外は省略可能です。
    * 例: `x68k.iocs(x68k.i.B_KEYINP)`  # IOCS _B_KEYINPを実行します
  * `a1`, `a2`には整数値を直接指定できる他、バッファオブジェクトを指定することでそのオブジェクトの先頭アドレスが渡されます。与えられたバッファに書き込みを行うようなIOCSに対しては、名前を `a1w`, `a2w` にして`bytearray`のような書き換え可能なオブジェクトを与える必要があります。
  * IOCS実行後のd0レジスタの値が関数の戻り値となります。他のレジスタの値が必要な場合は、`rd`, `ra`にそれぞれデータレジスタ、アドレスレジスタの個数を指定することでそれらのレジスタ値を並べたタプルを返します。
    * `rd=2`, `ra=1` を指定すると、関数の戻り値は `(d0,d1,d2,a1)` のタプルとなります。
