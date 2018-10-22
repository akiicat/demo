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

我們需要分 production、staging、vagrant 的群組，另外還有 web、task、RabbitMQ、Postgre 的群組。範例 inventory file 的參考寫法：

```ini
inventory/hosts 
[production]
delaware.example.com
georgia.example.com
maryland.example.com
newhampshire.example.com
newjersey.example.com
newyork.example.com
northcarolina.example.com
pennsylvania.example.com
rhodeisland.example.com
virginia.example.com

[staging]
ontario.example.com
quebec.example.com

[vagrant]
vagrant1 ansible_host=127.0.0.1 ansible_port=2222
vagrant2 ansible_host=127.0.0.1 ansible_port=2200
vagrant3 ansible_host=127.0.0.1 ansible_port=2201

[lb]
delaware.example.com

[web]
georgia.example.com
newhampshire.example.com
newjersey.example.com
ontario.example.com
vagrant1

[task]
newyork.example.com
northcarolina.example.com
maryland.example.com
ontario.example.com
vagrant2

[rabbitmq]
pennsylvania.example.com
quebec.example.com
vagrant3

[db]
rhodeisland.example.com
virginia.example.com
quebec.example.com
vagrant3
```

我們不需要在前面把不分群組的 server 重新寫過一次，因為沒有必要，而且會讓 server 看起來更長。

### Aliases 跟 Ports

```ini
[vagrant]
vagrant1 ansible_host=127.0.0.1 ansible_port=2222
vagrant2 ansible_host=127.0.0.1 ansible_port=2200
vagrant3 ansible_host=127.0.0.1 ansible_port=2201
```

`vagrant1` `vagrant2` `vagrant3` 這些是匿名，並不是真的 hostname，但可以使用這些名字來連到 server。

Ansible 也支援 `<hostname>:<port>` 語法，所以可以把 `vagrant1` 改成 `127.0.0.1:2222`，但是不會正常運作：

```ini
[vagrant]
127.0.0.1:2222
127.0.0.1:2200
127.0.0.1:2201
```

因為 Ansible 只會把其中一個 host 跟 127.0.0.1 做連結，所以 vagrant 這個群組只會有一個 host 而不是 3 個。

### Groups 中 Groups

我們可以定義 `django` 這個群組，包含 web 和 task 的群組， 

```ini
[django:children]
web
task
```

注意我們在命名群組中的群組的格式不同，這樣 Ansible 才知道 `web` 跟 `task` 是 groups 而不是 hosts。

### 有順序的 Hosts (Pets vs Cattle)

在現實的狀況下，你的 server 會 scale-out，假設你的 server 有 15 個，你不必在 inventory file 寫 15 次。

