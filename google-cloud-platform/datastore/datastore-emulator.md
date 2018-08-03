# Datastore Emulator

## 安裝

```shell
gcloud components install cloud-datastore-emulator
```

## 開啟模擬器

```shell
gcloud beta emulators datastore start
```

## 修改環境變數

一個指令完成：

```shell
$(gcloud beta emulators datastore env-init)
```

```shell
$ gcloud beta emulators datastore env-init
export DATASTORE_EMULATOR_HOST=localhost:8432
export DATASTORE_PROJECT_ID=my-project-id
```

## 停止模擬器

```shell
$(gcloud beta emulators datastore env-unset)
```

```shell
$ gcloud beta emulators datastore env-unset
unset DATASTORE_EMULATOR_HOST
unset DATASTORE_PROJECT_ID
```

