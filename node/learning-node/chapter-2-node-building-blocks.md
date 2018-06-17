# Node Building Blocks: Global Objects, Events, and Node's Asynchronous Nature

網頁的 application 跟 Node.js application 都是由 Javascript 所建立的，不過他們有非常大的區別。

## The global and process Objects

兩個基本的物件`global`和`process`物件，在 Node 裡的`global`物件跟瀏覽器中的全域變數相似。但是`process`物件只有在 Node 裡才有。

### The global object

在瀏覽器中，在最頂層宣告變數的話就會是 global 的變數，但是在 Node 中，宣告變數在 module 不會變全域變數，所以你宣告全域變數在其中一個 module 時，而在另外一個 module 又使用它，這樣不會發生衝突

```js
// add2.js
var base = 2;

function addtwo(input) {
   return parseInt(input) + base;
}
```

```html
<!--chap2-1.html-->
<!DOCTYPE html>
<html>
   <head>
      <script src="add2.js"></script>
      <script>
         var base = 10;
         console.log(addtwo(10)); // 20
      </script>
   </head>
<body>
</body>
</html>
```

在瀏覽器中又定義`base = 10`會把`add2.js`裡的 base 覆蓋過去，把原本的 2 變成了 10。相加的時候會變成 20。

```js
// addtwo.js
var base = 2;
exports.addtwo = function(input) {
  return parseInt(input) + base;
};
```

```js
// chap2-2.js
var addtwo = require('./addtwo').addtwo;
var base = 10;
console.log(addtwo(base));
```

```shell
$ node chap2-2.js
12
```

在 Node application 裡面會是 12。在 chap2-2.js 宣告一個新的`base`不會影響他在 module 裡的 base，因為兩個存在不同的 global namespace。

在 module 之間沒有共享的變數，`global`物件提供環境能夠存取全域的變數或是 function。可以查看他：

```shell
node -p "global"
```

### The process object

在 Node 的環境中`process`的物件是必要的元件，因為它提供了 runtime environment 的資訊。此外，`process`還提供 standard input/output (I/O)。

- versions：版本
- env：環境變數
- release：發布版本
- lts：Long-Term Support 版本

```shell
$ node -p "process.versions"
{ http_parser: '2.8.0',
  node: '8.11.1',
  v8: '6.2.414.50',
  uv: '1.19.1',
  zlib: '1.2.11',
  ares: '1.10.1-DEV',
  modules: '57',
  nghttp2: '1.25.0',
  openssl: '1.0.2o',
  icu: '60.1',
  unicode: '10.0',
  cldr: '32.0',
  tz: '2017c' }

$ node -p "process.env"
{ ... }

$ node -p "process.release"
{ name: 'node',
  lts: 'Carbon',
  sourceUrl: 'https://nodejs.org/download/release/v8.11.1/node-v8.11.1.tar.gz',
  headersUrl: 'https://nodejs.org/download/release/v8.11.1/node-v8.11.1-headers.tar.gz' }

$ node -p "process.release.lts"
Carbon
```

Node Application 能夠存取 standard I/O，standard steams 會在 application 和 environment 預先建立通道，這個通道會提供 communication 在 Node application 和 terminal 之間。

Node 提供三個 process function：

- `process.stdin`
- `process.stdout`
- `process.stderr`

在關閉 application 前你不能關閉這些 stream。

`process` I/O function 是從`EventEmitter`繼承下來，EventEmitter 意味著你可以 emit event 然後捕捉這些 event 且處理這些資料。

要能處理`process.stdin`輸入的資料，必須先設定 stream 的編碼，如果沒有設定的話你會拿到 buffer 的結果而非 string。

下一步，我們聽`readable`的 event，讓我們知道有資料準備被讀取，然後使用`process.stdin.read()`來讀這些資料，如果這些資料不是`null`他就使用`process.stdout.write()`把些同的內容輸出到`process.stdout`。

```js
// example2-1.js
process.stdin.setEncoding('utf8');
process.stdin.on('readable', function() {
   var input = process.stdin.read();
   if (input !== null) {
      // echo the text
      process.stdout.write(input);
   }
});
```

即使沒使用`setEncoding`也會得到相同的結果，他會讀取 buffer 然後寫入 buffer，有使用`setEncoding`的話則是讀取 string 寫入 string。

可以使用`process.exit(0)`來終止程式

```js
// example2-1.js
process.stdin.setEncoding('utf8');

process.stdin.on('readable', function() {
   var input = process.stdin.read();

   if (input !== null) {
      // echo the text
      process.stdout.write(input);

      var command = input.trim();
      if (command == 'exit')
         process.exit(0);
   }
});
```

現在輸入 **exit** 的話，作用就如同 Ctrl-C，終止程式。

如果現在把`process.stdin.setEncoding()`這行移除的話，程式將會出錯，因為 buffer 上面沒有`trim()`的 function，可以改成：

```js
var command = input.toString().trim();
```

但是最好的方式還是加上 encoding，避免任何未預期的問題發生。

`stderr`的通道是用來區分預期的 output 跟發生的問題。

## Buffers, Typed Arrays, and Strings

