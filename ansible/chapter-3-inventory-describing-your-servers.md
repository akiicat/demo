# Inventory: Describing Your Servers

到目前為止，我們只有一個 server，不過在現實的狀況會有很多個 hosts，在 Ansible 裡一群的 hosts 稱作 *inventory*。

## Inventory File

在 Ansible 裡預設描述 hosts 的方法是寫在文字檔裡面，這個檔案稱作 *inventory files*，一個簡單的 inventory file：

```txt
ontario.example.com
newhampshire.example.com
maryland.example.com
virginia.example.com
newyork.example.com
quebec.example.com
rhodeisland.example.com
```

Ansible 預設使用 SSH，如果你使用其他像是 Paramiko connection plugin 則不適用。

Ansible 會自動加 *localhost* 到 inventory，因為 Ansible 會直接執行 local machine，而不是用 SSH 連線。

雖然 Anisble 會自動加 localhost，但是你還是在檔案裡面寫最少一個 host，不然會出錯：

```
ERROR: provided hosts list is empty
```

如果只有 localhost 的話，設定檔要寫成：

```ini
localhost ansible_connection=local
```

## 初始化多台機器

先把之前的虛擬機關掉

```shell
vagrant destroy 95aeb03 --force
```

開啟三個虛擬機

```ruby
# Vagrantfile 
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  # Use the same key for each machine
  config.ssh.insert_key = false

  config.vm.define "vagrant1" do |vagrant1|
    vagrant1.vm.box = "ubuntu/trusty64"
    vagrant1.vm.network "forwarded_port", guest: 80, host: 8080
    vagrant1.vm.network "forwarded_port", guest: 443, host: 8443
  end
  config.vm.define "vagrant2" do |vagrant2|
    vagrant2.vm.box = "ubuntu/trusty64"
    vagrant2.vm.network "forwarded_port", guest: 80, host: 8081
    vagrant2.vm.network "forwarded_port", guest: 443, host: 8444
  end
  config.vm.define "vagrant3" do |vagrant3|
    vagrant3.vm.box = "ubuntu/trusty64"
    vagrant3.vm.network "forwarded_port", guest: 80, host: 8082
    vagrant3.vm.network "forwarded_port", guest: 443, host: 8445
  end
end
```

Vagrant 1.7 版以上每個 host 使用不同的 SSH key，這邊把 ssh 的設定關掉：

```ruby
config.ssh.insert_key = false
```

使用相同的 SSH 會比較方便管理：

```ini
# ansible.cfg
[defaults]
inventory = inventory
remote_user = vagrant
private_key_file = ~/.vagrant.d/insecure_private_key
host_key_checking = False
```

建立三個 server：

```shell
vagrant up
vagrant ssh-config
```

設定 inventory file 像是這樣：

```ini
# inventory/hosts
vagrant1 ansible_host=127.0.0.1 ansible_port=2222
vagrant2 ansible_host=127.0.0.1 ansible_port=2200
vagrant3 ansible_host=127.0.0.1 ansible_port=2201
```

測試是否有建立成功

```shell
$ ansible vagrant2 -a "ip addr show dev eth0"
vagrant2 | CHANGED | rc=0 >>
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP group default qlen 1000
    link/ether 08:00:27:1e:c1:01 brd ff:ff:ff:ff:ff:ff
    inet 10.0.2.15/24 brd 10.0.2.255 scope global eth0
       valid_lft forever preferred_lft forever
    inet6 fe80::a00:27ff:fe1e:c101/64 scope link 
       valid_lft forever preferred_lft forever
```

## Inventory 參數

在 inventory 裡面可以設定 host 的參數，Ansible 稱這些參數 *behavioral inventory parameters*

