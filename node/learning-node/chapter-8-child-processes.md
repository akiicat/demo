# Child Processes

作業系統提供了很棒的功能，但是大多數都要透過指令來執行，如果可以透過 Node 來存取這些功能的話會很棒，這就是為什麼 child processes 會出現：

Node 允許我們在 child process 執行系統的指令，然後聽指令 input/output，這能夠在指令傳入參數，甚至可以從另一個指令 pipe 後的結果傳入另一個指令。

這些指令只能在 Unix 環境，Window 不行。

## child_process.spawn

有四種方法可以建立 child process，最常使用的就是`spawn`，會在新的 process 傳入任何的參數，然後發出一個指令，pipe 會在應用程式跟 child process 建立`stdin`、`stdout`、`stderr`。

我們用 child process 呼叫 Unix `pwd` 指令，印出當前的目錄：

```js
// chap8-1.js
var spawn = require('child_process').spawn,
    pwd = spawn('pwd');

pwd.stdout.on('data', function (data) {
  console.log('stdout: ' + data);
});

pwd.stderr.on('data', function (data) {
  console.error('stderr: ' + data);
});

pwd.on('close', function (code) {
  console.log('child process exited with code ' + code);
});
```

```shell
$ node chap8-1.js 
stdout: /Users/akiicat/Github/LearningNode2/chap8

child process exited with code 0
```

任何從 child process 輸出的資料會觸發`stdout`的`data`事件。如果要測試錯誤的例子：

```js
var spawn = require('child_process').spawn,
    pwd = spawn('pwd', ['-g']);
```

```shell
$ node chap8-1.js 
stderr: pwd: illegal option -- g
usage: pwd [-L | -P]

child process exited with code 1
```

剛剛 demo 了`stdout`和`stderr`，那`stdin`呢。Node 文件有包含`stdin`的範例，他會模擬 Unix pipe 的指令：

```shell
find . -ls | grep test
```

```shell
$ find . -ls | grep test
20102338        8 -rw-r--r--    1 akiicat          staff                   5  6 20 05:55 ./sub/test.txt
```

`find`的指令傳入兩個參數，`grep`傳入一個參數，還有傳入的`stdin`，`grep`的`stdout`會設定`setEncoding`，最後會把結果印出來：

```js
// example8-1.js
var spawn = require('child_process').spawn,
    find = spawn('find',['.','-ls']),
    grep = spawn('grep',['test']);

grep.stdout.setEncoding('utf8');

find.stdout.pipe(grep.stdin);

// now run grep and output results
grep.stdout.on('data', function (data) {
  console.log(data);
});

// error handling for both
find.stderr.on('data', function (data) {
  console.log('grep stderr: ' + data);
});
grep.stderr.on('data', function (data) {
  console.log('grep stderr: ' + data);
});

// and exit handling for both
find.on('close', function (code) {
  if (code !== 0) {
    console.log('find process exited with code ' + code);
  }

});

grep.on('exit', function (code) {
  if (code !== 0) {
    console.log('grep process exited with code ' + code);
  }
});
```

```shell
$ node example8-1.js 
20102338        8 -rw-r--r--    1 akiicat          staff                   5  6 20 05:55 ./sub/test.txt
```

有些環境可能會出現警告，因為資料並沒有馬上執行，有些 child process 在執行前會先 buffer 在 block。

在這個例子裡，從 find process 的輸出是有限的，所以 input 的 grep process 不能超過 block size，通常是 4096，但可能會因環境或設定而有所不同。

更多 buffer： 

