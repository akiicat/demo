# Full-Stack Node Development

MEAN：

- MongoDB
- Express
- AngularJS
- Node

## The Express Application Framework

Express 變得這麼受歡迎：它以最小的努力提供了大多數的功能。

[Express](http://expressjs.com/) 已經是 Node.js 基金會裡的一部分。

建立新的資料夾，使用 npm 指令建立 package.json，把 app.js 當作進入點：

```shell
npm init
```

```shell
npm install express --save
```

最小的 Hello World Express 程式：

```js
// app.js
// chap10-1.js
const express = require('express');
const app = express();

app.get('/', function (req, res) {
   res.send('Hello World!');
});

app.listen(3000, function () {
   console.log('Example app listening on port 3000!');
});
```

`app.get()`會處理任何 GET web request，request 跟 response 的有之前使用預設的功能，另外是他們還有 Express 額外的功能，像是你可以使用`res.write()`和`res.end()`去回應你的 web request，就如同你在之前章節所使用的一樣，你也可以使用`res.send()`同時做完兩件事情。

我們不必手動建立應用程式，我們可以使用 Express 產生器來產生架構：

```shell
sudo npm install express-generator -g
```

新建專案：

```shell
express bookapp
cd bookapp
npm install
```

Express 已經建立好了，你可以運行他，如果不想要 debug 可以把前面去掉：

```shell
DEBUG=bookapp:* npm start
```

預設 port 是 3000

```shell
$ tree
.
├── app.js
├── bin
│   └── www
├── package.json
├── public
│   ├── images
│   ├── javascripts
│   └── stylesheets
│       └── style.css
├── routes
│   ├── index.js
│   └── users.js
└── views
    ├── error.jade
    ├── index.jade
    └── layout.jade
```

- `public`：公開的靜態檔案、像是圖片或 CSS
- `view`：動態的檔案
- `routes`：網頁末端，監聽網頁請求，然後 render 網頁

Jade 這名字因為違反商標，所以改名成 Pug。

在`bin`裡的`www`檔案是起始腳本，它轉成 command-llne 的應用程式，當你在 package.json 的時候會看到：

```json
// package.json
{
  "name": "bookapp",
  "version": "0.0.0",
  "private": true,
  "scripts": {
    "start": "node ./bin/www"
  },
  "dependencies": {
    "cookie-parser": "~1.4.3",
    "debug": "~2.6.9",
    "express": "~4.16.0",
    "http-errors": "~1.6.2",
    "jade": "~1.11.0",
    "morgan": "~1.9.0"
  }
}
```

你可以在 bin 裡安裝其他的腳本來控制你的應用程式，像是 test、restart。

`app.js`是應用程式的進入點，裡面有很多套件：

```js
// app.js
var createError = require('http-errors');
var express = require('express');
var path = require('path');
var cookieParser = require('cookie-parser');
var logger = require('morgan');

var indexRouter = require('./routes/index');
var usersRouter = require('./routes/users');

var app = express();
```

- `express`：Express 應用程式
- `path`：Node core 套件，檔案路徑
- `serve-favicon`：服務 favicon.ico 檔案
- `cookie-parser`：分析 cookie header 然後填在`req.cookies`
- `body-parser`：提供四種不同型態的 body parser，但是不能處理 multipart body。

每個 middleware 套件都是 vanilla HTTP server。

Middleware 是一個屆在 system/operating system/database 和應用程式的中介層。middleware 是應用程式練的一部分，每個都會處理一個特定的 function。不管怎樣 [Express middleware](http://expressjs.com/en/resources/middleware.html) 是非常全面的。

下一個部分的 app.js 程式碼是掛上 middleware，讓他在應用程式內可以使用，透過`app.use()`。如果你在家其他的 middleware 功能，middleware 掛載的順序非常重要。

一開始定義了 view engine 的位置：

```js
// app.js
// view engine setup
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');

app.use(logger('dev'));
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));
```

最後一個使用`express.static`，Express 內建的 middleware，用來處理 static 檔案，如果使用者請求了 HTML、JPEG 或其他靜態檔案，`express.static`會處理請求，當 middleware 掛上去後，他會依相對照路徑服務所有靜態檔案，這裡的例子就是`public`的資料夾。

`app.set()` function 會呼叫 view engine，你也可以使用 template engine，其中一個最受歡迎的是 Jade，其他像是 Mustache 和 EJS 可以簡單地被使用，engine setup 定義了 template files (views) 的子目錄應該要使用哪種 view engine (Jade)。

因為 jade 已經換成 pug 了，可以修改他：

```shell
npm install pug --save
```

```js
// app.js
app.set('view engine', 'pug');
```

在 *views* 的資料夾裡會有三個檔案 *error.jade*、*index.jade*、*layout.jade*，會用來整合資料：

```jade
// views/index.jade
extends layout

block content
  h1= title
  p Welcome to #{title}
```

`extends layout`會將檔案跟`layout.jade`合併，`title`的值會指派給`h1` header，這些東西 render 完後會回傳到 *app.js* 的這裡：

```js
// app.js
app.use('/', indexRouter);
app.use('/users', usersRouter);
```

這是應用程式的末端，功能是用來回傳 client 的請求，在 *routes* 資料夾裡 `/`最頂層的請求是 *index.js* 檔，而 users 是 *users.js* 檔。

在 *index.js* 檔案裡面，我們會介紹 Express *router*，它提供了 response-handling 的功能，如同 Express 文件所記載的，router 的行為會符合這種規則：

```js
app.METHOD(PATH, HANDLER);
```

HTTP 方法包含熟悉的`get`、`post`、`put`、`delete``以及可能不熟悉的merge`、`search`、`head`、`options`等等。path 是網頁路徑，處理請求的功能：

```js
// routes/index.js
var express = require('express');
var router = express.Router();

/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('index', { title: 'Express' });
});

module.exports = router;
```

這邊的資料和 view 這兩個東西會在`res.render`的呼叫裡，使用的 view 是 *index.jade*，你可以看到 `title` 屬性的值會傳入 template，你可以試著把 `title` 裡的 "Express" 改掉，然後重新載入頁面，你會發現頁面會有所不同。

合併資料，建議的書籍 [Javascript Cookbook](http://shop.oreilly.com/product/0636920033455.do)

如果你這邊有安裝 pug 的話，且使用 pug 的話，要把 view 裡面的副檔名改成 *.pug* 才能運作。

## MongoDB and Redis database Systems

[Mysql driver for Node](https://github.com/mysqljs/mysql)

比較新的 SQL Server 存取套件 [Tedious package](https://github.com/tediousjs/tedious)

Node 可以存取不同的資料庫，這裡簡單介紹兩種資料庫：MongoDB、Redis。

### MongoDB

MongoDB 是 document-based 的資料庫，用 BSON 編碼，一個二進位的 JSON，這就是為什麼他在 Javascript 中受歡迎的原因，MongoDB 沒有 table row 而是 BSON document，沒有 table 而是 collection。

MongoDB 不是集中式的資料庫，還有很多版本型態的資料儲存 Apache 的 CouchDB、Amazon 的 SimpleDB、RavenDB、Lotus Notes。

MongoDB 並不是瑣碎的資料庫系統，你在合併到應用程式之前必須先花時間學習他的功能，[MongoDB Native NodeJS Driver](https://github.com/mongodb/node-mongodb-native) 支援 MongoDB，使用 [Mongoose](http://mongoosejs.com/) 支援物件導向。

這邊只會做一個小 CRUD demo：

```js
// example10-1.js
var MongoClient = require('mongodb').MongoClient;

// Connect to the db
MongoClient.connect("mongodb://localhost:27017/exampleDb", function(err, database) {
   if(err) { return console.error(err); }

   // access or create widgets collection
   var db = database.db('exampleDb')
   db.collection('widgets', function(err, collection) {
      if (err) return console.error(err);

      // remove all widgets documents
      collection.remove(null,{safe : true}, function(err, result) {
         if (err) return console.error(err);
         console.log('result of remove ' + result.result);

         // create two records
         var widget1 = {title : 'First Great widget',
                         desc : 'greatest widget of all',
                         price : 14.99};
         var widget2 = {title : 'Second Great widget',
                         desc : 'second greatest widget of all',
                         price : 29.99};

         collection.insertOne(widget1, {w:1}, function (err, result) {
            if (err) return console.error(err);
            console.log(result.insertedId);

            collection.insertOne(widget2, {w:1}, function(err, result) {
               if (err) return console.error(err);
               console.log(result.insertedId);

               collection.find({}).toArray(function(err,docs) {
                  console.log('found documents');
                  console.dir(docs);

                  //close database
                  database.close();
               });
            });
        });
     });
   });
});

```

MongoClient 物件可以用來連線到資料庫，預設 port 27017，資料庫是`exampleDB`，用 URL 連線，collection 是`widgets`。

MongoDB 的 function 是非同步的。在插入紀錄之前，城市會先刪除在 collection 裡存在的紀錄，使用的是`collection.remove()`的 function，不用特別的 query，如果沒有做到的話，就沒辦法複製資料，因為的 id 是由 MongoDB 系統產生的，他不會確保`title`或是其他欄位的值是唯一的。

新增紀錄是使用`collection.insertOne()`，傳入 JSON 的物件資料，`{w:1}`代表 *write concern*，讓 MongoDB 知道寫入的權限。

一旦資料被寫入，會使用`collection.find()`去找尋紀錄，一樣都不需要特別寫 query。這個 function 會建立一個 *cursor*，然後`toArray()`的 function 會回傳 cursor 結果的 array，然後使用`console.dir()`將結果印出來：

```shell
$ node example10-1.js 
(node:39782) DeprecationWarning: current URL string parser is deprecated, and will be removed in a future version. To use the new parser, pass option { useNewUrlParser: true } to MongoClient.connect.
result of remove [object Object]
5b3cee202857539b6618513e
5b3cee202857539b6618513f
found documents
[ { _id: ObjectID { _bsontype: 'ObjectID', id: [Object] },
    title: 'First Great widget',
    desc: 'greatest widget of all',
    price: 14.99 },
  { _id: ObjectID { _bsontype: 'ObjectID', id: [Object] },
    title: 'Second Great widget',
    desc: 'second greatest widget of all',
    price: 29.99 } ]
```

id 仍然是個物件，因為他是 BSON，所以他印不出來，如果你想要提供更乾淨的 output，你可以存取每個欄位，然後用`toHexString()`轉換 BSON 的 id 成十六進位字元。

```js
docs.forEach(function(doc) {
    console.log('ID : ' + doc._id.toHexString());
    console.log('desc : ' + doc.desc);
    console.log('title : ' + doc.title);
    console.log('price : ' + doc.price);
});
```

```shell
$ node example10-1.js 
...
ID : 5b3cf13e1437f29bee19382b
desc : greatest widget of all
title : First Great widget
price : 14.99
ID : 5b3cf13e1437f29bee19382c
desc : second greatest widget of all
title : Second Great widget
price : 29.99
```

用 MongoDB command-line 也可以看到紀錄：

1. 輸入`mongo`
2. 改變資料庫：`use exampleDb`
3. `show collections`
4. `db.widgets.find()`查看 Widget 紀錄

如果你想要合併 object-based 的方法將你的 MongoDB 合併到程式裡的話，你可能會使用 Mongoose。

沒有要使用的時候記得關掉 MongoDB。

[MongoDB documentation](https://mongodb.github.io/node-mongodb-native/api-articles/nodekoarticle1.html)

### Redis Key/Value Store

NoSQL 的資料結構的型態是 key/value pairs，他會存在記憶體，能夠非常快速的存取。三個最受歡迎的 in-memory key/value 儲存是：Memcached、Cassandra、Redis。Node 三種都能夠支援。

Memcached 主要在記憶體內可以快速存取資料的方是。也很適合分散式計算，但會受限於更複雜的資料，適合執行很多的 queries，但不適合做很多的寫入跟讀取。

[EARN Stack](https://www.airpair.com/express/posts/earn-stack)：Express、AngularJS、Redis、Node

安裝 Redis 套件：

```shell
npm install redis
```

如果你有要在 Redis 大的操作時，建議安裝 hiredis 套件，他是 non-blocking 而且能夠增加效能：

```shell
npm install hiredis redis
```

Redis 套件做了最小的包裝，因此你可能需要花時間學習 Redis 的指令，以及 Redis 如何運作。

使用`createClient`建立 Redis client：

```js
var redis = require('redis');
var client = redis.createClient();
```

`createClient`有三個選擇性的參數：`port`、`host`、options。預設 host 是`127.0.0.1`，port 是 6379，這個 port 是 Redis server 預設的 port。

一旦連線完且傳完訊息後，直到你呼叫`client.quit()`才會關閉 Redis server 的連線。如果你想要強制關閉可以使用`client.end()`，他不會等所有的回復被解析，如果程式當掉的話看使用他。

透過 client connection 執行 Redis 指令是相當直觀的程序。所有的指令都變成 client 物件的方法，然後指令的參數會傳入。因為這是 Node，最後一個參數是 callback function，callback 會回傳錯誤跟資料的回應。

`client.hset()`用來設定 *hash* 的屬性，在 Redis 裡，hash 會 mapping 這兩個東西 string fields 跟 value，就像 lastname 會對映到你的姓，firstname 會對映到你的名：

```js
client.hset("hashid", "propname", "propvalue", function(err, reply) {
    // do something with error or reply
});
```

`hset`指令設定了值，所以沒有資料會回傳，只有 Redis 的 acknowledgment。如果用`client.hvals`呼叫很多值，像是`client.hvals`，他 callback function 的第二個餐數會是 array，不管裡面只有一個值或是物件。

```js
client.hvals(obj.member, function (err, replies) {
   if (err) {
      return console.error("error response - " + err);
   }

   console.log(replies.length + " replies:");
   replies.forEach(function (reply, i) {
      console.log(" " + i + ": " + reply);
   });
});
```

因為 Node 的 callback 太普遍了，而 Redis 指令很多時候只是回傳確認成功的訊息，所以 Redis 套件提供`redis.print`可以傳入最後的參數：

```js
client.set("somekey", "somevalue", redis.print);
```

不論是錯誤或是訊息都會印出來。

為了 demo Redis，我建立了 *message queue*，message queue 是一個程式，他會把所有存放 input communication 的型態，直到有人接收他才會 pop off queue，然後傳送到接收者。communication 是非同步的，因為存到 queue 的時候不需要接收者連線。

第一個應用程式：message queue 的 demo，建立 Node 去存取網頁的 log 檔案，會使用 child process 和 Unix `tail -f` 的指令存取紀錄。log 紀錄有兩個表達式物件：提取資源、檢查資源是否是圖片檔，如果是圖片檔，就會把資源的 URL 透過 TCP 送到 message queue。

第二個應用程式：mesage queue 會監聽所有 port 是 3000 的訊息，然後儲存任何資料到 Redis data store。

第三個應用程式：是 web server 她的 port 是 8124，他會存取 Redis 資料庫，然後把最前面的訊息 pop off，透過 response 回傳物件，如果 Redis database 的圖片資源回傳`null`，那代表已經在 message queue 的最末端了。

第一個應用程式的`tail`可以顯示最後幾行文件的資料，`-f`可以持續監聽檔案，如果有新的一行，他就會顯示出來，也可以同時使用在不同的檔案。

因為只是 demo，regexp 不會做多餘的檢查。

```js
// example10-2.js
var spawn = require('child_process').spawn;
var net = require('net');

var client = new net.Socket();
client.setEncoding('utf8');

// connect to TCP server
client.connect ('3000', 'localhost', function() {
    console.log('connected to server');
});

// start child process
var logs = spawn('tail', ['-f',
        '/home/main/logs/access.log',
        '/home/tech/logs/access.log',
        '/home/shelleypowers/logs/access.log',
        '/home/green/logs/access.log',
        '/home/puppies/logs/access.log']);

// process child process data
logs.stdout.setEncoding('utf8');
logs.stdout.on('data', function(data) {

   // resource URL
   var re = /GET\s(\S+)\sHTTP/g;

   // graphics test
   var re2 = /\.gif|\.png|\.jpg|\.svg/;

   // extract URL
   var parts = re.exec(data);
   console.log(parts[1]);

   // look for image and if found, store  
   var tst = re2.test(parts[1]);
   if (tst) {
      client.write(parts[1]);
   }
});
logs.stderr.on('data', function(data) {
   console.log('stderr: ' + data);
});

logs.on('exit', function(code) {
   console.log('child process exited with code ' + code);
   client.end();
});
```

```log
// access.log
GET /robots.txt HTTP
GET /weblog HTTP
GET /writings/fiction?page=10 HTTP
GET /images/kite.jpg HTTP
GET /node/145 HTTP
GET /culture/book-reviews/silkworm HTTP
GET /feed/atom/ HTTP
GET /images/visitmologo.jpg HTTP
GET /images/canvas.png HTTP
GET /sites/default/files/paws.png HTTP
GET /feeds/atom.xml HTTP
GET /image/p1.png HTTP
```

存到 Redis 資料庫是用 Redis `rpush` 的指令：

```js
// example10-3.js
var net = require('net');
var redis = require('redis');

var server = net.createServer(function(conn) {
   console.log('connected');

   // create Redis client
   var client = redis.createClient();

   client.on('error', function(err) {
     console.log('Error ' + err);
   });

   // sixth database is image queue
   client.select(6);
   // listen for incoming data
   conn.on('data', function(data) {
      console.log(data + ' from ' + conn.remoteAddress + ' ' +
        conn.remotePort);

      // store data
      client.rpush('images',data);
   });

}).listen(3000);
server.on('close', function(err) {
   client.quit();
});

console.log('listening on port 3000');
```



```shell
$ echo "GET /image/p2.png HTTP" >> access.log 
```

```shell
$ node example10-3.js 
listening on port 3000
connected
/image/p2.png from ::ffff:127.0.0.1 58653
```

```shell
$ node example10-2.js 
/writings/fiction?page=10
connected to server
/image/p2.png
```

最後一部分，web server 接到請求後，會向 Redis 資料庫要一筆 entry，然後把資料回傳。

```js
// example10-4.js
var redis = require("redis"),
    http = require('http');

var messageServer = http.createServer();

// listen for incoming request
messageServer.on('request', function (req, res) {

   // first filter out icon request
   if (req.url === '/favicon.ico') {
      res.writeHead(200, {'Content-Type': 'image/x-icon'} );
      res.end();
      return;
   }

  // create Redis client
  var client = redis.createClient();


  client.on('error', function (err) {
    console.log('Error ' + err);
  });

  // set database to 6, the image queue
  client.select(6);

   client.lpop('images', function(err, reply) {
      if(err) {
        return console.error('error response ' + err);
      }

      // if data
      if (reply) {
         res.write(reply + '\n');
      } else {
         res.write('End of queue\n');
      }
      res.end();
   });
   client.quit();

});

messageServer.listen(8124);

console.log('listening on 8124');
```

```shell
$ node example10-4.js 
listening on 8124
```

重新整理頁面直到 message queue 沒有資料為止。

這就是為什麼 Redis 非常適合這類型的應用程式，未完成的資料不會花太多力氣，為了跟你的 Node 應用程式合併。

#### When to Create Redis Client

有時候是為了延續應用程式的生命，而有時候是一旦 Redis 指令完成的時候就釋放他。

什麼時候建立一個 persist Redis connection，什麼時候建立 connection 然後 release 它呢？

## AngularJS and Other Full-Stak Frameworks

[TodoMVC](http://todomvc.com/)

