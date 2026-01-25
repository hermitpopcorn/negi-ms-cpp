[negi-ms](https://github.com/hermitpopcorn/negi-ms)のC++版。機能、アーキは同じ。ただ、できるだけ外部パッケージを使わず、ありのままのLinuxのAPIを使う（HTMLパーサーなどのような難しいロジックはありかな…）。

OAuth2を自分で実装するのは非推奨らしいのでGoogle SheetsとのやりとりはGoogleのC++ライブラリを使わせていただく。

C++のOAuth2ライブラリが存在しない模様。Rustと合成する。

---

libcurlではPUTリクエストが４１１で失敗している。Content-Lengthのヘッダーをちゃんと送信している（と思う）のに、、、
