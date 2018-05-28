# Kubernetes up and running

## Verify

### Components Statuses

檢查 cluster 的健康狀態

```shell
$ kubectl get componentstatuses
NAME                 STATUS    MESSAGE              ERROR
scheduler            Healthy   ok                   
controller-manager   Healthy   ok                   
etcd-0               Healthy   {"health": "true"} 
```

- controller-manager：ensuring that all of the replicas of a service are available and healthy
- scheduler：placing different pods onto different nodes in the cluster
- etcd：storage for the cluster where all of the api abjects are stored

### Nodes

```shell
$ kubectl get nodes
$ kubectl describe nodes node-1
```

### Porxy

```shell
$ kubectl get daemonSets --namespace=kube-system kube-proxy
```

### DNS

DNS 檔案會存在容器中的 `/etc/resolv.conf`，當不知道 namespace 的網址的時候會去這個檔案裡的 nameserver 詢問

```shell
$ kubectl get deployments --namespace=kube-system kube-dns
$ kubectl get services --namespace=kube-system kubedns
```

### UI

```shell
$ kubectl get deployments --namespace=kube-system kubenetes-dashboard
$ kubectl get services --namespace=kube-system kubenetes-dashboard
```

可以使用 `kubectl proxy` 查看 UI 介面，server 會跑在 `http://localhost:8001/ui` 上

```shell
$ kubectl proxy
```

## 常見的 kubectl 指令

### Context

用來管理不同的 cluster 和不同的 users，在 `set-context` 時使用 `--users` 或 `--clusters` 的標籤

#### set-context

```shell
kubectl config set-context my-context --namespace=mystuff
```

```yaml
contexts:
- context:
    cluster: ""
    namespace: mystuff
    user: ""
  name: my-context
```

#### use-context

設定好 context 並不代表已經使用 context

```shell
kubectl config use-context my-context
```

```yaml
>>>>>>
current-context: minikube
======
current-context: my-context
<<<<<<
```

### 查看 Kubernetes API 物件

```shell
kubectl proxy
```

https://localhost:8001/api/v1/namespace/default/pods/my-pod

`kubectl`指令會發出 HTTP 請求到 URLs 去存取 kubernetes 的物件

```shell
kubectl get <resource-name>
kubectl get <resource-name> <object-name>
```

`kubectl`省略的很多東西為了適合人類閱讀，要獲得完整的資訊可以使用

```shell
kubectl get <resource-name> [<object-name>] -o yaml
kubectl get <resource-name> [<object-name>] -o json
```

如果要使用 awk 過濾資料可以使用 `--no-headers` 參數，或是用上面的 json 也可以

```shell
kubectl get <resource-name> [<object-name>] --no-headers | awk ...
```

使用 jsonpath 過濾特定的欄位

```shell
kubectl get pod my-pod -o jsonpath --namespace={.status.podIP}
```

或是使用 `describe`

```shell
kubectl describe <resource-name> <object-name>
```

### CRUD

Create and Update

```shell
kubectl apply -f obj.yaml
```

如果想要用 interactive 的方式編輯可以使用，他會開啟編輯器，編輯完之後儲存就會自動更新 kubernetes cluster

```shell
kubectl edit <reource-name> <object-name>
```

Delete

```shell
kubectl delete -f obj.yaml
```

如果想要用 interactive 的方式刪除，注意`kubectl`不會有任何提示訊息，而是直接刪除

```shell
kubectl delete <reource-name> <object-name>
```

### Labeling and Annotating

Update label，因為 label 是唯一的如果要更改的話必須使用`--overwrite`修改

```shell
kubectl label pods my-pod color=red
kubectl label pods my-pod color=red --overwrite
```

Remove label

```shell
kubectl label pods my-pod -color
kubectl label pods my-pod -<label-name>
```

### Debugging Commands

logs

- `-c`後面接 pod 裡的 container-name
- `-f` 連續 stream log
- `--previous` 如果不停的重新啟動，可以用這個標籤來看前一個容器的內容

```shell
kubectl logs <pod-name>
kubectl logs <pod-name> -c <container-name>
kubectl logs <pod-name> -f
kubectl logs <pod-name> --previous
```

exec

在 container 中執行指令

```shell
kubectl exec -ti <pod-name> -- bash
```

cp

複製檔案從 container 到本地端

```shell
kubectl cp <pod-name>:/path/to/remote/file /path/to/local/file
```

## Pods

Pod 內的容器是否要放在一起的原則：Will these containers work correctly if they land on different machines?

- No：則把容器放同一個 Pod
- Yes：把不同容器放在不同的 Pod 會比較好

### Create

Pod 建立會經由 Deployment 和 ReplicaSet

```shell
kubectl run kuard --image=gcr.io/kuar-demo/kuard-amd64:1
kubectl get pods
kubectl get deployments
kubectl get replicasets
```

### Delete

```shell
kubectl delete deployments/kuard
```

### Pod manifest

```shell
# kuard-pod.yaml
apiVersion: v1
kind: Pod
metadata:
  name: kuard
spec:
  containers:
    - image: gcr.io/kuar-demo/kuard-amd64:1
      name: kuard
      ports:
        - containerPort: 8080
          name: http
          protocol: TCP
```

#### Running

```shell
kubectl apply -f kuard-pod.yaml
kubectl get pods
kubectl get deployments
kubectl get replicasets
kubectl describe pods kuard
```

#### Delete

當 Pod 被刪除的時候並不會馬上被刪掉，而會轉變成 Terminating 的狀態，當 Pod 轉變成 Terminating 的狀態的時候，不會再接收任何的請求，在停止之前，預設會有 30 秒的 grace period 讓 Pod 完成現有的任務。

儲存在 Pod 裡的 Data 也會一並刪除

```shell
kubectl delete pods/kuard
kubectl delete -f kuard-pod.yaml
```

### Port-Forwarding

透過 Kubernetes master 會建立一條安全的通道，從 master node 連到 worker nodes，只要這條指令有在運行，你就能一直存取這個 Pod。

```shell
kubectl port-forward kuard 8080:8080
```

### 在容器中執行指令

```shell
kubectl exec kuard date
kubectl exec -ti kuard ash
```

### Copy

Generally speaking, copying files into a container is an antipattern. You really should treat the contents of a container as immutable.But occasionally it's the most immediate way to stop the bleeding and restore your service to health, since it is quicker than building, pushing, and rolling out a new image.

```shell
kubectl cp <pod-name>:/path/to/file /path/to/file
```

### Health Checks

當你的 application 跑在容器裡的時候，Kubernetes 會自動的偵測 process 是否存活，如果 process 不存在，Kubernetes 就會重新啟動

