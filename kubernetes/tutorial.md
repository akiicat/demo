# Kubernetes

## Minikube

- cluster: master 多個機器
- node: 單一一台機器



執行 Kubernetes cluster

```shell
minikube start
```



cluster 的詳細資料跟健康狀態

```shell
kubectl cluster-info
```



查看 cluster 裡面的 nodes

```shell
kubectl get nodes
```



部署第一個 container 到 cluster 上

```shell
kubectl run first-deployment --image=katacoda/docker-http-server --port=80
```



顯示剛剛部署的訊息

```shell
kubectl get pods
```



NodePort 提供動態開放的 port

```shell
kubectl expose deployment first-deployment --port=80 --type=NodePort
```



獲得 port

```shell
export PORT=$(kubectl get svc first-deployment -o go-template='{{range.spec.ports}}{{if .nodePort}}{{.nodePort}}{{"\n"}}{{end}}{{end}}')
echo "Accessing host01:$PORT"
curl host01:$PORT
```



## Kubeadm

初始化 cluster，token??（好像要自己先產生）

```shell
kubeadm init --token=102952.1a7dd4cc8d1f4cc5 --kubernetes-version $(kubeadm version -o short)
```



我們需要 kubeadm 的設定檔以及憑證

```shell
sudo cp /etc/kubernetes/admin.conf $HOME/
sudo chown $(id -u):$(id -g) $HOME/admin.conf
export KUBECONFIG=$HOME/admin.conf
```



列出 kubeadm token

```shell
kubeadm token list
```



使別台機器加入 master

```shell
kubeadm join --discovery-token-unsafe-skip-ca-verification --token=102952.1a7dd4cc8d1f4cc5 172.17.0.37:6443
```

- `--discovery-token-unsafe-skip-ca-verification` 用於繞過 token 的驗證



在 master 使用 Kubernetes CLI，可以在 cluster 中找到所有 nodes

```shell
kubectl get nodes
```



### 透過 Container Networking Interface (CNI) 部署

這裡是使用 weavekube 可以用 `kubectl apply` 來部署

https://www.weave.works/docs/net/latest/kubernetes/kube-addon/

```shell
cat /opt/weave-kube
kubectl apply -f /opt/weave-kube
kubectl get pod -n kube-system
```



```shell
kubectl run http --image=katacoda/docker-http-server:latest --replicas=1
kubectl get pods # master
docker ps | grep docker-http-server # node
```





dashboard https://github.com/kubernetes/dashboard/wiki/Creating-sample-user

```shell
kubectl apply -f dashboard.yaml
```



```shell
kubectl -n kube-system describe secret $(kubectl -n kube-system get secret | grep admin-user | awk '{print $1}')
```















