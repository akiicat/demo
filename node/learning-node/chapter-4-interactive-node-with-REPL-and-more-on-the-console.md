# Interactive Node with REPL and More on the Console

REPL (read-eval-print-loop) 支援指令輸入，大多你在 REPL 執行的跟你在 Javascript 檔案內執行的沒什麼不同。REPL 是 Node 開發工具。

## REPL: First Lookds and Undefined Expressions

先輸入`node`，不需要任何其他參數：

```shell
node
```

然後 REPL 提供了預設的指令提示`>`，任何你輸入的東西，都會被 V8 Javascript engine 所執行。

```js
> a = 2;
2
```

使用`_`存取最後一行的 expression：

```js
> a = 2;
2
> ++_;
3
> ++_;
4
```

你在裡面使用`var`宣告變數的話，他會回傳`undefined`

```js
> var a = 2;
undefined
```

因為`var`指派變數並不會回傳任何東西：

```js
console.log(eval('a = 2'));
console.log(eval('var a = 2'));
```

## Benefits of REPL: Getting a Closer Understanding of JavaScript Under the Hood

```js
> 3 > 2 > 1;
false
```

```js
> 3 > 2;
true
> true > 1;
false
```

## Multiline and More Complex Javascript

你在 REPL 輸入的就像你在檔案裡面輸入的一樣，包括`require`要匯入套件：

```js
$ node
> qs = require('querystring');
{ unescapeBuffer: [Function],
  unescape: [Function: qsUnescape],
  escape: [Function],
  encode: [Function],
  stringify: [Function],
  decode: [Function],
  parse: [Function] }
> val = qs.parse('file=main&file=secondary&test=one').file;
[ 'main', 'secondary' ]
```

因為我們沒有使用`var`，所以結果有被印出來。

Node REPL 裡也支援多行輸入，只要在`{``(`括號裡面就會變成多行：

```js
> var test = function (x, y) {
... var val = x * y;
... return val;
... };
undefined
> test(3,4);
12
```

`...`代表 expression 尚未完成。

越多`...`代表越內層：

```js
> var test = function (x, y) {
... var test2 = function (x, y) {
..... return x * y;
..... }
... return test2(x,y);
... }
undefined
> test(3,4);
12
```

不過多行在用上下鍵的時後匯有點麻煩，他一次只會顯示多行內的其中一行。

Tab 可以自動完成輸入全域或區域變數

| Keyboard entry | What it does                                                 |
| -------------- | ------------------------------------------------------------ |
| Ctrl-C         | Terminates current command. Pressing Ctrl-C twice forces an exit. |
| Ctrl-D         | Exits REPL.                                                  |
| Tab            | Autocompletes global or local variable.                      |
| Up arrow       | Traverses up through command history.                        |
| Down arrow     | Traverses down through command history.                      |
| Underscore (_) | References result of last expression.                        |

如果你擔心在 REPL 上編輯完沒辦法儲存的話，可以使用`.save`儲存你輸入的結果。

### REPL Commands

REPL 有一些簡單好用的指令，像是`.save`指令可以儲存你現在 REPL 的文字檔，除非你使用`.clear`指令清空：

```js
> .save ./path/save.js
```

只有你輸入的才會儲存，輸出結果不會存。

- `.break`：跳出多行文字，不過也會失去所有內容。
- `.clear`：重置內文並清除任何多行文字。這個指令基本上可以讓你重新開始 REPL。
- `.exit`：離開
- `.help`：help
- `.save`：儲存成檔案
- `.load`：載入檔案`.load /path/to/file.js`

如果你是用 REPL 在撰寫的話，記得要時常儲存`.save`，然後從儲存的程式碼中，挑你要的，然後重新建立一份新的程式碼。

### REPL and rlwarp

`rlwrap`他是 GNU `readline` library 功能包裝，它可以讓 command line 更加得彈性輸入，也提供其他額外的功能，像是能更好的編輯，或是永久的指令紀錄。

在使用之前必須先安裝：

```shell
brew install rlwrap
```

這裡快速的 demo `rlwrap` 跟 REPL 要如何搭配使用，讓`>`符號是紫色的：

```shell
NODE_NO_READLINE=1 rlwrap -ppurple node
```

如果常用的話，可以把這行加入 bashrc：

```shell
alias node="NODE_NO_READLINE=1 rlwrap -ppurple node"
```

改變箭頭符號跟顏色：

```shell
NODE_NO_READLINE=1 rlwrap -ppurple -S "::> " node
```

再離開 node 之後，重新輸入的話可以拿到上一次輸入的東西。

最後`rlwrap`可以幫我們解決`var`顯示 `undefined`，我們可以自己調整，客製化我們的 REPL。

