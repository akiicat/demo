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



##### Resource Requests: Minimum Required Resources

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

#### 









## Reference

[Kubernetes Up and Running Example Github](https://github.com/kubernetes-up-and-running/examples)