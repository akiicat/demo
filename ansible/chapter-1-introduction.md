# Introduction

## Ansible 有什麼好處？

- *Capistrano* 和 *Fabric* 這兩個都是開源的 development tool
- Ansible 同時是 development tool 也有 configuration management

## Ansible 怎麼運作

在 ansible playback 的設定檔裡面有分兩個部分，描述設定了哪些 hosts，跟在這些 hosts 上執行一連串的 tasks。

像是安裝 nginx：

```
- name: Install nginx
  apt: name=nginx
```

Ansible 會執行下列的步驟：

1. 產生安裝 Nginx 套件的 Python 腳本
2. 複製腳本到 web1, web2, web3
3. 在 web1, web2, web3 執行腳本
4. 等待所有 hosts 執行完腳本

Ansible 會等所有 hosts 上的 task 執行完之後，才會執行下一個 task，一樣是運行這四個步驟。注意以下幾點：

- Ansible 在所有 hosts 上同時運行每個 task
- Ansible 等待所有 hosts 完成 task，然後再執行下一個 task
- Ansible 按照順序執行 task

## Ansible 的好處

- 容易閱讀：YAML 格式
- 不必在遠端 server 上安裝東西：server 只需要 SSH 跟 Python 2.5 版以上。或是 Python 2.4 且安裝 *simplejson* 的 library 也行，不需要在 hosts 上預先安裝 agent。
- Push Based
- Scale Down：管理上千台 node 也能夠 scale down。
- 內建 modules
- 提供非常輕薄的抽象層

### Push Based

有些管理系統預設是 *pull based*，會使用 agent 像是 Chef 和 Puttet，安裝在 server 的 agent 會定期的檢查 central service，然後在拉下訊息，改變設定檔的行為會像是這樣：

1. 你：更改配置管理腳本。
2. 你：把腳本推到配置管理的 central service。
3. server 上的 agent：週期性的觸發醒來。
4. server 上的 agent：連接到配置管理的 central service。
5. server 上的 agent：下載新的配置管理腳本。
6. server 上的 agent：在本地執行配置管理腳本，然後改變 server 的狀態。

相反的，Ansible 是 *push based*，行為會像這樣：

1. 你：改變腳本。
2. 你：運行新的腳本。
3. Ansible：連接到 server 並執行 modules，modules 會改變 server 的狀態。

只要使用 `ansible-playbook` 就會執行上面的步驟。

Ansible 也有 pull-based 的指令叫做 *ansible-pull*。

### 內建 modules

使用 modules 可以執行任務，像是安裝套件，重啟 service，複製設定檔。Ansible 的 modeuls 是 *declarative* 宣告模式，像是 module 想要帳戶名稱是 `deploy` 然後在 group 群組是 `web`：

```yaml
user: name=deploy group=web
```

Module 是 *idempotent*，如果 `deploy` 使用者不存在，Ansible 就會建立，如果存在，Ansible 就不會做任何事。

Idempotent 是一個很棒的屬性，重複運行 playbook 很多次的話，也是會有一樣的結果，這對於自己寫的腳本運行第二次的時候會有不同的影響是很大的改善。

Ansibel 會使用 YAML 檔案的格式與 Jinja2 樣板語言，不過兩個都不會太困難。

## 安裝 Ansible

mac 建議使用 brew 安裝

LInux 系統建議使用 pip 安裝

```shell
sudo pip install ansible
```

如果不想要安裝在 root 的話，可以使用 Python 的 *virtualenv* 安裝，如果不熟悉 *virtualenv* 的話，可以使用 *pipsi* 自動安裝：

```shell
wget https://raw.githubusercontent.com/mitsuhiko/pipsi/master/get-pipsi.py
python get-pipsi.py
pipsi install ansible
```

pipsi 安裝完後，要在 PATH 環境變數裡面加上 *~/.local/bin*，如果已經安裝 pipsi 然後想要在安裝其他的套件想是 *docker-py* 或 *boto*：

```shell
cd ~/.local/venvs/ansible
source bin/activate
pip install docker-py boto
```

如果你想要使用最新的可以從 GitHub：

```shell
git clone https://github.com/ansible/ansible.git --recursive
```

如果你跑在 development 的分支，需要設定環境變數：

```shell
cd ./ansible
source ./hacking/env-setup
```

