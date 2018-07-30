# Serverless on Google with Cloud Functions

## Create a new gcloud project

設定 google cloud project

```shell
gcloud config set core/project sunlit-inquiry-164609
gcloud config set compute/zone us-east1
```

建立 node 專案

```shell
npm init
```

```shell
$ cat package.json 
{
  "name": "serverless-example",
  "version": "0.0.1",
  "description": "",
  "main": "index.js",
  "scripts": {
    "test": "echo \"Error: no test specified\" && exit 1"
  },
  "author": "",
  "license": "ISC"
}
```

Cloud function emulator 允許你在本地電腦上開發和測試功能，這大幅加快了開發速度。

```shell
npm install -g @google-cloud/functions-emulator
functions config set projectId sunlit-inquiry-164609
functions start
functions deploy posts --trigger-http
```

`posts` 是 `index.js` 裡的進入點，然後使用 [node-fetch](https://www.npmjs.com/package/node-fetch) 套件可以依照頁數抓取 o'reilly 的網頁：

```js
// index.js
var fetch = require("node-fetch");
const cors = require("cors")();

//===========================================================================
// Get the topic types from ORM site
//===========================================================================
exports.posts = (req, res) => {
  // Parse the body to get the passed values
  var params;
  try {
    // try to parse the body for passed parameters
    params = JSON.parse(req.body);
  } catch (e) {
    // provide default values if there is an error
    params = { page: 1 };
  }
  // Wrap function in a CORS header so is can be called in the browser
  cors(req, res, () => {
    // Lookup the account type based on the email.
    fetch(
      "https://gateway.oreilly.com/clients/website/feed/all/page/" +
        params["page"],
      {}
    )
      .then(res => {
        return res.json();
      })
      .then(data => {
        res.send(data["posts"]);
      });
  });
};
```

deploy 在本地測試：

```shell
$ functions deploy posts --trigger-http
Warning: You're using Node.js v8.11.1 but Google Cloud Functions only supports v6.11.5.
Copying file:///var/folders/3s/4__r12zn00z5ncz49rd1zhr40000gn/T/tmp-15907g3W83RBz3i15.zip...
Waiting for operation to finish...done.
Deploying function......done.
Function posts deployed.
┌────────────┬───────────────────────────────────────────────────────────────────────────────────┐
│ Property   │ Value                                                                             │
├────────────┼───────────────────────────────────────────────────────────────────────────────────┤
│ Name       │ posts                                                                             │
├────────────┼───────────────────────────────────────────────────────────────────────────────────┤
│ Trigger    │ HTTP                                                                              │
├────────────┼───────────────────────────────────────────────────────────────────────────────────┤
│ Resource   │ http://localhost:8010/sunlit-inquiry-164609/us-central1/posts                     │
├────────────┼───────────────────────────────────────────────────────────────────────────────────┤
│ Timeout    │ 60 seconds                                                                        │
├────────────┼───────────────────────────────────────────────────────────────────────────────────┤
│ Local path │ /Users/akiicat/Github/serverless-example                                          │
├────────────┼───────────────────────────────────────────────────────────────────────────────────┤
│ Archive    │ file:///var/folders/3s/4__r12zn00z5ncz49rd1zhr40000gn/T/tmp-15907g3W83RBz3i15.zip │
└────────────┴───────────────────────────────────────────────────────────────────────────────────┘
```

 `cors` 會在 header 加上瀏覽器的  [cross-origin security restrictions](https://developer.mozilla.org/en-US/docs/Web/HTTP/Access_control_CORS) ，沒有這個會出錯。

 ```shell
npm install --save node-fetch
npm install --save cors
 ```

然後使用 httpie 請求

```json
$ http -b http://localhost:8010/sunlit-inquiry-164609/us-central1/posts page=1
[
    {
        "byline": {
            "authors": [
                {
                    "first-name": "Ben",
                    "last-name": "Lorica",
                    "url": "/people/4e7ad-ben-lorica"
                }
            ]
        },
...
```

一但有新增新的功能在 `index.js` 裡面，只要儲存過後就會 live reload。而且確保每個你新加的 function 有被個別加入

```shell
functions deploy yourNewFunction --trigger-http
```

## Deploying your function to Google Cloud

建立 storage bucket `gs://akiicat-temp`，然後將程式碼上傳：

```shell
gsutil mb -p sunlit-inquiry-164609 gs://akiicat-temp       # 建立 storage
```

部署到 google cloud function 上

```shell
gcloud beta functions deploy posts --stage-bucket akiicat-temp --trigger-http
```

```shell
$ gcloud beta functions list
NAME   STATUS  TRIGGER       REGION
posts  ACTIVE  HTTP Trigger  us-central1

$ gcloud beta functions describe posts
availableMemoryMb: 256
entryPoint: posts
httpsTrigger:
  url: https://us-central1-sunlit-inquiry-164609.cloudfunctions.net/posts
labels:
  deployment-tool: cli-gcloud
name: projects/sunlit-inquiry-164609/locations/us-central1/functions/posts
runtime: nodejs6
serviceAccountEmail: sunlit-inquiry-164609@appspot.gserviceaccount.com
sourceArchiveUrl: gs://akiicat-temp/us-central1-projects/sunlit-inquiry-164609/locations/us-central1/functions/posts-cyqgtbxjhgcv.zip
status: ACTIVE
timeout: 60s
updateTime: '2018-07-30T14:56:18Z'
versionId: '2'
```



## Reference

https://www.oreilly.com/learning/serverless-on-google-with-cloud-functions-and-react