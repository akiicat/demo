# Node in Development and Production

## Debugging Node Applications

### The Node Debugger

在文件中加入`debugger`

```js
// debugtest.js
var fs = require('fs');
var concat = require('./external.js').concatArray;

var test = 10;
var second = 'test';

for (var i = 0; i <= test; i++) {
   debugger;
   second+=i;
}

setTimeout(function() {
   debugger;
   test = 1000;
   console.log(second);
}, 1000);

fs.readFile('./log.txt', 'utf8', function (err,data) {
   if (err) {
      return console.log(err);
   }

   var arry = ['apple','orange','strawberry'];
   var arry2 = concat(data,arry);
   console.log(arry2);
});
```

```shell
node debug debugtest.js
```

- `sb`：`sb('file.js', 10)` set breakpoint
- `cb`：`cb('file.js', 10)` clear breakpoint
- `c`：continue
- `n`：next
- `s`：step
- `repl`
- `watch`：`watch(arr)`
- `unwatch`：`unwatch(arr)`
- `bt`：backtrace
- `o`：out 離開你現在在裡面的 function
- `run`
- `kill`
- `restart`：`kill` + `run`，會將所有的 breakpoint 跟 watcher 都清除。

### Node Inspector

安裝

```shell
sudo npm install -g node-inspector
```

使用

```shell
node-debug debugtest.js
```

你要把 Chrome 或 Opera 設成你預設的瀏覽器，應用程式會幫你打開，如果不是的話你必須手動貼上 URL 打開。

node 版本超過 8 以上可以用：

```shell
node --inspect-brk debugtest.js
```

## Unit Testing

大多的 unit test 是使用 Assert 套件建立的。

### Unit Testing with Assert

*Assertion tests* 會評估表達式，結果的值可以是`true`或`false`。先引用 Assert 套件：

```js
var assert = require('assert');
```

`assert.equal()`跟`assert.strictEqual()`兩個方法有相同的參數：第一個是預期的回傳值，第二個是表達式的值，如果兩個值相等的話，代表成功，不會有任何輸出，保持沈默。

如果兩個值不相等的話，就跳出 exception。

- `assert.equal`
- `assert.strictEqual`
- `assert.notEqual`
- `assert.notStrictEqual`
- `assert.deepEqual`
- `assert.notDeepEqual`
- `assert.deepStritEqual`
- `assert.notDeepStrictEqual`

deep 的方法是用在複雜物件像是 array 或 object。

```js
assert.deepEqual([1,2,3],[1,2,3]); // success
assert.equal([1,2,3],[1,2,3]); // fail
```

`assert`的方法等同於`assert.isEqual(true)`或是`assert.ok`

```js
var val = 3;
assert(val == 3, 'Test 1 Not Equal');
assert.equal(true, val == 3, 'Test 1 Not Equal');
assert.ok(val == 3, 'Test 1 Not Equal');
```

`assert.fail`有四個參數：value、expression、message、operator，最後一個 operator 用來區分 value 跟 expression：

```js
// assert.js
var assert = require('assert');

try {
  var val = 3;
  assert.fail(val, 4, 'Fails Not Equal', '==');
} catch(e) {
  console.log(e);
}
```

```shell
$ node assert.js 
{ AssertionError [ERR_ASSERTION]: Fails Not Equal
    at Object.<anonymous> (/Users/akiicat/Github/LearningNode2/chap11/assert.js:5:10)
    at Module._compile (module.js:652:30)
    at Object.Module._extensions..js (module.js:663:10)
    at Module.load (module.js:565:32)
    at tryModuleLoad (module.js:505:12)
    at Function.Module._load (module.js:497:3)
    at Function.Module.runMain (module.js:693:10)
    at startup (bootstrap_node.js:191:16)
    at bootstrap_node.js:612:3
  generatedMessage: false,
  name: 'AssertionError [ERR_ASSERTION]',
  code: 'ERR_ASSERTION',
  actual: 3,
  expected: 4,
  operator: '==' }
```

`assert.ifError`檢查錯誤物件：

```js
assert.ifError(err); // throws only if true value
```

`assert.throws`跟`assert.doesNotThrow`，兩個第一個傳入都是 code block，第二個的參數錯誤 object，第三個是錯誤 message。錯誤的 object 可以是：constructor、regular expression、validation function。

```js
assert.throws(
    function() {
        throw new Error("Wrong value");
    },
    /something/
);
```