在早期的 Javascript 不支援 binary data，因為原本的 Javascript 只需要處理 string 和輸出到 windows，即使 Ajax 的出現，也只是改變 client 和 server 之間使用 Unicode 的字串。

但是很多事情在 Javascript 上變得更加複雜，不只有 Ajax 要使用，還有 WebSocket，甚至還有最新的科技像是 WebGL 和 Canvas。

在 Javascript 裡的解決方法是使用`ArrayBuffer`，由 **typed array **來操作 ArrayBuffer，而在 Node 裡的解決方法是用 Buffer。

原先兩種並不相同，而是在 v4.5 版之後開始支援 **typed array**，現在的 Node buffer 是由`Uint8Array`來支援。在 Node 裡最主要的資料結構是`Buffer` class 被使用來大多數的 I/O。

Node Buffer 轉換成 **typed array**：

- Buffer 的記憶體是會被複製，而不是共享。
- Buffer 的記憶體被解釋為 array 而不是 byte array。`new Uint32Array(new Buffer([1,2,3,4]))` 會建立四個`Uint32Array`的元件，而不是`Uint32Array`單個元件像是`[0x10203040]`或`[0x40302010]`

所以在 Node 裡你可以使用兩種型態`Buffer`和`ArrayBuffer`來處理 octet stream，但大多數你會使用 buffer。

Node buffer 是被 allocate 在 V8 heap 之外的 raw binary data，透過 class 可以管理 Buffer，而且一旦被 allocate 的話， buffer 就不能被 resize。

buffer 是存取資料的預設型態。除非在讀取和寫入文件時提供了特定的編碼，否則資料將被讀入或讀出 buffer。

在 Node v4 你可以直接建立 buffer：

```js
let buf = new Buffer(24)
```

需要注意的是，他建立 Node buffer 不會初始化資料，不像`ArrayBuffer`，如果你想要確保你不想拿到外預期的資料的話，你可以在他被建立之後馬上使用`fill()`

```js
let buf = new Buffer(24)
buf.fill(0)
```

標明他的開示和結尾的位置，你可以填入部分的值到 buffer 裡：

```js
buf.fill(string[, start[, end]][, encoding])
buf.fill(1, 2, 4)
```

- string：填入的字串
- start：起始位置
- end：結尾的前一個位置
- encoding：編碼

你可以傳入 octet、buffer 或是 string 直接建立一個新的 buffer，buffer 建立的時候會複製所有裡面的內容，預設編碼是 UTF-8，如果不是 UTF-8 的話需要特別指定他。

```js
let str = 'New String';
let buf = new Buffer(str);
```

`Buffer.from()`傳入 array 會回傳複製的的內容

> 不過傳入 ArrayBuffer 會共享想同的記憶體（他寫的，不確定是不是

傳入 buffer 會複製 buffer 的內容，傳入 string 也會複製 string 的內容。

`Buffer.alloc()`建立 buffer 特定的長度，`Buffer.allocUnsafe()`一樣是建立特定的長度，不過可能會包含舊的資料。

```js
// chap2-2b.js
'use strict';
let a = [1,2,3];
let b = Buffer.from(a);
console.log(b);

let a2 = new Uint8Array([1,2,3]);
let b2 = Buffer.from(a2);
console.log(b2);

let b3 = Buffer.alloc(10);
console.log(b3);

let b4 = Buffer.allocUnsafe(10);
console.log(b4);
```

```shell
$ node chap2-2b.js 
<Buffer 01 02 03>
<Buffer 01 02 03>
<Buffer 00 00 00 00 00 00 00 00 00 00>
<Buffer 00 00 00 00 00 00 00 00 68 f9>
```

### Buffer, JSON, StringDecoder, and UTF-8 Strings

Buffer 可以轉換成 JSON 以及 string：

```js
// chap2-3.js
"use strict";

let buf = new Buffer('This is my pretty example');
let json = JSON.stringify(buf);

console.log(json);
```

```shell
$ node chap2-3.js 
{"type":"Buffer","data":[84,104,105,115,32,105,115,32,109,121,32,112,114,101,116,116,121,32,101,120,97,109,112,108,101]}
```

JSON 指明了物件的型態 Buffer，和裡面的 data，可以看到裡面的資料不是人類能閱讀的

Buffer 跟 JSON 可以互相轉換，最後再用`Buffer.toString()`轉成 string：

```js
// example2-2.js
"use strict";

let buf = new Buffer('This is my pretty example');
let json = JSON.stringify(buf);

let buf2 = new Buffer(JSON.parse(json).data);

console.log(buf2.toString()); // This is my pretty example
```

```shell
$ node example2-2.js 
This is my pretty example
```

`toString()`預設是 UTF-8 如果要轉成其他的：

```js
console.log(buf2.toString('ascii')); // This is my pretty example
```

也可以指名他的開頭和結尾的位置：

```js
console.log(buf2.toString('utf8', 11, 17)); // pretty
```

`Buffer.toString()`不是唯一一種轉成 string 的方法，我們可以使用`StringDecoder`，他的目的是用來解碼成 UTF-8 的字串，但是他更加的彈性和復原。如果用`buffer.toString()`遇到未完成的 UTF-8 字元時，她會出現亂碼。而`StringDecoder`會先 buffer 未完成的字元，直到他完成字元然後輸出結果。所以如果你在接收 UTF-8 字元的時候，應該要使用`StringDecoder`。

