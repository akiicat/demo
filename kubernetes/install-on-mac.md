# 安裝

https://gist.github.com/kevin-smets/b91a34cea662d0c523968472a81788f7

## hyperkit

```
brew tap markeissler/hyperkit
brew install hyperkit
```

## minikube

版本會變看[這裡](https://github.com/kubernetes/minikube/releases)

```
brew cask install minikube
```

## kubectl

其他環境的安裝[在這](https://kubernetes.io/docs/tasks/tools/install-kubectl/)

```
brew install kubectl
```

## Check the kubectl configuration

```
kubectl cluster-info
```

如果是安裝在本地端會看到這樣的錯誤

```
The connection to the server <server-name:port> was refused - did you specify the right host or port?
```

要先安裝 minikube 然後再跑下面的指令

```
kubectl cluster-info dump
```

## Enabling shell autocompletion

```
brew install bash-completion # BASH_VERION 3.2
kubectl completion bash > $(brew --prefix)/etc/bash_completion.d/kubectl
```