# Node and the Local System

Node 努力在任何環境上都能執行，不過有時候會成功，有時候需要一點套件幫助你。

這個章節會比較偏向 OS 層面的功能，最後會介紹兩個套件 ReadLine 跟 ZLib，提供了互動式的指令，跟壓縮能力。

## Exploring the Operating System

作業系統的能會有些不同，不過你只要建立你的 Node 應用程式，就能在任何地方執行，有時候 Node 可以自己處理，有時候需要第三方的套件。

存取 OS core 套件可以直接獲得作業系統的訊息，他能幫我們建立跨平台的應用程式，也提供資源使用量的資訊。

```js
// chap6-1.js
var os = require('os');

console.log('Using end of line' + os.EOL + 'to insert a new line');
console.log('endiannes: ', os.endianness());
console.log('tmpdir: ', os.tmpdir());
console.log('homedir: ', os.homedir());

console.log('freemem', os.freemem());
console.log('loadavg', os.loadavg());
console.log('totalmem', os.totalmem());

console.log('cpus', os.cpus());
```

```shell
$ node chap6-1.js 
Using end of line
to insert a new line
endiannes:  LE
tmpdir:  /var/folders/hx/23x_p0k95d39xk0pflvfs70h0000gn/T
homedir:  /Users/akiicat
freemem 21770240
loadavg [ 2.78369140625, 3.4716796875, 3.42724609375 ]
totalmem 17179869184
cpus [ { model: 'Intel(R) Core(TM) i5-5675R CPU @ 3.10GHz',
    speed: 3100,
    times: 
     { user: 52028290,
       nice: 0,
       sys: 110001070,
       idle: 321641820,
       irq: 0 } },
  { model: 'Intel(R) Core(TM) i5-5675R CPU @ 3.10GHz',
    speed: 3100,
    times: 
     { user: 49576080,
       nice: 0,
       sys: 111086770,
       idle: 323007670,
       irq: 0 } },
  { model: 'Intel(R) Core(TM) i5-5675R CPU @ 3.10GHz',
    speed: 3100,
    times: 
     { user: 50491760,
       nice: 0,
       sys: 112931130,
       idle: 320247630,
       irq: 0 } },
  { model: 'Intel(R) Core(TM) i5-5675R CPU @ 3.10GHz',
    speed: 3100,
    times: 
     { user: 50383360,
       nice: 0,
       sys: 112126890,
       idle: 321160270,
       irq: 0 } } ]
```

確保你能跨平台執行，你可以看`os.EOL` end-of-line 是否支援，然後系統使 big-endian (BE) 或是 little-endian (LE)，暫存資料夾跟家目錄。

`os.loadavg()`使系統的平均工作量，會有三個數字，分別是 1、5、15 分鐘的平均值。

`os.cpus()`會回傳 CPU 的資訊，他會回傳五個 CPU 的參數`user`、`nice`、`sys`、`idle`、`irq` 花了多少微秒。

- `user`：CPU 花在 user space process 的時間。
- `idle`：CPU 閒置的時間。
- `sys`：CPU 花在 system process (kernel) 的時間。
- `nice`：CPU 花在低優先權的 user space process 時間。
- `irq`：CPU 處理硬體 interrupt request 的時間。

> https://en.wiktionary.org/wiki/nice#Verb
>
> niced: To run a process with a specified (usually lower) priority. 降低 process 的優先程度。

CPU 的值不是百分比，但你可以除以五個值的和，再乘以 100，或是使用套件。

## Streams and Pipes

stream 的技術出現在整個 Node core，他給 HTTP 以及其他網路型態提供了 stream 的功能。stream 也提供了File System 的功能。

Stream 物件是一個抽象的介面，意思是你不會直接建立 stream，你必須跟其他的物件一起運作，像是 HTTP request、File System read or write stream、ZLib 壓縮、`process.stdout`，如果你要建立你自己的 stream 的話，才需要實作 stream API。

因為 Node 裡很多物件都有實作 stream 介面。在 Node 裡的所有 stream 有的基本的功能：

- 你可以使用`setEncoding`改變 stream data 的編碼。
- 你可以檢查 stream，不論 stream 是 readable、writable 或是兩者都是。
- 你可以捕捉 stream 事件，像是 data received 或 connection closed。
- 你可以暫停和回復 stream。
- 你可以 pipe 資料，從 readable stream pipe 到 writable stream。

可以同時 reabable 跟 writable 的 stream 又稱作 duplex stream，有很多種 duplex stream，最常見的是`transform`，常用於 input 跟 output 十分相關的時候。

