# Netcat

注意，使用的時候，如果 port 是在 0 到 1023 之間的話，是需要有 root 權限。

## 檢測

### Single port

檢測伺服器特定的連接埠（port）是否有開啟：

```shell
nc -v 192.168.0.175 5000
nc -v <ip-address> <port>
```

### Port Scanning

這行指令會掃描指定機器 1 ~ 1000 與 2000 ~ 3000 這兩個範圍的 TCP 的 port，看看哪些 port 有開啟：

```shell
nc -vnz -w 1 192.168.233.208 1-1000 2000-3000
```

這行則是掃描 UDP 的連接埠：

```shell
nc -vnzu 192.168.1.8 1-65535
```

## 傳送

### 檔案

特點是不需要登入的動作，分別在兩個主機上輸入就能傳送檔案：

接收檔案

```shell
nc -l 5000 > my.jpg
```

傳送檔案

```shell
nc hostB.com 5000 < my.jpg
```

### 目錄

使用 `tar` 指令將整個目錄壓縮成一個檔案後再進行傳送：

接收端：

```shell
nc -l 5000 | tar xvf -
```

傳送端：

```shell
tar cvf - /path/to/dir | nc hostB.com 5000
```

### UDP

#### 傳送測試用的 UDP 封包到遠端伺服器

下面這行指令會傳送 UDP 的測試封包到指定的機器與連接埠：

- `-w1`：timeout 的時間為 1 秒
- `-u`：傳送 UDP 封包

```shell
echo -n "foo" | nc -u -w1 192.168.1.8 5000
```

#### 開啟 UDP 連接埠接收資料

下面這行指令會開啟一個指定的 UDP 連接埠，並將接收到的文字資料直接輸出在終端機中：

```shell
nc -lu localhost 5000
```

## References

- [Netcat（Linux nc 指令）網路管理者工具實用範例](https://blog.gtwang.org/linux/linux-utility-netcat-examples/)

