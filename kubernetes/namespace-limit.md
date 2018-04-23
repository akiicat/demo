# Kubernetes Ｎamespace Limit

## Namespace

- ResourceQuota：可以限制同一個 Namespace 裡的所以容器總和
- LimitRange：可以限制同一個 Namespace 裡的單一個容器

### ResourceQuota

#### 限制 namespace 下的總 POD 數 

設定 namespace 最大 pods 的數量為 ２

```yaml
apiVersion: v1
kind: ResourceQuota
metadata:
  name: pod-demo
spec:
  hard:
    pods: "2"
```

套用設定到 namespace 是 quota-pod-example 上

```shell
kubectl create namespace quota-pod-example
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/quota-pod.yaml --namespace=quota-pod-example
kubectl get resourcequota pod-demo --namespace=quota-pod-example --output=yaml
```



##### 測試

```yaml
apiVersion: apps/v1beta1
kind: Deployment
metadata:
  name: pod-quota-demo
spec:
  replicas: 3
  template:
    metadata:
      labels:
        purpose: quota-demo
    spec:
      containers:
      - name: pod-quota-demo
        image: nginx
```

```shell
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/quota-pod-deployment.yaml --namespace=quota-pod-example
kubectl get deployment pod-quota-demo --namespace=quota-pod-example --output=yaml
```



#### 限制 namespace 總 CPU 和 Memory 的使用量

使用情境：

- 集群中每個節點擁有 2 GB 內存。您不希望任何 Pod 請求超過 2 GB 的內存，因為集群中沒有節點能支持這個請求。
- 集群被生產部門和開發部門共享。 您希望生產負載最多使用 8 GB 的內存而將開發負載限制為 512 MB。這種情況下，您可以為生產環境和開發環境創建單獨的 namespace，並對每個 namespace 應用內存限制。

```yaml
apiVersion: v1
kind: ResourceQuota
metadata:
  name: mem-cpu-demo
spec:
  hard:
    requests.cpu: "1"
    requests.memory: 1Gi
    limits.cpu: "2"
    limits.memory: 2Gi
```

```shell
kubectl create namespace quota-mem-cpu-example
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/quota-mem-cpu.yaml --namespace=quota-mem-cpu-example
kubectl get resourcequota mem-cpu-demo --namespace=quota-mem-cpu-example --output=yaml
```

```yaml
status:
  hard:
    limits.cpu: "2"
    limits.memory: 2Gi
    requests.cpu: "1"
    requests.memory: 1Gi
  used:
    limits.cpu: "0"
    limits.memory: "0"
    requests.cpu: "0"
    requests.memory: "0"
```



##### 創建一個 POD

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: quota-mem-cpu-demo
spec:
  containers:
  - name: quota-mem-cpu-demo-ctr
    image: nginx
    resources:
      limits:
        memory: "800Mi"
        cpu: "800m" 
      requests:
        memory: "600Mi"
        cpu: "400m"
```

```shell
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/quota-mem-cpu-pod.yaml --namespace=quota-mem-cpu-example
kubectl get pod quota-mem-cpu-demo --namespace=quota-mem-cpu-example
kubectl get resourcequota mem-cpu-demo --namespace=quota-mem-cpu-example --output=yaml
```

```yaml
status:
  hard:
    limits.cpu: "2"
    limits.memory: 2Gi
    requests.cpu: "1"
    requests.memory: 1Gi
  used:
    limits.cpu: 800m
    limits.memory: 800Mi
    requests.cpu: 400m
    requests.memory: 600Mi
```



##### 建立第一個會超過使用量的 POD

因為前一個 POD 用了 600Mi 加上這個 700Mi > 1Gi，所以建立時會發生錯誤。

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: quota-mem-cpu-demo-2
spec:
  containers:
  - name: quota-mem-cpu-demo-2-ctr
    image: redis
    resources:
      limits:
        memory: "1Gi"
        cpu: "800m"      
      requests:
        memory: "700Mi"
        cpu: "400m"
```

```shell
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/quota-mem-cpu-pod-2.yaml --namespace=quota-mem-cpu-example
```

```shell
Error from server (Forbidden): error when creating "docs/tasks/administer-cluster/quota-mem-cpu-pod-2.yaml":
pods "quota-mem-cpu-demo-2" is forbidden: exceeded quota: mem-cpu-demo,
requested: requests.memory=700Mi,used: requests.memory=600Mi, limited: requests.memory=1Gi
```