readable stream 一開始都是暫停的，直到開始`stream.read()`或是`stream.resume()`發生的時候，才會有資料進來。然而像是我們使用的 File System readable stream，只要依我們寫道 data event 裡面，stream 就會轉變成 flowing mode。在 flowing mode，只要 stream 可以使用的時候，程式就會存取和送出資料。

```js
// example5-3.js
// create and pipe readable stream
var file = fs.createReadStream(pathname);

file.on("open", function() {
   res.statusCode = 200;
   file.pipe(res);
});
```

readable stream 支援很多事件，但最感興趣的三個事件：data、end、error。當有新的 chunk of data 準備使用時發出 data event。當所有的 data 被消耗的時候會發出 end event。當有錯誤發生的時候會發出 error event。

writable stream 是資料要被寫入的地方，我們所監聽的事件是`error`跟`finish`，當`end()`被呼叫的時候，代表所有資料已經被寫入，就會發出`finish`事件。當嘗試寫入資料的時候回傳`false`，有可能發生另一個事件`drain`。

duplex stream 有同時有 readable stream 和 writable stream。transform stream 是 duplex stream 不過沒有獨立的 internal input 跟 output buffer，transform stream 是將兩個 stream 連接再一起，中間有個 intermediate step。

```
                             Duplex Stream
                          ------------------|
                    Read  <-----               External Source
            You           ------------------|   
                    Write ----->               External Sink
                          ------------------|
            You don't get what you write. It is sent to another source.
```

```
                                 Transform Stream
                           --------------|--------------
            You     Write  ---->                   ---->  Read  You
                           --------------|--------------
            You write something, it is transformed, then you read something.
```

要了解 transform stream 就要看`pipe()` function 有什麼功能。

`pipe`做了什麼才能那到檔案的資料，且輸出到`http.ServerResponse`的物件，在 Node 文件中，這個物件是用 writable stream interface 實作的，然後`fs.createReadStream()`會回傳`fs.ReadStream`，是由 readable stream 所實作的。readable stream 其中的一個方法`pipe()`支援將資料寫到 writable stream。

```js
// chap6-10.js
var zlib = require('zlib');
var fs = require('fs');

var gzip = zlib.createGzip();

var inp = fs.createReadStream('test.png');
var out = fs.createWriteStream('test.png.gz');

console.log(inp.constructor.name); // ReadStream
console.log(out.constructor.name); // WriteStream

inp.pipe(gzip).pipe(out);
```

input 是 readable stream，output 是 writable stream，內容會 pipe 給下一個 stream，但資料會先經過壓縮。這個就是 transform。

## A Formal Introduction to the File System

Node 應用程式有遵守 POSIX 的標準，所以能夠支援任何平台，甚至更新的。

File System module 提供了同步 function 的版本和非同步的版本。哪個比較好並沒有什麼意義，只要他們存在且可以自由地被使用就行。

非同步的將錯誤方在 callback 的第一個參數，同步的會直接拋出錯誤，同步的你必須用`try...catch`接住，非同步的可以直接存取 error 物件。

此外，File System 支援四個 class：

- `fs.FSWatcher`：watch 檔案改變的事件
- `fs.ReadStream`：A readable stream
- `fs.WriteStream`：A writable stream
- `fs.Stats`：`*stat` function 會回傳訊息

### The fs.Stats Class

如果你使用`fs.stat()`、`fs.lstat()`、`fs.fstat()`的話他們會回傳`fs.Stats`物件，它可以用來檢查檔案或目錄是否存在，他還會回傳像是檔案的訊息：UNIX domain socket、檔案權限、物件最後修改的時間...等。Node 提供了一些存取訊息的 functions，像是`fs.isFile()`和`fs.isDirectory()`來決定這個物件是檔案還是目錄，你也可以直接存取 data：

```js
// chap6-2.js
var fs = require('fs');
var util = require('util');

fs.stat('./phoenix5a.png', function(err,stats) {
   if (err) return console.log(err);
   console.log(util.inspect(stats));
});
```

```shell
$ node chap6-2.js 
{ dev: 16777218,
  mode: 33188,
  nlink: 1,
  uid: 501,
  gid: 20,
  rdev: 0,
  blksize: 4096,
  ino: 20102296,
  size: 219840,
  blocks: 432,
  atime: 2018-06-26T14:58:50.000Z,
  mtime: 2018-06-19T21:55:47.000Z,
  ctime: 2018-06-19T21:55:47.000Z,
  birthtime: 2018-06-19T21:55:47.000Z }
```

