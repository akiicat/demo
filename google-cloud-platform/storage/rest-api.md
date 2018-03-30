# GCP Storage Rest Api

- 新增 Bucket
- 編輯 Bucket 權限

## 新增 Bucket 公開讀取權限

- 新增成員填寫 `allUsers`
- 角色填寫 `Storage Legacy` -> `Storage 繼承物件讀取者`

## 新增 Bucket 寫入權限

- 新增成員填寫 `allUsers`
- 角色填寫 `Storage Legacy` -> `Storage 繼承 Bucket 寫入者`

## 讀取 (Public)

```
GET https://storage.googleapis.com/<BUCKET_ID>/<FILE_PATH_AND_FILE_NAME>
```

## 寫入

新增 Bucket 寫入權限後：

### 單一檔案上傳

[文件](https://cloud.google.com/storage/docs/json_api/v1/how-tos/simple-upload)

```
POST https://www.googleapis.com/upload/storage/v1/b/<BUCKET_ID>/o?uploadType=media&name=<FILE_PATH_AND_FILE_NAME>
Content-Type: image/jpeg
Content-Length: <NUMBER_OF_BYTES_IN_FILE>

<IMAGE_DATA>
```

## 檔案狀態

```
GET https://www.googleapis.com/storage/v1/b/<BUCKET_ID>/o/<FILE_PATH_AND_FILE_NAME>
```

## 改網域

https://cloud.google.com/storage/docs/request-endpoints
