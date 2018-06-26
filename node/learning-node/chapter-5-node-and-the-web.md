# Node and the Web

Node 在跨平台有很好的支援

## The HTTP Module: Server and Client

HTTP 套件支援多種物件，包括`http.Server`、`http.createServer`，我可以鑲嵌 callback function 去處理 web request，因為`http.Server`是從`EventEmitter`繼承下來的。

```js
// chap4-1.js
var http = require('http');

var server = http.createServer().listen(8124);

server.on('request', function(request,response) {

   response.writeHead(200, {'Content-Type': 'text/plain'});
   response.end('Hello World\n');
});

console.log('server listening on 8124');
```

你可以監聽像是其他的事件，像是連線已建立，或是 client 請求。

`HTTP.Server`是基於 TCP `Net.Server`實作而成的，TCP 是 transportation layer，HTTP 是 application layer。

callback function 支援兩個參數：request 跟 response。第二個參數`response`是一個`http.ServerResponse`型態的物件，他支援一些 function，包括`response.writeHead()`建立 response header、`response.write()`寫入 response data 和`response.end()`結束 response。

第一個參數是`request`，它是`IncomingMessage` class 的實利，也是一個 readable stream，你可以從 request 中存取到的資訊有：

- `request.headers`：request/response header 物件
- `request.httpVersion`：HTTP request 版本
- `request.method`：只有`http.Server`的 request，然後會回傳 HTTP verb (GET or POST)
- `request.rawHeaders`：Raw headers
- `request.rawTrailers`：Raw trailers

要看`request.headers`跟`request.rawHeaders`的不同，可以用`console.log()`印出來：

```js
// chap5-2.js
var http = require('http');

var server = http.createServer().listen(8124);

server.on('request', function(request,response) {

   console.log(request.headers);
   console.log(request.rawHeaders);

   // pulling host
   console.log(request.headers.host);
   console.log(request.rawHeaders[0] +  ' is ' + request.rawHeaders[1]);

   response.writeHead(200, {'Content-Type': 'text/plain'});
   response.end('Hello World\n');
});

console.log('server listening on 8214');
```

raw header 是存放在一個 array 中，第一個欄位是屬性，第二個欄位是值

```shell
$ node chap5-2.js 
server listening on 8214
{ host: 'localhost:8124',
  connection: 'keep-alive',
  'upgrade-insecure-requests': '1',
  'user-agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.87 Safari/537.36',
  accept: 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8',
  'accept-encoding': 'gzip, deflate, br',
  'accept-language': 'zh-TW,zh;q=0.9,en-US;q=0.8,en;q=0.7',
  cookie: '_ga=GA1.1.669717692.1523293397; _xsrf=2|f6c43ca5|ec6d17d9ef5bd95c9daeb88a62194182|1528483395; username-localhost-8888="2|1:0|10:1528524230|23:username-localhost-8888|44:MmJiYjg1N2EzOWQ2NDNkMmI5MjgxNGZjZGJjZjA5YWE=|2dde94cafc9fa4f1447cf82bfba4ad8e7523b21d6969b6f78d31dbb564a030c1"; username-localhost-8889="2|1:0|10:1529565556|23:username-localhost-8889|44:MjBhYzg4YjVlYWExNDNhN2I3ZGNkOWQxZjJhYzI2Nzk=|4884057347b78b9c590098e91677b5ab922a2522d791f7aa8c5c40eee74fa749"' }
[ 'Host',
  'localhost:8124',
  'Connection',
  'keep-alive',
  'Upgrade-Insecure-Requests',
  '1',
  'User-Agent',
  'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.87 Safari/537.36',
  'Accept',
  'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8',
  'Accept-Encoding',
  'gzip, deflate, br',
  'Accept-Language',
  'zh-TW,zh;q=0.9,en-US;q=0.8,en;q=0.7',
  'Cookie',
  '_ga=GA1.1.669717692.1523293397; _xsrf=2|f6c43ca5|ec6d17d9ef5bd95c9daeb88a62194182|1528483395; username-localhost-8888="2|1:0|10:1528524230|23:username-localhost-8888|44:MmJiYjg1N2EzOWQ2NDNkMmI5MjgxNGZjZGJjZjA5YWE=|2dde94cafc9fa4f1447cf82bfba4ad8e7523b21d6969b6f78d31dbb564a030c1"; username-localhost-8889="2|1:0|10:1529565556|23:username-localhost-8889|44:MjBhYzg4YjVlYWExNDNhN2I3ZGNkOWQxZjJhYzI2Nzk=|4884057347b78b9c590098e91677b5ab922a2522d791f7aa8c5c40eee74fa749"' ]
localhost:8124
Host is localhost:8124
```

