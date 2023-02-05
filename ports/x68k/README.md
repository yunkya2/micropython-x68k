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
  * グラフィックVRAMおよびテキストVRAMのフレームバッファとなる bytearray オブジェクトを返します。
  * `framebuf.FrameBuffer` オブジェクトのコンストラクタに与えることで、`framebuffer`クラスによる画面描画が可能になります。

