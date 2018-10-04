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

### One Shot

跑在一個 Pod 且必須成功一次後終止，如果失敗的話 Job 會再重新建立 Pod 直到成功為止。

有很多方法可以建立 one shot Job，最簡單的是使用`kubectl`指令：

```shell
$ kubectl run -i oneshot \
  --image=gcr.io/kuar-demo/kuard-amd64:1 \
  --restart=OnFailure \
  -- --keygen-enable \
     --keygen-exit-on-complete \
     --keygen-num-to-gen 10
If you don't see a command prompt, try pressing enter.
2018/05/29 16:02:41 (ID 0 2/10) Item done: SHA256:8MMtZENDLSCyfzKtzo6/nK06fUxvINufZPU9Qe8hpnc
2018/05/29 16:02:43 (ID 0 3/10) Item done: SHA256:LafQAc5ePLZEpSl2A2davBHmyszW0wF+Kr53O5JqIyo
2018/05/29 16:02:51 (ID 0 4/10) Item done: SHA256:qDZcCxjpVXZcBffF2DJqGVwRhwDfyLN0/U+vhDbv4Cs
2018/05/29 16:02:54 (ID 0 5/10) Item done: SHA256:tvMKs7NJQFqtgXzkPBrLz6cFg78dm6jKTc4r1vCyAFY
2018/05/29 16:02:57 (ID 0 6/10) Item done: SHA256:8cUlRwT0+eLm7EwYvNeSkvSufqCNLvdRpxKWd9XNu54
2018/05/29 16:03:00 (ID 0 7/10) Item done: SHA256:tZzNr+Xjg6AA6XT5KHdl7OIeHUyfEGcDLcSQxbINih4
2018/05/29 16:03:09 (ID 0 8/10) Item done: SHA256:jkgVH3SrDOZuxs7Z4ZgXJWVRPUW8uHf+/pD9qqO2vrU
2018/05/29 16:03:10 (ID 0 9/10) Item done: SHA256:+zu4ljBKQov99VR7kpII0TyQJ7gkTAMM7eln4uVhMGg
2018/05/29 16:03:11 (ID 0 10/10) Item done: SHA256:d98hDVAb6CQ6GZmHoKFXYuf9ckIwQfXXAKkEmt/hwZo
2018/05/29 16:03:11 (ID 0) Workload exiting
```

- `-i`參數代表這是一個互動的指令。`kubectl`會等到 Job 跑完，然後印出 Job 裡第一個 Pod  的 log。
- `--restart=OnFailure`告訴`kubectl`建立 Job 的選項。
- 所有的指令在這個`--`都會變成指令的參數傳進 container 中，我們會用來測試產生 10 組 4096 bit 的 SSH key 然後離開。
- `kubectl`加上`-i`通常會把第一行 output 吃掉。

Job 跑完之後會繼續留著，所以可以直接看 Pod 跟 Job 的 log。

```shell
$ kubectl get jobs
NAME      DESIRED   SUCCESSFUL   AGE
oneshot   1         1            21m
```

刪除 Job

```shell
kubectl delete jobs oneshot
```

其他建立 one-shot Job 的方法是使用設定檔：

```yaml
# job-oneshot.yaml
apiVersion: batch/v1
kind: Job
metadata:
  name: oneshot
  labels:
    chapter: jobs
spec:
  template:
    metadata:
      labels:
        chapter: jobs
    spec:
      containers:
      - name: kuard
        image: gcr.io/kuar-demo/kuard-amd64:1
        imagePullPolicy: Always
        args:
        - "--keygen-enable"
        - "--keygen-exit-on-complete"
        - "--keygen-num-to-gen=10"
      restartPolicy: OnFailure
```