```shell
$ node
> assert.throws(
...     function() {
.....         throw new Error("Wrong value");
.....     },
...     /something/
... );
Error: Wrong value
    at repl:3:15
    at tryBlock (assert.js:619:5)
    at innerThrows (assert.js:638:18)
    at Function.throws (assert.js:662:3)
```

使用 Assert 套件可以建立粗糙的 unit test，你需要使用很多包裝整個測試的腳本才不會因為一個錯誤而無法運行。因此可以使用更高層的 unit testing framework，像是 Nodeunit。

### Unit Testing with Nodeunit

```shell
sudo npm install -g nodeunit
```

Nodeunit 提供一個簡單運行操是的方法，不用使用 try/catch block，他支援所有 Assert 套件的測試，為了控制測試提供一組方法，test case 會由物件所包起來，然後 export 物件，每個 test case 都會拿到一個 control object，命名它為`test`，`expect`會告訴 Nodeunit 有多少個測試要測。最後一個會呼叫`done`方法，告訴 Nodeunit test case 已經完成了：

```js
// thetest.js
var util = require('util');

module.exports = {
    'Test 1' : function(test) {
        test.expect(4);
        test.equal(true, util.isArray([]));
        test.equal(true, util.isArray(new Array(3)));
        test.equal(true, util.isArray([1,2,3]));
        test.notEqual(true, 1 > 2);
        test.done();
    },
    'Test 2' : function(test) {
        test.expect(2);
        test.deepEqual([1,2,3], [1,2,3]);
        test.ok('str' === 'str', 'equal');
        test.done();
    }
};
```

```shell
$ nodeunit thetest.js 

thetest.js
✔ Test 1
✔ Test 2

OK: 6 assertions (10ms)
```

### Other Testing Frameworks

簡單介紹三個測試的框架：Mocha、Jasmine、Vows

#### Mocha

```shell
npm install -g mocha
```

Mocha 是由另一個受歡迎的測試框架 Espresso 繼承下來的。

在 Node 或是在瀏覽器都可以使用，透過`done` function 允許非同步的測試，同步測試可以忽略：

```js
// testcase.js
assert = require('assert')
describe('MyTest', function() {
  describe('First', function() {
    it('sample test', function() {
      assert.equal('hello','hello');
    });
  });
});
```

```shell
$ mocha testcase.js


  MyTest
    First
      ✓ sample test


  1 passing (5ms)
```

#### Vows

Vows 是一個 behavior-driven development (BDD) 的測試框架，他比其他的更晚整：更完善的文件。測試是由 testing suites 所組成的，testing suites 是由很多的 batches of executed tests 所組成的。一個 batch 是由一個或多個 contexts 或一個 topic 所組成，context 可以同步執行，這個測試就是 vow，提供了 tested topic 跟 test vow。

雖然有點不熟悉這些字，看範例會比較好一點：

```shell
npm install vows
```

範例是測試 circle 套件，他會回傳面積跟圓周，因為他的結果是浮點數，所以我們限制小數點後四位：

```js
// circle.js
const PI = Math.PI;

exports.area = function (r) {
  return (PI * r * r).toFixed(4);
};

exports.circumference = function (r) {
  return (2 * PI * r).toFixed(4);
};
```

```js
// vowstest.js
// one batch, one context, one topic, and two vows
var vows = require('vows'),
    assert = require('assert');

var circle = require('./circle');

var suite = vows.describe('Test Circle');

suite.addBatch({
    'An instance of Circle': {
        topic: circle,
        'should be able to calculate circumference': function (topic) {
            assert.equal (topic.circumference(3.0), 18.8496);
        },
        'should be able to calculate area': function(topic) {
            assert.equal (topic.area(3.0), 28.2743);
        }
    }
}).run();
```

在這個例子中，circle 物件是 *topic*，area 跟 circumference 是 *vows*，兩個都會由 Vows context 封裝起來，*suite* 是總體的測試應用程式，*batch* 是 test instance (circle and two methods)。

```js
$ node vowstest.js 
·· ✓ OK » 2 honored (0.004s) 
```

topic 永遠都是非同步的 function，而不是使用`circle`作為 topic