在 Node 文件中，說了一些`IncomingMessage`的屬性，像是`statusCode`和`statusMessage`，只能讓 response 存取，是由`HTTP.ClientRequest`物件獲得的 response。你可以建立 Server 監聽 request，同樣的你也可以建立 Client 發出 request。你可以使用`http.request()` function 來實利化`ClientRequest` class 。

為了要 demo server 和 client 兩個不同的差別，我們會在 client 送出 POST 的請求，讓 server 讀到 data，server 需要稍做一些修改，`IncomingMessage`的 readable stream 的部分是資料進來的地方。

```js
// example5-1.js
var http = require('http');
var querystring = require('querystring');

var server = http.createServer().listen(8124);

console.log(server.constructor.name); // http.Server

server.on('request', function(request,response) {

   console.log(request.constructor.name); // http.IncomingMessage
   console.log(response.constructor.name); // http.ServerResponse
   if (request.method == 'POST') {
        var body = '';

        // append data chunk to body
        request.on('data', function (data) {
            console.log(data.constructor.name) // Buffer
            body += data;
        });

        // data transmitted
        request.on('end', function () {
            var post = querystring.parse(body);
            console.log(post);
            response.writeHead(200, {'Content-Type': 'text/plain'});
            response.end('Hello World\n');
        });
    }
});
console.log('server listening on 8214');
```

應用程式會監聽一個或多個的`data`事件，而不是監聽`request`事件，你在 request 裡可以使用`data`事件拿到 chunks of data。應用程式會持續接收到 chunks 直到他在 request 物件裡面收到`end`事件。

Query String 是很好用的 Node 套件，可以分析資料，然後顯示出來。

```js
// example5-2.js
var http = require('http');
var querystring = require('querystring');

var postData = querystring.stringify({
  'msg' : 'Hello World!'
});

var options = {
  hostname: 'localhost',
  port: 8124,
  method: 'POST',
  headers: {
    'Content-Type': 'application/x-www-form-urlencoded',
    'Content-Length': postData.length
  }
};

var req = http.request(options, function(res) {
  console.log(res.constructor.name); // http.IncomingMessage
  console.log('STATUS: ' + res.statusCode);
  console.log('HEADERS: ' + JSON.stringify(res.headers));
  res.setEncoding('utf8');

  // get data as chunks
  res.on('data', function (chunk) {
    console.log('BODY: ' + chunk);
  });

  // end response
  res.on('end', function() {
    console.log('No more data in response.')
  })
});

req.on('error', function(e) {
  console.log('problem with request: ' + e.message);
});

console.log(req.constructor.name); // http.ClientRequest

// write data to request body
req.write(postData);
req.end();
```

`http.ClientRequest`是實作 writable stream，使用在範例中的方法`req.write()`。

上面的程式碼幾乎是從 Node 文件範例內複製過來的。server 和 client 都是相同的地方，我們可以使用`localhost`當作 host。此外，我們不特別指明路徑的話，預設會是`/`，請求的 header 的 content type 是`application/x-www-form-urlencoded`，用於回傳要 POST 的資料。client 會從 server 的`response`收到資料，在`http.request()`的 callback function 裡是唯一的參數。client 會收到 server response 的資料，然後印在螢幕上，因為資料回傳的資料很短，所以只會觸發一次`data`事件。

事實上 POST 的 request 並不是非同步處理的，因為我們是執行這個 action，而不是在等待 action 時 blocking 它。

開啟第二個終端機執行 example5-2.js，他就會向 server 發送請求，然後回傳 Hello World。

這邊講了四個 HTTP 的套件：

- `http.ClientRequest`
- `http.IncomingMessage`
- `http.Server`
- `http.ServerResponse`

|        | Request                | Response               |
| ------ | ---------------------- | ---------------------- |
| Server | `http.IncomingMessage` | `http.ServerResponse`  |
| Client | `http.ClientRequest`   | `http.IncomingMessage` |

```mermaid
graph LR
    Client -- http.ClientRequest -- request -- http.IncomingMessage --> Server
    Server -- http.ServerResponse -- response -- http.IncomingMessage --> Client
```

