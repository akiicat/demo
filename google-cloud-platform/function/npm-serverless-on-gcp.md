# Serverless Google Function

## Install

```shell
npm install -g serverless
serverless login
```

## Google - Quick Start

使用 `create` 指令可以建立服務 `my-service`，用的 template 是 `google-nodes`，沒有加上 `--path` 不會幫你建立資料夾，加上 `--path` 會幫你建立資料夾，且自動填上服務的名字。

```shell
serverless create --template google-nodejs --path my-service
```

建立好後會看到兩個檔案在裡面：

- `serverless.yml`
- `index.js`

### serverless.yml

每個 `service` 配置都在 `serverless.yml` 檔案中進行管理。這個檔案的主要職責是：

- 宣告 serverless 的服務
- 在服務中定義一個或多個功能
  - 定義部署服務的程序（以及提供的運行時）
  - 定義要使用的任何自訂的套件
  - 定義觸發每個要執行的函數的事件（例如HTTP請求）
  - 允許事件在部署時自動創建事件所需的資源
  - 允許使用 serverless 變數進行彈性的配置

```js
# serverless.yml

service: my-gcloud-service

provider:
  name: google

plugins:
  - serverless-google-cloudfunctions

functions:
  first:
    handler: http
    events:
      - http: path
```

安裝依賴套件

```shell
cd my-service
npm install
```

在 `serverless.yml` 加上 [serverless 官網](https://dashboard.serverless.com/tenants/akiicat/applications/) 上專案的名稱，`project` 的名稱也要改：

```yaml
service: my-service

app: my-service
tenant: akiicat

provider:
  name: google
  runtime: nodejs
  project: <project-id>
  # the path to the credentials file needs to be absolute
  credentials: ~/.gcloud/keyfile.json
```

## Credentials

需要開啟的 API

- Cloud Storage
- Cloud Function
- Stackdriver Logging
- Google Cloud Deployment Manager

### 建立憑證

1. 選取左邊的**憑證**
2. **建立憑證**->**服務帳戶金鑰**
3. 在下拉選單中選取**新增服務帳戶**
4. **服務帳戶名稱**輸入，像是 serverless-framework
5. **角色**選取 **Project** --> **擁有者**
6. **金鑰類型**選 **JSON**
7. **建立**
8. 會下一個 `keyfile` 的檔案
9. 妥善保存 `keyfile`，建議放在 `~/.gcloud/keyfile.json`，你也可以放在任何你記得的地方。

### 更新 serverless.yml

將路徑指到 `keyfile` 的位置：

```yaml
# serverless.yml
provider:
  name: google
  runtime: nodejs
  project: my-serverless-project-1234
  credentials: ~/.gcloud/keyfile.json
```

## 部署

部署 serverless 到 google cloud function 上。

```shell
$ serverless deploy
Serverless: Packaging service...
Serverless: Excluding development dependencies...
Serverless: Compiling function "first"...
Serverless: Uploading artifacts...
Serverless: Artifacts successfully uploaded...
Serverless: Updating deployment...
Serverless: Checking deployment update progress...
.............
Serverless: Done...
Service Information
service: my-service
project: sunlit-inquiry-164609
stage: dev
region: us-central1

Deployed functions
first
  https://us-central1-sunlit-inquiry-164609.cloudfunctions.net/http
```

## 呼叫

呼叫 `serverless.yml` 裡的 first function：

```shell
$ serverless invoke --function first
Serverless: kafb0nnth8fg Hello World!
```

## 移除

移除已經 deploy 的 google cloud function，本地的檔案還會留著：

```shell
serverless remove
```

## Nested

讓 function 結構化，在 functions 裡引用 function 的 yml

```yaml
# serverless.yml
...

functions:
  - ${file(../foo-functions.yml)}
  - ${file(../bar-functions.yml)}
```

```yaml
# foo-functions.yml
getFoo:
  handler: handler.foo
deleteFoo:
  handler: handler.foo
```

## 修改版本

修改 `serverless.yml` 的檔案，因為無法將版本固定在 `package.json` 裡面。

### 確切版本

```yaml
# serverless.yml
frameworkVersion: "=1.0.3"
```

### 範圍版本

```yaml
# serverless.yml
frameworkVersion: ">=1.0.0 <2.0.0"
```

### 使用特定版本的 serverless

```shell
# from within a service
npm install serverless --save-dev
```

然後之後去呼叫他：

```shell
node ./node_modules/serverless/bin/serverless deploy
```

## 限制

限制記憶體跟 timeout

```yaml
# serverless.yml

provider:
  memorySize: 1024
  timeout: 90s

functions:
  first:
    handler: first
  second:
    handler: second
    memorySize: 512
    timeout: 120s
```

## Label

```yaml
# serverless.yml

provider:
  name: google
  labels:
    application: Serverless Example

functions:
  first:
    handler: httpFirst
    events:
      - http: path
    labels:
      team: GCF Team
  second:
    handler: httpSecond
    events:
      - http: path
    labels:
      application: Serverless Example - Documentation
```

`httpFirst` 有兩個 label：`application`、`team`

`httpSecond`只有一個 label：`application`

## Packaging

package 完會存放在 `.serverless` 資料夾下。

```shell
serverless package
```

### Exclude / include

前面加 `!` ，會在 exclude 中 include 這個東西，serverless 會有順序的運行這個東西：

```yaml
package:
  exclude:
    - node_modules/**
    - '!node_modules/node-fetch/**'
```

排除所有 `src` 檔案，但是包含 `handler.js`

```yaml
package:
  exclude:
    - src/**
  include:
    - src/function/handler.js
```

```yaml
package:
  exclude:
    - tmp/**
    - .git/**
```

###  Packaging functions separately

可以在 function 裡面再分別使用 exclude 和 include：

```yaml
service: my-service
package:
  individually: true
  exclude:
    - excluded-by-default.json
functions:
  hello:
    handler: handler.hello
    package:
      # We're including this file so it will be in the final package of this function only
      include:
        - excluded-by-default.json
  world:
    handler: handler.hello
    package:
      exclude:
        - some-file.js
```

或是想要某個 function 個別的 package：

```yaml
service: my-service
functions:
  hello:
    handler: handler.hello
  world:
    handler: handler.hello
    package:
      individually: true
```

### Development dependencies

或是讓 serverless 自動偵測要 exclude 的檔案：

```yaml
package:
  excludeDevDependencies: false
```

## Variables

https://serverless.com/framework/docs/providers/google/guide/variables/

## Plugins

https://serverless.com/framework/docs/providers/google/guide/plugins/

## Workflow

https://serverless.com/framework/docs/providers/google/guide/workflow/

## Reference

https://serverless.com/framework/docs/getting-started/

https://serverless.com/framework/docs/providers/google/guide/credentials/

https://serverless.com/framework/docs/providers/google/cli-reference/create/