```js
// vowstest2.js
// one batch, two context, two topic, and two vows
var vows = require('vows'),
    assert = require('assert');

var circle = require('./circle');

var suite = vows.describe('Test Circle');

suite.addBatch({
    'Testing Circle Circumference': {
        topic: function() { return circle.circumference;},
        'should be able to calculate circumference': function (topic) {
            assert.equal (topic(3.0), 18.8496);
        },
    },
    'Testing Circle Area': {
        topic: function() { return circle.area;},
        'should be able to calculate area': function(topic) {
            assert.equal (topic(3.0), 28.2743);
        }
    }
}).run();
```

## Keeping Node Up and Running

Forever 是一個工具，確保你的應用程式掛了會重新啟動：

```shell
sudo npm install -g forever
```

並不是直接用 node 運行：

```shell
forever start -a -l forever.log -o out.log -e err.log finalserver.js
```

這兩個參數的預設值：`minUpTime` (1000ms) 跟 `spinSleepTime` (1000ms)。

運行的腳本是`finalserver.js`，然後有 Forever log、output log、 error log。如果 log 存在的話，他會附加在最後面。

如果有什麼原因導致腳本掛掉的話，Forever 會重新啟動他，而且確保他會一直運行，即便你終止了你的視窗。

Forever 的有參數也有動作，查看`help`。

```js
// chap5-1.js
var http = require('http');

var server = http.createServer().listen(8124);

server.on('request', function(request,response) {

   response.writeHead(200, {'Content-Type': 'text/plain'});
   response.end('Hello World\n');
});

console.log('server listening on 8214');
```

`list`可以列出現在運行的狀態：

```shell
$ forever start -a -l forever.log -o out.log -e err.log chap5-1.js
forever listwarn:    --minUptime not set. Defaulting to: 1000ms
warn:    --spinSleepTime not set. Your script will exit if it does not stay up for at least 1000ms
info:    Forever processing file: chap5-1.js

$ forever list
info:    Forever processes running
data:        uid  command                                            script     forever pid   id logfile                             uptime      
data:    [0] 71X- /Users/akiicat/.nvm/versions/node/v8.11.3/bin/node chap5-1.js 89280   89282    /Users/akiicat/.forever/forever.log 0:0:0:0.177 
```

還有一些其他參數：

- `-s` `--slient`：安靜的運行腳本
- `-v` `--verbose：開啟 Forever 的 verbose
- `--sourceDir`：腳本目錄

你可以跟 forever-monitor 套件一起使用

```js
var forever = require('forever-monitor');

var child = new (forever.Monitor)('serverfinal.js', {
	max: 3,
	silent: true,
	args: []
});

child.on('exit', function () {
	console.log('serverfinal.js has exited after 3 restarts');
});

child.start();
```

你可以跟 Nodemon 套件一起使用，如果你的 source code 更新，他會確保你的應用程式刷新。

```shell
npm install -g nodemon
```

他會檢查目錄使否有更動，如果有，就會重新啟動 app：

```shell
nodemon app.js
```

傳入參數：

```shell
nodemon app.js param1 param2
```

CoffeeScript：

```shell
nodemon someapp.coffee
```

watch 不同的目錄，可以使用`--watch`標籤：

```shel
nodemon --watch dir1 --watch libs app.js
```

其他標籤參考[文件](https://github.com/remy/nodemon/)

Nodemon 要跟 Forever 已起使用的話，記得要加上`--exitcrash`選項，確保程式會 crash，Nodemon 會乾淨的退出，讓 Forever 重新啟動：

```shell
forever start nodemon --exitcrash serverfinal.js
```

如果找不到 Nodemon 的話，使用全名：

```shell
forever start /usr/bin/nodemon --exitcrash serverfinal.js
```

### Benchmark and Load Testing with Apache Bench

Performance testing 是由 benchmark testing 跟 load testing 所組合而成的。Benchmark testing 又稱作 comparsion testing，他會運行多個版本，來決定哪個比較好。他是增加你效能很有效的工具，你建立標準的版本跟其他的做比較。

Load testing 是基本的壓力測試。你可以查看什麼時候你的應用程式開始變慢或是掛掉，或是想要讓應用程式掛掉。

ApacheBench 可以同時處理兩種測試，又簡稱 ab，可以選擇要有多少數量`-n 100`，跟同時存取網頁的數量`-c 20`。

```shell
ab -n 100 -c 20 http://burningbird.net/
```

重要的是要提供最後的斜線。

Loadtest 應用程式：

```shell
sudo install -g loadtest
loadtest --help
```

 