### LimitRange

#### 限制 CPU Limit 和 Request

設定 CPU 使用量

```yaml
apiVersion: v1
kind: LimitRange
metadata:
  name: cpu-limit-range
spec:
  limits:
  - default:
      cpu: 1
    defaultRequest:
      cpu: 0.5
    type: Container
```



```shell
kubectl create namespace default-cpu-example
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/cpu-defaults.yaml --namespace=default-cpu-example
```



```shell
$ kubectl describe namespace default-cpu-example
Name:         default-cpu-example
Labels:       <none>
Annotations:  <none>
Status:       Active

No resource quota.

Resource Limits
 Type       Resource  Min  Max  Default Request  Default Limit  Max Limit/Request Ratio
 ----       --------  ---  ---  ---------------  -------------  -----------------------
 Container  cpu       -    -    500m             1              -
```



##### 測試：不加任何限制

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: default-cpu-demo
spec:
  containers:
  - name: default-cpu-demo-ctr
    image: nginx
```

```shell
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/cpu-defaults-pod.yaml --namespace=default-cpu-example
kubectl get pod default-cpu-demo --output=yaml --namespace=default-cpu-example
```

```yaml
containers:
- image: nginx
  imagePullPolicy: Always
  name: default-cpu-demo-ctr
  resources:
    limits:
      cpu: "1"
    requests:
      cpu: 500m
```



##### 測試：限制 CPU 上限，但未指定請求值

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: default-cpu-demo-2
spec:
  containers:
  - name: default-cpu-demo-2-ctr
    image: nginx
    resources:
      limits:
        cpu: "1"
```

```shell
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/cpu-defaults-pod-2.yaml --namespace=default-cpu-example
kubectl get pod default-cpu-demo-2 --output=yaml --namespace=default-cpu-example
```

```yaml
resources:
  limits:
    cpu: "1"
  requests:
    cpu: "1"
```



##### 測試：指定 CPU 請求值，但未指定上限

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: default-cpu-demo-3
spec:
  containers:
  - name: default-cpu-demo-3-ctr
    image: nginx
    resources:
      requests:
        cpu: "0.75"
```

```shell
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/cpu-defaults-pod-3.yaml --namespace=default-cpu-example
kubectl get pod default-cpu-demo-3 --output=yaml --namespace=default-cpu-example
```

```yaml
resources:
  limits:
    cpu: "1"
  requests:
    cpu: 750m
```



#### 限制 Memory Limit 和 Request

- 在 namespace 運行的每一個容器必須有它自己的 memory limit。
- 在 namespace 中所有的 container 使用的 memory 總量不能超出指定的 limit。
- 如果一個容器沒有指定它自己的內存限額，它將被賦予默認的限額值，然後它才可以在被配額限制的命名空間中運行。

```yaml
apiVersion: v1
kind: LimitRange
metadata:
  name: mem-limit-range
spec:
  limits:
  - default:
      memory: 512Mi
    defaultRequest:
      memory: 256Mi
    type: Container
```

```shell
kubectl create namespace default-mem-example
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/memory-defaults.yaml --namespace=default-mem-example
```



##### 測試：沒有任何限制

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: default-mem-demo
spec:
  containers:
  - name: default-mem-demo-ctr
    image: nginx
```

創建 Pod

```shell
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/memory-defaults-pod.yaml --namespace=default-mem-example
kubectl get pod default-mem-demo --output=yaml --namespace=default-mem-example
```

```yaml
containers:
- image: nginx
  imagePullPolicy: Always
  name: default-mem-demo-ctr
  resources:
    limits:
      memory: 512Mi
    requests:
      memory: 256Mi
```

刪除 POD

```shell
kubectl delete pod default-mem-demo --namespace=default-mem-example
```



##### 測試：指定 memory 的上限，但未指定請求值

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: default-mem-demo-2
spec:
  containers:
  - name: defalt-mem-demo-2-ctr
    image: nginx
    resources:
      limits:
        memory: "1Gi"
```

```shell
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/memory-defaults-pod-2.yaml --namespace=default-mem-example
kubectl get pod default-mem-demo-2 --output=yaml --namespace=default-mem-example
```

```yaml
resources:
  limits:
    memory: 1Gi
  requests:
    memory: 1Gi
