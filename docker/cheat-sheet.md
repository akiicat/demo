# Cheat Sheet

## Remove

### container

```shell
docker ps -aq # 列出所有 container id

docker ps -q | xargs docker stop # 停止所有 container

docker ps -aq | xargs docker rm # 移除所有 container
```

### images

```shell
docker images -f dangling=true -q | xargs docker rmi # 移除標籤為 <none> 的 images

docker images -q | xargs docker rmi # 移除所有 images
```

### Volumes

```shell
docker volume ls -qf dangling=true | xargs docker volume rm
```

## Trouble Shoot

### restart after reboot in CoreOS

https://github.com/docker/compose/issues/3241#issuecomment-206925136

```shell
sudo systemctl enable docker.service
sudo systemctl start docker.service
```