像是`€` 字元是由三個八位數的 bit 所組成的：

```js
// chap2-4.js
"use strict";

let StringDecoder = require('string_decoder').StringDecoder;
let decoder = new StringDecoder('utf8');

let euro = new Buffer([0xE2, 0x82]);
let euro2 = new Buffer([0xAC]);

console.log(decoder.write(euro));
console.log(decoder.write(euro2));

console.log(euro.toString());
console.log(euro2.toString());
```

前面的 buffer 是用兩個 octet 所組成的，最後一個是第三個 octet。

```shell
$ node chap2-4.js 

€
�
�
```

可以使用`buffer.write()`寫入字元，buffer 要有足夠的字元容納 UTF-8 的字：

```js
let buf = new Buffer(3);
buf.write('€', 'utf-8');
console.log(buf.length); // 3
console.log(buf); // <Buffer e2 82 ac>
```

可以用`buffer.length`來查看 buffer 的大小。

### Buffer Manipulation

你可以讀取和寫入 buffer 然後給定他的 type，範例是寫入四個 unsigned 8-bit integer：

```js
var buf = new Buffer(4);

// write values to buffer
buf.writeUInt8(0x63,0);
buf.writeUInt8(0x61,1);
buf.writeUInt8(0x74,2);
buf.writeUInt8(0x73,3);

// now print out buffer as string
console.log(buf.toString()); // cats

```

可以使用`buffer.readUInt8()`讀取每一個 unsigned 8-bit 的字元：

```js
buf.readUInt8(); // 99
buf.readUInt8(1); // 97
```

Node 支援讀取和寫入 signed 和 unsigned 的 8-, 16-, 32- 的 integer，以及 float 和 double，他也支援 little-endian 或 big-endian 的格式，舉例：

- `buffer.readUintLE()`
- `buffer.writeUint16BE()`
- `buffer.readFloatLE()`
- `buffer.writeDoubleBE()`

你也可以用 array 的方式直接指派值：

```js
var buf = new Buffer(4)
buf[0] = 0x63;
buf[1] = 0x61;
buf[2] = 0x74;
buf[3] = 0x73;
```

如果想要讀寫特定一段 buffer 的話可以使用`buffer.slice()`，我們想要使用新的 buffer 來修改原本的 buffer ：

```js
// example2-3.js
var buf1 = new Buffer('this is the way we build our buffer');
var lnth = buf1.length;

// create new buffer as slice of old
var buf2 = buf1.slice(19,lnth);
console.log(buf2.toString()); // build our buffer

//modify second buffer
buf2.fill('*',0,5);
console.log(buf2.toString()); // ***** our buffer

// show impact on first buffer
console.log(buf1.toString()); // this is the way we ***** our buffer
```

會發先修改新的 buffer 會連原本舊的 buffer 一併修改，在處理內容的時候很方便。

如果想要測試兩個 buffer 是不是想同的話可以使用`buffer.equals()`：

```js
if (buf1.equals(buf2)) console.log('buffers are equal'); // false
```

你也可以使用`buffer.copy()`來複製 buffer，如果 buffer 的長度不夠容納所有的內容的話，就只會複製部分的 byte。

```js
// chap2-6.js
var buf1 = new Buffer('this is a new buffer with a string');

// copy buffer
var buf2 = new Buffer(10);
buf1.copy(buf2);

console.log(buf2.toString()); // this is a
```

如果想要比較 buffer 的話，可以使用`buffer.compare()`，他會傳值會是 -1, 0, 1：

```js
// chap2-7.js
var buf1 = new Buffer('1 is number one');
var buf2 = new Buffer('2 is number two');

var buf3 = new Buffer(buf1.length);
buf1.copy(buf3);

console.log(buf1.compare(buf2)); // 小於 -1
console.log(buf2.compare(buf1)); // 大於 1
console.log(buf1.compare(buf3)); // 等於 0
```

還有另外的 buffer class，像是`SlowBuffer`，如果需要小量的 buffer 且存留一段比較長的時間，可以使用`SlowBuffer`，buffer 必須小於 4KB，且 garbage collection 不會處理非常小的記憶體，如果有需要提升效能的話可以使用。

#### Endianness

