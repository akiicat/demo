# The Node Environment

## 安裝 Node

自已在 ubuntu 上 compile 的話：

```shell
apt-get install make g++ libssl-dev git
```

如果安裝在 Raspberry Pi 的話，選擇相對應的 ARM 版本：

- Raspberry Pi：需要 ARMv6
- Raspberry Pi 2：需要 ARMv7

```shell
wget https://nodejs.org/dist/v4.0.0/node-v4.0.0-linux-armv7l.tar.gz
tar -xvf node-v4.0.0-linux-armv7l.tar.gz
cd node-v4.0.0-linux-armv7l
sudo cp -R * /usr/local/
```

## Hello World

```js
// chap1-1.js
var http = require('http');

http.createServer(function (request, response) {
  response.writeHead(200, {'Content-Type': 'text/plain'});
  response.end('Hello World\n');
}).listen(8124);

console.log('Server running at http://127.0.0.1:8124/');
```

`require` 的結果會被指派到區域變數上，一旦 import 進來後，就可以使用區域變數來產生 web server。

`http.createServer()`的參數是 function，他會將匿名函數傳入，function 會將 request 和 response 傳入到程式碼中來處理網頁請求和回應。

```shell
node chap1-1.js
```

因為 Javascript 是 single thread，所以 Node 在 single thread 裡模擬非同步的方式是透過 event loop 與 callback function 關聯，一但被觸發就會呼叫 function。

所以一旦 create server 後就會立刻印出 log。

## Hello World, Tweaked

```js
var http = require('http');
var fs = require('fs');

http.createServer(function (req, res) {
   var name = require('url').parse(req.url, true).query.name;

   if (name === undefined) name = 'world';

   if (name === 'burningbird') {
      var file = '../images/phoenix5a.png';
      fs.stat(file, function (err, stat) {
         if (err) {
            console.error(err);
            res.writeHead(200, {'Content-Type': 'text/plain'});
            res.end("Sorry, Burningbird isn't around right now \n");
         } else {
            var img = fs.readFileSync(file);
            res.contentType = 'image/png';
            res.contentLength = stat.size;
            res.end(img, 'binary');
         }
      });
   } else {
      res.writeHead(200, {'Content-Type': 'text/plain'});
      res.end('Hello ' + name + '\n');
   }
}).listen(8124);

console.log('Server running at port 8124');
```

在 query string 輸入`?name=burningbird`的會去抓取檔案內的圖片，其他時候則顯示文字

有其他的 module import 可以在 function 內串接起來，這通常用來處理 URL module：

```js
var name = require('url').parse(req.url, true).query.name;
```

如果檔案不存在的話會出現錯誤的訊息，他會直接將所有的 error 訊息印出來：

```json
{ Error: ENOENT: no such file or directory, stat '../images/phoenix5a.png'
  errno: -2,
  code: 'ENOENT',
  syscall: 'stat',
  path: '../images/phoenix5a.png' }
```

讀取檔案同步的寫法：

```js
var img = fs.readFileSync(file);
res.contentType = 'image/png';
res.contentLength = stat.size;
res.end(img, 'binary');
```

不過通常在網路上找到的都會使用非同步的寫法，一般來說，在網頁請求中使用同步的操作是一個禁忌：

```js
fs.readFile(file, function(err,data) {
             res.contentType = 'image/png';
             res.contentLength = stat.size;
             res.end(data, 'binary');
           });
```

有時候不管使用哪種都不會影響到 I/O 的性能，而且同步版本雖然比較乾淨也比較容易使用，也有更少的 nested code。

通常需要在同步的 function 中使用 exception handling 像是`try...catch`來處理未預期的錯誤，但是在同步的 function 中則無法使用 error handling，因為 callback function 的第一個參數就是 error 值。

## Node Command-Line Options

- `-c`參數可以檢查 syntax error
- `-p`參數可以執行單一行指令且印出回傳值
- `--v8-option `列出 V8 選項的參數
- `--harmony`可以編譯 Harmony Javascript

```shell
node -c hello.js
node -p 'process.env'
node --v8-option
```