```



##### 測試：指定請求值，未指定上限

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: default-mem-demo-3
spec:
  containers:
  - name: default-mem-demo-3-ctr
    image: nginx
    resources:
      requests:
        memory: "128Mi"
```

```shell
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/memory-defaults-pod-3.yaml --namespace=default-mem-example
kubectl get pod default-mem-demo-3 --output=yaml --namespace=default-mem-example
```

```yaml
resources:
  limits:
    memory: 512Mi
  requests:
    memory: 128Mi
```



#### 限制 Memory 最大最小值

使用情境：

- 集群中每個節點擁有 2 CPU。您不希望任何 Pod 請求超過 2 CPU 的資源，因為集群中沒有節點能支持這個請求。
- 集群被生產部門和開發部門共享。 您希望生產負載最多使用 3 CPU 而將開發負載限制為 1 CPU。這種情況下，您可以為生產環境和開發環境創建單獨的 namespace，並對每個 namespace 應用 CPU 限制。

```yaml
apiVersion: v1
kind: LimitRange
metadata:
  name: mem-min-max-demo-lr
spec:
  limits:
  - max:
      memory: 1Gi
    min:
      memory: 500Mi
    type: Container
```

```shell
kubectl create namespace constraints-mem-example
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/memory-constraints.yaml --namespace=constraints-mem-example
kubectl get limitrange mem-min-max-demo-lr --namespace=constraints-mem-example --output=yaml
```

```yaml
spec:
  limits:
  - default:
      memory: 1Gi
    defaultRequest:
      memory: 1Gi
    max:
      memory: 1Gi
    min:
      memory: 500Mi
```

現在，每當在 constraints-mem-example namespace 中創建一個容器時，Kubernetes 都會執行下列步驟：

- 如果容器沒有指定自己的內存請求（request）和限制（limit），系統將會為其分配默認值。
- 驗證容器的內存請求大於等於 500 MiB。
- 驗證容器的內存限制小於等於 1 GiB。



##### 測試：指定 limit 和 request

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: constraints-mem-demo
spec:
  containers:
  - name: constraints-mem-demo-ctr
    image: nginx
    resources:
      limits:
        memory: "800Mi"
      requests:
        memory: "600Mi"
```

```shell
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/memory-constraints-pod.yaml --namespace=constraints-mem-example
kubectl get pod constraints-mem-demo --namespace=constraints-mem-example
kubectl get pod constraints-mem-demo --output=yaml --namespace=constraints-mem-example
```

```yaml
resources:
  limits:
     memory: 800Mi
  requests:
    memory: 600Mi
```

刪除 POD

```shell
kubectl delete pod constraints-mem-demo --namespace=constraints-mem-example
```



##### 測試：嘗試建立一個超過最大內存限制的 Pod

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: constraints-mem-demo-2
spec:
  containers:
  - name: constraints-mem-demo-2-ctr
    image: nginx
    resources:
      limits:
        memory: "1.5Gi"
      requests:
        memory: "800Mi"
```

```shell
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/memory-constraints-pod-2.yaml --namespace=constraints-mem-example
```

錯誤訊息

```shell
Error from server (Forbidden): error when creating "docs/tasks/administer-cluster/memory-constraints-pod-2.yaml":
pods "constraints-mem-demo-2" is forbidden: maximum memory usage per Container is 1Gi, but limit is 1536Mi.
```



##### 測試：嘗試創建一個不符合最小內存請求的 Pod

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: constraints-mem-demo-3
spec:
  containers:
  - name: constraints-mem-demo-3-ctr
    image: nginx
    resources:
      limits:
        memory: "800Mi"
      requests:
        memory: "100Mi"
```

```shell
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/memory-constraints-pod-3.yaml --namespace=constraints-mem-example
```

錯誤訊息

```shell
Error from server (Forbidden): error when creating "docs/tasks/administer-cluster/memory-constraints-pod-3.yaml":
pods "constraints-mem-demo-3" is forbidden: minimum memory usage per Container is 500Mi, but request is 100Mi.
```



##### 測試：創建一個沒有指定任何內存請求和限制的 Pod

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: constraints-mem-demo-4
spec:
  containers:
  - name: constraints-mem-demo-4-ctr
    image: nginx
```