然而如果發生 deadlocked 的情況，而無法接收請求，process 因為還在跑，所以狀態又是健康的，要改善這個提供了 application liveness 來做健康狀態的檢查

#### Liveness Probe

- `initialDelaySeconds`：Server 開機後過幾秒才會開始探測健康狀態
- `timeoutSeconds`：必須在幾秒之內回應探測的請求
- `periodSeconds`：每幾秒探測一次
- `failureThreshold`：連續失敗幾次才會重新啟動

```shell
kubectl apply -f https://github.com/kubernetes-up-and-running/examples/blob/master/5-2-kuard-pod-health.yaml
```

```yaml
# kuard-pod-health.yaml
apiVersion: v1
kind: Pod
metadata:
  name: kuard
spec:
  containers:
    - image: gcr.io/kuar-demo/kuard-amd64:1
      name: kuard
      livenessProbe:
        httpGet:
          path: /healthy
          port: 8080
        initialDelaySeconds: 5
        timeoutSeconds: 1
        periodSeconds: 10
        failureThreshold: 3
      ports:
        - containerPort: 8080
          name: http
          protocol: TCP
```

##### Type of Health Checks

- TCP
- HTTP

#### Resource Management

- requests：資源必須使用的最小量
- limits：資源可以被消耗的最大量



##### 資源使用下限 requests

定義最小資源，可以是 CPU、記憶體或是 GPU。特別注意，他的資源是限制在每個 Container 而不是每個 Pod

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/5-3-kaurd-pod-resreq.yaml
```

```yaml
# kaurd-pod-resreq.yaml
apiVersion: v1
kind: Pod
metadata:
  name: kuard
spec:
  containers:
    - image: gcr.io/kuar-demo/kuard-amd64:1
      name: kuard
      resources:
        requests:
          cpu: "500m"
          memory: "128Mi"
      ports:
        - containerPort: 8080
          name: http
          protocol: TCP
```

###### Request limit details

想像一個 Pod 裡面有一個 Container，Container 的 requests 是 0.5 CPU。假設我們的 Kubernetes cluster 有 2 CPU，然後在 cluster 裡建立一個 Pod 請求 0.5 CPU。

因為只有一個 Pod 在機器裡，所以我們可以消耗所有 2.0 個可以使用的 CPU，儘管請求的數量只有  0.5 CPU。

如果建立第二個相同 Pod 在同個機器上，這樣每個 Pod 可以獲得 1.0 CPU。

如果建立第三個相同 Pod，每個 Pod 可以獲得 0.66 個 CPU。最後如果建立第四個相同的 Pod，每個 Pod 可以獲得 0.5 CPU。節點的容量將會被佔滿。

CPU 請求的實作是使用 `cpu-shares` functionality in the Linux kernel。

Memory 的請求跟 CPU 的很類似，但還是有一點很重要的不同之處。如果 Container 超過 Memory request，OS 不可能只從 process 移除 memory，因為記憶體已經被 allocated。所以系統發出 Out of Memory，`kubelet` 會終止超過 memory 請求使用量的 Container。這些 Container 會自動的重新啟動，但是 Container 只能消耗更少可用的記憶體。

#####資源使用上限 limits

假設設定 0.5 CPU 是他的上限，即使有其他閒置的 CPU，他還是只能使用 0.5 CPU。memory 的原理也相同。

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/5-4-kaurd-pod-reslim.yaml
```

```yaml
# kaurd-pod-reslim.yaml
apiVersion: v1
kind: Pod
metadata:
  name: kuard
spec:
  containers:
    - image: gcr.io/kuar-demo/kuard-amd64:1
      name: kuard
      resources:
        requests:
          cpu: "500m"
          memory: "128Mi"
        limits:
          cpu: "1000m"
          memory: "256Mi"
      ports:
        - containerPort: 8080
          name: http
          protocol: TCP
```

### Persisting Data with Volumes

只允許 Pod 裡面的 Container 互相存取檔案，使用 `hostDir` volume type。

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/5-5-kuard-pod-vol.yaml
```

```yaml
# kuard-pod-vol.yaml
apiVersion: v1
kind: Pod
metadata:
  name: kuard
spec:
  volumes:
    - name: "kuard-data"
      hostPath:
        path: "/var/lib/kuard"
  containers:
    - image: gcr.io/kuar-demo/kuard-amd64:1
      name: kuard
      volumeMounts:
        - mountPath: "/data"
          name: "kuard-data"
      ports:
        - containerPort: 8080
          name: http
          protocol: TCP
```



#### 在 Pod 中使用 Volumes 的時機

##### Communication/synchronization

Pod 是使用 `emptyDir` volume，生命週期跟 Pod 相同，可以共享於兩個 Container 間，用於溝通。

##### Cache

以空間換取時間，有些計算可以暫存起來，不需要再重複計算。或是 health check 失敗的時候，Container 重新啟動（不是以 Pod 為單位嗎？），`emptyDir`會幫你暫存所有的東西。

##### Persistent data

跨節點間的 volume

##### Mounting the host filesystem

`hostDir`volume type，將本地端的資料夾掛到 Container 裡。

#### 使用遠端磁碟的 Persisting Data

有時候我們不想要 Pod 換到另外一個節點的時候資料就不見，為了達到這個目標，我們可以掛在遠端網路儲存空間上。Kubernetes 支援 NFS、iSCSI 以及 cloud provider based storage APIs。在大多情況下，cloud provider 會自動建立磁碟，如果資料夾不存在的時候。

```shell
volumes:
  - name: "kuard-data"
    nfs:
      server: my.nfs.server.local
      path: "/exports"
```



### Putting It All Together

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/5-6-kuard-pod-full.yaml
```

```yaml
# kuard-pod-full.yaml
apiVersion: v1
kind: Pod
metadata:
  name: kuard
spec:
  volumes:
    - name: "kuard-data"
      nfs:
        server: my.nfs.server.local
        path: "/exports"
  containers:
    - image: gcr.io/kuar-demo/kuard-amd64:1
      name: kuard
      ports:
        - containerPort: 8080
          name: http
          protocol: TCP
      resources:
        requests:
          cpu: "500m"
          memory: "128Mi"
        limits:
          cpu: "1000m"
          memory: "256Mi"
      volumeMounts:
        - mountPath: "/data"
          name: "kuard-data"
      livenessProbe:
        httpGet:
          path: /healthy
          port: 8080
        initialDelaySeconds: 5
        timeoutSeconds: 1
        periodSeconds: 10
        failureThreshold: 3
      readinessProbe:
        httpGet:
          path: /ready
          port: 8080
        initialDelaySeconds: 30
        timeoutSeconds: 1
        periodSeconds: 10
        failureThreshold: 3
```

