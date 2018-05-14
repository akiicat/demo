# Docker

## Install

```shell
brew cask install docker
```



## Image

### pull

```shell
docker pull ubuntu:16.04
```

完成後，即可隨時使用該映像檔了，例如建立一個容器，讓其中執行 bash。

```shell
docker run -t -i ubuntu:16.04 /bin/bash
```



### show

顯示本機已有的映像檔

```shell
docker images
```

映像檔的 `ID` 唯一標識了映像檔

如果沒有指定 `TAG`，預設使用  `latest`



### Create

#### 從現有的映像檔做修改

目前 id 是 `a02834e1d1ae`

```
docker run -t -i training/sinatra /bin/bash
# root@a02834e1d1ae:/#
```



在容器中加入 json 的 gem 套件

```shell
# root@a02834e1d1ae:/# gem install json
gem install json
```



退出容器

```shell
# root@a02834e1d1ae:/# exit
exit
```



現在我們的容器已經被改變了，使用 `docker commit` 命令來提交更新後的副本。

```shell
docker commit -m "Added json gem" -a "Docker Newbee" a02834e1d1ae ouruser/sinatra:v2
# sha256:5378a931eaad3def63ca0bef8daca63e548f9144424ca2590f5d46cc9359f229
```

- `-m` 指定提交的說明信息，跟我們使用的版本控制工具一樣
- `-a` 可以指定更新的使用者信息
- 下一個參數是：用來建立映像檔的容器的 ID
- 最後指定新映像檔的名稱和 tag

建立成功後會印出新映像檔的 ID



使用 `docker images` 查看新建立的映像檔

```shell
docker images
# REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
# ouruser/sinatra     v2                  5378a931eaad        3 minutes ago       453MB
# training/sinatra    latest              49d952a36c58        3 years ago         447MB
```



之後，可以使用新的映像檔來啟動容器

```shell
docker run -t -i ouruser/sinatra:v2 /bin/bash
# root@bfc9f5828b4d:/#
```



#### 利用 Dockerfile 建立映像檔

```shell
mkdir sinatra
cd sinatra
touch Dockerfile
```



Dockerfile 中每一條指令都會建立一層映像檔

```shell
# Dockerfile
FROM ubuntu:14.04
MAINTAINER Docker Newbee <newbee@docker.com>
RUN apt-get -qq update
RUN apt-get -qqy install ruby ruby-dev
RUN gem install sinatra

# Dockerfile
FROM ruby:2.4.3
RUN gem install sinatra
```

Dockerfile 基本的語法是

- 使用`#`來註釋
- `FROM` 指令告訴 Docker 使用哪個映像檔作為基底
- `MAINTAINER` 接著是維護者的信息
- `RUN`開頭的指令會在建立中執行，比如安裝一個套件，在這裏使用 apt-get 來安裝了一些套件



建立映像檔

```shell
docker build -t="ouruser/sinatra:v2" .
```

- `-t` 標記添加 tag，指定新的映像檔的使用者信息
-  “.” 是 Dockerfile 所在的路徑（當前目錄），也可以換成具體的 Dockerfile 的路徑



另外

```
# put my local web site in myApp folder to /var/www
ADD myApp /var/www
# expose httpd port
EXPOSE 80
# the command to run
CMD ["/usr/sbin/apachectl", "-D", "FOREGROUND"]
```

- `ADD` 命令複製本地檔案到映像檔
- `EXPOSE` 命令向外部開放埠號
- `CMD` 命令描述容器啟動後執行的程序，`docker run` 如果沒有給參數就會預設執行此命令



利用新建立的映像檔啟動一個容器

```shell
docker run -t -i ouruser/sinatra:v2 /bin/bash
```



修改映像檔的標籤

```
sudo docker tag 5db5f8471261 ouruser/sinatra:devel
sudo docker images ouruser/sinatra
REPOSITORY          TAG     IMAGE ID      CREATED        VIRTUAL SIZE
ouruser/sinatra     devel   5db5f8471261  11 hours ago   446.7 MB
ouruser/sinatra     v2      5db5f8471261  11 hours ago   446.7 MB
```



#### 從本機匯入

