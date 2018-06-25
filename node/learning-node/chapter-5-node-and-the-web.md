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











​	