[How to fix stdio buffering](https://www.perkin.org.uk/posts/how-to-fix-stdio-buffering.html)

[Buffering in standard streams](http://www.pixelbeat.org/programming/stdio_buffering/)

我們可以使用`--line-buffered`關掉 grep line buffering：

```js
// chap8-2.js
var spawn = require('child_process').spawn,
    ps    = spawn('ps', ['ax'], { shell: true }),
    grep  = spawn('grep', ['--line-buffered', 'apache2']);

ps.stdout.pipe(grep.stdin);

ps.stderr.on('data', function (data) {
  console.error('ps stderr: ' + data);
});

ps.on('close', function (code) {
  if (code !== 0) {
    console.log('ps process exited with code ' + code);
  }
});

grep.stdout.on('data', function (data) {
  console.log('' + data);
});


grep.stderr.on('data', function (data) {
  console.log('grep stderr: ' + data);
});

grep.on('close', function (code) {
  if (code !== 0) {
    console.log('grep process exited with code ' + code);
  }
});
```

如果你 Node 的版本高於 5.7.0 的話，要在 ps 後面加上`shell`的選項，這樣 child process 才會為你的 process 產生 shell。

他有其他選項：

- `cwd`：change working directory
- `env`：環境變數 key/value pair 的 array
- `detached`：child 的會跟 parent 跑在不同的環境
- `stdio`：child `stdio` 選項的 array

`child-process.spawnSync()`是同步的 spawn function。

### child_process.exec and child_process.execFile

此外，想要產生 child process，你還可以使用`child_process.exec()`和`child_process.execFile()`。

`child_process.exec()`跟`child_process.spawn()`很相似，`spawn()`只要當成是執行的時候他就會立刻回傳結果的 stream，在 example8-1 的範例裡。`child_process.exec()`跟`child_process.execFile()`會將結果 buffer 住。`exec()`會產生一個 shell 去執行程式，不像`child_process.execFile()`會直接執行。這讓`child_process.execFile()`比較有效率，比起在 shell 上跑`child_process.spawn()`或`child_process.exec()`。

|                     | spawn | exec | execFile |
| ------------------- | ----- | ---- | -------- |
| 立刻回傳結果 stream | v     |      |          |
| 結果會 buffer 住    |       | v    | v        |
| 產生 shell 執行     | v     | v    |          |
| 直接本地執行        |       |      | v        |



第一個參數：`exec()`是指令，`execFile()`是根據他當前位置執行的檔案。第二個參數：是指令的選項。第三個參數：callback function，callback function 會有三個參數`error`、`stdout`、`stderr`，如果沒有出錯的話，`stdout`會 buffer 住資料。

```js
// app
#!/usr/local/bin/node

console.log(global);
```

```js
// chap8-4.js
var execfile = require('child_process').execFile,
    child;

child = execfile('./app', function(error, stdout, stderr) {
  if (error == null) {
    console.log('stdout: ' + stdout);
  }
});
```

```js
// chap8-5.js
var exec = require('child_process').exec,
    child;

child = exec('./app', function(error, stdout, stderr) {
   if (error) return console.error(error);
   console.log('stdout: ' + stdout);
});
```

```shell
$ node chap8-4.js
stdout: { ... }

$ node chap8-5.js 
stdout: { ... }
```

`child_process.exec()`有三個參數，第一個是指令，第二個是`options`物件，第三個是 callback。`opeions`包含了`encoding`、`uid` (user id)、`gid` (group identity)。

用第六章的例子 example6-4 用指令複製 PNG 檔案並加上 polaroid 特效：

```shell
$ tree 
├── chap8-6.js
├── chap8-7.js
├── phoenix5a.png
└── snaps
    ├── phoenix5a.png
    └── polaroid
```

```js
// polaroid
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

使用`exec()`執行指令的字串，`cwd`會改變當前執行這個檔案的位置：

```js
// chap8-6.js
var exec = require('child_process').exec,
    child;

child = exec('./polaroid -s phoenix5a.png -f phoenixpolaroid.png', 
         {cwd: 'snaps'}, function(error, stdout, stderr) {
                if (error) return console.error(error);
                console.log('stdout: ' + stdout);
});
```

使用`execFile()`執行 array 的指令：

```js
// chap8-7.js
var execfile = require('child_process').execFile,
    child;

child = execfile('./polaroid',
                 ['-s', 'phoenix5a.png', '-f', 'phoenixpolaroid.png'],
                  {cwd: 'snaps'}, function(error, stdout, stderr) {
   if (error) return console.error(error);
   console.log('stdout: ' + stdout);
});
```

```shell
$ node chap8-6.js 
stdout: child process exited with code 0


$ node chap8-7.js 
stdout: child process exited with code 0
```

因為`child_process.execFile()`不會開一個新的 shell 執行，他可能在某些環境下會無法執行，Node 文件說明你不能直接使用 I/O 和 file globbing (`*`通用符號 eg. `*.jpg`)，然而如果你想要運行互動式的 child process 的話，`execFile()`會比`exec()`來得好：

```js
// chap8-9.js
'use strict';
const cp = require('child_process');
const child = cp.execFile('node', ['-i'], (err, stdout, stderr) => {
  console.log(stdout);
});

child.stdin.write('process.versions;\n');
child.stdin.end();
```

上面會建立一個互動式的 Node，然後印出 process version 後結束。（not work 6.9.1)

有同步的版本：`child_process.execSync()`和`child_process.execFileSync()`兩個 function。

### child_process.fork

最後的 child process 的方法是`child_process.fork()`，這`spawn()`的變種是為了產生 Node process。

讓`child_process.fork()`的 process 跟其他的 process 有所不同的是，在 child process 上會建立一個溝通的通道。每一個 process 都需要新的 V8 實利，V8 實利同時有時間跟記憶體。

`child_process.fork()`其中一個的使用方法是完全拆分原本 Node 實利的功能。假設你有個 Node 的 server，你想要增加你的效能，整合第二台 Node 來幫你回答 server 的請求，像是 TCP server 的範例。同樣也可以使用平行的方法達成。

使用一個 TCP server 將資料透過 socket 傳到兩個不同 child 的 HTTP server。

跟要 demo Node 對於 master/child parallel TCP server，很相似。在 master 會建立 HTTP server 然後使用`child_process.send()`的 function 將 server 送到 child process 上。

```js
// chap8-9.js
var cp = require('child_process'),
    cp1 = cp.fork('child.js'),
    http = require('http');

var server = http.createServer();

server.on('request', function (req, res) {
   res.writeHead(200, {'Content-Type': 'text/plain'});
   res.end('handled by parent\n');
});

server.on('listening', function () {
    cp1.send('server', server);
});

server.listen(3000);
```

child process 透過`process`物件收到 HTTP server 的訊息。他會監聽 connection 事件，當在 child HTTP server 收到訊息觸發 connection 事件的時候，會傳入 socket 形成 connection 的末端。

```js
// child.js
var http = require('http');

var server = http.createServer(function (req, res) {
   res.writeHead(200, {'Content-Type': 'text/plain'});
   res.end('handled by child\n');
});

process.on('message', function (msg, httpServer) {
   if (msg === 'server') {
      httpServer.on('connection', function (socket) {
          server.emit('connection', socket);
      });
   }
});
```

他會開一個 parent HTTP server 跟一個 child HTTP server，有時候 child server 會做事有時候不會，process 數量會有兩個。

```shell
$ ps aux | grep node
akiicat  6156 0.0 0.1  3031948  21480 s010  S+ 5:29下午  0:00.08 /usr/local/bin/node child.js
akiicat  6155 0.0 0.1  3031944  21004 s010  S+ 5:29下午  0:00.10 node chap8-9.js
```

Node Cluster 套件是基於`child_process.fork()`，還有一些其他的功能。