```shell
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/memory-constraints-pod-4.yaml --namespace=constraints-mem-example
kubectl get pod constraints-mem-demo-4 --namespace=constraints-mem-example --output=yaml
```

```yaml
resources:
  limits:
    memory: 1Gi
  requests:
    memory: 1Gi
```

```shell
kubectl delete pod constraints-mem-demo-4 --namespace=constraints-mem-example
```



#### 限制 CPU 最大最小

```yaml
apiVersion: v1
kind: LimitRange
metadata:
  name: cpu-min-max-demo-lr
spec:
  limits:
  - max:
      cpu: "800m"
    min:
      cpu: "200m"
    type: Container
```

```shell
kubectl create namespace constraints-cpu-example
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/cpu-constraints.yaml --namespace=constraints-cpu-example
kubectl get limitrange cpu-min-max-demo-lr --output=yaml --namespace=constraints-cpu-example
```

```yaml
limits:
- default:
    cpu: 800m
  defaultRequest:
    cpu: 800m
  max:
    cpu: 800m
  min:
    cpu: 200m
  type: Container
```



##### 測試：請求 CPU 是 500m 的 POD

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: constraints-cpu-demo
spec:
  containers:
  - name: constraints-cpu-demo-ctr
    image: nginx
    resources:
      limits:
        cpu: "800m"
      requests:
        cpu: "500m"
```

```shell
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/cpu-constraints-pod.yaml --namespace=constraints-cpu-example
kubectl get pod constraints-cpu-demo --namespace=constraints-cpu-example
kubectl get pod constraints-cpu-demo --output=yaml --namespace=constraints-cpu-example
```

```yaml
resources:
  limits:
    cpu: 800m
  requests:
    cpu: 500m
```

刪除 POD

```shell
kubectl delete pod constraints-cpu-demo --namespace=constraints-cpu-example
```



##### 測試：嘗試創建一個超過最大 CPU 限制的 Pod

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: constraints-cpu-demo-2
spec:
  containers:
  - name: constraints-cpu-demo-2-ctr
    image: nginx
    resources:
      limits:
        cpu: "1.5"
      requests:
        cpu: "500m"
```

```shell
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/cpu-constraints-pod-2.yaml --namespace=constraints-cpu-example
```

```shell
Error from server (Forbidden): error when creating "docs/tasks/administer-cluster/cpu-constraints-pod-2.yaml":
pods "constraints-cpu-demo-2" is forbidden: maximum cpu usage per Container is 800m, but limit is 1500m.
```



##### 測試：嘗試創建一個不符合最小 CPU 請求的 Pod

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: constraints-cpu-demo-4
spec:
  containers:
  - name: constraints-cpu-demo-4-ctr
    image: nginx
    resources:
      limits:
        cpu: "800m"
      requests:
        cpu: "100m"
```

```shell
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/cpu-constraints-pod-3.yaml --namespace=constraints-cpu-example
```

```shell
Error from server (Forbidden): error when creating "docs/tasks/administer-cluster/cpu-constraints-pod-3.yaml":
pods "constraints-cpu-demo-4" is forbidden: minimum cpu usage per Container is 200m, but request is 100m.
```



##### 測試：創建一個沒有指定任何 CPU 請求和限制的 Pod

因為您的容器沒有指定自己的 CPU 請求和限制，所以它將從 LimitRange 獲取 [默認的 CPU 請求和限制值](http://docs.kubernetes.org.cn/747.html)。

到目前為止，您的容器可能在運行，也可能沒有運行。回想起來，有一個先決條件就是節點必須至少擁有 1 CPU。如果每個節點都只有 1 CPU，那麼任何一個節點上都沒有足夠的可用 CPU 來容納 800 millicpu 的請求。如果碰巧使用的節點擁有 2 CPU，那麼它可能會有足夠的 CPU 來容納 800 millicpu 的請求。

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: constraints-cpu-demo-4
spec:
  containers:
  - name: constraints-cpu-demo-4-ctr
    image: vish/stress
```

```yaml
kubectl create -f https://k8s.io/docs/tasks/administer-cluster/cpu-constraints-pod-4.yaml --namespace=constraints-cpu-example
kubectl get pod constraints-cpu-demo-4 --namespace=constraints-cpu-example --output=yaml
```

```yaml
resources:
  limits:
    cpu: 800m
  requests:
    cpu: 800m
```