現在我們沒有用看過的事`http.Agent`，是用來 pooling socket，因為連線有數量限制，所以當有大量請求的時候會遇到瓶頸，所以在請求的時候加上選項`agent: false`，當連線結束的時候會斷開。

```js
var options = {
  hostname: 'localhost',
  port: 8124,
  method: 'POST',
  headers: {
    'content-type': 'application/x-www-form-urlencoded',
    'content-length': postData.length
  },
  agent: false
};
```

你也可以用`agent.maxFreeSockets`調最大的 socket pool 的數量，預設是 256 個，注意，調高的話會使用更多的記憶體和資源。

## What's Involved in Creating a Static Web Server

 我們已經有所有功能能建立間單的 router 或是服務 static files。但是要可以使用跟可以簡單地使用是兩回事。

要建立一個簡單的靜態檔案的 server 有幾個步驟：

1. 建立 HTTP server 且等待請求。
2. 當請求送達的時候，分析請求的 URL 決定回傳的檔案。
3. 檢查檔案是否存在。
4. 如果檔案不存在，直接 response。
5. 如果檔案存在，打開檔案且讀取它。
6. 準備 response header。
7. 把檔案寫到 response。
8. 等待下一個請求。

我們只需要 core module 就能執行這些功能。建立 HTTP server 和讀檔分別需要 HTTP module 和 File System module。此外，我們想要將路徑定義在全域變數，或使用`__dirname`。

```js
// chap5-3.js
var http = require('http'),
    fs = require('fs'),
    base = '/home/examples/public_html';

http.createServer(function (req, res) {

   pathname = base + req.url;
   console.log(pathname);

}).listen(8124);

console.log('Server web running at 8124');
```

我們將第一章 Hello World 的範例拿來改寫，`http.createServer()`建立 server，callback 有兩個參數 request 和 response。可以直接請求`request.url`獲得 URL 路徑。