- [Official Ansible install docs](http://docs.ansible.com/intro_installation.html)
- [Pip](http://pip.readthedocs.org/)
- [Virtualenv](https://docs.python-guide.org/dev/virtualenvs/)
- [Pipsi](https://github.com/mitsuhiko/pipsi)

## 建立測試的 Server

你可以使用 GCP、AWS、Linode，只要你能夠 ssh 和有 root 的權限就行。

### 使用 Vagrant 建立 Server

如果偏好不花錢的方式的話，可以安裝 Vagrant，Vagrant 是一個開源的工具來管理虛擬機。

需要安裝 *VirtualBox* 和 *Vagrant*，然後執行：

```shell
mkdir playbooks
cd playbooks
vagrant init ubuntu/trusty64
vagrant up
```

如果建立好了會長這樣：

```shell
$ vagrant up
Bringing machine 'default' up with 'virtualbox' provider...
==> default: Box 'ubuntu/trusty64' could not be found. Attempting to find and install...
    default: Box Provider: virtualbox
    default: Box Version: >= 0
==> default: Loading metadata for box 'ubuntu/trusty64'
    default: URL: https://vagrantcloud.com/ubuntu/trusty64
==> default: Adding box 'ubuntu/trusty64' (v20181002.0.0) for provider: virtualbox
    default: Downloading: https://vagrantcloud.com/ubuntu/boxes/trusty64/versions/20181002.0.0/providers/virtualbox.box
    default: Download redirected to host: cloud-images.ubuntu.com
==> default: Successfully added box 'ubuntu/trusty64' (v20181002.0.0) for 'virtualbox'!
==> default: Importing base box 'ubuntu/trusty64'...
==> default: Matching MAC address for NAT networking...
==> default: Checking if box 'ubuntu/trusty64' is up to date...
==> default: Setting the name of the VM: playbooks_default_1539196755143_33656
==> default: Clearing any previously set forwarded ports...
Vagrant is currently configured to create VirtualBox synced folders with
the `SharedFoldersEnableSymlinksCreate` option enabled. If the Vagrant
guest is not trusted, you may want to disable this option. For more
information on this option, please refer to the VirtualBox manual:

  https://www.virtualbox.org/manual/ch04.html#sharedfolders

This option can be disabled globally with an environment variable:

  VAGRANT_DISABLE_VBOXSYMLINKCREATE=1

or on a per folder basis within the Vagrantfile:

  config.vm.synced_folder '/host/path', '/guest/path', SharedFoldersEnableSymlinksCreate: false
==> default: Clearing any previously set network interfaces...
==> default: Preparing network interfaces based on configuration...
    default: Adapter 1: nat
==> default: Forwarding ports...
    default: 22 (guest) => 2222 (host) (adapter 1)
==> default: Booting VM...
==> default: Waiting for machine to boot. This may take a few minutes...
    default: SSH address: 127.0.0.1:2222
    default: SSH username: vagrant
    default: SSH auth method: private key
    default: 
    default: Vagrant insecure key detected. Vagrant will automatically replace
    default: this with a newly generated keypair for better security.
    default: 
    default: Inserting generated public key within guest...
    default: Removing insecure key from the guest if it's present...
    default: Key inserted! Disconnecting and reconnecting using new SSH key...
==> default: Machine booted and ready!
==> default: Checking for guest additions in VM...
    default: The guest additions on this VM do not match the installed version of
    default: VirtualBox! In most cases this is fine, but in rare cases it can
    default: prevent things such as shared folders from working properly. If you see
    default: shared folder errors, please make sure the guest additions within the
    default: virtual machine match the version of VirtualBox you have installed on
    default: your host and reload your VM.
    default: 
    default: Guest Additions Version: 4.3.36
    default: VirtualBox Version: 5.2
==> default: Mounting shared folders...
    default: /vagrant => /Users/akiicat/Github/playbooks
```

然後可以 SSH 進去虛擬機裡面：

```shell
vagrant ssh
```

但是 Ansible 需要使用正常的 SSH 方法連進去，而不是使用 `vagrant ssh`

知道虛擬機的詳細資訊：

```shell
vagrant ssh-config
```

會長的像這樣

```shell
Host default
  HostName 127.0.0.1
  User vagrant
  Port 2222
  UserKnownHostsFile /dev/null
  StrictHostKeyChecking no
  PasswordAuthentication no
  IdentityFile /Users/akiicat/Github/playbooks/.vagrant/machines/default/virtualbox/private_key
  IdentitiesOnly yes
  LogLevel FATAL
```

比較重要的是這幾行

```shell
HostName 127.0.0.1
User vagrant
Port 2222
IdentityFile /Users/akiicat/Github/playbooks/.vagrant/machines/default/virtualbox/private_key
```

在這個範例中，應該就只有 `IdentityFile` 會是不一樣的，其他應該都是一樣的，然後使用 SSH 連線進去：

```shell
ssh vagrant@127.0.0.1 -p 2222 -i /Users/akiicat/Github/playbooks/.vagrant/machines/default/virtualbox/private_key
```

### 設定 Ansible 的 Server

Ansible 只能管理確切知道的 server，設定在 inventory file 裡面。每個 server 都需要名字，也可以給他 alias。

如果使用 INI 格式的話，要在 hosts 檔名後面加上 `.ini` 的副檔名，alias 的名稱是 `testserver`，且一個 server 以一行為單位。

```ini
# hosts.ini
testserver ansible_host=127.0.0.1 ansible_port=2222 ansible_user=vagrant ansible_private_key_file=.vagrant/machines/default/virtualbox/private_key
```

這邊有一個缺點，你會看到我們需要在為每一個 host 加上一堆參數，之後能夠改善。

如果是 Amazon EC2 的 host 可以寫成這樣：

```ini
# hosts.ini
 testserver ansible_host=ec2-203-0-113-120.compute-1.amazonaws.com ansible_user=ubuntu ansible_private_key_file=/path/to/keyfile.pem
```

這時候可以使用 `ansible` 的指令連到 `testserver`，傳入 `hosts.ini` 檔案跟呼叫 `ping` 的 module：

```shell
ansible testserver -i hosts.ini -m ping
```

成功畫面會像這樣：

```shell
$ ansible testserver -i hosts.ini -m ping
testserver | SUCCESS => {
    "changed": false, 
    "ping": "pong"
}
```

詳細 log 檔的話，在後面加上 `-vvvv`：

```shell
ansible testserver -i hosts.ini -m ping -vvvv
```

`"changed": false` 這行的意思是，執行這個 module 不會改變 server 的任何狀態。`"ping": "pong"` 是 ping 這個 module 所顯示的輸出。

Ping module 很常用來測試 server 的 ssh 有沒有正常運作。

