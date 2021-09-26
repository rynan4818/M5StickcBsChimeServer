# M5StickcChimeServer

# 必要ライブラリ
[arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets)

ただし、ボードマネージャのM5Stack 1.0.9では arduinoWebSocketsは2.3.5だとエラーになるため、2.3.4を使用した。
https://github.com/Links2004/arduinoWebSockets/releases/tag/2.3.4

個別に修正が必要な箇所は以下の通り

    const char *ssid = "WiFi-AP-SSID";
    const char *password = "WiFi-AP-PASSWORD";
    const int chimePin = 33; //Chime A Contact Pin

chimePinは33だと、Groveコネクタの1番ピン(黄色)になります。

ドアホンのA接点端子にGroveコネクタの1ピンと4ピン(黒:GND)をつないで下さい。

ドアホンがリレー接点以外で電圧が出ているとM5StickCが破損しますので、必ず事前にテスタで電圧が出ないか確認して下さい。