### Custom REPL

Node 提供介面上我們可以建立客製化的 REPL，首先要引用 REPL module：

```js
var repl = require("repl");
```

在`repl`上使用`start`：

```js
repl.start(options);
```

`options`是可以輸入的參數：

- `prompt`：預設是`>`
- `input`：預設是`process.stdin`
- `output`：預設是`process.stdout`
- `eval`：預設是`async` 包裝 eval
- `useGlobal`：預設`false` to start a new context rather than use the global object
- `useColors`：Whether writer function should use colors. Defaults to REPL’s terminal value
- `ignoreUndefined`：忽略`undefined`回應，預設是`false`。
- `terminal`：Set to true if stream should be treated like a tty (terminal), including support for ANSI/VT100 escape codes.
- `writer`：每行指令回傳的格式，預設是`util.inspect`
- `replMode`：REPL 使用 strict mode、default、hybrid 解析 expression
  - `repl.REPL_MODE_SLOPPY`：使用 default mode
  - `repl.REPL_MODE_STRICT`：使用 strict mode
  - `repl.REPL_MODE_MAGIC`：先使用 default 解析 expression，出錯了再用 strict mode。

會忽略回傳是`undefined`，提示符號是`my repl> `，然後使用 strict mode：

```js
// repl.js
var repl = require('repl');

repl.start( {
  prompt: 'my repl> ',
  replMode: repl.REPL_MODE_STRICT,
  ignoreUndefined: true,
});
```

```shell
$ node repl.js 
my repl> let ct = 0;
my repl> ct++;
0
my repl> console.log(ct);
1
my repl> ++ct;
2
my repl> console.log(ct);
2
```

`eval` callback 大概的格式要像是這樣，`writer`會將所有值變大寫。

```js
var repl = require('repl');

repl.start({prompt: '>', eval: myEval, writer: myWriter});

function myEval(cmd, context, filename, callback) {
  callback(null,cmd);
}

function myWriter(output) {
  return output.toUpperCase();
}
```

`input`和`output`可以有很多種的變化，REPL 可以同時跑在 standard input 以及 socket，REPL 的文件提供了這個程式碼，可以同時聽到 unix socket 跟 TCP socket：

```js
var net = require("net"),
    repl = require("repl");

connections = 0;

repl.start({
  prompt: "node via stdin> ",
  input: process.stdin,
  output: process.stdout
})

net.createServer(function (socket) {
  connections += 1;
  repl.start({
    prompt: "node via Unix socker> ",
    input: socket,
    output: socket
  }).on('exit', function() {
    socker.end();
  })
}).listen("/tmp/node-repl-sock");

net.createServer(function (socket) {
  connections += 1;
  repl.start({
    prompt: "node via TCP socker> ",
    input: socket,
    output: socket
  }).on('exit', function() {
    socket.end();
  });
}).listen(5001);
```

你可以透過 TCP socket、Unix socket、standar input 跑 REPL，每個 repl 是獨立的：

```shell
$ node socker.js 
node via stdin> 
```

```shell
# mac
$ telnet -u /tmp/node-repl-sock
Trying /tmp/node-repl-sock...
Connected to (null).
Escape character is '^]'.
node via Unix socker> var a = 3;
undefined
node via Unix socker> a
3
```

```shell
# mac
$ nc localhost 5001
node via TCP socker> var a = 3;
undefined
node via TCP socker> a
3
```

有一個比較實用的 REPL，在開啟 REPL 後預先載入套件：

```js
// example4-1.js
var repl = require('repl');
var context = repl.start({prompt: '>> ',
                          ignoreUndefined: true,
                          replMode: repl.REPL_MODE_STRICT}).context;

// preload in modules
context.request = require('request');
context.underscore = require('underscore');
context.q = require('q');
```

執行之後就可以直接使用套件：

```shell
$ node example4-1.js
>> request
>> q
>> underscore
```

如果你想要像應用程式一樣執行，可以在第一行加上：

```js
#!/usr/local/bin/node
```

```shell
chmod u+x replcontext.js
./replcontext.js
```

## Stuff Happens - Save Often

Node REPL 有互動式的工具，可以讓你開發更簡單。REPL 不只讓我們能先引入檔案，還可以互動的建立應用程式，最後儲存結果。

另一個實用的 REPL 是可以客製化 REPL，可以消除`undefined`回應，預載套件，改變提示符號，`eval`的規則，可以用的還有更多。

`rlwrap`可以儲存到中斷連線前的指令，或是在不同視窗的，這可能會讓你主要節省時間的東西。

## The Necessity of the Console

接下來有幾個範例，大多時候適合用`console.log()`印出訊息：

### Console Message Types, Console Class, and Blocking