要從本機匯入一個映像檔，可以使用 OpenVZ（容器虛擬化的先鋒技術）的模板來建立： OpenVZ 的模板下載位址為 [http://openvz.org/Download/templates/precreated。](http://openvz.org/Download/templates/precreated%E3%80%82)

比如，先下載一個 ubuntu-14.04 的映像檔，之後使用以下命令匯入：

```
cat ubuntu-14.04-x86_64-minimal.tar.gz  |docker import - ubuntu:14.04
```

然後查看新匯入的映像檔。

```shell
$ docker images
REPOSITORY          TAG                 IMAGE ID            CREATED             VIRTUAL SIZE
ubuntu              14.04               05ac7c0b9383        17 seconds ago      215.5 MB
```



#### 上傳映像檔

```shell
docker push ouruser/sinatra
```



### 匯入匯出

#### 儲存映像檔

如果要建立映像檔到本地檔案，可以使用 `docker save` 命令。

```
$ docker images
REPOSITORY          TAG                 IMAGE ID            CREATED             VIRTUAL SIZE
ubuntu              14.04               c4ff7513909d        5 weeks ago         225.4 MB
...
$ docker save -o ubuntu_14.04.tar ubuntu:14.04
```



#### 載入映像檔

可以使用 `docker load` 從建立的本地檔案中再匯入到本地映像檔庫，例如

```shell
docker load --input ubuntu_14.04.tar
docker load < ubuntu_14.04.tar
```

這將匯入映像檔以及其相關的元資料信息（包括標籤等）。



### 移除本地端映像檔

如果要移除本地端的映像檔，可以使用 `docker rmi` 命令。注意 `docker rm` 命令是移除容器。

```
$ sudo docker rmi training/sinatra
Untagged: training/sinatra:latest
Deleted: 5bc342fa0b91cabf65246837015197eecfa24b2213ed6a51a8974ae250fedd8d
Deleted: ed0fffdcdae5eb2c3a55549857a8be7fc8bc4241fb19ad714364cbfd7a56b22f
Deleted: 5c58979d73ae448df5af1d8142436d81116187a7633082650549c52c3a2418f0
```

注意：在刪除映像檔之前要先用 `docker rm` 刪掉依賴於這個映像檔的所有容器。



## 容器

### 新建並啟動

所需要的命令主要為 `docker run`

```shell
# 命令輸出一個 “Hello World”，之後終止容器
docker run ubuntu:14.04 /bin/echo 'Hello world'
# 啟動一個 bash 終端，允許使用者進行互動
docker run -t -i ubuntu:14.04 /bin/bash
```

- `-t` 選項讓Docker分配一個虛擬終端（pseudo-tty）並綁定到容器的標準輸入上
- `-i` 則讓容器的標準輸入保持打開。



### 啟動已終止容器

可以利用 `docker start` 命令，直接將一個已經終止的容器啟動執行。給他 NAMES 參數

```shell
$ docker ps
CONTAINER ID        IMAGE               COMMAND             CREATED              STATUS              PORTS               NAMES
086c4f12b169        ubuntu:16.04        "/bin/bash"         About a minute ago   Up About a minute                       affectionate_hopper
$ docker start -i affectionate_hopper
```



### 守護態執行

更多的時候，需要讓 Docker 容器在後臺以守護態（Daemonized）形式執行。此時，可以透過新增 `-d` 參數來實作。

```shell
docker run -d ubuntu:14.04 /bin/sh -c "while true; do echo hello world; sleep 1; done"
```



透過 `docker ps` 命令來查看容器訊息

```shell
$ docker ps
CONTAINER ID        IMAGE               COMMAND                  CREATED             STATUS              PORTS               NAMES
35fa6a9b642a        ubuntu:14.04        "/bin/sh -c 'while t…"   41 seconds ago      Up 49 seconds                           friendly_kare
```



要取得容器的輸出訊息，可以透過 `docker logs` 命令

```shell
docker logs friendly_kare
```



### 終止容器

可以使用 `docker stop`

```shell
docker stop friendly_kare
docker container stop friendly_kare # 可以存目前狀態？
```



透過 `docker ps -a` 可以看到完整的訊息

```shell
docker ps -a
```



### 重新啟動容器

終止狀態的容器，可以透過 `docker start` 命令來重新啟動

```shell

docker start friendly_kare
docker restart friendly_kare
```



### 進入容器

在使用 `-d` 參數時，容器啟動後會進入背景執行。 某些時候需要進入容器進行操作，有很多種方法，包括使用 `docker attach` 命令或 `nsenter` 工具等

#### exec

`docker exec` 是Docker內建的命令

```shell
$ docker run -idt ubuntu
243c32535da7d142fb0e6df616a3c3ada0b8ab417937c853a9e1c251f499f550
$ docker ps
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES
243c32535da7        ubuntu:latest       "/bin/bash"         18 seconds ago      Up 17 seconds                           nostalgic_hypatia
$ docker exec -ti nostalgic_hypatia bash
root@243c32535da7:/#
```



#### attach 命令

`docker attach` 亦是Docker內建的命令。下面示例如何使用該命令。

```shell
$ docker run -idt ubuntu
243c32535da7d142fb0e6df616a3c3ada0b8ab417937c853a9e1c251f499f550
$ docker ps
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES
243c32535da7        ubuntu:latest       "/bin/bash"         18 seconds ago      Up 17 seconds                           nostalgic_hypatia
$ docker attach nostalgic_hypatia
root@243c32535da7:/#
```

按下 `ctrl` + `P` 然後 `ctrl` + `Q` 跳離容器，讓它繼續在背景執行。

但是使用 `attach` 命令有時候並不方便。當多個窗口同時 attach 到同一個容器的時候，所有窗口都會同步顯示。當某個窗口因命令阻塞時,其他窗口也無法執行操作了。



#### nsenter 命令

https://philipzheng.gitbooks.io/docker_practice/content/container/enter.html



### 匯出容器

如果要匯出本地某個容器，可以使用 `docker export` 命令。

```shell
docker export 7691a814370e > ubuntu.tar
```



### 匯入容器快照

可以使用 `docker import` 從容器快照檔案中再匯入為映像檔

```shell
cat ubuntu.tar | docker import - test/ubuntu:v1.0
```



指定 URL 或者某個目錄來匯入

```shell
docker import http://example.com/exampleimage.tgz example/imagerepo
```

*註：使用者既可以使用 `docker load` 來匯入映像檔儲存檔案到本地映像檔庫，也可以使用 `docker import` 來匯入一個容器快照到本地映像檔庫。這兩者的區別在於容器快照檔案將丟棄所有的歷史記錄和原始資料訊息（即僅保存容器當時的快照狀態），而映像檔儲存檔案將保存完整記錄，檔案體積也跟著變大。此外，從容器快照檔案匯入時可以重新指定標籤等原始資料訊息。



### 刪除容器

使用 `docker rm` 來刪除一個處於終止狀態的容器

```shell
docker rm trusting_newton
docker ps -aq --no-trunc | xargs docker rm # 刪除所有已停止的容器
```

如果要刪除一個執行中的容器，可以新增 `-f` 參數。Docker 會發送 `SIGKILL` 信號給容器。

 `docker run` 的時候如果新增 `--rm` 標記，則容器在終止後會立刻刪除。注意，`--rm` 和 `-d` 參數不能同時使用



## Docker Hub

### 登錄

```shell
docker login
```



### 基本操作

使用者無需登錄即可透過 `docker search`命令來查詢官方倉庫中的映像檔

```shell
docker search centos
```



### 自動建立

Automated Builds

https://philipzheng.gitbooks.io/docker_practice/content/repository/dockerhub.html



## 私有倉庫

### 安裝執行 docker-registry

```shell
docker run -d -p 5000:5000 registry
```



### 上傳

建立好私有倉庫之後，就可以使用 `docker tag` 來標記一個映像檔，然後推送它到倉庫，別的機器上就可以下載下來了。例如私有倉庫位址為 `localhost:5000`

格式為 `docker tag IMAGE[:TAG] [REGISTRYHOST/][USERNAME/]NAME[:TAG]`

```shell
docker tag af3afc48ecb5 localhost:5000/test
```

使用 `docker push` 上傳標記的映像檔

```shell
docker push localhost:5000/test
```



### 下載

```shell
docker pull localhost:5000/test
```



### 搜尋

localhost:5000/v2/_catalog



### 存放在 GCP

docker 使用者認證

https://cloud.google.com/container-registry/docs/advanced-authentication

docker pull push

https://cloud.google.com/container-registry/docs/pushing-and-pulling?hl=zh_TW&_ga=2.136122993.-1930404928.1517137001

gcloud container 教學

https://ahmet.im/blog/google-container-registry-tips/



## 資料卷 Data volumes

在用 `docker run` 命令的時候，使用 `-v` 標記來建立一個資料卷並掛載到容器裡。在一次 run 中多次使用可以掛載多個資料卷

下面建立一個 web 容器，並載入一個資料卷到容器的 `/webapp` 目錄

```shell
docker run -d -P --name web -v /webapp training/webapp python app.py
```

注意：也可以在 Dockerfile 中使用 `VOLUME` 來新增一個或者多個新的卷到由該映像檔建立的任意容器。



使用 `-v` 標記也可以指定掛載一個本地主機的目錄到容器中去

```shell
docker run -d -P --name web -v /src/webapp:/opt/webapp training/webapp python app.py
```

上面的命令載入主機的 `/src/webapp` 目錄到容器的 `/opt/webapp` 目錄

本地目錄的路徑必須是絕對路徑，如果目錄不存在 Docker 會自動為你建立它



`-v` 標記也可以從主機掛載單個檔案到容器中

```shell
docker run --rm -it -v ~/.bash_history:/.bash_history ubuntu /bin/bash
```

注意：如果直接掛載一個檔案，很多檔案編輯工具，包括 `vi` 或者 `sed --in-place`，可能會造成檔案 inode 的改變，從 Docker 1.1 .0起，這會導致報錯誤訊息。所以最簡單的辦法就直接掛載檔案的父目錄。



## 資料卷容器 Data volume containers

如果你有一些持續更新的資料需要在容器之間共享，最好建立資料卷容器。

資料卷容器，其實就是一個正常的容器，專門用來提供資料卷供其它容器掛載的。

首先，建立一個命名的資料卷容器 dbdata：

```shel
docker run -d -v /dbdata --name dbdata training/postgres echo Data-only container for postgres
```

然後，在其他容器中使用 `--volumes-from` 來掛載 dbdata 容器中的資料卷。

```shel
docker run -d --volumes-from dbdata --name db1 training/postgres
docker run -d --volumes-from dbdata --name db2 training/postgres
```

還可以使用多個 `--volumes-from` 參數來從多個容器掛載多個資料卷。 也可以從其他已經掛載了容器卷的容器來掛載資料卷。

```shel
docker run -d --name db3 --volumes-from db1 training/postgres
```

注意：使用 `--volumes-from` 參數所掛載資料卷的容器自己並不需要保持在執行狀態。

如果刪除了掛載的容器（包括 dbdata、db1 和 db2），資料卷並不會被自動刪除。如果要刪除一個資料卷，必須在刪除最後一個還掛載著它的容器時使用 `docker rm -v` 命令來指定同時刪除關聯的容器。 這可以讓使用者在容器之間升級和移動資料卷。具體的操作將在下一節中進行講解。



### 備份、恢復

https://philipzheng.gitbooks.io/docker_practice/content/data_management/management.html



### 範例

可以用來 debug 使用

建立好的資料容器不會在 `docker ps`裡顯示

```shell
docker create -v /config --name dataContainer busybox
```

然後可以把檔案複製到容器裡

```shell
docker cp config.conf dataContainer:/config/
```

根目錄下會掛著資料容器的資料夾 `/config` 

```shell
docker create -v /config --name dataContainer busybox
docker run  -ti --volumes-from dataContainer ubuntu /bin/bash
```

匯出資料庫容器

```shell
docker export dataContainer > dataContainer.tar
```

匯入資料庫容器

```shell
docker import dataContainer.tar
```



### redis 持續儲存

這邊的 `/docker/redis-data` 會存在本地端上，與 redis 裡面的 `/data` 做連接，資料會存放在本地端

```shell
docker run  -v /docker/redis-data:/data \
  --name r1 -d redis \
  redis-server --appendonly yes
```

測試看看是否有聯通，裡面可以自己方一些東西

```shell
docker run  -v /docker/redis-data:/backup ubuntu ls /backup
```

所以當 `--volumes-from r1` 把資料容器載入就會預設在 `/data` 路徑裡

```shell
docker run --volumes-from r1 -it ubuntu ls /data
```

還可使用 `ro`讓資料夾裡的檔案只有讀取權限

```shell
docker run -v /docker/redis-data:/data:ro -it ubuntu rm -rf /data
```





## 外部存取容器

容器中可以執行一些網路應用，要讓外部也可以存取這些應用，可以通過 `-P` 或 `-p` 參數來指定連接埠映射。

當使用 -P 參數時，Docker 會隨機映射一個 `49000~49900` 的連接埠到內部容器開放的網路連接埠。

使用 `docker ps` 可以看到，本地主機的 49155 被映射到了容器的 5000 連接埠。此時連結本機的 49155 連接埠即可連結容器內 web 應用提供的界面。

```shell
docker run -d -P training/webapp python app.py
```



可以透過 `docker logs` 命令來查看應用的訊息。

```shell
docker logs -f nostalgic_morse
```



-p（小寫的）則可以指定要映射的連接埠，並且在一個指定連接埠上只可以綁定一個容器。支援的格式有 `ip:hostPort:containerPort | ip::containerPort | hostPort:containerPort`

### 映射所有遠端位址

使用 `hostPort:containerPort` 格式本地的 5000 連接埠映射到容器的 5000 連接埠，可以執行

```shell
docker run -d -p 5000:5000 training/webapp python app.py
```



### 映射到指定位址的指定連接埠

使用 `ip:hostPort:containerPort` 格式指定映射使用一個特定位址，比如 localhost 位址 127.0.0.1

```shell
docker run -d -p 127.0.0.1:5000:5000 training/webapp python app.py
```



### 映射到指定位址的任意連接埠

使用 `ip::containerPort` 綁定 localhost 的任意連接埠到容器的 5000 連接埠，本地主機會自動分配一個連接埠

```shell
docker run -d -p 127.0.0.1::5000 training/webapp python app.py
```

可以使用 udp 標記來指定 udp 連接埠

```shell
docker run -d -p 127.0.0.1:5000:5000/udp training/webapp python app.py
```



### 查看映射連接埠配置

```shell
docker port nostalgic_morse
docker port nostalgic_morse 5000
```



注意：

- 容器有自己的內部網路和 ip 位址（使用 `docker inspect` 可以獲取所有的變數，Docker 還可以有一個可變的網路設定。）
- -p 標記可以多次使用來綁定多個連接埠

```shell
docker run -d -p 5000:5000  -p 3000:80 training/webapp python app.py
```



## 容器互聯

容器的連接（linking）系統是除了連接埠映射外，另一種跟容器中應用互動的方式。

該系統會在來源端容器和接收端容器之間創建一個隧道，接收端容器可以看到來源端容器指定的信息。

### 自定義容器命名

```shell
docker run -d -P --name web training/webapp python app.py
```



使用 `docker ps` 來驗證設定的命名

```shell
docker ps -l
```

- `-l` 顯示最新建立的 container

也可以使用 `docker inspect` 來查看容器的名字

```shell
docker inspect -f "{{ .Name }}" aed84ee21bde
```



### 容器互聯

使用 `--link` 參數可以讓容器之間安全的進行互動

```shell
docker run -d --name db training/postgres
```

建立一個新的 web 容器，並將它連接到 db 容器

```shell
docker run -d -P --name web --link db:db training/webapp python app.py
```

`--link` 參數的格式為 `--link name:alias`，其中 `name` 是要連接的容器名稱，`alias` 是這個連接的別名。



Docker 透過 2 種方式為容器公開連接訊息：

- 環境變數
- 更新 `/etc/hosts` 檔案



使用 `env` 命令來查看 web 容器的環境變數

```shell
$ docker run --rm --name web2 --link db:db training/webapp env
DB_NAME=/web2/db
DB_PORT=tcp://172.17.0.5:5432
DB_PORT_5000_TCP=tcp://172.17.0.5:5432
DB_PORT_5000_TCP_PROTO=tcp
DB_PORT_5000_TCP_PORT=5432
DB_PORT_5000_TCP_ADDR=172.17.0.5
```

除了環境變量，Docker 還新增 host 訊息到父容器的 `/etc/hosts` 的檔案

```shell
$ docker run -t -i --rm --link db:db training/webapp /bin/bash
root@aed84ee21bde:/opt/webapp# cat /etc/hosts
172.17.0.7  aed84ee21bde
172.17.0.5  db
```

這裡有 2 個 hosts，第一個是 web 容器，web 容器用 id 作為他的主機名，第二個是 db 容器的 ip 和主機名。 可以在 web 容器中安裝 ping 命令來測試跟db容器的連通

```shell
root@aed84ee21bde:/opt/webapp# ping db
```



## 進階網路設定

https://philipzheng.gitbooks.io/docker_practice/content/advanced_network/



## Docker 網路

我們要先建立一個網路 `backend-network`

```shell
docker network create backend-network
docker network ls
```

用 `--net=<network>` 可以將容器連上 docker 所建立的網路

```shell
docker run -d --name=redis --net=backend-network redis
```

操作的行為比較像傳統的網路，跟 links 不太一樣，他並不會有額外的環境參數和額外的網路設定 `/etc/hosts`

```shell
docker run --net=backend-network ubuntu env
docker run --net=backend-network ubuntu cat /etc/hosts
```

取而代之的是，他會把網路 ip 存在 `/etc/resolv.conf` 裡

```shell
$ docker run --net=backend-network ubuntu cat /etc/resolv.conf
nameserver 127.0.0.11
options ndots:0
```



建立前端 `frontend-network` 網路連線，將前後端網路連接起來

```shell
docker network create frontend-network
docker network connect frontend-network redis
```

測試看看是否有連到 redis

```shell
docker run -d -p 3000:3000 --net=frontend-network katacoda/redis-node-docker-example
curl docker:3000
```



建立前端 `frontend-network2` 網路連線，將前後端網路連接起來， `frontend-network2` 的別名命名為 `db`

```shell
docker network create frontend-network2
docker network connect --alias db frontend-network2 redis
```

如果有連到 `frontend-network2` 的都可以在裡面使用 `db` 

```shell
docker run --net=frontend-network2 alpine ping -c1 db
```



## Syslog

https://docs.docker.com/config/containers/logging/syslog/

```shell
docker run -d --name redis-server redis
docker run -d --name redis-syslog --log-driver=syslog redis
docker run -d --name redis-none --log-driver=none redis
```



```shell
docker logs redis-server
docker logs redis-syslog
docker logs redis-none
```



```shell
docker inspect --format '{{ .HostConfig.LogConfig }}' redis-server
docker inspect --format '{{ .HostConfig.LogConfig }}' redis-syslog
docker inspect --format '{{ .HostConfig.LogConfig }}' redis-none
```



## 自動重新啟動 Ensuring Uptime 

Docker 認為任何容器退出時，應該以非零的代碼退出。默認情況下，崩潰的容器將保持停止狀態。

這是會 Output 一個 message 然後退出的時候會回傳錯誤 1 代碼的虛擬機。

```shell
docker run -d --name restart-default scrapbook/docker-restart-example
```

查看所有的虛擬機，會發現他已經停止運作

```shell
docker ps -a
```

顯示他印出一個 output message

```shell
docker logs restart-default
```



在建立 docker 的時候，可以設定自動重試啟動的次數`-restart=on-failure:3`

```shell
docker run -d --name restart-3 --restart=on-failure:3 scrapbook/docker-restart-example
```

顯示四個 output message，一次是原本建立時顯示的，跟後來失敗重新啟動的三次

```shell
docker logs restart-3
```



或是可以設定成總是重新啟動崩潰的 docker，直到明確的告訴容器叫他停止。

```shell
docker run -d --name restart-always --restart=always scrapbook/docker-restart-example
```

可以透過 log 查看他重新啟動的紀錄

```shell
docker logs restart-always
```



## Docker Metadata & Labels

### 標籤容器

標籤可以附加在容器上，容器可以有很多的標籤，標籤可以在很多的容器中方便尋找

使用 `-l <key>=<value>` 加一個 label 到容器上

```shell
docker run -l user=12345 -d redis
```

或者是使用 `--label-file=<filename>` 可以添加多個 label

```
# labels
user=123461
role=cache
```

```shell
docker run --label-file=labels -d redis
```

查看容器標籤

```shell
docker inspect -f "{{json .Config.Labels }}" <container>
```

Filter 容器

```shell
docker ps --filter "label=user=scrapbook"
```





### 標籤映像檔

```shell
# Dockerfile
LABEL vendor=Katacoda
LABEL com.katacoda.version=0.0.5
LABEL com.katacoda.build-date=2015-07-01T10:47:29Z
LABEL com.katacoda.private-msg=HelloWorld
LABEL com.katacoda.course=Docker
```

查看映像檔標籤

```shell
docker inspect -f "{{json .ContainerConfig.Labels }}" <images>
```

Filter 映像檔

```shell
docker images --filter "label=vendor=Katacoda"
```



## Nginx proxy — load balancing containers

有三個東西

- Port 設定 80，確保可以使用 HTTP
- Socket 用 data container 來讀取 (Google IPC Unix Domain Socket)
- 設定 DEFAULT_HOST

```shell
docker run -d -p 80:80 \
-e DEFAULT_HOST=proxy.example \
-v /var/run/docker.sock:/tmp/docker.sock:ro \
--name nginx jwilder/nginx-proxy
```

這時候請求會得到 503，因為還沒有設定他 Load Balancing 的容器

```shell
curl http://docker
```



建立 Load Balancing 的容器，裡面是用 PHP + Apache Server，然後設定 Apache 的 VIRTUAL_HOST

```shell
docker run -d -p 80 -e VIRTUAL_HOST=proxy.example katacoda/docker-http-server
```

這時候在試就會有東西了，多建立幾個可以發現他會以 Round Robin 的方式請求 Server

```shell
curl http://docker
```



可以透過 `/etc/nginx/conf.d/default.conf` 這個檔案查看 nginx load balancing 的設定

```shell
docker exec nginx cat /etc/nginx/conf.d/default.conf
docker logs nginx
```



## Docker Compose

https://yeasy.gitbooks.io/docker_practice/content/compose/

https://docs.docker.com/compose/compose-file/#build

在使用多個容器時，可能難以管理 Links 跟 Variable，為了解決這個問題可以使用 Docker Compose 的工具來管理容器

可以用於快速部署，像是 Rails。或是運行多個 Docker 容器

`Compose` 中有兩個重要的概念：

- 服務 (`service`)：一個應用的容器，實際上可以包括若干運行相同鏡像的容器實例。
- 項目 (`project`)：由一組關聯的應用容器組成的一個完整業務單元，在 `docker-compose.yml` 文件中定義。

一個 project 可以有多個 service 所組成，然後 Compose 則管理 Project

```yaml
# docker-compose.yml
web:
  build: .
```

- `web` 一個 project
- `build` 一個 service，這邊他會建立當前目的 `Dockerfile` 作為映像檔



compose 支援所有 docker 的指令，要把兩個容器連接起來的話使用 `links`，也可以加上其他的屬性值。

```yaml
# docker-compose.yml
web:
  build: .
  links:
    - redis
  ports:
    - "3000"
    - "8000"
```



設定第二個 container

```yaml
# docker-compose.yml
redis:
  image: redis:alpine
  volumes:
    - /var/redis/data:/data
```



運行 `-d` 參數跟 docker 意思一樣，在背後運行

```shell
docker-compose up -d
```



`docker` 指令只能看一個容器的訊息，`docker-compose` 可以看所有容器的訊息

```shell
docker-compose ps
docker-compose logs
```



而且也很容易延展容器，scale up、scale down

```shell
docker-compose scale web=3
docker-compose scale web=1
```



終止所有的容器

```shell
docker-compose stop
```



刪除所有的容器

```shell
docker-compose rm
```



## Metric 

查看容器狀態，CPU 使用率、記憶體使用量、網路使用量、Block I/O、Pids

```shell
docker stats <container>
```



查看多個容器的狀態

```shell
docker stats <container1> <container2>
```



查看所有容器的狀態 `docker ps -q` 會列出所有容器的 id，利用 pipe 將參數傳入，用 `xargs` 接到後傳進 `docker stats`

```shell
docker ps -q | xargs docker stats
```



## 優化映像檔 multi-stage build functionality

非常適合用於 Golang 語言多階段構建，第一階段用較大的 docker 製作 Golang binary 檔，第二階段可以用更小的 binary 檔來部署 

```shell
# Dockerfile
# 第一階段編譯 First Stage
FROM golang:1.6-alpine

RUN mkdir /app
ADD . /app/
WORKDIR /app
RUN CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o main .

# 第二階段複製編譯好的檔案到新的容器 Second Stage
FROM alpine
EXPOSE 80
CMD ["/app"]

# 從第 0 個映像檔複製檔案 Copy from first stage 
COPY --from=0 /app/main /app
```



這時候建立映像檔後，查看所有映像檔

```shell
docker build -f Dockerfile.multi -t golang-app .
docker images
```

會產生兩個映像檔，有一個名字是沒有被標記的，另一個名字是 golang-app



如此一來在映像檔內就不必暫存沒有必要的程式碼

```shell
docker run -d -p 80:80 golang-app
curl localhost
```



## Formatting PS Output

練習用先隨便建立一個容器

```shell
docker run -d redis
```



```shell
$ docker ps
CONTAINER ID        IMAGE               COMMAND                  CREATED             STATUS  PORTS               NAMES
fc719ecf3fa5        redis               "docker-entrypoint.s…"   5 seconds ago       Up 4 seconds  6379/tcp            focused_poitras
```



使用 `--format` 可以自訂輸出的訊息

```shell
$ docker ps --format '{{.Names}} container is using {{.Image}} image'
focused_poitras container is using redis image
```



`table` 是 golang templates 的語言

```shell
$ docker ps --format 'table {{.Names}}\t{{.Image}}'
NAMES               IMAGE
focused_poitras     redis
```



`docker ps` 只會顯示容器公開的訊息，要顯示詳細的訊息像是 IP 位置，需使用 `docker inspect`，一樣可以使用 golang 的模板

```shell
$ docker ps -q | xargs docker inspect --format '{{ .Id }} - {{ .Name }} - {{ .NetworkSettings.IPAddress }}'
fc719ecf3fa5902c72de98f482ac1a7bba2002156ac8487453d44c93b5fb24ef - /focused_poitras - 172.18.0.2
```



## Swarm mode

### 基本觀念

節點 node

- 管理節點 manager node (leader)
- 工作節點 worker node (follower)


服務 services

- `replicated services` 按照一定規則在各個工作節點上運行指定個數的任務。
- `global services` 每個工作節點上運行一個任務



初始化 docker swarm 的節點

```shell
docker swarm init
```



使用 `join-token` 在第二台機器上紀錄第一台的 token 

```shell
token=$(docker -H 172.17.0.74:2345 swarm join-token -q worker) && echo $token
```



將第二台機器作為第一台 worker 

```shell
docker swarm join 172.17.0.74:2377 --token $token
```



在 manager 上可以看到 node 的狀態

```shell
docker node ls
```



建立 network，因為 node 是透過網路來溝通的

```shell
docker network create -d overlay skynet
```



在 service 上建立 `katacoda/docker-http-server` 

```shell
docker service create --name http --network skynet --replicas 2 -p 80:80 katacoda/docker-http-server
```

- `--replicas 2` 建立兩台
- `--network skynet` 附加在剛剛建立的網路
- `--name http` 服務名稱是 http



查看所有的 service

```shell
docker service ls
```



查看某個 service

```shell
docker service ps http
docker service inspect --pretty http
```





分別在兩台機器上輸入，可以看到 docker 在運行

```shell
docker ps
```



這時候請求會以 round robin 請求 server

```shell
curl docker
```



詢問當某個機器是否為 manager 節點

```shell
docker node ps self
```



查看某個節點

```shell
docker node ps $(docker node ls -q | head -n1)
```



將 service scale 到五台

```shell
docker service scale http=5
```



## Makefile

範例

```shell
NAME = benhall/docker-make-demo

default: build

build:
    docker build -t $(NAME) .

push:
    docker push $(NAME)

debug:
    docker run --rm -it $(NAME) /bin/bash

run:
    docker run --rm $(NAME)

release: build push
```





