Bill Baker of Microsoft 想到對待 server 要像 pets 還是要像 cattles 的差別，我們會給寵物單獨的名字，而且會獨自照顧他，相反的，牛會給他一個唯一的編號，參考[Pets vs Cattle](https://www.slideshare.net/randybias/pets-vs-cattle-the-elastic-cloud-story)。

Cattle 比較能夠 scalable，如果你有 20 個 servers 命名像是 *web1.example.com*, *web2.example.com*，在 inventory file 你可以寫成：

```ini
[web]
web[1:20].example.com
```

如果你預設是兩個字元的數字，以 0 開頭的話：

```ini
[web]
web[01:20].example.com
```

如果你的 server 是用英文字母命名的話，像是前 20 個英文字母 *web-a.example.com*, *web-b.example.com*：

```ini
[web]
web-[a-t].example.com
```

## Host 和 Group 有他們自己的檔案

當你的 host 和 group 越來越多的時候，會變得越難管理。

Ansible 提供了延伸性的方法來記錄 host 和 group 的變數，在路徑下建立不同的 host 和 group，使用的格式是 YAML。

Ansible 會找尋 *host_vars* 資料夾底下 host variable 的檔案，跟 *group_vars* 資料夾底下的 group variable 的檔案，Ansible  希望你把這些資料夾跟 inventory file 放在同一個地方。

```shell
$ tree
.
├── Vagrantfile
├── ansible.cfg
└── group_vars
    └── production
```

像是這個 group file：

```yaml
# group_vars/production
---
db_primary_host: rhodeisland.example.com
db_replica_host: virginia.example.com
db_name: widget_production
db_user: widgetuser
db_password: pFmMxcyD;Fc6)6
rabbitmq_host: pennsylvania.example.com
```

可以代表這樣：

```yaml
# group_vars/production
db:
    user: widgetuser
    password: pFmMxcyD;Fc6)6
    name: widget_production
    primary:
        host: rhodeisland.example.com
        port: 5432
    replica:
        host: virginia.example.com
        port: 5432
rabbitmq:
    host: pennsylvania.example.com
    port: 5672
```

拿取參數的話分別用：

```
{{ db_primary_host }}
{{ db.primary.host }}
```

Ansible 也允許你在資料夾裡面又在建立一個資料夾

```yaml
# group_vars/production/db
db:
    user: widgetuser
    password: pFmMxcyD;Fc6)6
    name: widget_production
    primary:
        host: rhodeisland.example.com
        port: 5432
    replica:
        host: virginia.example.com
        port: 5432
        
# group_vars/production/rabbitmq
rabbitmq:
    host: pennsylvania.example.com
    port: 5672
```

普遍來說，只需要讓你的變數保持簡單就行，不一定要分太多的資料夾。

## Dynamic Inventory

到目前為止，我們把 server 記錄在 inventory file，但是如果你同時使用不同的平台的 hosts，像是 Amazon EC2，你可以透過 `awscli` 知道你 server 狀態，如果你是使用自己管理的 server，可以使用像是 Cobbler 或是 Ubuntu Metal as a Service (MAAS)，來記錄你 server 的狀態，或是使用 configuration management database (CMDBs) 知道即時的資訊。

所以 Ansible 支援 *dynamic inventory*，避免你手動複製 host 的訊息。

### Dynamic Inventory 腳本介面

大多的 dynamic inventory script 支援這兩個指令：

- `--host=<hostname>`：顯示 host 詳細資訊
- `--list`：列出群組

#### 顯示 host 詳細資訊

要獲得特定 host 的詳細資料：

```shell
./vagrant.py --host=vagrant2
```

輸出會是 JSON 格式：

```json
{  
   "ansible_ssh_host":"127.0.0.1",
   "ansible_ssh_port":"2200",
   "ansible_ssh_user":"vagrant",
   "ansible_ssh_private_key_file":"/Users/akiicat/.vagrant.d/insecure_private_key"
}
```

#### 列出群組

列出所有 group 的 host：

```shell
./vagrant.py --list
```

輸出同樣是 JSON 格式，在 `_meta` 會包含所有獨自 hosts 的資訊：

```json
{  
   "vagrant":[  
      "vagrant1",
      "vagrant3",
      "vagrant2"
   ],
   "_meta":{  
      "hostvars":{  
         "vagrant1":{  
            "ansible_ssh_host":"127.0.0.1",
            "ansible_ssh_port":"2222",
            "ansible_ssh_user":"vagrant",
            "ansible_ssh_private_key_file":"/Users/akiicat/.vagrant.d/insecure_private_key"
         },
         "vagrant3":{  
            "ansible_ssh_host":"127.0.0.1",
            "ansible_ssh_port":"2201",
            "ansible_ssh_user":"vagrant",
            "ansible_ssh_private_key_file":"/Users/akiicat/.vagrant.d/insecure_private_key"
         },
         "vagrant2":{  
            "ansible_ssh_host":"127.0.0.1",
            "ansible_ssh_port":"2200",
            "ansible_ssh_user":"vagrant",
            "ansible_ssh_private_key_file":"/Users/akiicat/.vagrant.d/insecure_private_key"
         }
      }
   }
}
```

### 撰寫 Dynamic Inventory 腳本

如果使用 `vagrant status` 指令輸出像這樣：

```shell
$ vagrant status
Current machine states:

vagrant1                  running (virtualbox)
vagrant2                  running (virtualbox)
vagrant3                  running (virtualbox)

This environment represents multiple VMs. The VMs are all listed
above with their current state. For more information about a specific
VM, run `vagrant status NAME`.
```

Vagrant 已經紀錄有哪些機器，我們不需要在自己寫，而我們只需要自己寫 inventory file script，詢問正在運行的機器。一旦寫好腳本，不管 Vagrantfile 機器怎麼變，都不需要改動 Ansible inventory file。

我們的 dynamic inventory file 會試著解析指令

```shell
vagrant status --machine-readable
```

輸出如下，可以知道所有的機器：

```shell
1540128277,vagrant1,metadata,provider,virtualbox
1540128277,vagrant2,metadata,provider,virtualbox
1540128278,vagrant3,metadata,provider,virtualbox
1540128278,vagrant1,provider-name,virtualbox
1540128278,vagrant1,state,running
1540128278,vagrant1,state-human-short,running
1540128278,vagrant1,state-human-long,The VM is running. To stop this VM%!(VAGRANT_COMMA) you can run `vagrant halt` to\nshut it down forcefully%!(VAGRANT_COMMA) or you can run `vagrant suspend` to simply\nsuspend the virtual machine. In either case%!(VAGRANT_COMMA) to restart it again%!(VAGRANT_COMMA)\nsimply run `vagrant up`.
1540128279,vagrant2,provider-name,virtualbox
1540128279,vagrant2,state,running
1540128279,vagrant2,state-human-short,running
1540128279,vagrant2,state-human-long,The VM is running. To stop this VM%!(VAGRANT_COMMA) you can run `vagrant halt` to\nshut it down forcefully%!(VAGRANT_COMMA) or you can run `vagrant suspend` to simply\nsuspend the virtual machine. In either case%!(VAGRANT_COMMA) to restart it again%!(VAGRANT_COMMA)\nsimply run `vagrant up`.
1540128279,vagrant3,provider-name,virtualbox
1540128279,vagrant3,state,running
1540128279,vagrant3,state-human-short,running
1540128279,vagrant3,state-human-long,The VM is running. To stop this VM%!(VAGRANT_COMMA) you can run `vagrant halt` to\nshut it down forcefully%!(VAGRANT_COMMA) or you can run `vagrant suspend` to simply\nsuspend the virtual machine. In either case%!(VAGRANT_COMMA) to restart it again%!(VAGRANT_COMMA)\nsimply run `vagrant up`.
1540128279,,ui,info,Current machine states:\n\nvagrant1                  running (virtualbox)\nvagrant2                  running (virtualbox)\nvagrant3                  running (virtualbox)\n\nThis environment represents multiple VMs. The VMs are all listed\nabove with their current state. For more information about a specific\nVM%!(VAGRANT_COMMA) run `vagrant status NAME`.
```

為了拿到某台機器的詳細訊息：

```shell
vagrant ssh-config vagrant2
```

```shell
Host vagrant2
  HostName 127.0.0.1
  User vagrant
  Port 2200
  UserKnownHostsFile /dev/null
  StrictHostKeyChecking no
  PasswordAuthentication no
  IdentityFile /Users/akiicat/.vagrant.d/insecure_private_key
  IdentitiesOnly yes
  LogLevel FATAL
```

dynamic inventory file 會呼叫這些指令，解析裡面的內容，轉換成 JSON 格式

### 已經存在的 Inventory Script

可以到 [Ansible GitHub repo](https://github.com/ansible/ansible/tree/devel/contrib/inventory) 裡面的 *contrib/inventory* 目錄下找現有的腳本。

## 把 Inventory 拆成很多的檔案

如果你同時有正常的 inventory file 跟 dynamic inventory 腳本，把他們放在相同的資料夾裡面，然後透過 *ansible.cfg* 裡的 `inventory` 參數，或是在打指令的時候用 `-i` flag。Ansible 會把所有的檔案合併成一個 inventory。

```ini
# ansible.cfg 
[defaults]
inventory = inventory
```



### 在運行中使用 add_host 跟 group_by 增加 Entries

Ansible 允許你在執行 playbook 的時候增加 hosts 和 groups。

#### add_host

如果你用在 IAAS 上的虛擬機，這個功能實用。

即使你使用 dynamic inventory script，這個功能依然很實用，在你新增了一台機器的時候然後要設定它。

如果新的 host 在 playbook 運行的時候上線了，這個腳本並不會執行這台 host，因為 dynamic inventory script 會在 playbook 一開始的時候執行，所以當 playbook 在運行的時候，Ansible 會找不到新的 hosts。

呼叫 module 長得像這樣：

```ini
add_host name=hostname groups=web,staging myvar=myval
```

這是 `add_host` 指令的腳本，可以設定新開的機器：

```yaml
 - name: Provision a vagrant machine
      hosts: localhost
      vars:
        box: trusty64
      tasks:
        - name: create a Vagrantfile
          command: vagrant init {{ box }} creates=Vagrantfile
          
        - name: Bring up a vagrant machine
          command: vagrant up
          
        - name: add the vagrant machine to the inventory
          add_host: >
                name=vagrant
                ansible_host=127.0.0.1
                ansible_port=2222
                ansible_user=vagrant
                ansible_private_key_file=/Users/lorin/.vagrant.d/
                insecure_private_key
                
    - name: Do something to the vagrant machine
      hosts: vagrant
      become: yes
      tasks:
        # The list of tasks would go here
        -  ...
```

這個腳本是跑在本地端的腳本，另外我們在 task 裡面使用 `creates=Vagrantfile` 參數：

```yaml
- name: create a Vagrantfile
  command: vagrant init {{ box }} creates=Vagrantfile
```

這個會告訴 Ansible 說，如果 *Vagrantfile* 這個檔案存在的話，就不要再執行這個指令，確保這個指令只會執行一次 (idempotence)。

#### group_by

Ansible 也可以在運行的時候執行 playbook，使用 `group_by` 這個 module。

如果 group 會有不同的機器，像是 `asnible_machine` 參數有 `i386` (32-bit x86) 和 `x84_64` (64-bit x84)，可以讓群組的機器，在同一個指令裡面分辨使用不同的參數。

如果是使用不同的 Linux 系統，像是 Ubuntu, CentOS，可以使用 `ansible_distribution`：

```yaml
- name: create groups based on distro
  group_by: key={{ ansible_distribution }}
```

我們可以使用 `group_by` 建立 Ubuntu 的 hosts 跟 CentOS 的 hosts，然後分別使用 `apt` module 跟 `yum` module 安裝套件。

```yaml
- name: group hosts by distribution
  hosts: myhosts
  gather_facts: True
  tasks:
    - name: create groups based on distro
      group_by: key={{ ansible_distribution }}
- name: do something to Ubuntu hosts
  hosts: Ubuntu
  tasks:
    - name: install htop
      apt: name=htop
    # ...
- name: do something else to CentOS hosts
  hosts: CentOS
  tasks:
    - name: install htop
      yum: name=htop
    # ...
```