建立 one-shot Job

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/10-1-job-oneshot.yaml
```



```shell
$ kubectl describe jobs oneshot
Name:           oneshot
Namespace:      default
Selector:       controller-uid=cb0d863c-635d-11e8-b926-0242ac110046
Labels:         chapter=jobs
Annotations:    kubectl.kubernetes.io/last-applied-configuration={"apiVersion":"batch/v1","kind":"Job","metadata":{"annotations":{},"labels":{"chapter":"jobs"},"name":"oneshot","namespace":"default"},"spec":{"templat...
Parallelism:    1
Completions:    1
Start Time:     Tue, 29 May 2018 16:31:52 +0000
Pods Statuses:  0 Running / 1 Succeeded / 0 Failed
Pod Template:
  Labels:  chapter=jobs
           controller-uid=cb0d863c-635d-11e8-b926-0242ac110046
           job-name=oneshot
  Containers:
   kuard:
    Image:      gcr.io/kuar-demo/kuard-amd64:1
    Port:       <none>
    Host Port:  <none>
    Args:
      --keygen-enable
      --keygen-exit-on-complete
      --keygen-num-to-gen=10
    Environment:  <none>
    Mounts:       <none>
  Volumes:        <none>
Events:
  Type    Reason            Age   From            Message
  ----    ------            ----  ----            -------
  Normal  SuccessfulCreate  3m    job-controller  Created pod: oneshot-54k6q
```



```shell
$ kubectl logs oneshot-54k6q
2018/05/29 16:31:54 Starting kuard version: v0.7.2-1
2018/05/29 16:31:54 **********************************************************************
2018/05/29 16:31:54 * WARNING: This server may expose sensitive
2018/05/29 16:31:54 * and secret information. Be careful.
2018/05/29 16:31:54 **********************************************************************
2018/05/29 16:31:54 Config:
{
  "address": ":8080",
  "debug": false,
  "debug-sitedata-dir": "./sitedata",
  "keygen": {
    "enable": true,
    "exit-code": 0,
    "exit-on-complete": true,
    "memq-queue": "",
    "memq-server": "",
    "num-to-gen": 10,
    "time-to-run": 0
  },
  "liveness": {
    "fail-next": 0
  },
  "readiness": {
    "fail-next": 0
  },
  "tls-address": ":8443",
  "tls-dir": "/tls"
}
2018/05/29 16:31:54 Could not find certificates to serve TLS
2018/05/29 16:31:54 Serving on HTTP on :8080
2018/05/29 16:31:54 (ID 0) Workload starting
2018/05/29 16:31:58 (ID 0 1/10) Item done: SHA256:aLWzOXlZCNE/dYI0DFLjcqs/LCQjveVO5LXlmtZGGFI
2018/05/29 16:32:00 (ID 0 2/10) Item done: SHA256:yzqyIMeYxzW9KVik0cDZyrbAM6KOhJIeoH7KdUmAWKg
2018/05/29 16:32:02 (ID 0 3/10) Item done: SHA256:fyJFBJUOxb6NY/Q7rUeB4uqW15rdeWqBrTizmZ+MBck
2018/05/29 16:32:07 (ID 0 4/10) Item done: SHA256:SFZpmwHucyndvHgFrS33bHQ9lgfIEJGIKdCdHkSGqNc
2018/05/29 16:32:15 (ID 0 5/10) Item done: SHA256:3oGg0HvwQ6VGHkE8SofYZ/5FWUJFTqPh0ugzucM2+7Y
2018/05/29 16:32:17 (ID 0 6/10) Item done: SHA256:5R7ACsDP/MGoOs6zlA50TN3f6/oUqFovlre5H8zIZ5M
2018/05/29 16:32:20 (ID 0 7/10) Item done: SHA256:j0EiYAlvKKIL3m+OmY3UbJCC3mCNsKk4P2vroTRgv68
2018/05/29 16:32:25 (ID 0 8/10) Item done: SHA256:dVOFr/6Fz6wiZuqid81v76AYq4C/EKubWGAXWJmBmF4
2018/05/29 16:32:28 (ID 0 9/10) Item done: SHA256:vfyPUJNw01wcOdlShYkd62D5ijybp5a03XTjDVBIJNM
2018/05/29 16:32:33 (ID 0 10/10) Item done: SHA256:Yw6bynADWnRfa5CVcNX/DPU5WhxvmFmeqaOgfiWJfQA
2018/05/29 16:32:33 (ID 0) Workload exiting
```

Job 一樣跟 DaemonSet ReplicaSet deployment 也是使用 label 來標示。因為 Job 是有限的，會從開始到結束，通常會讓使用者建立很多，如果要建立一個唯一的 label 會變得更困難且危險，因為這樣， Job 物件會隨機挑選一個唯一的 label 然後用它去辨識一個已經建立的 Pod。在這種場景中，使用者可以選擇手動標示 label 或 selector，像是交換正在運行的 Job 而不殺死它正在管理的 Pod。

#### Pod failure

如果 Pod 執行任務失敗的話：

```yaml
# job-oneshot-failure1.yaml
apiVersion: batch/v1
kind: Job
metadata:
  name: oneshot
  labels:
    chapter: jobs
spec:
  template:
    metadata:
      labels:
        chapter: jobs
    spec:
      containers:
      - name: kuard
        image: gcr.io/kuar-demo/kuard-amd64:1
        imagePullPolicy: Always
        args:
        - "--keygen-enable"
        - "--keygen-exit-on-complete"
        - "--keygen-exit-code=1"
        - "--keygen-num-to-gen=3"
      restartPolicy: OnFailure
```



```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/10-2-job-oneshot-failure1.yaml
```



```shell
$ kubectl get pods -l job-name=oneshot
NAME            READY     STATUS             RESTARTS   AGE
oneshot-jqx8b   0/1       CrashLoopBackOff   2          1m
```

Pod 失敗後會重新啟動，通常是在某個地方有 bug 造成的，Kubernetes 在重新啟動前會先等一小段時間，避免重複的啟動吃掉所有的資源，這個全部都是由`kubelet`所處理的，Job 物件不會涉入其中。

刪除任務，然後把`restartPolicy`欄位的`OnFailure`值改成`Never`，然後再跑一次。

```yaml
# jobs-oneshot-failure2.yaml
...
spec:
  template:
    spec:
      restartPolicy: Never
```



```shell
kubectl delete jobs oneshot
kubectl apply -f jobs-oneshot-failure2.yaml
```

```shell
$ kubectl get pods -l job-name=oneshot
NAME            READY     STATUS    RESTARTS   AGE
oneshot-486np   1/1       Running   0          5s
oneshot-6ccnv   0/1       Error     0          23s
oneshot-rpgkl   0/1       Error     0          32s
```

當他發生錯誤的時候我們會看到很多重複的 Pod。藉由設定`restartPolicy: Never`我們會告訴`kubectl`不要在失敗的時候重新啟動 Pod，而是宣稱那個 Pod 執行失敗。Job 物件注意到之後就會立即建立一個新的 Pod 來頂替失敗的 Pod。如果你不關心的話，可能會在你 cluster 裡產生很多垃圾的 Pod。所以因為這樣，我們建議你使用`restartPolicy: OnFailure`。

```shell
kubectl delete jobs oneshot
```

但是 Job 失敗可能會有很多種方式，像是他跑到一半的時候卡住了，要解決這個問題可以在 Job 裡的 Pod 搭配 liveness probes 一起使用，如果 liveness probe 認定你的 Pod 掛了，他就會幫你取代或是重新啟動 Pod。

### Parallelism

產生 key 的速度很慢，所以用平行化產生大量的 key，目標產生 100 組 key，每個 Pod 會產生 10 組，我們可以設定`completions`為 10 和`parallelism`為 5。

```yaml
# job-parallel.yaml
apiVersion: batch/v1
kind: Job
metadata:
  name: parallel
  labels:
    chapter: jobs
spec:
  parallelism: 5
  completions: 10
  template:
    metadata:
      labels:
        chapter: jobs
    spec:
      containers:
      - name: kuard
        image: gcr.io/kuar-demo/kuard-amd64:1
        imagePullPolicy: Always
        args:
        - "--keygen-enable"
        - "--keygen-exit-on-complete"
        - "--keygen-num-to-gen=10"
      restartPolicy: OnFailure
```

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/10-3-job-parallel.yaml
```

使用`--watch`的標籤去觀察，Pod 會一直建立直到有 10 為止

```shell
kubectl get pods -w
```

Clean up

```shell
kubectl delete job parallel
```

### Work Queues

有個任務會建立很多的 work items 然後推到 work queue 裡面，work Job 會執行裡面的 work 直到 work quere 是空的為止。

#### Starting a work queue

我們會建立集中式的 work queue 的服務，`kuard`有內建一個簡單的 memory-base work queue，我們會建立一個簡單的 kuard instance 去協調所有的 work。

建立一個簡單的 ReplicaSet 去管理 work queue daemon，即使機器故障也會確保新的 Pod 會被建立。

```yaml
# rs-queue.yaml
apiVersion: extensions/v1beta1
kind: ReplicaSet
metadata:
  labels:
    app: work-queue
    component: queue
    chapter: jobs
  name: queue
spec:
  replicas: 1
  template:
    metadata:
      labels:
        app: work-queue
        component: queue
        chapter: jobs
    spec:
      containers:
      - name: queue
        image: "gcr.io/kuar-demo/kuard-amd64:1"
        imagePullPolicy: Always
```



```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/10-4-rs-queue.yaml
```

在這個時間點 work queue daemon 應該已經建立且在運行，我們使用 port-forwarding 去連接他：

```shell
QUEUE_POD=$(kubectl get pods -l app=work-queue,component=queue -o jsonpath='{.items[0].metadata.name}')
kubectl port-forward $QUEUE_POD 8080:8080
```

開啟網頁然後選擇 MemQ Server，接下來，我們需要用 service expose work queue server，這會讓 producer 跟 consumer 透過 DNS 分配 work queue 變得更簡單：

```yaml
# service-queue.yaml
apiVersion: v1
kind: Service
metadata:
  labels:
    app: work-queue
    component: queue
    chapter: jobs
  name: queue
spec:
  ports:
  - port: 8080
    protocol: TCP
    targetPort: 8080
  selector:
    app: work-queue
    component: queue
```



```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/10-5-service-queue.yaml
```

#### Loading up the queue

我們使用 curl 驅動 API 到 work queue server 然後建立大量的 work items，curl 會使用`kubectl port-forward`所開的連線。

```bash
# Create a work queue called 'keygen'
curl -X PUT localhost:8080/memq/server/queues/keygen

# Create 100 work items and load up the queue.
for i in work-item-{0..99}; do
  curl -X POST localhost:8080/memq/server/queues/keygen/enqueue \
    -d "$i"
done
```



```shell
curl -s https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/10-6-load-queue.sh | bash
```

跑完之後應該看到 100 個 json 的物件，包含每一個 work item 的 id 訊息。

```json
{ 
    "kind": "message",
    "id": "0c3a31d0541d62463a8e9c23af89f0a5",
    "body": "work-item-0",
    "creationTimestamp": "2018-05-30T08:40:18.242408766Z"
}
```

可以直接在 kuard 的 MemQ Server 頁面裡面看到，或是直接使用 API：

```shell
$ curl 127.0.0.1:8080/memq/server/stats
{
  "kind": "stats",
  "queues": [
    {
      "name": "keygen",
      "depth": 100,
      "enqueued": 100,
      "dequeued": 0,
      "drained": 0
    }
  ]
}
```

現在我們準備要開始建立一個 Job 來 consume work queue。

#### Createing the consume job

kuard 能夠和 consumer mode 配和，且能夠畫出 work items 在 work queue、建立 key 然後當 queue 空的時候會結束：

```yaml
# job-consumers.yaml
apiVersion: batch/v1
kind: Job
metadata:
  labels:
    app: message-queue
    component: consumer
    chapter: jobs
  name: consumers
spec:
  parallelism: 5
  template:
    metadata:
      labels:
        app: message-queue
        component: consumer
        chapter: jobs
    spec:
      containers:
      - name: worker
        image: "gcr.io/kuar-demo/kuard-amd64:1"
        imagePullPolicy: Always
        args:
        - "--keygen-enable"
        - "--keygen-exit-on-complete"
        - "--keygen-memq-server=http://queue:8080/memq/server"
        - "--keygen-memq-queue=keygen"
      restartPolicy: OnFailure
```

我們告訴 Job 能夠同時運行 5 個 Pod，因爲`completions`參數沒有設定，我們會把 Job 放到 worker pool mode。一旦第一個 Pod 正常離開的話， Job 就不會再開始任何新的 Pod。也就是，worker 只能在沒有工作的時候才能離開。

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/10-7-job-consumers.yaml
```

```shell
$ kubectl get pods
NAME              READY     STATUS    RESTARTS   AGE
consumers-2pq7j   1/1       Running   0          27s
consumers-67cqq   1/1       Running   0          27s
consumers-9vxqx   1/1       Running   0          27s
consumers-m5tm4   1/1       Running   0          27s
consumers-pqbgz   1/1       Running   0          27s
queue-x28zl       1/1       Running   0          51m
$ curl 127.0.0.1:8080/memq/server/stats
{
  "kind": "stats",
  "queues": [
    {
      "name": "keygen",
      "depth": 92,
      "enqueued": 100,
      "dequeued": 8,
      "drained": 0
    }
  ]
}
```

這五個 Pod 會同時並行的運行，直到 work queue 是空的為止。當 queue 空了的時候，consumer pods 將會離開，而且`consumers` Job 會被認為成功。成功之後再推 work items 進去就不會有動作了。

#### Cleaning up

```shell
kubectl delete rs,svc,job -l chapter=jobs
```

## ConfigMaps and Secrets

如果需要為每個新環境重新創建映像，測試和版本控制就會變得更加風險和復雜。但是，我們如何在運行時專門使用該映像？

這就是為什麼 ConfigMaps 和 secrets 的出現。ConfigMaps 被使用來提供設定工作的資訊，可能是很短的訊息或是一個檔案包含很多值。Secrets 很相似 ConfigMaps，但是會把敏感的訊息給遮住。兩個都可以使用像是 credentials 或是 TLS certificates。

### ConfigMaps

可以想像 ConfigMaps 是一個 Kubernetes 的物件，定義在一個小的檔案裡面。另一個方法是使用環境變數或是指令的參數。關鍵是在跑 Pod 之前他必須要跟 ConfigMap 綁定，這意味著定義 Pod 的設定檔可以被重複使用，只需要改變 ConfigMap 的檔案就行。

#### 建立 ConfigMaps

先使用命令式的方法

```txt
# my-config.txt
# This is a sample config file that I might use to configure an application
parameter1 = value1
parameter2 = value2
```

我們建立檔案內的 ConfigMaps 然後再新增幾個參數：

```shell
kubectl create configmap my-config \
  --from-file=my-config.txt \
  --from-literal=extra-param=extra-value \
  --from-literal=another-param=another-value
```

```shell
$ kubectl get configmaps my-config -o yaml
apiVersion: v1
data:
  another-param: another-value
  extra-param: extra-value
  my-config.txt: |
    # This is a sample config file that I might use to configure an application
    parameter1 = value1
    parameter2 = value2
kind: ConfigMap
metadata:
  creationTimestamp: 2018-05-31T03:29:57Z
  name: my-config
  namespace: default
  resourceVersion: "2607537"
  selfLink: /api/v1/namespaces/default/configmaps/my-config
  uid: e483913b-6482-11e8-b4d0-0800273680e4
```

ConfigMap 就像是一些鍵值對的數據存在物件裡。

#### Using a ConfigMap

有三種主要方式來使用 ConfigMap：

- Filesystem：你可以掛載一個 ConfigMap 到 Pod 上，那個檔案會紀錄每個 key 的條目，會把值設成該檔案的內容。
- Environment variable：可以在 ConfigMap 裡動態設定環境變數。
- Command-line argument：對於 ConfigMap 值上的 Container，Kubernetes 支援動態建立 command line。

全部一起建立：

```yaml
# kuard-config.yaml
apiVersion: v1
kind: Pod
metadata:
  name: kuard-config
spec:
  containers:
    - name: test-container
      image: gcr.io/kuar-demo/kuard-amd64:1
      imagePullPolicy: Always
      command:
        - "/kuard"
        - "$(EXTRA_PARAM)"
      env:
        - name: ANOTHER_PARAM
          valueFrom:
            configMapKeyRef:
              name: my-config
              key: another-param
        - name: EXTRA_PARAM
          valueFrom:
            configMapKeyRef:
              name: my-config
              key: extra-param
      volumeMounts:
        - name: config-volume
          mountPath: /config
  volumes:
    - name: config-volume
      configMap:
        name: my-config
  restartPolicy: Never
```

對於 Filesystem 的方法來說，我們建立一個新的 volume 在 Pod 裡面，然後命名為`config-volume`，接著我們定義這個 volume 叫做 ConfigMap volume，然後把它指到 ConfigMap 掛載上去，我們必須用`volumeMount`註明他要掛到`kuard`container 的哪裡，這個例子是掛載到`/config`上。

Environment variable 由特定的 valueFrom 指定，可以使用 ConfigMap 裡的資料。

Command-line argument 建立在 Environment variable，Kubernetes 會使用`$(<env-var-name>)`語法替換。

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/11-2-kuard-config.yaml
kubectl port-forward kuard-config 8080
```

點開 Server Env 的標籤，可以看到這兩個環境變數，原本設定在 ConfigMap 裡的。另外還在指令裡面傳入`EXTRA_PARAM`的參數。

```shell
/kuard extra-value
```

| Key           | Value         |
| ------------- | ------------- |
| ANOTHER_PARAM | another-value |
| EXTRA_PARAM   | extra-value   |



另外點選 File system brower，可以看到裡面有個資料夾叫做`/config`，每個 entry 都會是一個檔案，你還會看到一些隱藏的文件，用於在更新 ConfigMap 時乾淨交換新的值。

### Secrets

ConfigMap 用來紀錄大多數設定的資料，但是某些數據是非常敏感的，可能包含密碼、token 或其他 private key，總體來說，我們稱這些資料叫做 Secret。Kubernets 支援儲存這些資料並妥善地處理它。

Secret 能夠讓 container images 建立的時候不需要綁定敏感的資料，讓 container 能保留重複使用的特性，透過在 Pod manifests 或 Kubernetes API 明確的聲明，能夠讓 Secrets expose 在 Pod 中。Kubernetes secretes API 提供了以應用程式為中心的機制，用來 expose 敏感的資料到應用程式，這樣比較容易用來隔離本身的 OS 系統。

#### 建立 Secrets

可以使用 Kubernetes API 或`kubectl`來建立，Secrets 會存放一筆以上的資料，格式是鍵值對。

在這個章節我們會建立存放 TLS key andcertificate。

`kuard`容器印象檔不會把 TLS certificate or key 綁在一起。這樣的好處是，能夠讓`kuard`印象檔推到公開的 Docker repository。

```shell
curl -O https://storage.googleapis.com/kuar-demo/kuard.crt
curl -O https://storage.googleapis.com/kuar-demo/kuard.key
```

建立一個 secret 名字是`kuard-tls`使用`create secret`指令：

```shell
kubectl create secret generic kuard-tls \
  --from-file=kuard.crt \
  --from-file=kuard.key
```

```shell
$ kubectl describe secrets kuard-tls
Name:         kuard-tls
Namespace:    default
Labels:       <none>
Annotations:  <none>

Type:  Opaque

Data
====
kuard.crt:  1050 bytes
kuard.key:  1679 bytes
```

用 Pod 的 secrets volume 可以來消耗`kuard-tls`secret。

#### Consuming Secrets

可以使用 Kubernetes REST API 來消耗 Secrets，藉由應用程式知道要如何去直接呼叫 API。然而我們的目標是要保持應用程式可攜帶，不只在 Kubernetes 而且可以在其他平台不用修改的運行。

不只有 API server 可以使用，我們可以使用 secrets volume。

##### Secrets volumes

可以使用 secrets volume type 來暴露 Secret data，Secrets volumes 會由`kubelet`管理，而且在 Pod 建立的時候建立 secrets。Secrets 會被存在 tmpfs volumes (又稱作 RAM disk)，他不會寫到磁碟中。

會把 Secret `kuard-tls`掛載`/tls`上，裡面包含兩個案 kuard.crt kaurd.key。

```yaml
# kuard-secret.yaml
apiVersion: v1
kind: Pod
metadata:
  name: kuard-tls
spec:
  containers:
    - name: kuard-tls
      image: gcr.io/kuar-demo/kuard-amd64:1
      imagePullPolicy: Always
      volumeMounts:
      - name: tls-certs
        mountPath: "/tls"
        readOnly: true
  volumes:
    - name: tls-certs
      secret:
        secretName: kuard-tls
```



```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/11-3-kuard-secret.yaml
kubectl port-forward kuard-tls 8443:8443
```

瀏覽器連上 https://localhost:8443/ 可以看到 invalid certificate warning，連線進去後，點選 File system brower 可以看到 certificate 在磁碟上。

#### Private Docker Registries

另一種就是把 secrets 存在 private Docker registry，要抓取的時候需要認證，私人的映像檔可以存在多個私人的 registry，這對於管理來說是一個挑戰。

Image pull secrets 存放到 secrets API 會自動分配 private registry credential。Image pull secrets 儲存的方法就像一班的 secrets 一樣，可以在 Pod 的設定檔中的`spec.imagePulSecrets`去使用它。

```shell
kubectl create secret docker-registry my-image-pull-secret \
  --docker-username=<username> \
  --docker-password=<password> \
  --docker-email=<email-address>
```

```yaml
# kuard-secret-ips.yaml
apiVersion: v1
kind: Pod
metadata:
  name: kuard-tls
spec:
  containers:
    - name: kuard-tls
      image: gcr.io/kuar-demo/kuard-amd64:1
      imagePullPolicy: Always
      volumeMounts:
      - name: tls-certs
        mountPath: "/tls"
        readOnly: true
  imagePullSecrets:
  - name:  my-image-pull-secret
  volumes:
    - name: tls-certs
      secret:
        secretName: kuard-tls
```

### Naming Constraints

定義在 ConfigMap 和 Secret 裡的 Key 必須遵守下面的格式：

```
[.]?[a-zA-Z0-9]([.]?[-_a-zA-Z0-9]*[a-zA-Z0-9])*
```

ConfigMaps 沒辦法儲存 binary data，Secret data 可以儲存任何用 base64 加密的資料，用 base64 加密後能夠儲存 binary data，但是這會增加管理的難度，要把密碼存在 yaml 檔中。

### Managing ConfigMaps and Secrets

Secrets 和 ConfigMaps 一樣都可以使用`create`,`delete`,`get`,`describe`指令去操作物件。

#### Listing

```shell
$ kubectl get secrets
NAME                  TYPE                                  DATA      AGE
default-token-kvbsd   kubernetes.io/service-account-token   3         88d
kuard-tls             Opaque                                2         1h
```

```shell
$ kubectl get configmaps
NAME        DATA      AGE
my-config   3         8h
```

```shell
$ kubectl describe configmap my-config
Name:         my-config
Namespace:    default
Labels:       <none>
Annotations:  <none>

Data
====
another-param:
----
another-value
extra-param:
----
extra-value
my-config.txt:
----
# This is a sample config file that I might use to configure an application
parameter1 = value1
parameter2 = value2

Events:  <none>
```

使用下面的指令，你可以看到 secret 的資料！

```shell
$ kubectl get configmap my-config -o yaml
apiVersion: v1
data:
  another-param: another-value
  extra-param: extra-value
  my-config.txt: |
    # This is a sample config file that I might use to configure an application
    parameter1 = value1
    parameter2 = value2
kind: ConfigMap
metadata:
  creationTimestamp: 2018-05-31T03:29:57Z
  name: my-config
  namespace: default
  resourceVersion: "2607537"
  selfLink: /api/v1/namespaces/default/configmaps/my-config
  uid: e483913b-6482-11e8-b4d0-0800273680e4
```

```shell
$ kubectl get secret kuard-tls -o yaml
apiVersion: v1
data:
  kuard.crt: LS0tLS1C...tLS0K
  kuard.key: LS0tLS1C...tLQo=
kind: Secret
metadata:
  creationTimestamp: 2018-05-31T10:28:08Z
  name: kuard-tls
  namespace: default
  resourceVersion: "2622884"
  selfLink: /api/v1/namespaces/default/secrets/kuard-tls
  uid: 4f9f2928-64bd-11e8-b4d0-0800273680e4
type: Opaque
```

#### Creating

最簡單建立 Secret 和 ConfigMap 是透過

```shell
kubectl create secret generic
kubectl create configmap
```

有很多方法把資料傳進 Secret 或 ConfigMap：

- `--from-file=<filename>`：從檔案載入，key 的名稱會跟檔案相同。
- `--from-file=<key>=<filename>`：從檔案仔入，可以明確定義 key 的名稱。
- `--from-file=<directory>`：載入整個資料夾底下的所有檔案，key 的名稱跟檔案名稱想同。
- `--from-literal=<key>=<value>`：直接指定 key 跟 value 的值。

#### Updating

在正在運行的程式上，你可以更新 ConfigMap 或 Secret，不需要重新啟動應用程式，application 會重新讀取新的設定值，這是很少見的功能。

有三種方法來更新你的 ConfigMap 或 Secret：

##### Update from file

```shell
kubectl replace -f <filename>
kubectl apply -f <filename>
```

將資料放在檔案裡會顯得十分笨重，而且是在外部的檔案，這些資料會直接存在 YAML 檔案裡面，把密碼推到公開的地方小心會有洩密的可能。

##### Recreate and update

```shell
kubectl create secret generic kuard-tls \
  --from-file=kuard.crt --from-file=kuard.key \
  --dry-run -o yaml | kubectl replace -f -
```

如果只跑第一行的指令的話`kubectl`會跟你說，你的 secret 已經存在，但是後面有`--dry-run`資料並不會真的存進去，後面又加上`-o yaml`叫他列出 yaml 的格式，然後使用 pipe 把前面的`stdout`會變成後面的`stdin`，最後使用`-f -`讀取`stdin`的值。這樣我們可以更新磁碟上的 Secret 就不需要特別加密 base64-encode 的資料。

##### Edit current version

```shell
KUBE_EDITOR="vim" kubectl edit configmap my-config
KUBE_EDITOR="vim" kubectl edit secret kuard-tls
```

#### Live updates

一旦 ConfigMap 或 Secret 更新之後，所有的 volume 都會自動地更新，可能會花個幾秒鐘，而且不需要重新啟動 Pod。

現階段來說沒有內建的方法可以讓應用程式去偵測 ConfiMap 已經被更新，叫他去讀取新的 ConfigMap，這通常取決於應用程式或是寫個腳本去看設定檔是否改變然後重新載入他們。

`kuard`可以使用 File system brower 去看更新過後的變動。

### Cleaning up

```shell
kubectl delete pod kuard-config kuard-tls
kubectl delete secret kuard-tls
kubectl delete configmap my-config
```



## Deployments

`Deployment`物件存在是為了要管理新的版本發佈，能夠讓你更簡單地從一個版本更新到下一個版本，像是設定或是 health check 都是必須要顧慮到的，如果錯誤太多發生的話要馬上停止。使用`Deployment`可以更簡單更可靠的更新本版，且不需要停機或發生錯誤。

原本基於`kubectl rolling-update`指令，不過因為功能非常龐大，所以被歸到`Deployment`物件底下。

### Your First Deployment

```shell
kubectl run nginx --image=nginx:1.7.12
kubectl get deployments nginx
```

#### Deployment Internals

ReplicaSet 管理 Pod，Deployment 管理 ReplicaSet，之間的關係都是由 label 所定義的：

```shell
$ kubectl get deployments nginx \
    -o jsonpath --template {.spec.selector.matchLabels}
map[run:nginx]
```

由上面可以看出 Deployment 管理 ReplicaSet 是透過 label 為`run=nginx`的標籤：

```shell
$ kubectl get replicasets --selector=run=nginx
NAME               DESIRED   CURRENT   READY     AGE
nginx-59db8647bd   1         1         1         6m
```

我們可以用聲明式的指令`scale`去 resize Deployment：

```shell
$ kubectl scale deployments nginx --replicas=2
deployment "nginx" scaled
```

現在看 ReplicaSet：

```shell
$ kubectl get replicasets --selector=run=nginx
NAME               DESIRED   CURRENT   READY     AGE
nginx-59db8647bd   2         2         2         8m
```

Scaling Deployment 也會控制 scale ReplicaSet。

如果現在直接 scaling ReplicaSet：

```shell
$ kubectl scale replicasets nginx-59db8647bd --replicas=1
replicaset "nginx-59db8647bd" scaled
```

然後再看 ReplicaSet：

```shell
$ kubectl get replicasets --selector=run=nginx
NAME               DESIRED   CURRENT   READY     AGE
nginx-59db8647bd   2         2         2         12m
```

這很奇怪，儘管 scale ReplicaSet 變成一個後，他的 desired state 還是兩個。請記得，Kubernetes 是一個會自我修復的系統，在最上層的 Deployment 物件管理 ReplicaSet，所以當被調成一個的時候，不再符合 Deployment 的 desired state，Deployment controller 被通知然後才取行動條回兩個。

如果只想要直接管理 ReplicaSet，你必須用`--cascade=false`刪除 Deployment，否則 ReplicaSet 和 Pod 也會跟著刪除。

### 建立 Deployment

```shell
kubectl get deployments nginx --export -o yaml | kubectl replace -f - --save-config
```

所看到的文件會長這樣：

```yaml
# kubectl get deployments nginx --export -o yaml
apiVersion: extensions/v1beta1
kind: Deployment
metadata:
  annotations:
    deployment.kubernetes.io/revision: "1"
  creationTimestamp: null
  generation: 1
  labels:
    run: nginx
  name: nginx
  selfLink: /apis/extensions/v1beta1/namespaces/default/deployments/nginx
spec:
  replicas: 2
  selector:
    matchLabels:
      run: nginx
  strategy:
    rollingUpdate:
      maxSurge: 1
      maxUnavailable: 1
    type: RollingUpdate
  template:
    metadata:
      creationTimestamp: null
      labels:
        run: nginx
    spec:
      containers:
      - image: nginx:1.7.12
        imagePullPolicy: IfNotPresent
        name: nginx
        resources: {}
        terminationMessagePath: /dev/termination-log
        terminationMessagePolicy: File
      dnsPolicy: ClusterFirst
      restartPolicy: Always
      schedulerName: default-scheduler
      securityContext: {}
      terminationGracePeriodSeconds: 30
status: {}
```

因為是使用`kubectl replace --save-config`，他會記錄最後一次更改的內容，`kubectl apply -f`用於你第一次建立 deployment。

你可能會注意到有`spec.strategy.type`的選項，這邊主要有兩種不同方式來更新：`Recreate`和`RollingUpdate`。

### Managing Deployments

```shell
$ kubectl describe deployments nginx
Name:                   nginx
Namespace:              default
CreationTimestamp:      Thu, 31 May 2018 22:15:21 +0800
Labels:                 run=nginx
Annotations:            deployment.kubernetes.io/revision=1
                        kubectl.kubernetes.io/last-applied-configuration={"apiVersion":"extensions/v1beta1","kind":"Deployment","metadata":{"annotations":{"deployment.kubernetes.io/revision":"1"},"creationTimestamp":null,"ge...
Selector:               run=nginx
Replicas:               2 desired | 2 updated | 2 total | 2 available | 0 unavailable
StrategyType:           RollingUpdate
MinReadySeconds:        0
RollingUpdateStrategy:  1 max unavailable, 1 max surge
Pod Template:
  Labels:  run=nginx
  Containers:
   nginx:
    Image:        nginx:1.7.12
    Port:         <none>
    Environment:  <none>
    Mounts:       <none>
  Volumes:        <none>
Conditions:
  Type           Status  Reason
  ----           ------  ------
  Available      True    MinimumReplicasAvailable
OldReplicaSets:  <none>
NewReplicaSet:   nginx-59db8647bd (2/2 replicas created)
Events:          <none>
```

比較令人感興趣的欄位是`OldReplicaSets`跟`NewReplicaSet`，這些欄位指出 Deployment 正在管理 ReplicaSet 的物件。如果 Deployment 正在 rollout 中，就會看到這兩個欄位都有值，如果 rollout 成功後，這個`OldReplicaSets`欄位會被設成`<none>`。

`kubectl rollout status`可以獲得正在 rollout 的狀態，`kubectl rollout history` 獲得歷史 rollout 的紀錄

```shell
$ kubectl rollout history deployment nginx
deployments "nginx"
REVISION  CHANGE-CAUSE
1         <none>
$ kubectl rollout status deployment nginx
deployment "nginx" successfully rolled out
```

### 更新 Deployments

最常見的兩個 Deployment 的功能是 scaling 跟應用程式的更新：

#### Scaling a Deployment

前面有展示過聲明式的方式 scale Deployment，使用的是`kubectl scale`的指令。不過最好的做法還是修改 yaml 的檔案來增加你的 replica 的數量：

```yaml
# nginx-deployment.yaml
...
spec:
  replicas: 3
...
```

```shell
kubeclt apply -f nginx-deployment.yaml
```

```shell
$ kubectl get deployment nginx
NAME      DESIRED   CURRENT   UP-TO-DATE   AVAILABLE   AGE
nginx     3         3         3            3           23h
```

#### 更新 Container Image

一樣透過修改 yaml 檔案修改，這次是修改要更新的 container image 而不是 replica 的數量：

```yaml
# nginx-deployment.yaml
...
spec:
  template:
    spec:
      containers:
      - image: nginx:1.9.10
        imagePullPolicy: Always
...
```

然後再加上 annotation 到 template 裡，記錄這次的更新：

```shell
# nginx-deployment.yaml
...
spec:
  template:
    metadata:
      annotations:
        kubernetes.io/change-cause: "Update nginx to 1.9.10"
...
```

小心你加上的 annotation 是到 template 上，而不是 Deployment 上。且不要在 scaling 的時候加 key 為`change-cause`的 annotation，否則他會觸發新的 rollout

```shell
kubeclt apply -f nginx-deployment.yaml
```

```shell
$ kubectl rollout status deployment nginx
deployment "nginx" successfully rolled out
```

這時候你可以看到舊的跟新的 ReplicaSet 同時存在，讓你隨時可以 roll back 回去：

```shell
$ kubectl get replicasets -o wide
NAME               DESIRED   CURRENT   READY     AGE       CONTAINERS   IMAGES         SELECTOR
nginx-59db8647bd   0         0         0         23h       nginx        nginx:1.7.12   pod-template-hash=1586420368,run=nginx
nginx-85cb94577f   3         3         3         7m        nginx        nginx:1.9.10   pod-template-hash=4176501339,run=nginx
```

如果在 rollout 的途中你想要暫停他，可能是某些原因：

```shell
$ kubectl rollout pause deployments nginx
deployment "nginx" paused
```

調查完你想要的東西後，想要繼續執行 rollout：

```shell
$ kubectl rollout resume deployments nginx
deployment "nginx" resumed
```

#### Rollout History

```shell
$ kubectl rollout history deployment nginx
deployments "nginx"
REVISION  CHANGE-CAUSE
1         <none>
2         Update nginx to 1.9.10
```

revision 的編號會逐漸遞增，到目前為止有兩個版本：一個是初始化的 deployment 跟更新到 1.9.10 的。

如果你對某個版本很感興趣的話，你可以加上`--revision`的標籤去看裡面的詳細訊息：

```shell
$ kubectl rollout history deployment nginx --revision=2
deployments "nginx" with revision #2
Pod Template:
  Labels:	pod-template-hash=4176501339
	run=nginx
  Annotations:	kubernetes.io/change-cause=Update nginx to 1.9.10
  Containers:
   nginx:
    Image:	nginx:1.9.10
    Port:	<none>
    Environment:	<none>
    Mounts:	<none>
  Volumes:	<none>
```

我們用`kubectl apply`再多加一個版本`1.10.2`

```shell
$ kubectl rollout history deployment nginx
deployments "nginx"
REVISION  CHANGE-CAUSE
1         <none>
2         Update nginx to 1.9.10
3         Update nginx to 1.10.2
$ kubectl get replicasets -o wide
NAME               DESIRED   CURRENT   READY     AGE       CONTAINERS   IMAGES         SELECTOR
nginx-59db8647bd   0         0         0         1d        nginx        nginx:1.7.12   pod-template-hash=1586420368,run=nginx
nginx-85cb94577f   0         0         0         1h        nginx        nginx:1.9.10   pod-template-hash=4176501339,run=nginx
nginx-985f54645    3         3         3         59s       nginx        nginx:1.10.2   pod-template-hash=541910201,run=nginx
```

如果想要回到上一個版本：

```shell
kubectl rollout undo deployment nginx
```

可以看到 1.9.10 的本版正在運行，你可看到 Deployment 只是簡單的調整 ReplicaSet 的數目：

```shell
$ kubectl get replicasets -o wide
NAME               DESIRED   CURRENT   READY     AGE       CONTAINERS   IMAGES         SELECTOR
nginx-59db8647bd   0         0         0         1d        nginx        nginx:1.7.12   pod-template-hash=1586420368,run=nginx
nginx-85cb94577f   3         3         3         1h        nginx        nginx:1.9.10   pod-template-hash=4176501339,run=nginx
nginx-985f54645    0         0         0         2m        nginx        nginx:1.10.2   pod-template-hash=541910201,run=nginx
```

當你使用`kubectl rollout undo`的時候更新，並不代表回到過去的版本，而是使用`kubectl apply`前一個版本：

```shell
$ kubectl rollout history deployment nginx
deployments "nginx"
REVISION  CHANGE-CAUSE
1         <none>
3         Update nginx to 1.10.2
4         Update nginx to 1.9.10
```

revision 2 已經消失了！Deployment 會簡單的重複使用 template 然後重新命名 revision 的數字，這時候再使用`kubectl rollout undo`且用`--to-revision`限定他過去的版本：

```shell
$ kubectl rollout undo deployment nginx --to-revision
deployment "nginx" 
$ kubectl rollout history deployment nginx
deployments "nginx"
REVISION  CHANGE-CAUSE
1         <none>
4         Update nginx to 1.9.10
5         Update nginx to 1.10.2
```

相同的`undo`會套用 revision 3 然後重新命名 revision 5。

指定版本為 0 的話等同於指定前一個版本

```shell
kubectl rollout undo
kubectl rollout undo --to-revision=0
```

預設 Deployment 會把所有的歷史紀錄都記住，但如果時間一長的話，歷史紀錄會變得非常大，建議會設定最大歷史紀錄的數量，參數是在`spec.revisionHistoryLimit`裡設定：

```yaml
...
spec:
  # 每天更新的話，歷史紀錄會保持兩週
  revisionHistoryLimit: 14
...
```

### Deployment Strategies

Kubernetes 支援兩種不同 rollout 的策略：

- `Recreate`
- `RollingUpdate`

#### Recreate Strategy

Recreate 策略是在兩個 rollout 策略裡面最簡單的，他會簡單的更新 ReplicaSet，ReplicaSet 會去管理 Pod 使用新的映像檔，然後中止所有跟 Deployment 聯繫的 Pod。ReplicaSet 注意到沒有任何的 replica 的時候，他就會重新建立新的 Pod，並且會是用新的 image。一旦 Pod 被重新建立，他就會跑新的版本。

這個策略快又簡單，不過他有一個缺點，他有可能會導致停機，因為這樣，recreate 策略應該只用來在測試的 deployment，他不會直接服務使用者，可以接受短暫的停機。

#### RollingUpdate Strategy

`RollingUpdate`策略是最好用來面對使用者的策略，它更新的速度會比`Recreate`慢，但是他更複雜且更強大，使用`RollingUpdate`可以一邊 rollout 成新的版本，一邊接收使用者的流量，且不需要停機。

從策略的名字來推斷，他一次會更新一些 Pod，然後慢慢增加新的 Pod 的數量。

##### Managing multiple versions of your service

重點是會有一段時間同時服務兩個版本，你必須處理你的應用程式，能夠兼容比較舊的版本。

舉例來說你有兩個版本 v1 跟 v2，當今天使用者請求 v1 的網頁的時候，這時你更新到了 v2，使用者接收到網頁再次向 server 請求 javascript library 的時候，他會請求到 v2 的版本，萬一版本不兼容的話，你的應用程式可能不會正常運作，所以不如你如何更新都必須要兼容過去的版本，使你的應用程式變得更可靠。

這必須要去自己去 decouple 你的 API，如果是非常耦合的話那會很難快速的更新。

##### Configuring a rolling update

有兩個參數是你可以調整 rolling update 的行為：

- `maxUnavailable`
- `maxSurge`

`maxUnavailable`設定不能使用 Pod 的最大數量，可以設置數量或是百分比。這個參數幫助我們可以更新多快，假如設成 50%，現在有 4 個 replica，那 rolling update 會立即把舊的 ReplicaSet 降到 2 個 replica，然後再把新的 ReplicaSet 提升到 2 個 replica，最後再把舊的降到 0，新的提升到 4，完成更新，總共有四個步驟。

如果設成 25% 的話，每次就只會執行一個 replica，總共的步驟會多一倍。

如果設成 100% 的話，就會跟 recreate 的策略一樣。

通常是用在你資源有限的時候，資源不能超過幾個 replica，但如果今天資源有執行更新的話，那可以把`maxUnavailable`設成 0，然後調整`maxSurge`的參數。

maxUnavailable`跟`maxSurge`兩個很相似，假設今天有 10 個 replica，`maxUnavailable`是 0 和`maxSurge`是 20%，那第一次更新的時候會將新的 ReplicaSet 提升至 2 個 replica，總共會有 120% 的服務在裡面，12 個 replica，然後再把舊的 replica 下降至 8 個，直到更新成功為止。

如果把`maxSurge`設為 100% 的話就等於是 blue/green deployment。

#### Slowing Rollouts to Ensure Service Health

為了要確保新的 Pod 是健康的，在每次執行階段的時候，Pod 都必須要回報 readiness check，ready 之後才會繼續更新下一個 Pod。

不過有些錯誤並不是馬上就會出現的，像是 memory leak 直到一分鐘以後才會出現，或是所有請求之中只有 1% 的機率會發生錯誤，我們會先等待 Pod ready 了一段時間，提高新版本的可信度，才會前往更新下一個 Pod。

```yaml
...
spec:
  minReadySeconds: 60
...
```

設定`minReadySeconds`為 60 意思為 Deployment 會看 Pod 已經 ready 60 秒之後才會更新下一個 Pod。

我們還需要設定一個上線的時間，假如新的版本有 bug 在裡面，他從來不會 ready，這樣 Deployment 就會永遠停留在 roll-out 的狀態，可以使用`progressDeadlineseconds`參數：

```yaml
...
spec:
  progressDeadlineseconds: 600
...
```

這個範例設定的時間是 10 分鐘，如果更新失敗超過 10 分鐘的話，Deployment 就會把這次的更新標為 failed，然後中止更新。

他設定的是計時失敗的時間，如果有成功的建立或刪除，計時就會重新設成 0。

### 刪除 Deployment

命令式：

```shell
kubectl delete deployments nginx
```

聲明式：

```shell
kubectl delete -f nginx-deployment.yaml
```

刪除一樣會把所有的 ReplicaSet 跟 Pod 都刪掉，如果只想要刪除 Deployment 物件的話可以加上`--cascade=false`的flag。

## Integrating Storage Solutions and Kubernetes

在很多解偶狀態的應用程式中，盡可能地把建立的 microservices 變得越 stateless，這樣會最可靠、最大化系統管理。

然後在現在每個系統中，都有可能會會有複雜的的狀態，從資料庫的紀錄到網頁搜尋引擎裡的索引，有時候你就是會要存某些資料在某些地方。

在分散式系統中要用 container 和 container orchestration solution 來整合這些資料是最複雜的。從要把它移到 container 的架構裡，他必須是解偶的、不動的、聲明式的應用程式。這個用在無狀態的網頁應用程式是相對簡單的，但是 cloud-native 儲存方法像是 Cassandra 或 MongoDB 涉入一些手動或必要的步驟來建立一個可靠的、複製的解決方案。

假如在 ReplicaSet 裡設定 MongoDB 他會涉入 Mongo daemon 和他所跑的指令要去判斷誰是 leader，以及要去參與 Mongo cluster。當然這些可以用腳本寫，但是在 container 的世界裡這會很困難去看到要整合這些指令。同樣，即使用 DNS-resolvable names 給每一台 replica set 的 container 也是很有挑戰性。

這些複雜性來自於資料重量的事實，大多數的 container 的系統不是獨自建立的，通常是採用存在的系統去部署 VM，這些系統可能包含匯入或 migrate 的資料。

最後，向外部雲儲存發展意味著他不會真正存在 Kubernetes cluster 內部。

### Importing External Services

#### Services Without Selectors

外部的 services，沒有任何的 label query，只是使用一般的 DNS name 去指向正在跑的 database server，假設有個正在跑的 database server 是`database.company.com`，為了把外部的 database service 匯入到 Kubernetes，我們先建立不需要 Pod selecot 的 service，他會參照 database server 的 DNS name。

```yaml
# dns-service.yaml
kind: Service
apiVersion: v1
metadata:
  name: external-database
spec:
  type: ExternalName
  externalName: "database.company.com
```

使用一般的 service 來說，會創建一個 IP address，然後用 A record 記錄在 DNS 裡面。當`spec.type`標示了`ExternalName`的時候，他會變成 CNAME record 指向外部的名字`database.company.com`。在 cluster 裡有個應用程式使用 DNS lookup 這個 hostname `external-database.svc.default.cluster`，DNS protocol 別名他為`database.company.com`，然後他就會去解析外部網址的 IP address，如此一來，所有的 container 都會認為他是在跟其他 container 溝通，但實際上他會重新導向到外部的資料庫。

很多雲端資料庫都會提供 DNS name，你可以直接使用那個 DNS name 當作你`externalName`。

然而有一些沒有提供 DNS address 給外部資料庫，他只提供 IP address。首先，先建立不需要 label selector 的 Service，而且也不需要之前使用的`ExternalName`。

```yaml
# external-ip-service.yaml
kind: Service
apiVersion: v1
metadata:
  name: external-ip-database
```

這樣 Kubernetes 會分配一個虛擬的 Ip address 給這個 service，而且使用 A record。然而，因為 service 沒有 selector，這樣就沒有 endpoint 讓 loadbalancer 重新導向流量。

由於他是外部的服務，使用者必須手動新增 endpoint 的資源：

```yaml
# external-ip-endpoints.yaml
kind: Endpoints
apiVersion: v1
metadata:
  name: external-ip-database
subsets:
  - addresses:
    - ip: 192.168.0.1
    ports:
    - port: 3306
```

如果有多個 ip 可以使用 array 重複 ip，這樣 load balancer 就會平均分配流量。

如果使用者使用 ip 的話，那要盡可能確保 ip 要保持不變。

#### Limitations of External Services: Health Checking

因為他是外部的服務，Kubernetes 不會執行任何的 health check，使用有責任確保 endpoint 或 Kubernetes 提供的 DNS name 是可靠的，對於你的應用程式來講。

### Running Reliable Singletons

運行相同的資料庫在每個 ReplicaSet 會需要更多的儲存空間，而且不曉得他會存取到哪的資料庫，所以運行單一個資料庫就不會發生這個問題。

這很像違反了建立可靠的分散式系統的原則，將你的資料庫運行在單一的機器上，萬一更新失敗的時候，會有潛在停機的可能。當在大型的系統或危及任務的系統中可不能接受停機，但對於小系統來講，有限的停機時間是可以降低複雜性。

#### Running a MySQL Singleton

這個章節需要建立的三個物件：

- 一個 persisten volume 去管理磁碟上的儲存空間，跟 MySQL 應用程式的管理分開。
- MySQL Pod 裡面會運行 MySQL 的應用程式。
- Service 會暴露 Pod 給其他 container 使用。

persistent volume 的生命週期跟其他的 Pod 或 container 不一樣，即使容器掛了但是資料應該要繼續存活在磁碟上，或是移動到不同的機器。如果移動一個應用程式到不同的機器，那 volume 應該也要跟著移動，資料應該被保留，分散資料儲存空間變得有可能，首先我們要先建立 persistent volume。

這個範例會使用 NFS，有最大的可移值性，Kubernetes 支援很多不同種的 persistent volume drive type。可以簡單地取代`nfs`為 cloud provider volume type，像是`azure`、`awsElasticBlockStore`、`gcePersistentDisk`。你只需要改變這個，Kubernetes 就會幫你建立適當的儲存磁碟。

```yaml
# nfs-volume.yaml
apiVersion: v1
kind: PersistentVolume
metadata:
  name: database
  labels:
    volume: my-volume
spec:
  capacity:
    storage: 1Gi
  accessModes:
    - ReadWriteOnce
  nfs:
    server: 192.168.0.1
    path: "/exports"
```

這邊定義了 NFS persistent volume，有 1GB 儲存空間。

```shell
kubectl apply -f nfs-volume.yaml
```

我們現在要宣告這些空間要給我們的 Pod 來使用，所以要建立`PersistentVolumeClaim`的物件：

```yaml
# nfs-volume-claim.yaml
kind: PersistentVolumeClaim
apiVersion: v1
metadata:
  name: database
spec:
  resources:
    requests:
      storage: 1Gi
  accessModes:
    - ReadWriteOnce
  selector:
    matchLabels:
      volume: my-volume
```

```shell
kubectl apply -f nfs-volume-claim.yaml
```

這邊的`selector`會使用 label 找到我們之前定義的 volume。

這樣間接的定義似乎有點過於複雜，但是有一個目的，就是把定義的 Pod 和定義儲存的部分分開。你也可以直接在 Pod 裡面使用 volume，但他會鎖定程使用特定某一個 volume provider。透過 volume claims，你可以保持你的 Pod 規格，簡單的建立不同的 volume 指向不同的雲，然後使用`PersistentVolumeClaim`去綁定他們。

接下來我們可以使用 ReplicaSet 去建立我們單一的 Pod，這聽起來很怪，讓一個 ReplicaSet 去管理一個 Pod，但是這是為了可靠性，如果 Pod 上的那台機器掛了，那台機器上的 Pod 就不會再重新起動，但如果有 ReplicaSet 管理的話，就會重新安排他，所以為了確保資料庫的 Pod 會自我修復，我們需要使用更高層的 ReplicaSet controller 去管理他。

```yaml
# mysql-replicaset.yaml
apiVersion: extensions/v1beta1
kind: ReplicaSet
metadata:
  name: mysql
  # labels so that we can bind a Service to this Pod
  labels:
    app: mysql
spec:
  replicas: 1
  selector:
    matchLabels:
      app: mysql
  template:
    metadata:
      labels:
        app: mysql
    spec:
      containers:
      - name: database
        image: mysql
        resources:
          requests:
            cpu: 1
            memory: 2Gi
        env:
        # Environment variables are not a best practice for security,
        # but we're using them here for brevity in the example.
        # See Chapter 11 for better options.
        - name: MYSQL_ROOT_PASSWORD
          value: some-password-here
        livenessProbe:
          tcpSocket:
            port: 3306
        ports:
        - containerPort: 3306
        volumeMounts:
          - name: database
            # /var/lib/mysql is where MySQL stores its databases
            mountPath: "/var/lib/mysql"
      volumes:
      - name: database
        persistentVolumeClaim:
          claimName: database
```

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/13-6-mysql-replicaset.yaml
```

一但我們建立了 ReplicaSet 我們將可以建立 MySQL 的 Pod，最後只需要 expose service：

```yaml
# mysql-service.yaml
apiVersion: v1
kind: Service
metadata:
  name: mysql
spec:
  ports:
  - port: 3306
    protocol: TCP
  selector:
    app: mysql
```

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/13-7-mysql-service.yaml
```

現在我們有一個 MySQL 在運行，他的 service 名字是 mysql，可以透過全名的 domain 去存取`mysql.svc.default.cluster`。

如果你只想要簡單，可以有短暫的停機時間來處理資料庫更新或是機器掛了的話，這個儲存方法可以會不錯。

#### Dynamic Volume Provisioning

很多的 cluster 都會包含 dynamic volume provisioning。有了 dynamic volume provisioning，cluster 可以建立`StorageClass`的物件這邊有預設的儲存 class 會自動的規定 Microsoft Azure platform 的磁碟物件：

```yaml
# storageclass.yaml
apiVersion: storage.k8s.io/v1beta1
kind: StorageClass
metadata:
  name: default
  annotations:
    storageclass.beta.kubernetes.io/is-default-class: "true"
  labels:
    kubernetes.io/cluster-service: "true"
provisioner: kubernetes.io/azure-disk
```

一但這個被建立，你可以使用 persistent volume claim 去指向這個 storage class，而不是指到 persistent volume，他會動態的建立 volume 而且綁定 persistent volume claim。

```yaml
# dynamic-volume-claim.yaml
kind: PersistentVolumeClaim
apiVersion: v1
metadata:
  name: my-claim
  annotations:
    volume.beta.kubernetes.io/storage-class: default
spec:
  accessModes:
  - ReadWriteOnce
  resources:
    requests:
      storage: 10Gi
```

`volume.beta.kubernetes.io/storage-class`這個 annotation 是用來連結備份的到 storage class，我們剛剛所建立的。

Persisten volumes 是給傳統的應用程式所使用的儲存空間，但是如果你需要高可用、可拓展性的儲存空間的話，那可以使用`StatefulSet`的物件，接下來，我們會描述如何使用`StatefulSet`部署 Mongo DB。

### Kubernetes-Native Storage with StatefulSets

我們強調 ReplicaSet 是有同質性，在設計中，replica 沒有他獨自的 id 或設定，當如果有 stateful 的應用程式的時候，這會非常難去開發，所以 StatefulSets 就誕生了。

#### Properties of StatefulSets

StatefulSet 跟 ReplicaSet 非常相似，都是由一組 Pod 的 replica，但又不像 ReplicaSet，他有幾個特性：

- 每個 replica 都會帶有 unique index，像是 database-0、database-1。
- 每個 replica 的 index 都會由小到大依序增加，且在建立的時候，前一個 Pod 是健康而且可用的話，才會建立下一個。
- 當刪除的時候會由 index 最大的開始刪除，可以用於 scaling down。

#### Manually Replicated MongoDB with Statefulsets

這這個章節會建立 MongoDB cluster：

```yaml
# mongo-simple.yaml
apiVersion: apps/v1beta1
kind: StatefulSet
metadata:
  name: mongo
spec:
  serviceName: "mongo"
  replicas: 3
  template:
    metadata:
      labels:
        app: mongo
    spec:
      containers:
      - name: mongodb
        image: mongo:3.4.1
        command:
        - mongod
        - --replSet
        - rs0
        ports:
        - containerPort: 27017
          name: peer
```

這個設定檔跟 ReplicaSet 很相似，只有`apiVersion`和`kind`的欄位不同。

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/13-10-mongo-simple.yaml
```

一但建立可以看到 ReplicaSet 跟 StatefulSet 的不同：

```shell
$ kubectl get pods
NAME          READY     STATUS    RESTARTS   AGE
mongo-0       1/1       Running   0          44s
mongo-1       1/1       Running   0          5s
mongo-2       1/1       Running   0          4s
```

有兩個不同的點：第一 Pod 有數字的 index 而不是亂數得值。第二個不同是 Pod 會比較緩慢的建立，而不是 ReplicaSet 同時建立所有的 Pod。

一旦 StatefulSet 被建立，我們需要建立 headless service 來幫 StatefulSet 去管理 DNS entry。如果 service 沒有給 cluster 虛擬 IP 的話，service 被稱作為 headless service。因為 StatefulSet 的 Pod 有為一個 id，去 load balance IP address 並不合理。你使用`clusterIP: None`可以建立 headless service。

```yaml
# mongo-service.yaml
apiVersion: v1
kind: Service
metadata:
  name: mongo
spec:
  ports:
  - port: 27017
    name: peer
  clusterIP: None
  selector:
    app: mongo
```

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/13-11-mongo-service.yaml
```

你建立好 service 後，使用 DNS lookup 可以看到有四條 DNS entry，不像一般的 standard service，使用 DNS lookup 會看到所有的 hostname provider。此外這個 entry 會建立`mongo-0.mongo.default.svc.cluster.local`這非常方便用來設定你的 replica。

```
# DNS Type: A, Name: mongo
;; opcode: QUERY, status: NOERROR, id: 61497
;; flags: qr aa rd ra; QUERY: 1, ANSWER: 3, AUTHORITY: 0, ADDITIONAL: 0

;; QUESTION SECTION:
;mongo.default.svc.cluster.local.	IN	 A

;; ANSWER SECTION:
mongo.default.svc.cluster.local.	30	IN	A	172.17.0.5
mongo.default.svc.cluster.local.	30	IN	A	172.17.0.7
mongo.default.svc.cluster.local.	30	IN	A	172.17.0.8
```

```
# DNS Type: A, Name: mongo-0
;; opcode: QUERY, status: NXDOMAIN, id: 20148
;; flags: qr rd ra; QUERY: 1, ANSWER: 0, AUTHORITY: 1, ADDITIONAL: 0

;; QUESTION SECTION:
;mongo-0.	IN	 A

;; AUTHORITY SECTION:
.	3600	IN	SOA	a.root-servers.net. nstld.verisign-grs.com. 2018060400 1800 900 604800 86400
```

可以運行指令測試看看：

```shell
$ kubectl exec -ti mongo-0 -- ping mongo-1.mongo
PING mongo-1.mongo.default.svc.cluster.local (172.17.0.7): 56 data bytes
64 bytes from 172.17.0.7: icmp_seq=0 ttl=64 time=0.106 ms
64 bytes from 172.17.0.7: icmp_seq=1 ttl=64 time=0.082 ms
```

接下來我們要手動建立 Mongo replication，使用我們剛剛所建立的每個 hostname。

我們選擇`mongo-0.mongo`作為初始化，然後運行 mongo 的指令：

```shell
$ kubectl exec -ti mongo-0 mongo
> rs.initiate( {
    _id: "rs0",
    members:[ { _id: 0, host: "mongo-0.mongo:27017" } ]
  });
{ "ok" : 1 }
```

這個指令告訴 mongodb 建立 ReplicaSet`rs0`，使用`mongo-0.mongo`作為主要 replica。

`rs0`的名字是任意的，可以換成你想要的。

一但你建立完 Mongo ReplicaSet 之後，你可以新增剩下的 replica，運行下面的指令：

```shell
$ kubectl exec -ti mongo-0 mongo
> rs.add("mongo-1.mongo:27017");
> rs.add("mongo-2.mongo:27017");
```

你能看到我們正在使用 DNS name 去新增 replica 到我們的 Mongo cluster，我們的 MonogoDB replica 這在運行，但是我們沒辦法自動的運行，在下一個章節會使用校本去讓他自動運行。

#### Automating MongoDB Cluster Creation

為了要讓我們的 StatefulSet MongoDB cluster 自動部署的話，我們必須要在 container 裡執行一些初始化的腳本。

為了設置 Pod 且不需要建立新的 Docker image，我們使用 ConfigMap 去新增腳本到存在的 MongoDB image：

```yaml
- name: init-mongo
  image: mongo:3.4.1
  command:
  - bash
  - /config/init.sh
  volumeMounts:
  - name: config
    mountPath: /config
volumes:
- name: config
  configMap:
    name: "mongo-init"
```

現在要掛載 ConfigMap volume`mongo-init`，這個 ConfigMap 會包含初始化的腳本。首先他會決定他是否運行在`mongo-0`上，如果他運行在`mongo-0`的話，執行之前運行的指令，其他的話就會等到註冊自己。

```yaml
# mongo-configmap.yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: mongo-init
data:
  init.sh: |
    #!/bin/bash

    # Need to wait for the readiness health check to pass so that the
    # mongo names resolve. This is kind of wonky.
    until ping -c 1 ${HOSTNAME}.mongo; do
      echo "waiting for DNS (${HOSTNAME}.mongo)..."
      sleep 2
    done

    until /usr/bin/mongo --eval 'printjson(db.serverStatus())'; do
      echo "connecting to local mongo..."
      sleep 2
    done
    echo "connected to local."

    HOST=mongo-0.mongo:27017

    until /usr/bin/mongo --host=${HOST} --eval 'printjson(db.serverStatus())'; do
      echo "connecting to remote mongo..."
      sleep 2
    done
    echo "connected to remote."

    if [[ "${HOSTNAME}" != 'mongo-0' ]]; then
      until /usr/bin/mongo --host=${HOST} --eval="printjson(rs.status())" \
            | grep -v "no replset config has been received"; do
        echo "waiting for replication set initialization"
        sleep 2
      done
      echo "adding self to mongo-0"
      /usr/bin/mongo --host=${HOST} \
         --eval="printjson(rs.add('${HOSTNAME}.mongo'))"
    fi

    if [[ "${HOSTNAME}" == 'mongo-0' ]]; then
      echo "initializing replica set"
      /usr/bin/mongo --eval="printjson(rs.initiate(\
          {'_id': 'rs0', 'members': [{'_id': 0, \
           'host': 'mongo-0.mongo:27017'}]}))"
    fi
    echo "initialized"

    while true; do
      sleep 3600
    done
```

腳本在運行後就會永遠的運行，每個 container 都會有相同的`RestartPolicy`。因為我們想要我們主要的 Mongo container 能夠重新啟動，所以我們必須讓我們的 initialization container 永遠運行，否則 Kubernetes 會認為我們的 mongo Pod 是不健康的。

全部放在一起：

```yaml
# mongo.yaml
apiVersion: apps/v1beta1
kind: StatefulSet
metadata:
  name: mongo
spec:
  serviceName: "mongo"
  replicas: 3
  template:
    metadata:
      labels:
        app: mongo
    spec:
      containers:
      - name: mongodb
        image: mongo:3.4.1
        command:
        - mongod
        - --replSet
        - rs0
        ports:
        - containerPort: 27017
          name: web
      # This container initializes the mongodb, then sleeps.
      - name: init-mongo
        image: mongo:3.4.1
        command:
        - bash
        - /config/init.sh
        volumeMounts:
        - name: config
          mountPath: /config
      volumes:
      - name: config
        configMap:
          name: "mongo-init"
```

然後我們可以建立 Mongo cluster：

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/13-11-mongo-service.yaml
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/13-12-mongo-configmap.yaml
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/13-13-mongo.yaml
```

也可以全部寫在一個檔案裡，用`---`分別開來，且卻本你的順序是正確的，因為 StatefulSet 必須建立在 ConfigMap 前。

#### Persistent Volumes and StatefulSets

對 persistent storage 來說，你必須 mount persistent volume 到 /data/db 目錄。 在 Pod template，你必須要更新 persistent volume claim 的 mountPath。

```yaml
volumeMounts:
- name: database
  mountPath: /data/db
```

雖然這種方法類似於我們用可靠的單例看到的方法，但由於 StatefulSet 複製了多個 Pod，因此不能簡單地引用 persistent volume claim。相反，您需要添加 persisten volume claim template。您可以將 claim template 視為與 Pod template 相同，但不會創建 Pod，它會創建 volume claim。您需要將以下內容添加到 StatefulSet 定義的底部：

```yaml
volumeClaimTemplates:
- metadata:
    name: database
    annotations:
      volume.alpha.kubernetes.io/storage-class: anything
  spec:
    accessModes: [ "ReadWriteOnce" ]
    resources:
      requests:
        storage: 100Gi
```

當您將一個 volume claim template 添加到一個 StatefulSet 定義中時，每次 StatefulSet controller 創建一個屬於 StatefulSet 的 Pod 時，它將根據此 template 創建一個 persistem volume claim 作為該 Pod 的一部分。

為了讓這些 replica 的 persistent volume 正常工作，您需要為 persistent volume 設置自動配置，或者您需要預先填充 persisten volume 物件以供 StatefulSet controller 從中進行使用。如果沒有可創建的 claim，則 StatefulSet controller 將無法創建相應的 Pod。

#### One Final Thing: Readiness Probes

製作我們的 MongoDB cluster 的最後一部分是為我們的 Mongo-serving container 加上 health check：

```yaml
...
livenessProbe:
  exec:
    command:
      - /usr/bin/mongo
      - --eval
      - db.serverStatus()
    initialDelaySeconds: 10
    timeoutSeconds: 10
...
```

## Deploying Real-World Applications

三個真實世界的應用：

- Parse：一個開源 API server，專門給手機應用程式使用。
- Ghost：一個部落格和內容管理平台。
- Redis：輕量 key/value 儲存。

### Parse

https://github.com/parse-community/parse-server

Parse server 是一個 cloud API，致力於提供簡單讓使用者儲存手機的 server。他提供很多不同的 client library，方便你去整合 Android、iOS 和其他手機平台。

#### Prerequisites

Parse 使用 MongoDB cluster 來儲存資料，在前一章節有說明如何用 StatefulSet 建立 MongoDB，這個章節假設你已經有三個 Mongo cluster 正跑在 kubernetes 裡。且假設你有 Docker login。最後假設你的 Kuebrnetes cluster 有適當的部署。

### 製作 parse-server 映像檔

裡面有 Dockerfile 可以直接製作映像檔：

```shell
git clone https://github.com/parse-community/parse-server.git
cd parse-server
docker build -t ${USER}/parse-server .
docker push ${USER}/parse-server
```

### Deploying the parse-server

有三個參數要設定：

- `APPLICATION_ID`：授權應用程式的識別碼。
- `MASTER_KEY`：授權 root user 的識別碼。
- `DATABASE_URI`：MongoDB cluster 的 URI

```yaml
# parse.yaml
apiVersion: extensions/v1beta1
kind: Deployment
metadata:
  name: parse-server
  namespace: default
spec:
  replicas: 1
  template:
    metadata:
      labels:
        run: parse-server
    spec:
      containers:
      - name: parse-server
        image: akiicat/parse-server
        args:
        - --appId
        - $APP_ID
        - --masterKey
        - $MASTER_KEY 
        - --databaseURI
        - $DATABASE_URI
        env:
        - name: DATABASE_URI
          value: "mongodb://mongo-0.mongo:27017,\
            mongo-1.mongo:27017,mongo-2.mongo\
            :27017/dev?replicaSet=rs0"
        - name: APP_ID
          value: my-app-id
        - name: MASTER_KEY
          value: my-master-key
```

### Testing Parse

Expose service

```yaml
# parse-service.yaml
apiVersion: v1
kind: Service
metadata:
  name: parse-server
  namespace: default
spec:
  ports:
  - port: 1337
    protocol: TCP
    targetPort: 1337
  selector:
    run: parse-server
```

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/14-2-parse-service%2Cyaml
```

### Ghost

是一個 blog engine，用 Javascript 寫的。

#### Configuring Ghost

config 檔案也是使用 Javascript 寫的，我們會把它存在 ConfigMap 裡：

```js
// ghost-config.js
var path = require('path'),
    config;

config = {
    development: {
        url: 'http://localhost:2368',
        database: {
            client: 'sqlite3',
            connection: {
                filename: path.join(process.env.GHOST_CONTENT,
                                    '/data/ghost-dev.db')
            },
            debug: false
        },
        server: {
            host: '0.0.0.0',
            port: '2368'
        },
        paths: {
            contentPath: path.join(process.env.GHOST_CONTENT, '/')
        }
    }
};

module.exports = config;
```

建立 ConfigMap 物件

```shell
kubectl create configmap --from-file ghost-config.js ghost-config
```

ConfigMap 命名為`ghost-config`，我們會掛載 config file 到我們的 container 上，然後用 Deployment 物件建立 Ghost：

```yaml
# ghost.yaml
apiVersion: extensions/v1beta1
kind: Deployment
metadata:
  name: ghost
spec:
  replicas: 1
  selector:
    matchLabels:
      run: ghost
  template:
    metadata:
      labels:
        run: ghost
    spec:
      containers:
      - image: ghost
        name: ghost
        command:
        - sh
        - -c
        - cp /ghost-config/ghost-config.js /var/lib/ghost/config.js
          && docker-entrypoint.sh npm start
        volumeMounts:
        - mountPath: /ghost-config
          name: config
      volumes:
      - name: config
        configMap:
          defaultMode: 420
          name: ghost-config
```

有一件需要注意的事情，我們這裡把`config.js`從不同的地方複製進去，讓 Ghost 可以找到他，不過因為 ConfigMap 只能被掛載載目錄上，而不能是單一的檔案，Ghost 也期望該檔案不是在 ConfigMap 的目錄下，所以我們不能直接掛載到 /var/lib/ghost 這個地方，而是要複製進去。

```shell
kubectl apply -f ghost.yaml
```

一旦 Pod 建立好後，可以 expose service：

```shell
kubectl expose deployments ghost --port=2368
```

一旦 Service expose，可以使用`kubectl proxy`指令來存取 Ghost server：

```shell
kubectl proxy
```

http://localhost:8001/api/v1/namespaces/default/services/ghost/proxy

##### Ghost + MySQL

當然這樣並不太能 scalable，可以把 config.js 修改成：

```js
...
database: {
  client: 'mysql',
  connection: {
    host : 'mysql',
    user : 'root',
    password : 'root',
    database : 'ghost_db',
    charset : 'utf8'
  }
},
...
```

```shell
kubectl create configmap ghost-config-mysql --from-file config.js
```

然後接著修改 Ghost deployment ConfigMap mount 的名字，從`config-map`修改成`config-map-mysql`：

```yaml
# ghost.yaml
- configMap:
  name: ghost-config-mysql
```

建立 MySQL 資料庫：

```shell
$ kubectl exec -ti mysql-zzmlw -- mysql -u root -p
> create database ghost_db;
```

接著執行更新

```shell
kubectl apply -f ghost.yaml
```

因為現在 Ghost server 跟資料庫已經分開，所以可任意 scale Ghost server。

### Redis

是一個在記憶體 key/value 儲存方式，有很多額外的功能，對 Kubetnetes Pod 抽象性來說是一個很好的例子，因為 Redis 安裝需要兩個程式一起安裝。第一個`redis-server`負責儲存 key/value，另一個`redis-sentinel`負責實作 health check 和處理掛掉的 Redis。

當 Redis 部署多個的時候，有一個會是 master server，他可以有 read 和 write 的功能。此外，其他的 replia server，他會複製資料寫到 master，可以被用來 load-balancing read 操作。如果原本的 master 掛了。任何的 replica 都有可能成為 master，這個掛掉的判斷會由 Redis sentinel 來執行。在我們部署的例子中，Redis server 跟 Redis sentinel 都會在同一個檔案裡。

#### Configuring Redis

我們要使用 ConfigMap 來設定我們的 Redis，Redis 在 master 和 slave 有分別的設定檔，為了設定 master 建立一個檔案叫做 master.conf：

```conf
# master.conf
bind 0.0.0.0
port 6379

dir /redis-data
```

Redis 會直接綁定整個網路上，Redis 預設的 port 6379 和把資料存在 /redis-data。

在 slave 裡的設定檔，加了單一的`slaveof`的方向：

```conf
# slave.conf
bind 0.0.0.0
port 6379

dir .

slaveof redis-0.redis 6379
```

我們使用`redis-0.redis`作為 master Redis 的名字，我們將會用 service 和 StatefulSet 設定這個名字。

我們還需要 Redis sentinel 的設定檔：

```conf
# sentinel.conf
bind 0.0.0.0
port 26379

sentinel monitor redis redis-0.redis 6379 2
sentinel parallel-syncs redis 1
sentinel down-after-milliseconds redis 10000
sentinel failover-timeout redis 20000
```

我們建立好所有的設定檔，需要建立一個簡單的腳本，跑在 StatefulSet 裡面。

這個腳本會簡單的判斷這個 Pod 的 hostname，而且決定這是 master 或是 slave 然後再開啟適當的設定檔：

```bash
# init.sh
#!/bin/bash
if [[ ${HOSTNAME} == 'redis-0' ]]; then
  redis-server /redis-config/master.conf
else
  redis-server /redis-config/slave.conf
fi
```

其他的腳本是給 sentinel，我們必須等到`redis-0.redis`的 DNS name 是可用的：

```bash
# sentinel.sh
#!/bin/bash
while ! ping -c 1 redis-0.redis; do
    echo 'Waiting for server'
    sleep 1
done

redis-sentinel /redis-config/sentinel.conf
```

現在我們需要打包所有的東西到 ConfigMap 物件：

```shell
kubectl create configmap \
  --from-file=slave.conf=./slave.conf \
  --from-file=master.conf=./master.conf \
  --from-file=sentinel.conf=./sentinel.conf \
  --from-file=init.sh=./init.sh \
  --from-file=sentinel.sh=./sentinel.sh \
  redis-config
```

#### Creating a Redis Service

部署Redis的下一步是創建一個 Kubernetes 服務，為 Redis replica 提供命名和發現，像是`redis-0.redis`。我們建立 service 不需要 cluster IP。

```yaml
# redis-service.yaml
apiVersion: v1
kind: Service
metadata:
  name: redis
spec:
  ports:
  - port: 6379
    name: peer
  clusterIP: None
  selector:
    app: redis
```

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/14-9-redis-service.yaml
```

不用擔心 service 的 Pod 不存在。Kubernetes 不在乎，它會在創建 Pod 時會正確的添加名稱。

#### Deploying Redis

我們要開始建立 Redis cluster，我們會使用 StatefulSet：

```yaml
# redis.yaml
apiVersion: apps/v1beta1
kind: StatefulSet
metadata:
  name: redis
spec:
  replicas: 3
  serviceName: redis
  template:
    metadata:
      labels:
        app: redis
    spec:
      containers:
      - command: [sh, -c, source /redis-config/init.sh ]
        image: redis:3.2.7-alpine
        name: redis
        ports:
        - containerPort: 6379
          name: redis
        volumeMounts:
        - mountPath: /redis-config
          name: config
        - mountPath: /redis-data
          name: data
      - command: [sh, -c, source /redis-config/sentinel.sh]
        image: redis:3.2.7-alpine
        name: sentinel
        volumeMounts:
        - mountPath: /redis-config
          name: config
      volumes:
      - configMap:
          defaultMode: 420
          name: redis-config
        name: config
      - emptyDir:
        name: data
```

你會看到有兩個 conainer 在 Pod 裡面，一個是 Redis server 另一個是 sentinel 來監視 server。

你會看到有兩個 volume 定義在 Pod 裡面，一個是 ConfigMap 的 volume 去設定兩個 Redis application，另一個是`emptyDir` volume，他會對應到 Redis server container，存所有應用程式的資料，所以他被重啟的話資料還會留著。

```shell
kubectl apply -f https://raw.githubusercontent.com/kubernetes-up-and-running/examples/master/14-10-redis.yaml
```

#### Playing with Our Redis Cluster

我們可以看哪個 Redis sentinel 所監視到的是 master，我們可以使用`redis-cli`指令：

```shell
$ kubectl exec redis-2 -c redis -- redis-cli -p 26379 sentinel get-master-addr-by-name redis
172.17.0.9
6379
```

你可以列出 IP 來確定 master Redis 是那一台：

```shell
$ kubectl get pods -o wide
NAME        READY     STATUS     RESTARTS   AGE       IP            NODE
redis-0     2/2       Running    0          55s       172.17.0.9    minikube
redis-1     2/2       Running    0          46s       172.17.0.12   minikube
redis-2     2/2       Running    0          45s       172.17.0.13   minikube
```

接下來我們可以確認 replication 是真正運行的：

```shell
kubectl exec redis-2 -c redis -- redis-cli -p 6379 get foo
```

你會看到沒有任何資料回傳。

我們試著寫一些資料進去：

```shell
$ kubectl exec redis-2 -c redis -- redis-cli -p 6379 set foo 10
READONLY You can't write against a read only slave.
```

他會說你無法在 slave server 上寫入資料，我們改成 master `redis-0`：

```shell
$ kubectl exec redis-0 -c redis -- redis-cli -p 6379 set foo 10
OK
```

現在我們再從 slave 讀取資料：

```shell
$ kubectl exec redis-2 -c redis -- redis-cli -p 6379 get foo
10
```

他會印出正確的資料，你能看到資料在 master 和 slave 之間的複製。

## Reference

[Kubernetes Up and Running Example Github](https://github.com/kubernetes-up-and-running/examples)