## Labels and Annotation

### Labels

被使用來 grouping, viewing, operation

- Key
  - prefix：限制在 253 字元內的 DNS subdomain
  - name：必要的欄位且必須小於 63 字元，開始與結尾字元必須是英文字，且可以使用`-` `_` `.`字元
- Value：最大 63 字元

| Key                           | Value |
| ----------------------------- | ----- |
| acme.com/app-version          | 1.0.0 |
| appVersion                    | 1.0.0 |
| app.Version                   | 1.0.0 |
| kubernetes.io/cluster-service | true  |

#### 使用 Labels

接下來會建立兩種 app (alpaca 跟 bandicoot)，三種 env (prod、test、staging)，兩種 ver (1、2)

```shell
kubectl run alpaca-prod \
--image=gcr.io/kuar-demo/kuard-amd64:1 \
--replicas=2 \
--labels="ver=1,app=alpaca,env=prod"
kubectl run alpaca-test \
--image=gcr.io/kuar-demo/kuard-amd64:2 \
--replicas=1 \
--labels="ver=2,app=alpaca,env=test"
kubectl run bandicoot-prod \
--image=gcr.io/kuar-demo/kuard-amd64:2 \
--replicas=2 \
--labels="ver=2,app=bandicoot,env=prod"
kubectl run bandicoot-staging \
--image=gcr.io/kuar-demo/kuard-amd64:2 \
--replicas=1 \
--labels="ver=2,app=bandicoot,env=staging"
```

```shell
$  kubectl get deployments --show-labels
NAME                DESIRED   CURRENT   UP-TO-DATE   AVAILABLE   AGE       LABELS
alpaca-prod         2         2         2            2           1m        app=alpaca,env=prod,ver=1
alpaca-test         1         1         1            1           45s       app=alpaca,env=test,ver=2
bandicoot-prod      2         2         2            2           23s       app=bandicoot,env=prod,ver=2
bandicoot-staging   1         1         1            1           20s       app=bandicoot,env=staging,ver=2
```

#### 修改 Labels

在建立物件之後還可以更新 label

```shell
kubectl label deployments alpaca-test "canary=true"
kubectl get all -a --show-labels
kubectl get deployments -L canary
```

警告：用 `kubectl label`修改 label 只會修改 deployment 本身的 label，不會影響到其他的物件 (ReplicaSets、Pods)

#### 刪除 Labels

在 key 後面加上`-`代表刪除 label

```shell
kubectl label deployments alpaca-test "canary-"
```

#### Label Selectors

可以看到有`pod-template-hash`label 在跑`kubectl run`指令的時候會添加上去的 template version，這可以讓 deployments 去管理、更新、清理物件。

```shell
$ kubectl get pods --show-labels
NAME                                READY     STATUS    RESTARTS   AGE       LABELS
alpaca-prod-6cdd4997cf-q2hh5        1/1       Running   0          1h        app=alpaca,env=prod,pod-template-hash=2788055379,ver=1
alpaca-prod-6cdd4997cf-tc82z        1/1       Running   0          1h        app=alpaca,env=prod,pod-template-hash=2788055379,ver=1
alpaca-test-6c6b9686cc-zr9zc        1/1       Running   0          1h        app=alpaca,env=test,pod-template-hash=2726524277,ver=2
bandicoot-prod-7bd55cfd97-66rwk     1/1       Running   0          1h        app=bandicoot,env=prod,pod-template-hash=3681179853,ver=2
bandicoot-prod-7bd55cfd97-jxspz     1/1       Running   0          1h        app=bandicoot,env=prod,pod-template-hash=3681179853,ver=2
bandicoot-staging-6994cdbbf-qjsnj   1/1       Running   0          1h        app=bandicoot,env=staging,pod-template-hash=255078669,ver=2
```

如果只想要顯示`ver=2`的 Pod 可以使用`--selector`的標籤：

```shell
kubectl get pods --selector="ver=2"
```

兩個 label 會以 AND 邏輯運算：

```shell
kubectl get pods --selector="app=bandicoot,ver=2"
```

label 在某個集合中的值：

```shell
kubectl get pods --selector="app in (alpaca,bandicoot)"
```

##### Selector Operators

| Operator                   | Description                        |
| -------------------------- | ---------------------------------- |
| key=value                  | key is set to value                |
| key!=value                 | key is not set to value            |
| key in (value1, value2)    | key is one of value1 or value2     |
| key notin (value1, value2) | key is not one of value1 or value2 |
| key                        | key is set                         |
| !key                       | key is not set                     |

#### Label Selectors in API Objects

新版的形式支援集合表達式，`In``NotIn`分別為`=``!=`表達式

```yaml
selector:
  matchLabels:
    app: alpaca
  matchExpressions:
    - {key: ver, operator: In, values: [1, 2]}
```

舊版的形式只支援`=`表達式

```yaml
selector:
  app: alpaca
  ver: 1
```

### Annotations

annotations 提供一個地方儲存而外的 metadata，可以適用自動化工具標示

annotation key 的格式與 label key 的格式相同，因為他們通常用在兩個工具間資訊的溝通，所以在 key 中的 namespace 顯得格外重要。

舉例`deployment.kubernetes.io/revision` `kubernetes.io/change-cause`

annotation value 是任意字串

```yaml
metadata:
  annotations:
    example.com/icon-url: "https://example.com/icon.png"
```

### Cleanup

```shell
kubectl delete deployments --all
```

## Service Discovery

### The Service Object

`kubectl run`是建立 kubernetes deployment 簡單的方法，我們可以使用`kubectl expose`建立 service：

```shell
kubectl run alpaca-prod \
  --image=gcr.io/kuar-demo/kuard-amd64:1 \
  --replicas=3 \
  --port=8080 \
  --labels="ver=1,app=alpaca,env=prod"
kubectl expose deployment alpaca-prod

kubectl run bandicoot-prod \
  --image=gcr.io/kuar-demo/kuard-amd64:2 \
  --replicas=2 \
  --port=8080 \
  --labels="ver=2,app=bandicoot,env=prod"
kubectl expose deployment bandicoot-prod

```

```shell
$ kubectl get services -o wide
NAME             TYPE        CLUSTER-IP      EXTERNAL-IP   PORT(S)    AGE       SELECTOR
alpaca-prod      ClusterIP   10.101.231.94   <none>        8080/TCP   18s       app=alpaca,env=prod,ver=1
bandicoot-prod   ClusterIP   10.98.101.175   <none>        8080/TCP   5s        app=bandicoot,env=prod,ver=2
kubernetes       ClusterIP   10.96.0.1       <none>        443/TCP    82d       <none>
```

