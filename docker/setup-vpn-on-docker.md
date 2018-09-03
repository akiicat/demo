# VPN

## Server

雖然是 docker 不過還是會用到底層的環境，所以需要 sudo 來執行 modprobe，建議是在 Linux 環境下運行這個 docker。

```shell
git clone https://github.com/mobilejazz/docker-ipsec-vpn-server.git
cd docker-ipsec-vpn-server
./start.sh
```

安裝好之後，可以新增使用者：

```shell
./adduser.sh akiicat
```

這時候可以拿到連線 VPN 所需要的資訊：

- IP address
- User name
- Password for user
- Shared Password

## Client

下一步就是建立 client 端

不同的作業系統有不同的設定方式，可以參考[這裡](https://github.com/hwdsl2/setup-ipsec-vpn/blob/master/docs/clients.md)。

## References

- https://mobilejazz.com/blog/setting-up-a-vpn-server-in-5-minutes-with-docker/
- https://github.com/mobilejazz/docker-ipsec-vpn-server
- https://github.com/hwdsl2/setup-ipsec-vpn/blob/master/docs/clients.md