[Wikipedia Endianness](https://en.wikipedia.org/wiki/Endianness#Illustration)

- Little endian：位數最小的 byte 會從 memory 最小地址的地方開始填入
- Big endian：位數最大的 byte 會從 memory 最小地址的地方開始填入

## Node's Callback and Asynchronous Event Handling

Javascript 是 single-threaded，他是同步執行，意思是他會逐行執行，直到應用程式結束。

如果你必須要等某些事，像是開一個檔案、網頁回應，然後他會把應用程式 block 住，直到那個操作結束，這樣在網頁裡會是一個單點錯誤的應用程式。

預防 blocking 的解決方法是 event loop。

### The Event Queue (Loop)

為了能夠非同步的執行 function，應用程式有兩種方法可以達成。

其中一種是使用 thread 到每個 time-consuming process，但問題是開 thread 對資源來說也是很昂貴的，而且會增加程式的複雜度。

第二個方法是採用 event-driven 的架構，檔每個 time-consuming process 被呼叫，應用程式不會等他結束，當有人 emit 發出一個事件 event 的時候會通知 process，這個事件會被加入 queue 或是 event loop，任何有依賴關係的 function 會去註冊他所感興趣的 event，當依賴關係的 function 被呼叫，event 會從 event loop 裡執行，且傳入相對應的資料。

Javascript 不管是在瀏覽器裡或是在 Node 裡，都是採用第二種方法。

Node 會有順序的處理 queue 中的 event，當收到感興趣的 event，他就會呼叫你所提供的 callback function，然後傳入任何相關的資料。

這是在我們第一章節所看到的程式碼，可以看到 event loop 在其中運作：

```js
// chap1-1.js
var http = require('http');

http.createServer(function (request, response) {
  response.writeHead(200, {'Content-Type': 'text/plain'});
  response.end('Hello World\n');
}).listen(8124);

console.log('Server running at http://127.0.0.1:8124/');
```

可以把它拆成各自的動作，監聽更多的事件：

```js
// example2-4.js
var http = require('http');

var server = http.createServer();

server.on('request', function (request, response) {

   console.log('request event');

   response.writeHead(200, {'Content-Type': 'text/plain'});
   response.end('Hello World\n');
});

server.on('connection', function() {
   console.log('connection event');
});

server.listen(8124, function() {

   console.log('listening event');
});

console.log('Server running on port 8124');
```

注意`requestListener()`server 請求的 callback 不再被`http.createServer()`所呼叫，應用程式指派 HTTP server 到一個新的變數上，然後使用它來捕捉兩個 event：

- `request`：每次 client 請求的時候發出 event。
- `connection`：每次新的使用者連線到 web application 的時候發出 event。

在兩個例子中，event 被`on()`所訂閱，HTTP server 從`EventEmitter` class 繼承了`on()` function，這裡還有另外一個 event 被訂閱，`server.listen()` event，也是使用 callback function 來存取。

- `listen`

他分別監聽三個事件`request`、`connection`、`listen`。

```shell
$ node example2-4.js
Server running on port 8124
listening event
connection event
request event
request event
```

一開始會馬上印出`Server running on port 8124`，因為當 server 建立的時候或開始監聽 request 的時候應用程式沒有 block 住，所以第一個`console.log()`被印出來是在我們執行所有 non-blocking 非同步的 function 之後。

接下來是印出`listening event`，如果你在`http.createServer()`加入 callback function，他會先印出這裡面的訊息。

在前面的例子是把`http.serverCreate()`跟`server.listen()`串在一起，不過也可以不必這們做，串在一起是比較方便且對程式碼必較簡潔，而且不一定要事件驅動，`server.listen()`在當有人發出 listen event 的時候他才會被執行。

直到 connection 斷掉前，他都不會在印出`connection event`。

中斷連線有可能是你關掉瀏覽器或是連線逾時，不同也頁面會分別建立不同的連線（要看瀏覽器）。

### Creating An Asynchronous Callback Function

```js
// example2-5.js
var fib = function (n) {
    if (n < 2) return n;
    return fib(n - 1) + fib(n - 2);
};

var Obj = function() { };

Obj.prototype.doSomething = function(arg1_) {
   var callback_ = arguments[arguments.length - 1];
   callback = (typeof(callback_) == 'function' ? callback_ : null);
   var arg1 = typeof arg1_ === 'number' ? arg1_ : null;

   if (!arg1)
      return callback(new Error('first arg missing or not a number'));

      process.nextTick(function() {

         // block on CPU
         var data = fib(arg1);
         callback(null, data);
    });
}

var test = new Obj();
var number = 10;

test.doSomething(number, function(err,value) {
      if (err)
         console.error(err);
      else
         console.log('fibonaci value for %d is %d', number, value);
});

console.log('called doSomething');
```

第一個功能是他確保最後的參數事 callback function，在 callback function 裡的第一個參數是 error，callback function 的第一個參數通常都會是 errback，因為我們不能決定使用者的意圖，但我們必須確定最後一個參數是 function。

第二個功能是建立新的`Error`物件，如果有錯誤發生，他就會回傳錯誤結果到 callback function。我們不能在非同步的世界中依賴`throw...catch`，所以錯誤 error handling 必須要在 callback 回傳`Error`物件處理。

最後一個功能是呼叫 callback function，如果沒有發生錯誤的話，會傳入 function 的資料。若要確保 callback 是非同步，我們需要在裡面呼叫`process.nextTick()`，原因是`process.nextTick()`確保 callback function 被呼叫前會清除 event loop，意思是如果有任何同步的 function 被呼叫，他 block 住，同步的 function 會繼續執行，將`number`由 10 改成 50，可以看到他會有明顯的差別，取決於你的 CPU 運算的能力，在`process.nextTick()`呼叫 Fibonacci function 是確保會用非同步的方式執行。

建立非同步的 function 有四個關鍵的點：

- 確保最後一個參數是 callback function。
- 建立`Error`物件，如果有錯誤的時候，在 callback function 的第一個參數裡回傳他。
- 如果沒有錯誤發生，呼叫 callback function，設定 error 參數為`null`，然後傳入相對應的資料。
- callback function 必須在`process.nextTick()`被呼叫，確保 process 不會被 block 住。

在 Node 中要做到非同步的方法相當簡單，而且會有一致性的結果。

Node 裡的 event loop 是單線程的。 但是，這並不意味著沒有多個線程忙於在後台工作。

Node 呼叫的功能，例如用 C++ 而不是 JavaScript 實現的 File System (fs)。 fs 功能利用 worker 線程來完成其功能。 另外，Node 使用 libuv library，該 library 使用工作線程 pool 來實現某些功能。 多少個線程與操作系統有關。如果感興趣的話可以看這篇看：

- [An Introduction to libuv](https://nikhilm.github.io/uvbook/index.html)
- [When is thread pool used?](https://stackoverflow.com/questions/22644328/when-is-the-thread-pool-used)
- [Multi threading and multiple process in Node.js](https://itnext.io/multi-threading-and-multi-process-in-node-js-ffa5bb5cde98)

### EventEmitter

只要有任何物件觸發那一個 event，那 event 就會用`on()` function 來處理。

```js
// example2-6.js
var eventEmitter = require('events').EventEmitter;
var counter = 0;

var em = new eventEmitter();

setInterval(function() { em.emit('timed', counter++); }, 3000);

em.on('timed', function(data) {
  console.log('timed ' + data);
});
```

通常會用兩個步驟來建立`EventEmitter`，當有特別的 event 被觸發的時候，`EventEmitter.on()` event handler 會被呼叫，第一個參數就是事件的名稱，第二個參數是 callback function。

然後使用`EventEmitter.emit()`去觸發 event。`EventEmitter.on()`會來捕捉 event。

有趣的是，他並不實用，我們通常都會把`EventEmitter`的功能加到我們現有的物件裡，而不是在應用程式裡使用`EventEmitter`，在`http.Server`有做的這件事。

因為`EventEmitter`的功能是被繼承的，所以我們可以使用另外一個物件`Util`，做到讓其他的物件可以繼承`EventEmitter`。

```js
// example2-7.js
"use strict";

var util = require('util');
var eventEmitter = require('events').EventEmitter;
var fs = require('fs');

function InputChecker (name, file) {
   this.name = name;
   this.writeStream = fs.createWriteStream('./' + file + '.txt',
      {'flags' : 'a',
       'encoding' : 'utf8',
       'mode' : 0o666});
};

util.inherits(InputChecker,eventEmitter);

InputChecker.prototype.check = function check(input) {

  // trim extraneous white space
  let command = input.trim().substr(0,3);

  // process command
  // if wr, write input to file
  if (command == 'wr:') {
     this.emit('write',input.substr(3,input.length));

  // if en, end process
  } else if (command == 'en:') {
     this.emit('end');

  // just echo back to standard output
  } else {
     this.emit('echo',input);
  }
};

// testing new object and event handling
let ic = new InputChecker('Shelley','output');

ic.on('write', function(data) {
   this.writeStream.write(data, 'utf8');
});

ic.on('echo', function( data) {
   process.stdout.write(ic.name + ' wrote ' + data);
});

ic.on('end', function() {
   process.exit();
});

// capture input after setting encoding
process.stdin.setEncoding('utf8');
process.stdin.on('readable', function() {
   let input = process.stdin.read();
   if (input !== null)
      ic.check(input);
});
```

`util.inherits()`這個 function 可以做到繼承的功能。它可以讓一個 constructor 去繼承另一個 supercontructor prototype 的方法。你也可以在 contructor 內的 function 直接存取 supercontructor。

`util.inherits()`允許我們繼承任何 class Node event queue 的功能，並且包括`EventEmitter

```js
util.inherits(InputChecker,eventEmitter);
```

在使用`util.inherits()`之後，你可以在物件的 method 裡觸發 method：

```js
InputChecker.prototype.check = function check(input) {
  this.emit('event',input);
};

InputChecker.on('event', function() { });
```

在新的應用程式中，新的`InputChecker` class 被建立了，他的 contructor 需要兩個值：一個是人的名字，另一個是檔案。它將該人的姓名分配給一個 property，並使用 File System module 的`createWriteStream`方法創建對可寫入流的 stream。

物件有另一個方法`check`會把進來的資料給對應的指令，其中一個指令`wr:`會觸發寫入 event，另一個`en:`會觸發結束 event，如果沒有其他指令的話，就會觸發`echo`事件，因為物件觸發了三個 event，所以要分別對不同的 event 加上 handler。

所有的 input 都是從`process.stdin`輸入的，輸出的話使用 writable stream，他可以在背景執行，queue 住，讓你未來在寫入，他是一個非常有效率的 file 輸出的方法，如果你很頻繁的使用。

`echo`的話就只會使用`process.stdout`標準輸出到終端機上。

因為使用嚴格模式`use strict`，所以不能使用 octal literals 像是 (0666) 來作為文件寫入的標示，而是使用符號 0o666，這是一個 ES6 風格的文字。

```shell
SyntaxError: Octal literals are not allowed in strict mode.
```

`on`function 是`EventEmitter.addListener`的捷徑，兩個作用是相同的：

```js
ic.addListener('echo', function(data) {
  console.log(this.name + ' wrote ' + data);
});
// exactly equivalent to
ic.on('echo', function(data) {
  console.log(this.name + ' wrote ' + data);
});
```

你可以用`EventEmitter.once()`來監聽下一個事件：

```shell
ic.on('echo', function( data) {
   process.stdout.write(ic.name + ' wrote ' + data);
});

ic.once('echo', function( data) {
   process.stdout.write(ic.name + ' wrote once ' + data);
});
```

```shell
$ node example2-7.js 

Shelley wrote 
Shelley wrote once 

Shelley wrote
```

如果你的 event 超過 10 個 listener 他預設會警告你

```js
$ node example2-7.js 
(node:10701) MaxListenersExceededWarning: Possible EventEmitter memory leak detected. 11 echo listeners added. Use emitter.setMaxListeners() to increase limit

Shelley wrote 
Shelley wrote once 
Shelley wrote once 
Shelley wrote once 
Shelley wrote once 
Shelley wrote once 
Shelley wrote once 
Shelley wrote once 
Shelley wrote once 
Shelley wrote once 
Shelley wrote once 
Shelley wrote once 
```

你可以使用`emitter.setMaxListeners() `把值設為零，可以有無限的 listener。

```js
let ic = new InputChecker('Shelley','output');
ic.setMaxListeners(0);
```

如果想要移出 listener 可以使用`EventEmitter.removeListener()`：

```js
ic.on('echo', callback);
ic.removeListener('echo',callback);
```

他會從`EventEmitter.listeners()`這個陣列中移除，一旦陣列有任何變動，不論什麼原因，他會被從新建立。

### The Node Event Loop and Timers

timer 有兩個`setTimeout()`和`setInterval()`兩個 function，這兩個並不完全相同，在瀏覽器中他是使用瀏覽器的引擎，在 Node 的 event loop 是由 C++ library libuv 處，但他們不同的地方幾乎微不足道。

`setTimeout()`的第一個參數是 callback function，第二個參數是延遲時間，單位微秒：

```js
// chap2-8.js
setTimeout(function(name) {
             console.log('Hello ' + name);
           }, 3000, 'Shelley');

console.log("waiting on timer...");
```

```shell
$ node chap2-8.js 
waiting on timer...
Hello Shelley
```

name 是 callback function 裡的參數，可以在`setTimeout()`裡當作參數傳入。

你可以用`clearTimeout()`取消 timer：

```js
// chap2-9.js
var timer1 = setTimeout(function(name) {
             console.log('Hello ' + name);
           }, 30000, 'Shelley');

console.log("waiting on timer...");

setTimeout(function(timer) {
            clearTimeout(timer);
            console.log('cleared timer');
           }, 3000, timer1);
```

timer 需要非常長的一段時間才會執行，時間長到他在執行之前就會呼叫取消 timer。

`setInterval()`跟`setTimeout()`的運作非常相似，`setInterval()`在執行完之後他會重新計時，直到應用程式取消他，可以使用`clearInterval()`取消他：

```js
// chap2-10.js
var interval = setInterval(function(name) {
             console.log('Hello ' + name);
           }, 3000, 'Shelley');

setTimeout(function(interval) {
            clearInterval(interval);
            console.log('cleared timer');
           }, 30000, interval);

console.log('waiting on first interval...');
```

在他停止之前會重複印出 9 次。

```shell
$ node chap2-10.js 
waiting on first interval...
Hello Shelley
Hello Shelley
Hello Shelley
Hello Shelley
Hello Shelley
Hello Shelley
Hello Shelley
Hello Shelley
Hello Shelley
cleared timer
```

Node 文件裡有提到，他並不保證 callback function 會確切地在 n 微秒被呼叫，瀏覽器裡的`setTimeout()`並沒有不同，我們沒辦法絕對的控制環境，和他所延遲的時間。在大部分情況下，當涉及到 timer 功能時，我們感覺不到任何時間差異。不過，如果我們正在製作動畫，我們實際上可以看到影響。

有兩個 Node 特別的 function 你可以使用在 timer/interval 裡：`ref()`和`unref()`。

- 如果你 timer 呼叫`unref()`，而且在 event queue 裡面是唯一的 event 的話，這種情況下允許終止程式。
- 如果你在同個 timer 呼叫`ref()`，這會使程序繼續運行，直到 timer 處理完畢。

第一個例子是跑比較長的 timer，然後呼叫`unref()`：

```js
// chap2-11.js
var timer = setTimeout(function(name) {
             console.log('Hello ' + name);
           }, 30000, 'Shelley');

timer.unref();

console.log("waiting on timer...");
```

```shell
$ node chap2-11.js 
waiting on timer...
```

然後修改成 interval 以及 timeout，然後再呼叫`unref()`

```js
// chap2-12.js
var interval = setInterval(function(name) {
             console.log('Hello ' + name);
           }, 3000, 'Shelley');

var timer = setTimeout(function(interval) {
            clearInterval(interval);
            console.log('cleared timer');
           }, 30000, interval);

timer.unref();

console.log('waiting on first interval...');
```

```shell
$ node chap2-12.js
waiting on first interval...
Hello Shelley
Hello Shelley
Hello Shelley
Hello Shelley
Hello Shelley
Hello Shelley
Hello Shelley
Hello Shelley
Hello Shelley
cleared timer
```

`unref()`允許終止程式的條件是，event queue 裡面只有一個 timer event，但是現在有兩個 timer event，所以他會等到`clearInterval()`被呼叫之後，event queue 剩下一個 timer event，最後程式才會被終止，在被終止前，他都會繼續運行。

最後一組在 Node 才有的 timer-like function：`setImmediate()`跟`clearImmediate()`。`setImmediate()`建立 event，但 event 的優先權高於`setTimeout()`和`setInterval()`，然而他的優先權不會高過 I/O event，但是高過任何 timer event。如果你在 callback function 裡面呼叫他的話，它會放到下一個 event loop，在這個完成之後它被接著被呼叫。這是一種向當前或下一個 event loop 添加 event 而不需要指定時間的方法。`setImmediate()`比`setTimeout(callback, 0)`的效率更高，因為它優先於其他 timer event。

跟`process.nextTick()`很類似，在當前的 event loop 結束之後會馬上被執行，會在新的 I/O event 之前被呼叫。而`setImmediate()`會在 I/O event 之後。

## Nested Callbacks and Exception Handling

在 Javascript 裡很常看到：

```js
val1 = callFunctionA();
val2 = callFunctionB(val1);
val3 = callFunctionC(val2);
```

 function 會有順序的被呼叫，前一個 function 的結果會傳入到下一個 function。因為所有的 function 是同步的，我們不需擔心 function 的呼叫順序，不會有未預期的結果。

同步的方法打開檔案，然後將所有 apple 的單字改成 orange，然後寫入黨案。

```js
// example2-8.js
var fs = require('fs');

try {
   var data = fs.readFileSync('./apples.txt','utf8');
   console.log(data);
   var adjData = data.replace(/[A|a]pple/g,'orange');

   fs.writeFileSync('./oranges.txt', adjData);
} catch(err) {
  console.error(err);
}
```

因為我們沒辦法預期錯誤會出現在哪裡，不過我們可以使用`try`來捕捉 exception handling。

```shell
$ node example2-8.js 
{ Error: ENOENT: no such file or directory, open './apples.txt'
    at Object.fs.openSync (fs.js:646:18)
    at Object.fs.readFileSync (fs.js:551:33)
    at Object.<anonymous> (/Users/akiicat/Github/LearningNode2/chap2/example2-8.js:4:18)
    at Module._compile (module.js:652:30)
    at Object.Module._extensions..js (module.js:663:10)
    at Module.load (module.js:565:32)
    at tryModuleLoad (module.js:505:12)
    at Function.Module._load (module.js:497:3)
    at Function.Module.runMain (module.js:693:10)
    at startup (bootstrap_node.js:188:16)
  errno: -2,
  code: 'ENOENT',
  syscall: 'open',
  path: '../apples.txt' }
```

把同步的程式轉換成非同步的程式需要做一些修改。首先，我們需要先把所有的 function 改成非同步的。然而，我們沒辦法保證 function 獨自被呼叫的時候會正確地執行，為了要確保每個 function 正確的被呼叫就需要使用 nested callback。

```js
// example2-9.js
var fs = require('fs');
fs.readFile('./apples.txt','utf8', function(err,data) {
   if (err) {
      console.error(err);
   } else {

     var adjData = data.replace(/[A|a]pple/g,'orange');

     fs.writeFile('./oranges.txt', adjData, function(err) {

        if (err) console.error(err);
     });
   }
});
```

上面是非同步的版本，所有的 File System function 都被換成非同的，function 在 nested callback 理會正確的的被呼叫，此外`try...catch`被移掉了。

我們不能使用`try...catch`，因為我們使用非同步的 function，在`try...catch`內的程式會在執行非同步的 function 前就先執行完了。所以如果在 callback function 裡丟出錯誤的話，他會被外面的程序所接住，而不會在`try...catch`裡被接到。取而代之的是，我們需要直接處理錯誤：如果有錯誤，回傳錯誤，如果沒有錯誤，繼續執行 callback function。

在非同步的 function 中呼叫另外一個 callback function，帶來新的挑戰是 error handling。

下一個範例是獲得某個資料夾底下的所有檔案名稱，然後會用`replace`將所有的檔案內的 domain name 取代掉，寫回原本的檔案，然後將所以改過檔案的紀錄寫道 log.txt 檔案內。

```js
// example2-10.js
var fs = require('fs');
var writeStream = fs.createWriteStream('./log.txt',
      {'flags' : 'a',
       'encoding' : 'utf8',
       'mode' : 0666});

writeStream.on('open', function() {
   // get list of files
   fs.readdir('./data/', function(err, files) {

      // for each file
      if (err) {
         console.log(err.message);
      } else {
         files.forEach(function(name) {

            // modify contents
            fs.readFile('./data/' + name,'utf8', function(err,data) {

               if (err){
                  console.error(err.message);
               } else {
                  var adjData = data.replace(/somecompany\.com/g,
                             'burningbird.net');

                  // write to file
                  fs.writeFile('./data/' + name, adjData, function(err) 
                    {

                     if (err) {
                        console.error(err.message);
                     } else {

                        // log write
                        writeStream.write('changed ' + name + '\n',
                        'utf8', function(err) {

                           if(err) console.error(err.message);
                        });
                     }
                  });
               }
            });
         });
      }
    });
});

writeStream.on('error', function(err) {
  console.error("ERROR:" + err);
});
```

首先，我們看到一些新東西：當我們呼叫`fs.createWriteStream()` function 時，使用 event handling 來處理錯誤。使用 event handling 的原因是`createWriteStream()`是非同步的，所以我們不能使用傳統的`try ... catch`錯誤處理。 同時，它也不能捕捉錯誤的 callback function。相反，我們使用`error`事件並通過寫出錯誤消息來處理它。然後我們使用`open`事件來處理成功時的操作。

注意我們是使用非同步來執行，所以應用程式會獨自的分別寫入 log，如果你執行多次的話，會發現結果會不一定相同：

```js
// log.txt
changed data1.txt
changed data2.txt
changed data5.txt
changed data4.txt
changed data3.txt

changed data4.txt
changed data3.txt
changed data1.txt
changed data2.txt
changed data5.txt
```

如果您想檢查所有文件何時被修改以便執行某些操作，則會出現另一個問題。`forEach`方法使用非同步來呼叫 callback function，所以它不會 block 住。在使用`forEach`之後添加一條程式碼，如下所示：

```js
console.log('all finished');
```

並不意味著成程式馬上結束，只是`forEach`不在被 block 住。

```js
// log write
writeStream.write('changed ' + name + '\n', 'utf8', function(err) {
  if(err) {
    console.log(err.message);
  } else {
    console.log('finished ' + name);
  }
}); 
```

```shell
$ node example2-11.js 
all finished
finished data3.txt
finished data4.txt
finished data2.txt
finished data1.txt
finished data5.txt
```

為了解決這個問題，加一個 counter 在每個 log message 裡，然後檢查 counter 是否跟 file 的數量一樣，如果一樣就印出 all done 的訊息：

```js
var counter = 0;
...
  // log write
  writeStream.write('changed ' + name + '\n',
   function(err) {
     if(err) {
        console.error(err.message);
     } else {
        console.log('finished ' + name);
        counter++;
        if (counter >= files.length) {
           console.log('all done');
        }
      }
  });
```

你會在最後看到預期的 all done 訊息顯示在檔案最後的地方：

```shell
$ node example2-11.js 
all finished
finished data2.txt
finished data1.txt
finished data3.txt
finished data4.txt
finished data5.txt
all done
```

如果你目錄底下還有目錄的話，程式會發生錯誤。

```shell
$ node example2-10.js 
EISDIR: illegal operation on a directory, read
```

透過使用`fs.stats`方法返回表示一個物件來防止這類型的錯誤。這個物件包含有關它是否是檔案的訊息。`fs.stats`當然是另一種非同步方法，需要更多 nested callback function。

```js
// example2-11.js
var fs = require('fs');
var writeStream = fs.createWriteStream('./log.txt',
      {flags : 'a',
       encoding : 'utf8',
       mode : 0666});

writeStream.on('open', function() {

   var counter = 0;

   // get list of files
   fs.readdir('./data/', function(err, files) {

      // for each file
      if (err) {
         console.error(err.message);
      } else {
         files.forEach(function(name) {

            fs.stat('./data/' + name, function (err, stats) {

               if (err) return err;
               if (!stats.isFile()) {
                  counter++;
                  return;
               }

               // modify contents
               fs.readFile('./data/' + name,'utf8', function(err,data) {

                  if (err){
                     console.error(err.message);
                  } else {
                     var adjData = data.replace(/somecompany\.com/g,
                             'burningbird.net');

                     // write to file
                     fs.writeFile('./data/' + name, adjData, 
                                                  function(err) {

                        if (err) {
                           console.error(err.message);
                        } else {

                           // log write
                           writeStream.write('changed ' + name + '\n',
                            function(err) {

                              if(err) {
                                 console.error(err.message);
                              } else {
                                 console.log('finished ' + name);
                                 counter++;
                                 if (counter >= files.length) {
                                    console.log('all done');
                                 }
                               }
                           });
                        }
                     });
                  }
               });
            });
         });
         console.log('all done');
       }
    });
});

writeStream.on('error', function(err) {
  console.error("ERROR:" + err);
});
```

這個程式完成它的目的，而且也能正確地執行，但是這是很難閱讀和維護！這種 nested callback 又可以稱作 callback spaghetti 或是 [pyramid of doom](https://en.wikipedia.org/wiki/Pyramid_of_doom_%28programming%29)。

nested callback 會繼續把程式碼往右邊推移，使撰寫程式碼更加困難。但是我們又不能將 nested callback 拆掉，因為這些 function 必須要有順序的被呼叫：

- 開始尋找某個資料夾下的檔案。
- 過濾掉子資料夾。
- 閱讀每個文件的內容。
- 修改內容。
- 回寫到原始文件。

我們要做的是，找到不需要依賴 nested callback 的方法。參考第三章

另一種解決方法是使用命名 function。這樣，你可以將金字塔壓扁，它可以簡化呼叫。但是，這種方法沒有解決其他問題，像是確定何時全部過程已完成。所以，你仍然需要非同步的控制處理 module。