如果我們到`SELECTOR`的欄位，我們能看到 service 會給 selector 一個名字，和哪一個 port 可以連接。`kubectl expose`會順便把 deployment 所定義的 label selector 和 relevant ports 一同 pull 下來。

此外，service 還會被指派到一個新的 virtual IP 被稱作為 cluster IP。這個特別的 IP 會 load-balance 他所有的 pod，pod 是由 selector 來辨識的。

可以使用`port-forward`來跟`alpaca`裡的其中一個 pod 互動：

```shell
ALPACA_POD=$(kubectl get pods -l app=alpaca -o jsonpath='{.items[0].metadata.name}')
kubectl port-forward $ALPACA_POD 48858:8080
```

### Service DNS

因為 cluster IP 是虛擬的，他穩固所以適合給他一個 DNS 的地址。

A record for alpaca-prod：

```shell
;; opcode: QUERY, status: NOERROR, id: 61875
;; flags: qr aa rd ra; QUERY: 1, ANSWER: 1, AUTHORITY: 0, ADDITIONAL: 0

;; QUESTION SECTION:
;alpaca-prod.default.svc.cluster.local.	IN	 A

;; ANSWER SECTION:
alpaca-prod.default.svc.cluster.local.	30	IN	A	10.101.231.94
```

- `alpaca-prod`：service 的名字
- `default`：service 所在的 namespace
- `svc`：這是一個 service
- `cluster.local`：允許 kubernetes to expose 其他不同 DNS 的型態。

如果在同一個 namespace 下，可以使用`alpaca-prod`指向另一個同 namespace 下的 service。可以用`alpaca-prod.default`指向不同 namespace 的 service。當然也可以使用完整的 service 名字`alpaca-prod.default.svc.cluster.local`。

### Readiness Checks

有時候應用程式開啟的時候並不能馬上處理請求，因為很常會需要一小段的時間來初始化他，有可能幾秒鐘或甚至幾分鐘。Servcie 提供了 pod 是否準備好了透過 readiness check。我們修改先前 deloyment 然後加上 readiness check：

```shell
KUBE_EDITOR="vim" kubectl edit deployment/alpaca-prod
```

```yaml
spec:
  ...
  template:
    ...
    spec:
      containers:
      ...
        name: alpaca-prod
        readinessProbe:
          httpGet:
            path: /ready
            port: 8080
          periodSeconds: 2
          initialDelaySeconds: 0
          failureThreshold: 3
          successThreshold: 1
```

- `periodSeconds`：每兩檢查一次
- `initialDelaySeconds`：開機幾秒後才開始檢查
- `failureThreshold`：幾次檢查失敗，這個 pod 就會被認為 not ready
- `successThreshold`：幾次檢查成功，這個 pod 就會被認為 ready

只有 ready 的 pod 才會送流量。

更新之後 kubectl 會重新建立 pod：

```shell
ALPACA_POD=$(kubectl get pods -l app=alpaca -o jsonpath='{.items[0].metadata.name}')
kubectl port-forward $ALPACA_POD 48858:8080
```

在另一個終端機，用`watch`指令看`alpaca-prod`的 service。Endpoint 是一個低階的方法，讓 Service 可以找哪些 endpoint 送流量：

```shell
kubectl get endpoints alpaca-prod --watch
```

Readiness check 用於對 overloaded and sick server，發送給 server 一個訊號，當他們不想要再接收其他流量的時候。

這也很適合用來終止 pods，server 發出訊號不在接收流量，等待 pod 所存在的連線關閉，然後再乾淨的離開。

### Looking Beyond the Cluster

pod 上的 IP 只能從 cluster 裡的 service 獲得，所以我們要一個新的方式來讓流量進來，這型態稱作為`NodePort`。

除了 cluster IP 之外，系統還會挑選一個 port（或者用戶可以指定一個 port），然後 cluster 中的每個 node 都會將流量轉發到該 port 以訪問該 service。

我們可以聯繫到 cluster 裡的任何一個 node 的 service，使用`NodePort`不必知道哪個 service 裡的 Pod。他可以被軟硬體整合的 load balancer 去發現 service。

```shell
KUBE_EDITOR="vim" kubectl edit service alpaca-prod
```

修改`spec.type`的欄位成`NodePort`，也可以在建立 service 的時候用`kubectl expose`指令標示`--type=NodePort`。

```shell
$ kubectl describe service alpaca-prod
Name:                     alpaca-prod
Namespace:                default
Labels:                   app=alpaca
                          env=prod
                          ver=1
Annotations:              <none>
Selector:                 app=alpaca,env=prod,ver=1
Type:                     NodePort
IP:                       10.101.231.94
Port:                     <unset>  8080/TCP
TargetPort:               8080/TCP
NodePort:                 <unset>  31054/TCP
Endpoints:                172.17.0.11:8080,172.17.0.7:8080,172.17.0.8:8080
Session Affinity:         None
External Traffic Policy:  Cluster
Events:                   <none>
```

現在我們去對 node 上的 port 做連線就能存取 service 

在同一個網路的話可以直接連線，如果是在 cloud 上的某個地方可以透過 ssh 連線查看：

```shell
ssh <node> -L 8080:localhost:32711
```

重新載入多次可以發現他會隨機連到不同的 pod。

### Cloud Intergration

如果是跑在 cloud 上的話，可以使用`LoadBalancer`的 type。他會使用`NodePort`然後再將 cloud 上的 load balancer 指到 cluster 的每個 node。

```shell
KUBE_EDITOR="vim" kubectl edit service alpaca-prod
```

修改`spec.type`的欄位成`LoadBalancer`

如果這時候用`kubectl get services`看會發現`EXTERNAL-IP`欄位寫著`<pending>`，過一陣子 cloud 會指定 public ip 給你。

```shell
$ kubectl describe service alpaca-prod
```

### Endpoints

有些應用程式或是系統本身不想要使用 cluster IP。這可以使用另外一個物件叫做`Endpoints`。對於每個`Service`被建立的時候，都會有相對應的`Endpoints`被建立，Endpoint 裡包含著 service 所服務的 ip addresses。

```shell
$ kubectl describe endpoints alpaca-prod
Name:         alpaca-prod
Namespace:    default
Labels:       app=alpaca
              env=prod
              ver=1
Annotations:  <none>
Subsets:
  Addresses:          172.17.0.11,172.17.0.7
  NotReadyAddresses:  172.17.0.8
  Ports:
    Name     Port  Protocol
    ----     ----  --------
    <unset>  8080  TCP

Events:  <none>
```

