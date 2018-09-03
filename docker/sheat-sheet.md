# Cheat Sheet

## Remove

### container

```shell
docker ps -aq # 列出所有 container id

docker stop $(docker ps -aq) # 停止所有 container

docker rm $(docker ps -aq) # 移除所有 container
```

### images

```shell
docker rmi $(docker images -f dangling=true -q) # 移除標籤為 <none> 的 images

docker rmi $(docker images -q) # 移除所有 images
```

### Volumes

```shell
docker volume rm $(docker volume ls -q --filter dangling=true)
```

## Trouble Shoot

### restart after reboot in CoreOS

https://github.com/docker/compose/issues/3241#issuecomment-206925136

```shell
sudo systemctl enable docker.service
sudo systemctl start docker.service
```
