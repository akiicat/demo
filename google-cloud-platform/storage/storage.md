# GCP Storage

[Quickstart](https://cloud.google.com/storage/docs/quickstart-gsutil)

## 建立

`gsutil mb` 可以建立唯一一個名稱的 bucket

```shell
gsutil mb gs://my-awesome-bucket/
```

## 儲存

```shell
gsutil cp Desktop/kitten.png gs://my-awesome-bucket
```

## 下載

```shell
gsutil cp gs://my-awesome-bucket/kitten.png Desktop
```

## 在 bucket 裡複製檔案

```shell
gsutil cp gs://my-awesome-bucket/kitten.png gs://my-awesome-bucket/just-a-folder/
```

## 移動

```shell
gsutil mv gs://my_bucket/* dir
gsutil mv ./dir gs://my_bucket
gsutil mv gs://my_bucket/olddir gs://my_bucket/newdir
```

## 顯示 bucket

```shell
gsutil ls gs://my-awesome-bucket
```

詳細資訊

```shell
gsutil ls -l gs://my-awesome-bucket/kitten.png
```

## 權限

所有使用者獲得讀取權限

```shell
gsutil acl ch -u AllUsers:R gs://my-awesome-bucket/kitten.png
```

移除上面的權限

```shell
gsutil acl ch -d AllUsers gs://my-awesome-bucket/kitten.png
```

讓某個使用者獲得讀取和寫入的權限

```shell
gsutil acl ch -u user@gmail.com:W gs://my-awesome-bucket
```

移除某個使用者的權限

```shell
gsutil acl ch -d user@gmail.com gs://my-awesome-bucket
```

## 刪除

```shell
gsutil rm gs://my-awesome-bucket/kitten.png
```

```shell
gsutil rm -r gs://my-awesome-bucket
```

## 同步

```shell
gsutil rsync -r src_dir gs://my-awesome-bucket
```

## 平行化

```shell
gsutil -m rsync -r src_dir gs://my-awesome-bucket
```

## Cross Domain

https://cloud.google.com/storage/docs/gsutil/commands/cors

```json
# cors-config.json
[  
   {  
      "origin":[  
         "*"
      ],
      "responseHeader":[  
         "Content-Type"
      ],
      "method":[  
         "GET",
         "HEAD",
         "POST"
      ],
      "maxAgeSeconds":3600
   }
]
```

```shell
echo '[{"origin": ["*"],"responseHeader": ["Content-Type"],"method": ["GET", "HEAD", "POST"],"maxAgeSeconds": 3600}]' > cors-config.json
gsutil cors set cors-config.json gs://<your-bucket-name>
gsutil cors get gs://<your-bucket-name>
```