Service 可以直接查看 Kubernetes API 的 endpoints 然後呼叫他。Kubernetes API 能夠 watch 一個物件，且當他改動的時候會去通知。因此當 service 的 ip 改變時 client 可以立即的反應：

```shell
$ kubectl get endpoints alpaca-prod --watch
```

```shell
kubectl delete deployment alpaca-prod
kubectl run alpaca-prod \
  --image=gcr.io/kuar-demo/kuard-amd64:1 \
  --replicas=3 \
  --port=8080 \
  --labels="ver=1,app=alpaca,env=prod"
```

```shell
$ kubectl get endpoints alpaca-prod --watch
NAME          ENDPOINTS                                          AGE
alpaca-prod   172.17.0.11:8080,172.17.0.7:8080,172.17.0.8:8080   17h
alpaca-prod   172.17.0.7:8080,172.17.0.8:8080   17h
alpaca-prod   <none>    17h
alpaca-prod   172.17.0.7:8080   17h
alpaca-prod   172.17.0.5:8080,172.17.0.7:8080   17h
alpaca-prod   172.17.0.5:8080,172.17.0.7:8080,172.17.0.8:8080   17h
```

### Manual Service Discovery

Kubernetes services 是建立在 pods 上的 label selector。這意味著，你可以使用 Kubernetes API 去做初步的 service discovery，不需要 Service 物件的幫忙。

```shell
$ kubectl get pods -o wide --show-labels
NAME                            READY     STATUS    RESTARTS   AGE       IP            NODE       LABELS
alpaca-prod-7d8597db98-6xqzg    1/1       Running   0          30m       172.17.0.8    minikube   app=alpaca,env=prod,pod-template-hash=3841538654,ver=1
alpaca-prod-7d8597db98-8plhk    1/1       Running   0          30m       172.17.0.5    minikube   app=alpaca,env=prod,pod-template-hash=3841538654,ver=1
alpaca-prod-7d8597db98-x9v69    1/1       Running   0          30m       172.17.0.7    minikube   app=alpaca,env=prod,pod-template-hash=3841538654,ver=1
bandicoot-prod-d78b5d76-7h7m9   1/1       Running   0          17h       172.17.0.10   minikube   app=bandicoot,env=prod,pod-template-hash=83461832,ver=2
bandicoot-prod-d78b5d76-8htgm   1/1       Running   0          17h       172.17.0.9    minikube   app=bandicoot,env=prod,pod-template-hash=83461832,ver=2
kuard                           1/1       Running   0          2d        172.17.0.4    minikube   <none>
```

```shell
$ kubectl get pods -o wide --selector=app=alpaca,env=prod
NAME                           READY     STATUS    RESTARTS   AGE       IP           NODE
alpaca-prod-7d8597db98-6xqzg   1/1       Running   0          31m       172.17.0.8   minikube
alpaca-prod-7d8597db98-8plhk   1/1       Running   0          31m       172.17.0.5   minikube
alpaca-prod-7d8597db98-x9v69   1/1       Running   0          31m       172.17.0.7   minikube
```

我們有了最基本的 servcie discovery！但是保持同步且正確使用一組標籤可能會很棘手。這就是服務對像被創建的原因。 

### kube-proxy and Cluster IPs

Cluster IPs 是穩定的 virtual Ips，能夠 load-balance 流量到 service 裡的 endpoints。這是由 kube-proxy 所執行的，他運行在每個 node 裡。

`kube-proxy`會透過 API server watches 在 cluster 裡新的 service。cluster IP 是一個 iptables 會把近來流量的封包改寫成 service endpoint 的目標，如果 service 裡的 endpoint 目標改變（可能是 pod readiness check 失敗），那麼 iptable 的規則也會重寫。

當 service 建立的時候 API server 會指派 cluster IP 給自己。然而當建立 service 使用者可以指定 cluster IP。一旦被指派之後就不能再修改，除非刪除後再重新建立 service 物件。

Kubernetes service address range 是用在`kube-apiserver`裡的`--service-cluster-ip-range`的標籤所設定的。service address 的範圍不能跟 IP subnets 和 IP range 重疊。

此外任何明確的 cluster IP 必須從 service address range 來，而且不能是被使用的 cluster IP。

### Cluster IP Environment Variables

當 pod 開啟的時候會把環境變數注入進去，開啟網頁查看：

```shell
BANDICOOT_POD=$(kubectl get pods -l app=bandicoot \
  -o jsonpath='{.items[0].metadata.name}')
kubectl port-forward $BANDICOOT_POD 48858:8080
```

有一個問題是資源需要以特定順序建立，在 pod reference service 前 service 必須先建立。這在部署一組構成更大應用程序的服務時可能會帶來相當大的複雜性。另外，僅使用環境變量對許多用戶來說似乎很陌生。 出於這個原因，DNS 可能是更好的選擇。

### Cleanup

```shell
kubectl delete services,deployments -l app
```

## ReplicaSets

為什麼需要 replica

- Redundancy：越多的 instance 代表可以容忍越多的錯誤
- Scale：越多的 instance 代表可以處理更多的請求
- Sharding：不同的 replica 可以並行處理不同的部分

### Reconciliation Loops

- Desired state：ReplicaSet desired number of replicas
- Observed state：current state observed from the system

reconciliation loop 是持續觀察 current state，採去動作試圖讓 observed state 跟 desired stste 一樣。這樣有很多的好處，他本身會採取目標取向且自我修復的系統，還可只用幾行程式碼簡單的表達。

reconciliation loop 對於 ReplicaSets 來說只是一個單一的 loop。而且他可以處理使用者的動作 scale up 或 scale down ReplicaSet，以及 node 掛了或是有 node 重新加入 cluster。

### Pods 和 ReplicaSets 的關係

Kubernetes 的核心概念是相對於每個東西都是 modular，他們可以被其他的 conponents 替換或取代。ReplicaSets 和 Pods 的關係是 loosely coupled。雖然 ReplicaSets 可以建立且管理 Pods，但他並沒有擁有 Pods。ReplicaSets 使用 label query 去辨識這組它所管理的 Pods。ReplicaSets 同樣使用 Pod API，我們在第五章建立並管理 Pod 時所用的。在 Pods 和 ReplicaSets 之間的解偶是完全獨立的。

### Adopting Existing Containers

以前的要 replica container 是要先刪除舊有的 container 然後再開啟你要的數量。現在有了 ReplicaSets 可以無縫的從一台擴張到你要的數量。

### Quarantining Containers