[Port 列表](https://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers#Well-known_ports)，Port 低於 1024 都需要 root 權限。

瀏覽器會一直轉圈圈是正常的，因為我們沒有回傳給 reponse。

我們可以使用`fs.stat()`檢查檔案是否存在。

在檢查後移除檔案的問題，在`fs.stat()`檢查檔案和開啟檔案之間，如果檔案被移除了會發問題，可以直接使用像是`fs.open()`直接將檔案擋開，然後使用`fs.createReadStream()`讀取檔案，但是他不會提供這個是檔案還是資料夾的訊息，而且也不會提供檔案是否遺失或是被鎖住的訊息。所以使用`fs.stat()`檢查，不過在開檔的時候要再檢查一次。

說到讀檔，我們試著使用`fs.readFile()`讀取檔案內容，問題是`fs.readFile()`會想把所有檔案載入到記憶體。文件有可能會很大，此外可能同時間有很多的請求。像是`fs.readFile()`就沒辦法 scale。

透過預設的`fs.createReadStream()`的方法建立 read stream，這會比較簡單 pipe 檔案內容，寫入到 HTTP response 物件。因為檔案在結束的時候會呼叫`end`方法，所以我們不需要處理它。

```js
// example5-3.js
var http = require('http'),
    fs   = require('fs'),
    base = __dirname;

http.createServer(function (req, res) {

   pathname = base + req.url;
   console.log(pathname);

   fs.stat(pathname, function(err,stats) {
      if (err) {
        console.log(err);
        res.writeHead(404);
        res.write('Resource missing 404\n');
        res.end();
      } else {
         res.setHeader('Content-Type', 'text/html');

         // create and pipe readable stream
         var file = fs.createReadStream(pathname);

         file.on("open", function() {
            res.statusCode = 200;
            file.pipe(res);
         });

         file.on("error", function(err) {
           console.log(err);
           res.writeHead(403);
           res.write('file missing or permission problem');
           res.end();
         });

       }
    });
}).listen(8124);

console.log('Server running at 8124');
```

read stream 有兩個有趣的事件：`open`跟`error`。當準備好 stream 的時候就會呼叫`open`事件，發生錯誤就呼叫`error`事件。如果再檢查完，檔案刪除的話，這將就會發生錯誤，或是讀取到目錄，他就會回傳 403 的錯誤。

應用程式會在`open`的 callback function 裡呼叫`pipe`方法。

可以使用簡單的 HTML 測試：

```html
<!-- index.html -->
<!DOCTYPE html>
<head>
   <title>Test</title>
   <meta charset="utf-8" />
   <script>
   'use strict';
const foo = 1;
typeof foo === 'number';
</script> 
</head>
<body>
<img src="./phoenix5a.png" />
</body>
```

```html
<!-- video.html -->
<!DOCTYPE html>
<head>
   <title>Video</title>
   <meta charset="utf-8" />
</head>
<body>
<img src="./phoenix5a.png" />

   <video id="meadow" controls>
      <source src="videofile.mp4" />
      <source src="videofile.ogv" />
      <source src="videofile.webm" />
   </video>
</body>
```

如果找不到檔案救回傳 404 的錯誤

```shell
$ node example5-3.js 
Server running at 8124
/Users/akiicat/Github/LearningNode2/chap5/video.html
/Users/akiicat/Github/LearningNode2/chap5/phoenix5a.png
/Users/akiicat/Github/LearningNode2/chap5/videofile.mp4
{ Error: ENOENT: no such file or directory, stat '/Users/akiicat/Github/LearningNode2/chap5/videofile.mp4'
    at Error (native)
  errno: -2,
  code: 'ENOENT',
  syscall: 'stat',
  path: '/Users/akiicat/Github/LearningNode2/chap5/videofile.mp4' }
/Users/akiicat/Github/LearningNode2/chap5/videofile.ogv
{ Error: ENOENT: no such file or directory, stat '/Users/akiicat/Github/LearningNode2/chap5/videofile.ogv'
    at Error (native)
  errno: -2,
  code: 'ENOENT',
  syscall: 'stat',
  path: '/Users/akiicat/Github/LearningNode2/chap5/videofile.ogv' }
/Users/akiicat/Github/LearningNode2/chap5/videofile.webm
{ Error: ENOENT: no such file or directory, stat '/Users/akiicat/Github/LearningNode2/chap5/videofile.webm'
    at Error (native)
  errno: -2,
  code: 'ENOENT',
  syscall: 'stat',
  path: '/Users/akiicat/Github/LearningNode2/chap5/videofile.webm' }
```

每個影片回傳的 content type 是`text/html`，正確的做法要使用 mime type，可以安裝 mime 套件。

```shell
npm install mime
```

給 mime 套件檔名，它就能回傳正確的 mime type，然後回傳到 content type。

```js
var mime = require('mime');
```

content type 會放在 response header：

```js
// content type
var type = mime.getType(pathname);
console.log(type);
res.setHeader('Content-Type', type);
```

當我們存取 video 的時候，現在正常了，只有在存取路徑的時候，才會發生錯誤。

現在我們不必檢查檔案是否存在，只需要檢查是目錄還是檔案，如果目錄被存取的話，就要顯示錯誤。

這裡有另外一個更動，base path 使用`path.normalize`正規化路徑，用於不同環境時，Windows 也可以運作。

最後的版本，當檔案不存在時會回傳 404，如果沒有權限的話會回傳 403：

```js
// example5-4.js
var http = require('http'),
    url =  require('url'),
    fs   = require('fs'),
    mime = require('mime'),
    path = require('path');

var base = __dirname;

http.createServer(function (req, res) {

   pathname = path.normalize(base + req.url);
   console.log(pathname);

   fs.stat(pathname, function(err, stats) {
      if (err) {
        res.writeHead(404);
        res.write('Resource missing 404\n');
        res.end();
      } else if (stats.isFile()) {
         // content type
         var type = mime.getType(pathname);
         console.log(type);
         res.setHeader('Content-Type', type);

         // create and pipe readable stream
         var file = fs.createReadStream(pathname);
         file.on("open", function() {
            // 200 status - found, no errors
            res.statusCode = 200;
            file.pipe(res);
         });
         file.on("error", function(err) {
           console.log(err);
           res.statusCode = 403;
           res.write('file permission');
           res.end();
         });
       } else {
        res.writeHead(403);
        res.write('Directory access is forbidden');
        res.end();
       }
    });
}).listen(8124);
console.log('Server running at 8124');
```

```shell
$ node example5-4.js 
Server running at 8124
/Users/akiicat/Github/LearningNode2/chap5/video.html
text/html
/Users/akiicat/Github/LearningNode2/chap5/phoenix5a.png
image/png
/Users/akiicat/Github/LearningNode2/chap5/videofile.mp4
/Users/akiicat/Github/LearningNode2/chap5/videofile.ogv
/Users/akiicat/Github/LearningNode2/chap5/videofile.webm
```

`__dirname`可以獲得當前執行的資料夾。

雖然程式可以運作，不過他還不能處理其他的請秀，處理 security 和 caching，而且也沒有正確的處理影片的請求。有很多的問題困擾著，這就是為什麼大家會想使用更複雜的系統，像是 Express。

## Using Apache to Proxy a Node Application

使用 Apache 好的地方是它很強大、安全、功能又多，缺點是每個 request 都會建立一條新的 thread。

```shell
a2enmod proxy
a2enmod proxy_http
```

```conf
<VirtualHost ipaddress:80>
	ServerAdmin xxx@gmail.com 
	ServerName akiicat.com
	
	ErrorLog path-to-logs/error.log
	CustomLog path-to-logs/access.log combined
	
	ProxyRequests off
	
	<Location />
		ProxyPass http://ipaddress:2368/
		ProxyPassReverse http://ipaddress:2368/
	</Location>
</VirtualHost>
```

```shell
a2ensite akiicat.com
service apache2 reload
```

如果是更進階的話可以使用 iptable

```shell
# ubuntu
iptables -A input -i eth0 -p tcp --dport 2368 -j DROP
```

## Parsing the Query with Query String

Query String 套件唯一的目標就是準備且執行 query string。

你可以使用`querystring.parse()`把收到的 query string 轉換成物件。query string 預設的分隔符號是`&amp;`，function 的第二個參數可以覆蓋它。指派符號`=`可以在第三個參數覆蓋預設值。第四個參數包含`decodeURIComponent`，預設是`querystring.unescape()`，如果 query string 不是 UTF-8 的話要改它。

要看`querystring.parse()`是如何運作的話，先假設 query 如下：

```
somedomain.com/?value1=valueone&value1=valueoneb&value2=valuetwo
```

使用`querystring.parse()`回傳的物件：

```json
{
    value1: [ 'valueone', 'valueoneb' ],
    value2: 'valuetwo'
}
```

我們可以將物件傳入`querystrin.stringify()`進行編碼，你會獲得正確的 query string。

```shell
$ node
> str = 'msg=Hello World'
'msg=Hello World'
> querystring.parse(str)
{ msg: 'Hello World' }
> querystring.stringify(_)
'msg=Hello%20World'
```

注意，空白鍵被替換成跳脫字元，`querystring.stringify()`提供一些選擇性的參數，你可以傳入你自己的`encodeURIComponent`，預設值是`stringify.escape()`。

## DNS Resolution

並不是所有的應用程式都需要使用 DNS 的服務，但如果你需要的話，Node 也提供了 core DNS module 的功能。

有兩個 DNS 套件的 function：`dns.lookup()`和`dns.resolve()`。

`dns.lookup()`會回傳 domain name 的 IP address：

```js
// dns-lookup.js
var dns = require('dns');

dns.lookup('oreilly.com', function(err, address, family) {
    if (err) return console.log(err);

    console.log(address);
    console.log(family);
})
```

```shell
$ node dns-lookup.js 
199.27.145.64
4
```

family 的值不是 4 就是 6 取決於你的 IP address 是 IPv4 還是 IPv6。

你可以給定`options`物件：

- `family`：數字 4 或 6，他會對應你想顯示的地址 IPv4 或 IPv6。
- `hints`：支援數字的`getaddrinfo` flag。
- `all`：如果是`true`會回傳所有 address，預設是`false`。

```js
dns.lookup('oreilly.com', {all: true}, function(err, family) {
    if (err) return console.log(err);
    
    console.log(family);
})
```

```json
[ { address: '199.27.145.64', family: 4 },
  { address: '199.27.145.65', family: 4 } ]
```

`dns.resolve()`解析某個 record type 的 hostname：

- `A`：Default IPv4 address
- `AAAA`：IPv6 address
- `MX`：Ｍail exchange record
- `TXT`：Text records
- `SRV`：SRV records
- `PTR`：Used for reverse IP lookup
- `NS`：Name server
- `CNAME`：Canonical name records
- `SOA`：Start of authority record

```js
dns.resolve('oreilly.com', 'MX', function(err, addresses) {
    if (err) return err;
    console.log(addresses);
})
```

```json
[ { exchange: 'aspmx.l.google.com', priority: 1 },
  { exchange: 'aspmx2.googlemail.com', priority: 10 },
  { exchange: 'alt2.aspmx.l.google.com', priority: 5 },
  { exchange: 'alt1.aspmx.l.google.com', priority: 5 },
  { exchange: 'aspmx3.googlemail.com', priority: 10 } ]
```





















​	