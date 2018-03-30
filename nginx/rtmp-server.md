# OBS 如何設定私人的 RTMP Server using nginx


大多數人習慣使用 Twitch.tv 或 Youtube 等，可以將自己的影片傳送給觀眾。但是如果想要對串流進行更多的控制，像是想要別人直接串流給你，或是你想要同時在多個平台上開台，或者想要獲得 RTMP Stream。

接下來會設置簡單 RTMP Server，需要稍微懂一些 Linux 指令。

## Step 1. 找一台機器

RTMP Server 佔系統的資相對的輕量，它實際上在做的事情是，將 Input 的資料轉送到 Output，只是簡單的數據傳輸而已。

一台 Raspberry Pi 就能滿足所有需求了，或是租一台 Server，不過要確定他的頻寬是夠的，因惠他會佔很大的流量。

這邊的環境是用 Ubuntu，Windows 和 Mac 要安裝 nginx 得自己找了

windows：http://nginx-win.ecsds.eu/download/
mac：http://brew.sh/homebrew-nginx

## Step 2. 安裝 Nginx with RTMP module

安裝必要的 Library

```shell
sudo apt-get install build-essential libpcre3 libpcre3-dev libssl-dev
```

接下來要安裝 [nginx](http://nginx.org/) 

跟有人寫的模組 [nginx-rtmp-module](https://github.com/arut/nginx-rtmp-module)

在你的家目錄下，下載 nginx，我是載最新穩定版

```shell
wget http://nginx.org/download/nginx-1.12.2.tar.gz
```

如果有更新的話可以到這邊看他的版本 [nginx download page](http://nginx.org/en/download.html)

然後再下載 nginx rtmp module

```shell
wget https://github.com/arut/nginx-rtmp-module/archive/master.zip
```

解壓縮

```shell
tar -zxvf nginx-1.12.2.tar.gz
unzip master.zip
cd nginx-1.12.2
```

開始編譯

```shell
./configure --with-http_ssl_module --add-module=../nginx-rtmp-module-master
make
sudo make install
```

跑完之後，nginx 預設就會安裝在 /usr/local/nginx 這底下，可以跑下面的指令來開啟 Server：

```
sudo /usr/local/nginx/sbin/nginx
```

這時候去看你的網頁 http://<your server ip>/ ，應該就能該到 Welcome to nginx! 的頁面

## Step 3. 設定 nginx

要使用 RTMP 還需要對 nginx 做一些設定，所以先找到 /usr/local/nginx/conf/nginx.conf 這個檔案，然後在檔案的結尾加上

```shell
rtmp {
        server {
                listen 1935;
                chunk_size 4096;

                application live {
                        live on;
                        record off;
                }
        }
}
```

這是最簡單的設定，當有人請求 RTMP Stream 的時候，就會轉發給他。更詳細的設定可以參考附錄的連結。

然後重新啟動 nginx

```shell
sudo /usr/local/nginx/sbin/nginx -s stop
sudo /usr/local/nginx/sbin/nginx
```

## Step 4. 測試

現在就可以用 OBS 來測試看看 RTMP 有沒有正常運作

然後在你 OBS 設定串流裡面

- 串流類型：**自訂串流伺服器**
- URL：**rtmp://<your server ip>/live**
- 串流金鑰：**test**

串流金鑰打 test 會不會不安全，其實就只有你個人在使用，所以打你想要的都行。

這時候如果按開始串流不會跳出任何錯誤的話，就恭喜你成功了。

要看到你影片可以透過 VLC 播放器，然後在網路串流的地方打上 rtmp://<your server ip>/live/test 如果有在正常運作那就會看到。

如果是想輸入指令可以安裝 [ffplay](https://evermeet.cx/ffmpeg/)

```shell
ffplay rtmp://<your server ip>/live/test 
```

## 接下來？

如果你想要同時在兩個地方開台的話呢，把 `record off` 這行刪掉，然後加上：

```shell
push rtmp://<other streaming service rtmp url>/<stream key>
```

所以如果同時在 Twitch 和 Youtube 上開台的話，nginx 設定檔就會像這樣：

```shell
# /usr/local/nginx/conf/nginx.conf
rtmp {
        server {
                listen 1935;
                chunk_size 4096;

                application live {
                        live on;
                        push rtmp://live.twitch.tv/app/live_********_******************************;
                        push rtmp://a.rtmp.youtube.com/live2/****-****-****-****;
                }
        }
}
```

所有經過這裡的串流都會被推到 Twitch 和 Youtube 上。

## hls varient 

可以製作多版本的 m3u8 master playlists

```shell
application live { 
        live on; 
 
        exec ffmpeg -i rtmp://localhost/live/$name 
                -c:a libfdk_aac -b:a 32k  -c:v libx264 -b:v 128K -f flv rtmp://localhost/hls/$name_low 
                -c:a libfdk_aac -b:a 64k  -c:v libx264 -b:v 256k -f flv rtmp://localhost/hls/$name_mid 
                -c:a libfdk_aac -b:a 128k -c:v libx264 -b:v 512K -f flv rtmp://localhost/hls/$name_hi; 
} 
 
application hls { 
        live on; 
 
        hls on; 
        hls_path /tmp/hls; 
        hls_nested on; 
 
        hls_variant _low BANDWIDTH=160000;
        hls_variant _mid BANDWIDTH=320000;
        hls_variant _hi  BANDWIDTH=640000;
} 
```

[How to set up your own private RTMP server using nginx](https://obsproject.com/forum/resources/how-to-set-up-your-own-private-rtmp-server-using-nginx.50/)
[Here's the whole configuration guide](https://github.com/arut/nginx-rtmp-module/wiki/Directives)