這只是顯示基本的 POSIX `stat()` function，裡面的資料非常直觀，但是有些值容易混淆。

- `size`：單位 bytes
- `blksize`：在作業系統裡 block 的大小
- `blocks`：blocks 的數量
- `mode`：這個值包含了物件的權限

要看 mode 的權限，需要一些套件幫忙，stat-mode 套件可以幫助我們查看從`fs.stat()` function 獲得 stat 物件的值，且允許我們直接 query 值：

```js
// example6-1.js
var fs = require('fs');
var Mode = require('stat-mode');

fs.stat('./phoenix5a.png', function(err,stats) {
   if (err) return console.log(err);
   
   // get permissions
   var mode = new Mode(stats);

   console.log(mode.toString());
   console.log('Group execute ' + mode.group.execute);
   console.log('Others write ' + mode.others.write);
   console.log('Owner read ' + mode.owner.read);
});
```

```js
$ node example6-1.js 
-rw-r--r--
Group execute false
Others write false
Owner read true
```

### THe File System Watcher

很常看到應用程式需要 watch 一個檔案或資料夾，當有所改變的時候就執行某些任務。在 Node 裡有`fs.FSWatcher`這個介面可以使用，不幸的是，他不支援跨平台。

我們會忽略`fs.watch()`物件，然後使用第三方套件 [chokidar](https://github.com/paulmillr/chokidar)。

```shell
npm install chokidar
```

我們添加 watcher 到當前的目錄，他會檢查目錄的變動，包括檔案，他會執行 recursive watch：

```js
// chap6-3.js
var chokidar = require('chokidar');

var watcher = chokidar.watch('.', {
  ignored: /[\/\\]\./,
  persistent: true
});

var log = console.log.bind(console);

watcher
  .on('add', function(path) { log('File', path, 'has been added'); })
  .on('unlink', function(path) { log('File', path, 'has been removed'); })
  .on('addDir', function(path) { log('Directory', path, 'has been added'); })
  .on('unlinkDir', function(path) { log('Directory', path, 'has been removed'); })
  .on('error', function(error) { log('Error happened', error); })
  .on('ready', function() { log('Initial scan complete. Ready for changes.'); })
  .on('raw', function(event, path, details) { log('Raw event info:', event, path, details); });

watcher.on('change', function(path, stats) {
  if (stats) log('File', path, 'changed size to', stats.size);
});
```

### File Read and Write

有兩個方法可以讀取或寫入檔案，使用 non-streamd 的功能。

第一個 read/write 的方法是使用簡單的`fs.readFile()`或`fs.writeFile()`。這些 function 會開檔、讀或寫檔、然後關檔。

接下來的例子，會先開檔把內容寫入檔案，一旦寫入完成，檔案會再被讀檔開啟，然後印出內容：

```js
// chap6-4.js
var fs = require('fs');

fs.writeFile('./some.txt','Writing to a file',function(err) {
      if (err) return console.error(err);
      fs.readFile('./some.txt','utf-8', function(data,err) {
         if (err) return console.error(err);
         console.log(data);
      });
});
```

因為 input 跟 output 都是使用 buffer，所以可以將 buffer 轉成 string。

第二個 read/write 的方法是開啟一個檔案指派給 file descriptor (`fd`)，然後使用 filedescriptor 寫入然後讀取檔。

```js
// chap6-5.js
"use strict";

var fs = require('fs');

fs.open('./new.txt','a+',0x666, function(err, fd) {
     if (err) return console.error(err);
     fs.write(fd, 'First line', 'utf-8', function(err,written, str) {
        if (err) return console.error(err);
        var buf = new Buffer(written);
        fs.read(fd, buf, 0, written, 0, function (err, bytes, buffer) {
           if (err) return console.error(err);
           console.log(buf.toString('utf8'));
        });
      });
});
```

在 callback 裡會回傳 file descriptor，然後在`fs.write()`裡使用 file descriptor，因為是使用 append mode 打開檔案的，所以資料會寫在最後的地方。

`written`會回傳 bytes 的數量。

### Directory Access and Maintenance

`unlink``.gz`的檔案：

```js
// chap6-6.js
'use strict';

var fs = require('fs');
var path = require('path');

fs.readdir ('./',function(err, files) {
   for (let file of files) {
      console.log(file);
      if (path.extname(file) == '.gz') {
         fs.unlink('./' + file);
      }
   }
});
```

### File Streams

你可以使用`fs.createReadStream()`建立 readable stream，然後參數可以傳入路徑和`options`物件，或是 file description 然後把路徑設為`null`。在`fs.createWriteStream()`有相同的用法，[預設 readable stream 的 options](https://nodejs.org/api/fs.html#fs_fs_createreadstream_path_options)：

- `fd`：如果你想要使用 file descriptor 你可在 options 傳入。
- `autoClose`：一但檔案讀取結束時會自動關檔。
- `start``end`：如果你只想要讀取檔案的某個部份，可以設定開始和結尾，單位是 bytes。
- `encoding`：編碼，你也可以晚點使用`setEncoding()`修改

[預設 writable stream 的 options](https://nodejs.org/api/fs.html#fs_fs_createwritestream_path_options)：

- `fd`
- `defaultEncoding`：預設`utf8`
- `start`：想要寫入黨案的起始位置

```js
// example6-2.js
var fs = require('fs');

fs.open('./working.txt', 'r+',function (err, fd) {
   if (err) return console.error(err);

   var writable = fs.createWriteStream(null,{fd: fd, start: 10,
                                        defaultEncoding: 'utf8'});

   writable.write(' inserting this text ');
});
```

```js
// example6-3.js
var fs = require('fs');

var readable =
  fs.createReadStream('./working.txt').setEncoding('utf8');

var data = '';
readable.on('data', function(chunk) {
   data += chunk;
});

readable.on('end', function() {
   console.log(data);
});
```

```shell
$ node example6-3.js 
Now let's pull this all together, and read and write with a stream.

$ node example6-2.js 

$ node example6-3.js 
Now let's  inserting this text r, and read and write with a stream.
```

如果我們只有要開檔然後 pipe 結果到另一個檔案會省下很多時間，但是我們沒辦法修改 midstream 的結果，因為他不是 duplex stream 或 transform stream。但是你可以複製一個內容到另外一個檔案：

```js
// chap6-7.js
var fs = require('fs');

var readable = fs.createReadStream('./working.txt');
var writable = fs.createWriteStream('./working2.txt');

readable.pipe(writable);
```

## Resource Access with Path

你不必在 Linux 寫一個版本，然後又在 Window 寫另外一個版本，file system 全部都幫你處理好路徑的問題了。

```js
// chap6-8.js
'use strict';

var fs = require('fs');
var path = require('path');

fs.readdir ('./',function(err, files) {
   for (let file of files) {
      let ext = path.extname(file);
      let base = path.basename(file, ext);
      console.log ('file ' + base + ' with extension of ' + ext);
   }
});
```

`path.basename()`第二個參數如果沒有填的話就會回傳整個 file，他會把 ext 的值給濾掉。

```js
// chap6-9.js
var path = require('path');

console.log(process.env.PATH);
console.log(process.env.PATH.split(path.delimiter));
```

```shell
$ node chap6-9.js 
/anaconda3/bin:/Users/akiicat/.pyenv/shims:/Users/akiicat/.rvm/gems/ruby-2.4.3/bin:/Users/akiicat/.rvm/gems/ruby-2.4.3@global/bin:/Users/akiicat/.rvm/rubies/ruby-2.4.3/bin:/usr/local/opt/llvm/bin:/usr/local/opt/libxml2/bin:/Users/akiicat/google-cloud-sdk/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:/opt/X11/bin:/Applications/Wireshark.app/Contents/MacOS:/Users/akiicat/.rvm/bin:/Users/akiicat/.rvm/bin
[ '/anaconda3/bin',
  '/Users/akiicat/.pyenv/shims',
  '/Users/akiicat/.rvm/gems/ruby-2.4.3/bin',
  '/Users/akiicat/.rvm/gems/ruby-2.4.3@global/bin',
  '/Users/akiicat/.rvm/rubies/ruby-2.4.3/bin',
  '/usr/local/opt/llvm/bin',
  '/usr/local/opt/libxml2/bin',
  '/Users/akiicat/google-cloud-sdk/bin',
  '/usr/local/bin',
  '/usr/bin',
  '/bin',
  '/usr/sbin',
  '/sbin',
  '/opt/X11/bin',
  '/Applications/Wireshark.app/Contents/MacOS',
  '/Users/akiicat/.rvm/bin',
  '/Users/akiicat/.rvm/bin' ]
```

`path.delimeter`在 Linux 裡是冒號`:`在 Window 是分號`;`，如果你想要在兩個作業系統上都能運行的話，使用`path.delimeter`：

像是在不同系統會用不同的斜線 forward slash `/` 或 backslash `\`。在第五章有使用`path.normalize()`的方法：

```js
// example5-4.js
pathname = path.normalize(base + req.url);
```

路徑的轉換不能使用 String 物件或 RegExp，Path module 會轉換檔案系統的路徑。

如果你想要分析檔案路徑的話，可以使用`path.parse()`，`require.main.filename`跟`__filename`可以拿到當前執行的檔案：

```js
// parse.js
var path = require('path');

console.log(require.main.filename);
console.log(__filename);
console.log(path.parse(__filename));
```

```shell
$ node parse.js 
/Users/akiicat/Github/LearningNode2/chap6/parse.js
/Users/akiicat/Github/LearningNode2/chap6/parse.js
{ root: '/',
  dir: '/Users/akiicat/Github/LearningNode2/chap6',
  base: 'parse.js',
  ext: '.js',
  name: 'parse' }
```

## Creating a Command-Line Utility

可以不需要使用`node`指令執行。

我們會使用 ImageMagick 將現有的圖片加上一點 Ploaroid effect，然後存到新的檔案。

```js
// example6-4.js
#!/usr/bin/env node

var spawn = require('child_process').spawn;
var program = require('commander');

program
  .version ('0.0.1')
  .option ('-s, --source [file name]', 'Source graphic file name')
  .option ('-f, --file [file name]', 'Resulting file name')
  .parse(process.argv);

if ((program.source === undefined) || (program.file === undefined)) {
  console.error('source and file must be provided');
  process.exit();
}

var photo = program.source;
var file = program.file;

// conversion array
var opts = [
photo,
"-bordercolor", "snow",
"-border", "20",
"-background","gray60",
"-background", "none",
"-rotate", "6",
"-background", "black",
"(", "+clone", "-shadow", "60x8+8+8", ")",
"+swap",
"-background", "none",
"-thumbnail", "240x240",
"-flatten",
file];

var im = spawn('convert', opts);

im.stderr.on('data', (data) => {
  console.log(`stderr: ${data}`);
});

im.on('close', (code) => {
  console.log(`child process exited with code ${code}`);
});
```

為了轉換成 command-line utility，要將這行放在最前面：

```js
#!/usr/bin/env node
```

`#!`符號這個又稱做 shebang，他會用來執行這個程式，即使他最後不是`.js`結尾也能加上 chmod 執行：

```shell
cp example6-4.js polaroid
chmod a+x polaroid
./polaroid -h
./polaroid -s phoenix5a.png -f phoenix5apolaroid.png
```

utility 在 Windows 沒辦法使用。

用 NW.js 建立獨立的應用程式，你可以把你的應用程式用 NW.js 包裝起來，讓你在任何地方都能執行。

## Compression/Decompression with ZLib

ZLib 套件提供加縮和解壓縮的功能，他是使用 transform stream，壓縮檔案簡單的範例：

```js
// chap6-10.js
var zlib = require('zlib');
var fs = require('fs');

var gzip = zlib.createGzip();

var inp = fs.createReadStream('test.png');
var out = fs.createWriteStream('test.png.gz');

console.log(inp.constructor.name); // ReadStream
console.log(out.constructor.name); // WriteStream

inp.pipe(gzip).pipe(out);
```

input stream 會直接連到中間的 gzip output，然後壓縮檔案。ZLib 提供了`zlib`壓縮或`deflate`，`defalte`更加複雜更能控制的壓縮演算法，不像`zlib`可以使用 gunzip 或 unzip 指令解壓縮檔案，你必須使用 Node 或是其他的功能來解壓縮`deflate`。

```js
// chap6-11.js
var zlib = require('zlib');
var program = require('commander');
var fs = require('fs');

program
   .version ('0.0.1')
   .option ('-s, --source [file name]', 'Source File Name')
   .option ('-f, --file [file name]', 'Destination File name')
   .option ('-t, --type <mode>', /^(gzip|deflate)$/i)
   .parse(process.argv);

var compress;
if (program.type == 'deflate')
   compress = zlib.createDeflate();
else
   compress = zlib.createGzip();

var inp = fs.createReadStream(program.source);
var out = fs.createWriteStream(program.file);

inp.pipe(compress).pipe(out);
```

接下來的範例，要如何傳送一個檔案到 Server 然後解壓縮，會從第五章的範例做修改，讓他可以壓縮 PNG 檔，然後透過 HTTP request 傳送，然後 server 會解壓縮資料然後儲存起來。

```js
// example6-5.js
var http = require('http');
var zlib = require('zlib');
var fs = require('fs');

var server = http.createServer().listen(8124);

server.on('request', function(request,response) {
   if (request.method == 'POST') {
      var chunks = [];

      request.on('data', function(chunk) {
         chunks.push(chunk);
      });

      request.on('end', function() {
         var buf = Buffer.concat(chunks);
         zlib.unzip(buf, function(err, result) {
            if (err) {
               response.writeHead(500);
               response.end();
               return console.log('error ' + err);
            }
            var timestamp = Date.now();
            var filename = './done' + timestamp + '.png';
            fs.createWriteStream(filename).write(result);
         });

         response.writeHead(200, {'Content-Type': 'text/plain'});
         response.end('Received and undecompressed file\n');
      });
   }
});

console.log('server listening on 8214');
```

首先`chunks`會收到資料，然後我們使用`buffer.concat()`建立 Buffer，因為我們正在處理 Buffer 而不是 stream，所以我們沒有辦法使用`pipe()`，然後使用`zlib.unzip`傳入 Buffer 和 callback function，callback function 有 error 和 result 作為參數，result 同樣也是 Buffer，使用`write()`寫入 Buffer。為了要確保檔名不會重複，使用 timestamp 命名。

```js
// example6-6.js
var http = require('http');
var fs = require('fs');
var zlib = require('zlib');

var gzip = zlib.createGzip();

var options = {
  hostname: 'localhost',
  port: 8124,
  method: 'POST',
  headers: {
    'Content-Type': 'application/javascript',
    'Content-Encoding': 'gzip,deflate'
  }
};

var req = http.request(options, function(res) {
  res.setEncoding('utf8');
  var data = '';
  res.on('data', function (chunk) {
      data+=chunk;
  });


  res.on('end', function() {
    console.log(data)
  })

});

req.on('error', function(e) {
  console.log('problem with request: ' + e.message);
});

// stream gzipped file to server
var readable = fs.createReadStream('./test.png');
readable.pipe(gzip).pipe(req);
```

為了確保 header 有正確的`Content-Encoding`，他的值應該要是`gzip.deflate`，然後`Content-Type`也同樣改成`applicatioin/javascript`。

client 可以使用`pipe()`將壓縮後的資料寫入 writable stream，但是不能在 server 使用，因為他被轉換成 buffer chunks。

在記憶體 Buffer 檔案可能會造成 scaling 的問題，所以另一個作法使將它解壓縮然後存起來，刪除暫存的解壓縮完的檔案。

## Pipes and ReadLine

任何你所輸入的會直接輸出：

```js
process.stdin.resume();
process.stdin.pipe(process.stdout);
```

如果你想要繼續保持 stream 是開著的話：

- `end`：boolean，當 reader 結束的時候是否要同時結束 writer，預設`true`，[文件](https://nodejs.org/api/stream.html#stream_readable_pipe_destination_options)

```js
process.stdin.pipe(process.stdout);
```

REPL 會逐行的執行，很少 Readline 會沒有盡頭。你可以 include Readline 套件：

```js
var readline = require('readline');
```

注意一旦你引用建立了 interface，Node 的程式就不會結束，除非你關閉它。

```js
// example6-7.js
var readline = require('readline');

// create a new interface
var rl = readline.createInterface(process.stdin, process.stdout);

// ask question
rl.question(">>What is the meaning of life?  ", function(answer) {
   console.log("About the meaning of life, you said " + answer);
   rl.setPrompt(">> ");
   rl.prompt();
});

// function to close interface
function closeInterface() {
   rl.close();
   console.log('Leaving Readline');
}

// listen for .leave
rl.on('line', function(cmd) {
   if (cmd.trim() == '.leave') {
      closeInterface();
      return;
   }
   console.log("repeating command: " + cmd);
   rl.prompt();
});

rl.on('close', function() {
    closeInterface();
});
```

```shell
$ node example6-7.js 
>>What is the meaning of life?  ===
About the meaning of life, you said ===
>> ===
repeating command: ===
>> .leave
Leaving Readline
Leaving Readline
```

這個在第四章的`rlwrap`很相似，我們可以使用`rlwrap`覆蓋 REPL command-line 的設定：

```shell
env NODE_NO_READLINE=1 rlwrap node
```

在 command-line 程序裡，他正指示著不要使用 Node Readline 套件，而是使用`rlwrap`。