| Name                         | Default         | Description                                          |
| ---------------------------- | --------------- | ---------------------------------------------------- |
| `ansible_host`               | Name of host    | SSH 連線到的 Hostname 或是 IP address                |
| `ansible_port`               | 22              | SSH 的 Port                                          |
| `ansible_user`               | root            | SSH 的使用者                                         |
| `ansible_password`           | (None)          | SSH 的使用者密碼                                     |
| `ansible_connection`         | Smart           | 如何連線                                             |
| `ansible_private_key_file`   | (None)          | SSH private key                                      |
| `ansible_shell_type`         | sh              | 指令使用的 shell                                     |
| `ansible_python_interpreter` | /usr/bin/python | 在 host 使用的 python                                |
| `ansible_*_interpreter`      | (None)          | 像是 `ansible_python_interpreter` 一樣，但是其他語言 |

從命名來看很清楚他在做什麼。

### ansible_connection

Ansible 支援多重 *transports*，*transports* 是 Ansible 用來連線到 host 的機制。預設的 transport 是 `smart`，他會檢查本地端的 SSH client 是否支援 *ControlPersist* 的功能。如果支援 *ControlPersist* 的功能，Ansible 就會使用 local SSH client，否則 smart transport 會使用 Python-based 的 SSH client，叫做 *Paramiko*

### ansible_shell_type

遠端的 shell type，預設是 */bin/sh*，通常是不會更動 shell type，Ansible 也接受 `csf`, `fish`, `powershell`

### ansible_python_interpreter

在傳送 module 是用 Python 2 實作的，所以不相容 Python 3，如果遠端 Python 2 的位置不是在 */usr/bin/python*，就需要修改這個參數。像是，Arch Linux 的 Python 3 路徑是 */usr/bin/python*，而 Python 2 的路徑是 */usr/bin/python2*。

### 修改預設的 Behavioral Parameter

你可以在 *ansible.cfg* 裡面修改一些預設的設定

| Behavioral inventory parameter | ansible.cfg option |
| ------------------------------ | ------------------ |
| `ansible_port`                 | `remote_port`      |
| `ansible_user`                 | `remote_user`      |
| `ansible_private_key_file`     | `private_key_file` |
| `ansible_shell_type`           | `executable`       |

`executable` 的設定跟 `ansible_shell_type` 設定不完全一樣，executable 需要標示 shell 的完整路徑，像是 */usr/local/bin/fish*，Ansible 會找路徑的 base name *fish*，當作 `ansible_shell_type` 預設參數。

## Groups

Ansible 會自動把所有 hosts 群組起來稱作為 `all`，也可以用 `*` 取代：

```shell
$ ansible all -a "date"
$ ansible '*' -a "date"
vagrant3 | CHANGED | rc=0 >>
Sat Oct 20 17:24:49 UTC 2018

vagrant2 | CHANGED | rc=0 >>
Sat Oct 20 17:28:58 UTC 2018

vagrant1 | CHANGED | rc=0 >>
Sat Oct 20 17:24:49 UTC 2018
```

這邊把我們 hosts 群組起來稱作 vagrant，跟最前面的 hosts 寫在一起：

```ini
# inventory/hosts
ontario.example.com
newhampshire.example.com
maryland.example.com
virginia.example.com
newyork.example.com
quebec.example.com
rhodeisland.example.com

[vagrant]
vagrant1 ansible_host=127.0.0.1 ansible_port=2222
vagrant2 ansible_host=127.0.0.1 ansible_port=2200
vagrant3 ansible_host=127.0.0.1 ansible_port=2201
```

我們可以在群組的時候，同時把 hosts 列在上面：

```ini
# inventory/hosts
ontario.example.com
newhampshire.example.com
maryland.example.com
virginia.example.com
newyork.example.com
quebec.example.com
rhodeisland.example.com
vagrant1 ansible_host=127.0.0.1 ansible_port=2222
vagrant2 ansible_host=127.0.0.1 ansible_port=2200
vagrant3 ansible_host=127.0.0.1 ansible_port=2201

[vagrant]
vagrant1
vagrant2
vagrant3
```

### 範例：部署 Django