`console.log()`的訊息會`stdout`輸出。

`console.info()`等同於`console.log()`，都是`stdout`輸出。

`console.error()`則不一樣，是使用`stderr`。

`console.warn()`也是使用`stderr`。

兩種輸出都會在終端機顯示，所以你可以覺得哪裡有所不同。內建的 logger 沒有什麼不同，不過使用更複雜的工具，像是 [Bunyan](https://github.com/trentm/node-bunyan) 和 [Winston](https://github.com/winstonjs/winston) 可以支援不同的顯示。

```js
// using require
var Console = require('console').Console;

var cons = new Console(process.stdout, process.stderr);
cons.log('testing');

// using existing console object
var cons2 = new console.Console(process.stdout, process.stderr);

cons2.error('test');
```

```shell
$ node chap4-1.js 
testing
test
```

首先`console`物件是從`Console` class 實利化來的，我們可以建立它，然後製作我們版本的`console`，有兩種不同的建立方法：

可以匯入`Console` class 建立，或是使用 global 的`console`建立

```js
// chap4-1.js
// using require
var Console = require('console').Console;

var cons = new Console(process.stdout, process.stderr);
cons.log('testing');

// using existing console object
var cons2 = new console.Console(process.stdout, process.stderr);

cons2.error('test');
```

兩個都要傳入`process.stdout`跟`process.stderr`，讓它個別輸出 log 訊息和 error 訊息。

會分兩個是因為兩個是用不同的 stream，如果今天要將他們分別存成兩個檔案：

```shell
node app.js 1> app.log 2> error.log
```

```js
// chap4-2.js
// log messages
console.log('this is informative');
console.info('this is more information');

// error messages
console.error('this is an error');
console.warn('but this is only a warning');
```

```shell
$ node chap4-2.js 1> app.log 2> error.log
$ cat app.log 
this is informative
this is more information

$ cat error.log 
this is an error
but this is only a warning
```

使用`Console` class 的話，你可以直接做到跟上面一樣的功能，分別寫到不同的檔案裡。

```js
// log.js
var fs = require('fs'),
    Console = require('console').Console;

var output= fs.createWriteStream('./stdout.log');
var errorOutput = fs.createWriteStream('./stderr.log');
// custom simple logger
var logger = new Console(output, errorOutput);
// use it like console
var count = 5;
logger.log('count: %d', count);
// in stdout.log: count 5
```

```shell
$ node log.js 
$ cat stderr.log 
$ cat stdout.log 
count: 5
```

使用物件型態的優勢是，你一樣可以使用 global 的`console，然後建立不同的物件給不同格式的輸出。

`process`物件在第二張。

### Formatting the Message, with Help from util.format() and util.inspect()

`console`的 function 有`log()`、`warn()`、`error()`、`info()`，可以傳入任何的參數，非 object 的 type 會強制轉成 string 顯示出來，但如果是 object 的型態，他只會印出兩層的內容，如果你想要顯示更多層的話，應該要使用`JSON.stringify()`：

```js
// chap4-3.js
var test = {
   a : {
       b : {
          c : {
            d : 'test'
          }
       }
   }
}

// only two levels of nesting are printed out
console.log(test);

// three levels of nesting are printed
var str = JSON.stringify(test, null, 3);

console.log(str);
```

```shell
$ node chap4-3.js 
{ a: { b: { c: [Object] } } }
{
   "a": {
      "b": {
         "c": {
            "d": "test"
         }
      }
   }
}
```

如果你使用字串，可以使用 printf 的格式輸入：

```js
// chap4-4.js
var val = 10.5;
var str = 'a string';

console.log('The value is %d and the string is %s', val, str);
```

這個格式是由`util.format()`套件所提供的，也可以直接使用這個套件：

```js
// chap4-5.js
var util = require('util');

var val = 10.5,
    str = 'a string';

var msg = util.format('The value is %d and the string is %s',val,str);
console.log(msg);
```

如果你只使用一個 function 的話，就只要使用`console.log()`，提供的格式有：

- `%s`：string
- `%d`：number (both integer and float)
- `%j`：JSON. 如果有循環的物件在裡面的話會使用`['circular']`取代
- `%%`：顯示`%`符號

多出來的參數會接在字串的後面，參數不足的話他就不會被替換掉。

```js
var val = 3;

// results in 'val is 3 and str is %s'
console.log('val is %d and str is %s', val);
```

不只有這四個 function 還有其他的`console.dir()`

出入`console.dir()`內的會將物件傳入`util.inpsect()`，這個套件的 function 提供了第二個參數，用來控制物件，像是顯示的格式：

```js
// chap4-6.js
var test = {
   a : {
       b : {
          c : {
            d : 'test'
          }
       }
   }
}

var str = require('util').inspect(test, {showHidden: true, depth: 4 });
console.log(str);
```

```shell
$ node chap4-6.js 
{ a: { b: { c: { d: 'test' } } } }
```

可用的參數有：

- `showHidden`：顯示非 enumerable 或 符號的屬性，預設`false`
- `depth`：顯示到物件多少層，或是遞迴的次數，預設`2`
- `colors`：如果是`true`，輸出就會顯示 ANSI 色碼的顏色，預設`false`
- `customInspect`：如果是`false`，被定義在物件內的 custom inspect function 就不會被執行，預設`true`

顏色可以使用`util.inspect.styles`物件定義，你可以編輯全域的顏色，然後使用`console.log()`輸出：

```js
var util = require('util');

console.log(util.inspect.styles);
console.log(util.inspect.colors);
```

```shell
$ node color.js 
{ special: 'cyan',
  number: 'yellow',
  boolean: 'yellow',
  undefined: 'grey',
  null: 'bold',
  string: 'green',
  symbol: 'green',
  date: 'magenta',
  regexp: 'red' }
{ bold: [ 1, 22 ],
  italic: [ 3, 23 ],
  underline: [ 4, 24 ],
  inverse: [ 7, 27 ],
  white: [ 37, 39 ],
  grey: [ 90, 39 ],
  black: [ 30, 39 ],
  blue: [ 34, 39 ],
  cyan: [ 36, 39 ],
  green: [ 32, 39 ],
  magenta: [ 35, 39 ],
  red: [ 31, 39 ],
  yellow: [ 33, 39 ] }
```

例子裡面的型態有 data、number、boolean，然後我們將原本是黃色的 boolean 變成藍色，讓他跟數字有所區別，然後用很多方式顯示出來，有`util.iinspect()`、`console.dir()`、`console.log()`、`JSON.stringify()`。

```js
// example4-2.js
var util = require('util');

var today = new Date();

var test = {
   a : {
       b : {
          c : {
            d : 'test'
          },
          c2 : 3.50
       },
       b2 : true
   },
   a2: today
}

util.inspect.styles.boolean = 'blue';

// output with util.inspect direct formatting
var str = util.inspect(test, {depth: 4, colors: true });
console.log(str);

// output using console.dir and options
console.dir(test, {depth: 4, colors: true});

// output using basic console.log
console.log(test);

// and JSON stringify
console.log(JSON.stringify(test, null, 4));
```

```shell
$ node example4-2.js 
{ a: { b: { c: { d: 'test' }, c2: 3.5 }, b2: true },
  a2: 2018-06-24T15:24:07.932Z }
{ a: { b: { c: { d: 'test' }, c2: 3.5 }, b2: true },
  a2: 2018-06-24T15:24:07.932Z }
{ a: { b: { c: [Object], c2: 3.5 }, b2: true },
  a2: 2018-06-24T15:24:07.932Z }
{
    "a": {
        "b": {
            "c": {
                "d": "test"
            },
            "c2": 3.5
        },
        "b2": true
    },
    "a2": "2018-06-24T15:24:07.932Z"
}
```

`console.dir()`只支援三種`util.inspect()`的參數：`showHidden`、`depth`、`colors`。他不支援`customInspect`。

### Providing Richer Feedback with console and a Timer

另一個 console 的 function 是`console.time()`和`console.timeEnd()`，用來當計時器。

```js
// chap4-7.js
console.time('the-loop');

for (var i = 0; i < 10000; i++) {
   ;
}

console.timeEnd('the-loop');
```

越大的迴圈會需要夠多的時間，多久取決於你的機器，timer 是非同步執行，因為它可以等到迴圈結束，在顯示名字跟計時器。

```shell
$ node chap4-7.js 
the-loop: 0.145ms
```

修改第一章的 Hello World，加上計時器，計算兩次請求間隔的時間：

```js
// chap4-8.js
var http = require('http');

console.time('hello-timer');
http.createServer(function (request, response) {
  response.writeHead(200, {'Content-Type': 'text/plain'});
  response.end('Hello World\n');
  console.timeEnd('hello-timer');
  console.time('hello-timer');
}).listen(8124);

console.log('Server running at http://127.0.0.1:8124/');
```

```shell
$ node chap4-8.js 
Server running at http://127.0.0.1:8124/
hello-timer: 6668.834ms
hello-timer: 276.005ms
hello-timer: 1629.379ms
hello-timer: 111.182ms
hello-timer: 463.578ms
hello-timer: 102.308ms
```

這個例子是將非同步的 function 跟 timer 合併使用。

## Reference

- [Node.js documentation](https://nodejs.org/api/repl.html)