當 server 有錯誤的行為時，Pod 層級的 health checks 會自動地重新啟動 Pod。但是如果 health checks 的功能沒有完成的話，Pod 會持續地執行錯誤的行為，仍然是 replicate set 裡的一部分。如果開發人員要 debug 的話，可以修改 Pod 的 label，讓他跟 ReplicaSet 或 Service 斷開關係，這時 ReplicaSet controller 會通知有一個 Pod 消失了，再開一個新的，但是 Pod 仍然會持續在運行，開發人員可以與他互動來 debug。

### ReplicaSets 的設計

在你的架構中，ReplicaSets 的設計代表著一個可拓展的 microservice。ReplicaSets 關鍵的角色是一組同質的 Pod。ReplicaSet 是給 stateless 的服務。

### ReplicaSet Spec

所有的 ReplicaSets 都有必須要有一個名字，被定義在`metadata.name`的欄位裡。`spec`的部分描述 Pods 的數量，`template`描述 Pod，跟先前的定義 Pod 一樣。

最後 ReplicaSet 會透過 Kubernetes API 用 Pod labels 去 query 現有的 Pod 數量，來達到 desired state。

```yaml
# kuard-rs.yaml
apiVersion: extensions/v1beta1
kind: ReplicaSet
metadata:
  name: kuard
spec:
  replicas: 1
  template:
    metadata:
      labels:
        app: kuard
        version: "2"
    spec:
      containers:
        - name: kuard
          image: "gcr.io/kuar-demo/kuard-amd64:2"
```

### 建立 ReplicaSet

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/8-1-kuard-rs.yaml
```

當提交 ReplicaSet 之後，ReplicaSet controller 會偵測 current state 是否符合 desired state，因為目前沒有任何的 Pod，所以將會建立一個新的 Pod。

#### Inspecting a ReplicaSet

```shell
$ kubectl describe rs kuard
Name:         kuard
Namespace:    default
Selector:     app=kuard,version=2
Labels:       app=kuard
              version=2
