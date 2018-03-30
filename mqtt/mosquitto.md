```Shell
brew install mosquitto
# mosquitto has been installed with a default configuration file.
# You can make changes to the configuration by editing:
#     /usr/local/etc/mosquitto/mosquitto.conf
# 
# To have launchd start mosquitto now and restart at login:
#   brew services start mosquitto
# Or, if you don't want/need a background service you can just run:
#   mosquitto -c /usr/local/etc/mosquitto/mosquitto.conf
```

```shell
brew services start mosquitto
brew services stop mosquitto
brew services restart mosquitto
```

## sub

```shell
mosquitto_sub -v -t "topictest"
```

## pub

```shell
mosquitto_pub -t "topictest" -m "hello"
```

## Routes

- `+`代表匹配所有字元
- `#`代表匹配所有階層，通常放在結尾處

```
sensors/+/temperature/+
sensors/#
```



## 使用者認證

建立 Mosquitto 用的帳號密碼檔案，並新增 `gtwang` 這個使用者，並且設定 `gtwang` 的密碼

```shell
mosquitto_passwd -c /etc/mosquitto/passwd gtwang
```



```conf
# /etc/mosquitto/mosquitto.conf
# 設定帳號密碼檔案
password_file /etc/mosquitto/passwd

# 禁止匿名登入
allow_anonymous false
```



重新啟動 Mosquitto 

```shell
brew services restart mosquitto
```



測試

```shell
mosquitto_sub -t gtwang/test -u gtwang -P 1123
mosquitto_pub -t gtwang/test -u gtwang -P 1123 -m "Hello, world!"
```





## SSL

#### CA

```shell
cd ~/.ssh/
mkdir myCA
chmod 700 myCA
cd myCA
```



自動產生伺服器用的金鑰

```Shell
curl https://raw.githubusercontent.com/owntracks/tools/master/TLS/generate-CA.sh -o generate-CA.sh
```



金鑰複製到 Mosquitto 的目錄之中，然後重新啟動 Mosquitto 

```
cp ca.crt /usr/local/etc/mosquitto/ca_certificates/
cp raspberrypi.crt raspberrypi.key /usr/local/etc/mosquitto/certs/
```



在 mosquitto.conf 設定 CA

```conf
# /usr/local/etc/mosquitto/mosquitto.conf
pid_file /usr/local/var/run/mosquitto.pid
# persistence true
# persistence_location /usr/local/var/lib/mosquitto/
log_dest file /usr/local/var/log/mosquitto/mosquitto.log
cafile /usr/local/etc/mosquitto/ca_certificates/ca.crt
certfile /usr/local/etc/mosquitto/certs/myhost.crt
keyfile /usr/local/etc/mosquitto/certs/myhost.key
```



重新啟動 Mosquitto 

```Shell
brew services restart mosquitto
```



若檔案權限不足

```Shell
sudo chown mosquitto /usr/local/etc/mosquitto/certs/raspberrypi.key
```





檢查伺服器的金鑰是否正常運作

```shell
mosquitto_sub -t '$SYS/broker/bytes/#' -u gtwang -P 1123 -v --cafile ca.crt
```

`$SYS/broker/bytes/#` 這個主題每隔 10 秒會輸出統計資訊，類似這樣：

```
$SYS/broker/bytes/received 1008
$SYS/broker/bytes/sent 253
$SYS/broker/bytes/received 1092
$SYS/broker/bytes/sent 325
```