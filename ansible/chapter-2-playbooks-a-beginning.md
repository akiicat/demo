# Playbooks: A Beginning

大部分在使用 Ansible 的時候都是在寫 playbooks，playbooks 是 Ansible 用來寫設定管理腳本的術語。

這章會安裝 Nginx web server 且設定安全的連線

會使用到這些檔案：[GitHub 連結](https://github.com/ansiblebook/ansiblebook/tree/master/ch02/playbooks)

- playbooks/ansible.cfg
- playbooks/hosts
- playbooks/Vagrantfile
- playbooks/web-notls.yml
- playbooks/web-tls.yml 
- playbooks/files/nginx.key
- playbooks/files/nginx.crt
- playbooks/files/nginx.conf
- playbooks/templates/index.html.j2 • playbooks/templates/nginx.conf.j2 

## 預先設定

我們要把 Vagrant 虛擬機的 port 80 與 433 跟本地的 port 8080 與 8443 做結合，所以更改 *Vagrantfile* 如下：

```ruby
# Vagrantfile
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
    config.vm.box = "ubuntu/trusty64"
	config.vm.network "forwarded_port", guest: 80, host: 8080
    config.vm.network "forwarded_port", guest: 443, host: 8443
end
```

然後重新載入 Vagrant 的虛擬機

```shell
vagrant reload
```

會看到 port forward：

```shell
==> default: Forwarding ports...
    default: 80 (guest) => 8080 (host) (adapter 1)
    default: 443 (guest) => 8443 (host) (adapter 1)
    default: 22 (guest) => 2222 (host) (adapter 1)
```

## 非常簡單的 Playbook

會先設定一個非常簡單的 Nginx web server。之後會加上安全設定 Transport Layer Security (TLS)。

SSL 是比較舊的 protocol，用在瀏覽器跟 web server 之間的安全性，但現在已經被 TLS protocol 取代了，儘管還是有很多人使用 SSL 這個詞。

```yaml
# web-notls.yml
---
- name: Configure webserver with nginx
  hosts: webservers
  become: True
  tasks:
    - name: install nginx
      apt: name=nginx update_cache=yes
      
    - name: copy nginx config file
      copy: src=files/nginx.conf dest=/etc/nginx/sites-available/default
      
    - name: enable configuration
      file: >
        dest=/etc/nginx/sites-enabled/default
        src=/etc/nginx/sites-available/default
        state=link
        
    - name: copy index.html
      template: src=templates/index.html.j2 dest=/usr/share/nginx/html/index.html mode=0644
      
    - name: restart nginx
      service: name=nginx state=restarted
```

眼睛比較銳利的人可能會覺得會什麼在前面是使用 `True` 後面會使用 `yes`。

YAML parser 會把以下的是為相同的：

- YAML truthy：`true`, `True`, `TRUE`, `yes`, `Yes`, `YES`, `on`, `On`, `ON`, `y`, `Y`
- YAML falsey：`false`, `False`, `FALSE`, `no`, `No`, `NO`, `off`, `Off`, `OFF`, `n`, `N`

在 Ansible 的 Module 傳入的參數，會把它視為相同：

- module arg truthy：`yes`, `on`, `1`, `true` 
- module arg falsey：`no`, `off`, `0`, `false` 

Ansible 的文件是使用 `yes` 跟 `no`。

### 指定 Nginx 的設定檔

要讓 web server 可以運作之前，需要設定兩個檔案。

Nginx 的設定檔

```conf
# files/nginx.conf
server {
        listen 80 default_server;
        listen [::]:80 default_server ipv6only=on;

        root /usr/share/nginx/html;
        index index.html index.htm;

        server_name localhost;

        location / {
                try_files $uri $uri/ =404;
        }
}
```

### 建立 Home Page

```html
<!-- templates/index.html.j2 -->
<html>
  <head>
    <title>Welcome to ansible</title>
  </head>
  <body>
  <h1>nginx, configured by Ansible</h1>
  <p>If you can see this, Ansible successfully installed nginx.</p>

  <p>Running on {{ inventory_hostname }}</p>
  </body>
</html>
```

上面的 `inventory_hostname` 裡面會替換成 Ansible 的參數。

### 建立 Webservers 的群組

可以將 inventory file 群組起來。

Inventory file 如果是 *.ini* 格式的話，在前面加上 `[webservers]` 就能把 `testserver` 群組起來。

```ini
# hosts 
[webservers]
testserver ansible_host=127.0.0.1 ansible_port=2222
```

然後測試一下：

```shell
$ ansible webservers -m ping
testserver | SUCCESS => {
    "changed": false, 
    "ping": "pong"
}
```

### 運行 Playbook

```shell
ansible-playbook web-notls.yml
```

輸出結果

```shell
$ ansible-playbook web-notls.yml
PLAY [Configure webserver with nginx] *******************************************************

TASK [Gathering Facts] **********************************************************************
ok: [testserver]

TASK [install nginx] ************************************************************************
ok: [testserver]

TASK [copy nginx config file] ***************************************************************
changed: [testserver]

TASK [enable configuration] *****************************************************************
ok: [testserver]

TASK [copy index.html] **********************************************************************
changed: [testserver]

TASK [restart nginx] ************************************************************************
changed: [testserver]

PLAY RECAP **********************************************************************************
testserver                 : ok=6    changed=3    unreachable=0    failed=0   
```

如果沒有錯誤的話，在 *http://localhost:8080/* 上可以看到 nginx 的頁面，而且 `inventory_hostname` 會被替換成 `testserver`。

如果 playbook 是可以執行的檔案的話，在開頭加上：

```shell
#!/usr/bin/env ansible-playbook
```

然後就能直接執行：

```shell
./web-notls.yml
```

## Playbook 就是 YAML

在寫 Playbook 之前，先快速介紹 YAML 的用法：

### 檔案的開始

在 YAML 裡面，三個 `-` 可以代表文件的開始。

```yaml
---
```

不過你忘記寫這行的話，Ansible 也不會出錯。

### 註解

```yaml
# This is a YAML comment
```

### 字串

在 YAML 裡面你不需要用雙引號包著，就能有字串的效果，即使有空格：

```yaml
this is a sentence
```

等同於 JSON 的：

```json
"this is a sentence"
```

在 YAML 可以使用 `{{ braces }}` 替換參數。

### Booleans

YAML：

```yaml
True
```

JSON：

```json
true
```

Lists

array，在 YAML 叫做 *sequences*，但是在 Ansible 文件裡面叫作 *lists*

```yaml
- This is an apple
- This is an orange
- This is a banana
```

等同於 JSON 的：

```json
[
  "This is an apple",
  "This is an orange",
  "This is a banana"
]
```

YAML 支援一行的寫法：

```yaml
[This is an apple, This is an orange, This is a banana]
```

### Dictionaries

像是 JSON 的 *objects*、Python 的 *dictionaries*、Ruby 的 *hashes*，在 YAML 叫做 mappings，在 Ansible 文件裡面叫作 *dictionaries*：

```yaml
address: 830 Evergreen 
city: Taiwan
```

等同 JSON 的格式：

```json
{
    address: "830 Evergreen",
	city: "Taiwan"
}
```

YAML 支援一行的寫法：

```yaml
{address: 830 Evergreen, city: Taiwan}
```

### Line Folding

使用大於的符號，後面的字串會把換行字元取代成空白鍵：

```yaml
address: >
    Department of Computer Science,
    A.V. Williams Building,
    University of Maryland
city: College Park
```

JSON 的格式：

```json
{
	"address": "Department of Computer Science, A.V. Williams Building, University of Maryland",
    "city": "College Park"
}
```

### Plays

一個 play 的格式：

```yaml
- name: Configure webserver with nginx
  hosts: webservers
  become: True
  tasks:
    - name: install nginx
      apt: name=nginx update_cache=yes
      
    - name: copy nginx config file
      copy: src=files/nginx.conf dest=/etc/nginx/sites-available/default
      
    - name: enable configuration
      file: >
        dest=/etc/nginx/sites-enabled/default
        src=/etc/nginx/sites-available/default
        state=link
        
    - name: copy index.html
      template: src=templates/index.html.j2
               dest=/usr/share/nginx/html/index.html mode=0644
               
    - name: restart nginx
      service: name=nginx state=restarted
```

一個 play 一定會包含

- *hosts*
- *tasks*

可以想成連線到 hosts 需要做的 tasks。

其他選擇性的參數：

- `name`：Ansible 會印出這個 play 的名字。
- `become`：如果是 `True` 會預設使用 root 的使用者，在不能以 root 連線 SSH 的到 server 的時候很好用。
- `vars`：設定變數跟值。

### Tasks

我們的範例是一個 play 裡面有五個 tasks

```yaml
- name: install nginx
  apt: name=nginx update_cache=yes
```

name 是選擇性的，所以寫成這樣也是合法的：

```yaml
- apt: name=nginx update_cache=yes
```

不過建議寫 `name` 這樣之後才看得懂。

這個是 `apt` module 的參數 ` name=nginx update_cache=yes`，要安裝的套件名稱是 nginx，`update_cache=yes` 會在安裝之前執行 `apt-get update`。

因為參數是 string，而不是 object 型態，所以可以改成 line folding 的寫法：

```yaml
- name: install nginx
     apt: >
         name=nginx
         update_cache=yes
```

Ansible 也支援舊格式 `action` 的寫法：

```yaml
- name: install nginx
  action: apt name=nginx update_cache=yes
```

### Modules

是一個包裝好的腳本，這是常會使用的 modules：

- `apt`：安裝或移出 apt 的套件。
- `copy`：複製檔案。
- `file`：設定檔案。
- `service`：建立、停止或重新啟動 service。
- `template`：從 template 產生一個檔案然後複製到 hosts 裡面。

可以用 `ansible-doc` 查看詳細的說明：

```yaml
ansible-doc service
```