Annotations:  kubectl.kubernetes.io/last-applied-configuration={"apiVersion":"extensions/v1beta1","kind":"ReplicaSet","metadata":{"annotations":{},"name":"kuard","namespace":"default"},"spec":{"replicas":1,"templat...
Replicas:     1 current / 1 desired
Pods Status:  1 Running / 0 Waiting / 0 Succeeded / 0 Failed
Pod Template:
  Labels:  app=kuard
           version=2
  Containers:
   kuard:
    Image:        gcr.io/kuar-demo/kuard-amd64:2
    Port:         <none>
    Environment:  <none>
    Mounts:       <none>
  Volumes:        <none>
Events:
  Type    Reason            Age   From                   Message
  ----    ------            ----  ----                   -------
  Normal  SuccessfulCreate  4m    replicaset-controller  Created pod: kuard-tn246
```

#### 從 Pod 找 ReplicaSet

```shell
kubectl describe pods <pod-name>
```

#### 找一組 ReplicaSet 的 Pods

```shell
kubectl get pods -l app=kuard,version=2
```

### Scaling ReplicaSets

更新 ReplicaSets 的`spec.replica`可以達到 scale up or down。ReplicaSet 的物件儲存在 Kubernetes。當 Replicaset 被 scale up，新的 Pods 會透過定義在 ReplicaSet 裡的 Pod template 提交到 Kubernetes API。

#### Imperative Scaling with kubectl Scale

```shell
kubectl scale rs kuard --replicas=4
```

適合用來 demo 或是緊急快速的情況使用，重要的是之後記得要將 yaml 文件改成正確的數量。

舉例：Alice 在緊急的時候將 pod 數量從 5 增加到 10，使用的是`scale`的指令，後來則忘記改了設定檔。之後 Bob 將 server 更新，由於設定檔裡的數量仍然是 5，所以當 Bob 提交設定檔的時候，pod 的數量又會將回到原來的一半。

這描述了，任何的 imperative 的操作都會立即的改變，如果情況沒很嚴重，我們建議使用 declarative 的操作。

#### Declaratively Scaling with kubectl apply

```yaml
# kuard-rs.yaml
...
spec:
  replicas: 3
...
```

```shell
kubectl apply -f kuard-rs.yaml
```

多人操作的環境裡就會使用版本控制去檢查每一次的更動。

### Autoscaling a ReplicaSet

在某些情況下，您可能想要客製化的讓應用程式進行擴展。 Kubernetes 可以通過 horizontaol pod autoscaling (HPA) 處理所有這些情況。

HPA 要能正常運作前提是在你的 cluster 裡要存在一個`heapster`的 pod。`heapster`持續記錄所有的指標，並會提供一個 API 讓 HPA 來決定是否要 scaling。大多數的安裝預設都會包含`heapster`，可以查看`kube-system`的 pod 檢查：

```shell
kubectl get pods --namespace=kube-system
```

你應該會看到一個 Pod 的名字是`heapster`。如果沒有看到的話 autoscaling 就不會正常運作。

你可能會覺得奇怪，為什麼 Horizontal pod autoscaling 為什麼不簡單的稱做 autoscaling。Kubernetes 把他跟 horizontal scaling 和 vertical scaling 作區別。

- horizontal scaling：增加更多的 pod
- vertical scaling：增加更多的資源，像是 CPU 或 memory。

#### Autoscaling based on CPU

```shell
kubectl autoscale rs kuard --min=2 --max=5 --cpu-percent=80
```

要去查看、修改、刪除資源的話，可以使用`kubectl`指令，加上資源對象`horizontalpodautoscalers`，可以簡寫成`hpa`。

```shell
kubectl get horizontalpodautoscalers
kubectl get hpa
```

注意：由於 Kubernetes 的解偶特性，ReplicaSet 跟 pod autoscaler 並沒有關聯。他們可以很容易的模組化，也能夠使用一些 antipattern。像是 autoscaling 跟 imperative 或 declarative 的管理同時操作的話，很有可能會發生衝突，導致無法預期的行為。

### 刪除 ReplicaSets

要刪除 Pods 的話，透過刪除 ReplicaSet 來刪除。

```shell
kubectl delete rs kuard
kubectl get pods
```

如果你只想要刪除 ReplicaSet 物件，而不想刪除 Pods 的話，`--cascade=false`會確保你只會刪除 ReplicaSet 不是 Pods。

```shell
kubectl delete rs kuard --cascade=false
```

## DaemonSets

基於某些原因，有時候想要在每個 node 裡都跑同樣的 Pod，像是 logs collector 和 monitoring agent。

DaemonSet 跟 ReplicaSet 有一部分是相似的，他們都會建立 Pod，期望長期的運作，確保 desired state 跟 observed state 是相同的。

ReplicaSet 應該要使用在，你的應用程式跟你的 node 之前是完全沒有任何關聯，不用顧慮任何東西就能在一個 node 裡跑多個副本。

DaemonSet 在一個 node 裡跑一個副本，但是必須跑在所有的 node 上。

通常不會使用資源限制讓他有可能無法跑在 node 上。

### DaemonSet Scheduler

預設 DaemonSet 會建立一組 Pod 到每個節點上，除非某個節點上的 label 已經被使用了，他會盡量讓所有的 node 的 label 都符合一組 label。DaemonSet 是用 Pod spec 裡的`nodeName`欄位去辨識 Pod。最後，由 DaemonSet 所建立的 Pod 會被 Kubernetes scheduler 忽略。

DaemonSet 和 Replicaset 都是展現 Kubernetes 解偶架構的例子。Pod 是 top-level 的物件，任何所學的工具都適用於 Pod 運用。

### Createing DaemonSets

在 Kubernetes namepsace 下 DaemonSet 需要一個唯一的名字，`spec.template`則是用於創建 Pod 的設定。DaemonSet 預設會在每個 node 上建立 Pod，除非 label 的名字已經被使用了。

```yaml
# fluentd.yaml
apiVersion: extensions/v1beta1
kind: DaemonSet
metadata:
  name: fluentd
  namespace: kube-system
  labels:
    app: fluentd
spec:
  template:
    metadata:
      labels:
        app: fluentd
    spec:
      containers:
      - name: fluentd
        image: fluent/fluentd:v0.14.10
        resources:
          limits:
            memory: 200Mi
          requests:
            cpu: 100m
            memory: 200Mi
        volumeMounts:
        - name: varlog
          mountPath: /var/log
        - name: varlibdockercontainers
          mountPath: /var/lib/docker/containers
          readOnly: true
      terminationGracePeriodSeconds: 30
      volumes:
      - name: varlog
        hostPath:
          path: /var/log
      - name: varlibdockercontainers
        hostPath:
          path: /var/lib/docker/containers
```

[kubernetes playground](https://labs.play-with-k8s.com)

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/9-1-fluentd.yaml
```



```shell
$ kubectl get daemonset fluentd --namespace=kube-system
NAME      DESIRED   CURRENT   READY     UP-TO-DATE   AVAILABLE   NODE SELECTOR   AGE
fluentd   3         3         3         3            3           <none>          2m
```



```shell
$ kubectl describe daemonset fluentd --namespace=kube-system
Name:           fluentd
Selector:       app=fluentd
Node-Selector:  <none>
Labels:         app=fluentd
Annotations:    kubectl.kubernetes.io/last-applied-configuration={"apiVersion":"extensions/v1beta1","kind":"DaemonSet","metadata":{"annotations":{},"labels":{"app":"fluentd"},"name":"fluentd","namespace":"kube-system...
Desired Number of Nodes Scheduled: 3
Current Number of Nodes Scheduled: 3
Number of Nodes Scheduled with Up-to-date Pods: 3
Number of Nodes Scheduled with Available Pods: 3
Number of Nodes Misscheduled: 0
Pods Status:  3 Running / 0 Waiting / 0 Succeeded / 0 Failed
Pod Template:
  Labels:  app=fluentd
  Containers:
   fluentd:
    Image:  fluent/fluentd:v0.14.10
    Port:   <none>
    Limits:
      memory:  200Mi
    Requests:
      cpu:        100m
      memory:     200Mi
    Environment:  <none>
    Mounts:
      /var/lib/docker/containers from varlibdockercontainers (ro)
      /var/log from varlog (rw)
  Volumes:
   varlog:
    Type:  HostPath (bare host directory volume)
    Path:  /var/log
   varlibdockercontainers:
    Type:  HostPath (bare host directory volume)
    Path:  /var/lib/docker/containers
Events:
  Type    Reason            Age   From        Message
  ----    ------            ----  ----        -------
  Normal  SuccessfulCreate  1m    daemon-set  Created pod: fluentd-nm6lb
  Normal  SuccessfulCreate  1m    daemon-set  Created pod: fluentd-z52ks
  Normal  SuccessfulCreate  1m    daemon-set  Created pod: fluentd-qwtnb
```



```shell
$ kubectl get pods -o wide --namespace=kube-system
NAME               READY     STATUS              RESTARTS   AGE       IP            NODE
fluentd-nm6lb      1/1       Running             0          1m        10.46.0.1     node2
fluentd-qwtnb      1/1       Running             0          1m        10.40.0.1     node3
fluentd-z52ks      1/1       Running             0          1m        10.38.0.1     node4
```

馬上加入一個 node 到 cluster 裡

```shell
$ kubectl get pods -o wide --namespace=kube-system
NAME               READY     STATUS              RESTARTS   AGE       IP            NODE
fluentd-2shxv      0/1       ContainerCreating   0          9s        <none>        node5
fluentd-nm6lb      1/1       Running             0          3m        10.46.0.1     node2
fluentd-qwtnb      1/1       Running             0          3m        10.40.0.1     node3
fluentd-z52ks      1/1       Running             0          3m        10.38.0.1     node4
```

### Limiting DaemonSets to Specific Nodes

可能有些工作是需要 GPU 或是 SSD，只能在某些特定的 node 上運行。

#### Adding Labels to Nodes

先用 label 將 node 分類：

```shell
kubectl label nodes node2 ssd=true
kubectl label nodes node3 ssd=true
```



```shell
$ kubectl get nodes
NAME      STATUS    ROLES     AGE       VERSION
node1     Ready     master    17m       v1.10.2
node2     Ready     <none>    17m       v1.10.2
node3     Ready     <none>    17m       v1.10.2
node4     Ready     <none>    17m       v1.10.2
node5     Ready     <none>    12m       v1.10.2

$ kubectl get nodes --selector ssd=true
NAME      STATUS    ROLES     AGE       VERSION
node2     Ready     <none>    18m       v1.10.2
node3     Ready     <none>    18m       v1.10.2
```

#### Node Selector

Node Selector 可以被用來限制 Pod 可以跑在哪些 node 上。在`spec.template`下的`spec.nodeSelector`限制：

```yaml
# nginx-fast-storage.yaml
apiVersion: extensions/v1beta1
kind: "DaemonSet"
metadata:
  labels:
    app: nginx
    ssd: "true"
  name: nginx-fast-storage
spec:
  template:
    metadata:
      labels:
        app: nginx
        ssd: "true"
    spec:
      nodeSelector:
        ssd: "true"
      containers:
        - name: nginx
          image: nginx:1.10.0
```



```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/9-2-nginx-fast-storage.yaml
```

因為只有兩個 node 有`ssd=true`的標籤，所以 Pod 只會跑在兩個 node 上。

```shell
$ kubectl get pods -o wide
NAME                       READY   STATUS              RESTARTS   AGE       IP          NODE
nginx-fast-storage-6jrhg   0/1     ContainerCreating   0          10s       <none>      node2
nginx-fast-storage-b5dbw   1/1     Running             0          10s       10.40.0.2   node3
```

只要馬上將`ssd=true`的標籤加在某個 node 上的話，Pod 就會立即部署到那個 node。相反來說如果標籤從 node 被移除的話，Pod 就會被 DaemonSet controller 移除。

```shell
$ kubectl label node node4 ssd=true
node "node4" labeled
$ kubectl get pods -o wide
NAME                       READY   STATUS              RESTARTS   AGE       IP          NODE
nginx-fast-storage-445j7   0/1     ContainerCreating   0          5s        <none>      node4
nginx-fast-storage-6jrhg   1/1     Running             0          4m        10.46.0.2   node2
nginx-fast-storage-b5dbw   1/1     Running             0          4m        10.40.0.2   node3
```



```shell
$ kubectl label node node2 ssd-
node "node2" labeled
$ kubectl get pods -o wide
NAME                       READY    STATUS             RESTARTS   AGE       IP          NODE
nginx-fast-storage-445j7   1/1      Running            0          52s       10.38.0.2   node4
nginx-fast-storage-6jrhg   0/1      Terminating        0          5m        10.46.0.2   node2
nginx-fast-storage-b5dbw   1/1      Running            0          5m        10.40.0.2   node3
```

### Updating a Daemonset

在 Kubernetes 1.6 之前，更新 DaemonSet 管理的 Pod 唯一的方法是更新 DaemonSet，然後手動刪除由 DaemonSet 管理的每個 Pod，以便套用新的設定重新創建 Pod。Kubernetes 1.6 以後可以透過 Deployment 物件去更新 DaemonSet 的 rollout。

#### Updating a DaemonSet by Deleting Individual Pods

每 60 秒依序地刪除 DaemonSet 的 Pod：

```bash
PODS=$(kubectl get pods -o jsonpath --template='{.items[*].metadata.name}')
for x in $PODS; do
  kubectl delete pods ${x}
  sleep 60
done
```

更簡單的方法就是刪除 DaemonSet 然後再重新建立 DaemonSet，但缺點就是會停機。

#### Rolling Update of a DaemonSet

Kubernetes 1.6 之後，可以使用相同 Deployment 所更新的方式來更新，要讓 DaemonSet 使用 rolling update 的策略的話，要在`spec.updateStrategy.type`裡設定`RollingUpdate`。當 DaemonSet 的`spec.template`有任何改變的話，DaemonSet 會執行 rolling update。

```shell
kubectl edit daemonsets nginx-fast-storage
```

Rolling update 策略會逐漸更新 DaemonSet 的數量，直到所有的 Pod 都是新的設定為止，有兩個參數用來控制 DaemonSet 的 rolling update：

- `spec.minReadySeconds`：在執行 rolling update 的過程時，新的 Pod 必須要 Ready 多久。
- `spec.updateStrategy.rollingUpdate.maxUnavailable`：有多少個 Pod 可以同時 rolling update。

你可能會想要設定`spec.minReadySeconds`，可能 30-60 秒，確保在 rolling update 前你的 Pod 是真的健康的。

`spec.updateStrategy.rollingUpdate.maxUnavailable`決定你更新的速度 ( nodes 數量 x `maxReadySeconds`)。增加的話可以更新更快，不過當失敗的時候也會增加爆炸半徑，這個值決定了速度與安全。最好的做法是設成 1，當有人抱怨更新太慢的時候再把它調高。

一但你開始更新，你可以使用`kubectl rollout`指令查看現在 DaemonSet 更新的狀態。

```shell
$ kubectl rollout status ds nginx-fast-storage
Waiting for rollout to finish: 0 out of 2 new pods have been updated...
Waiting for rollout to finish: 0 out of 2 new pods have been updated...
Waiting for rollout to finish: 0 out of 2 new pods have been updated...
Waiting for rollout to finish: 0 out of 2 new pods have been updated...
Waiting for rollout to finish: 1 out of 2 new pods have been updated...
Waiting for rollout to finish: 1 out of 2 new pods have been updated...
Waiting for rollout to finish: 1 out of 2 new pods have been updated...
Waiting for rollout to finish: 1 of 2 updated pods are available...
daemon set "nginx-fast-storage" successfully rolled out
```

### Deleting a DaemonSet

使用`kubectl delete`刪除 DaemonSet 是非常奇怪的事。但只需要指定你想要刪除 DaemonSet 的名字。

```shell
kubectl delete -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/9-2-nginx-fast-storage.yaml
```

如果只想要刪除 DaemonSet，而不刪除 DaemonSet 底下的 Pod 的話，需要加上`--cascade=false`的 flag。

## Jobs

Kubernetes cluster 大多的工作量都是在跑長期的程式，如果只需要短期、單一的工作，這就需要用到`Job`物件來處理。一個`Job`會建立 Pods 跑成功後停止，`Job`很適合用來只執行一次的東西，像是資料庫 migration 或是批量的 jobs。如果跑在一般的 Pod 你的任務會一直重複的跑，成功之後退出後再建立新的 Pod。

### Job 物件

如果運行失敗，Job controller 會建立一個新的 Job pod template 繼續運行。Job 必須安排 Pod，如果沒有找到特定的資源，Job 有可能不會被執行。另外，由於分佈式系統的特性，在某些故障情況下，有小機率會為特定任務創建重複的 Pod。

#### Job Patterns

預設每個 Job 只會跑在一個 Pod 上，一但成功就會終止。Job Patterns 由兩個參數來定義

- `completions`：Job 完成的數量
- `parallelism`：Job 並行的數量

舉例：執行一個 Pod 直到成功，會把`completions`跟`parallelism`都設成1。

| Type                       | Use case                                               | Behavior                                                     | completions | parallelism |
| -------------------------- | ------------------------------------------------------ | ------------------------------------------------------------ | ----------- | ----------- |
| One shot                   | Database migrations                                    | A single pod running once until successful termination       | 1           | 1           |
| Parallel fixed completions | Multiple pods processing a set of work in parallel     | One or more Pods running one or more times until reaching a fixed completion count | 1+          | 1+          |
| Work queue: parallel Jobs  | Multiple pods processing from a centralized work queue | One or more Pods running once until successful termination   | 1           | 2+          |







## Reference

[Kubernetes Up and Running Example Github](https://github.com/kubernetes-up-and-running/examples)