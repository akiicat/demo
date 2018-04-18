# CoreOS 安裝

## 網路

先紀錄網卡名稱，設定網路時會用到

```shell
ifconfig
```

設定靜態網路，要安裝 coreos 務必先設定好網路

https://www.freedesktop.org/software/systemd/man/systemd.network.html

```Shell
# /etc/systemd/network/static.network
[Match]
Name=<ifconfig-find-your-name>

[Network]
Address=<your-ip-address>
Gateway=<your-gateway>
DNS=8.8.8.8
```

- `Name` ：用 `ifconfig` 看網卡名稱
- 注意：需要 mask 的話 address 欄位改成如下 `192.168.1.2/16`

套用網路設定

```shell
sudo systemctl restart systemd-networkd
```

測試

```shell
ping 8.8.8.8
wget www.google.com
```



## 硬碟

現有硬碟

```shell
df -h
```

列出所有硬碟，須事先紀錄好要安裝 coreos 的硬碟

```shell
lsblk
```



## 安裝

https://coreos.com/os/docs/latest/installing-to-disk.html#container-linux-configs

```shell
sudo coreos-install -d /dev/sda -C stable
```

- `-d` ：必須，安裝 coreos 的目標硬碟



https://www.youtube.com/watch?v=_I7ErvhwYeY

```shell
sudo openssl passwd -1 > cloud-config-file
```



```shell
# cloud-config-file
users:
  - name: "akiicat"
    passwd: "..."
    groups:
      - sudo
      - docker
hostname: coreos-01
```



```shell
sudo coreos-install -d /dev/sda -C stable -c cloud-config-file
reboot
```





## Tr

```shell
systemctl reboot -f
systemctl status initrd-cleanup.service
```

