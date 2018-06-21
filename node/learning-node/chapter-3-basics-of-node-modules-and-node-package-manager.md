# Basics of Node Modules and Node Package Manager (npm)

## An Overview of the Node Module System

Node 的基本實做盡可能簡化，developer 可以通過 module 提供其他功能，而不是將所有可能使用的 component 直接放到 Node 中。

在 CommonJS module system 之後，Node's module system 是一種模式，這是一種建立 module 的方式，以便它們可以互操作。系統的核心是讓 developer 能有一樣的撰寫模式，以確保 modules 能良好地合作。

用 Node 實現 CommonJS module system 需求包括：

- 支援`require` function，可以拿到 module id 並回傳匯出 API。
- module 的名字是字串，字串裡面可以包含斜線，用於資料夾的路徑。
- module 必須明確地匯出想要揭露的在 module 外的東西。
- 在 module 內的變數是 private。

有一些 Node 的功能是全域的，你不需要引用他就能使用，然而大多數的功能是需要通過 module 來合併使用。

### How Node Finds and Loads a Module

不管是你 Node 核心的 module 或是你安裝在 Node 應用程式外的 module，用`require`存取 Node module：

```js
var http = require('http');
```

或者你只想要拿到某個屬性：

```js
var name = require('url').parse(req.url, true).query.name;
```

或是存取特別的 module 物件：

```js
var spawn = require('child_process').spawn;
```

當應用程式請求 module 的時候，很多事情發生了。首先是 cached，他會先去 cached 檢查是否已經被 cached，沒有的話他就會去載入它，然後第一次載入的時候會把它暫存起來。

Node 只支援每個檔案一個 module。

如果沒有被 cached 住的話，他會先去找是否是 native module，native module 是預先編譯的檔案，像是 C++ add-ons。

如果 module 沒有被 cached 或不是 native module，則需要為它建立新的 Module 物件，並返回 module exports 的屬性。基本上是把 public 的功能回傳到應用程式。

如果你想要刪除 cached 的話：

```js
delete require('./circle.js');
```

下次需要他的時候會再重新載入到 cached 裡。

在載入 module，Node 必須解析它的位置。在找 module 文件時，它會一層一層的檢查。

首先，core module 有優先權，如果你把 module 的名稱命名為`http`，當你載入`http` module 的時候，他會載入 core 的`http`。要載入它的話必須連路徑一起提供給他，以便他讓區分是 core 的 module 還是自己寫的 module。

```js
var http = require('/home/mylogin/public/modules/https.js');
```

不管是絕對路徑或相對路徑都可以

```js
var someModule = require('./somemodule.js');
```

如果你給了 module extension，只給了 module 的名稱，他會先找當前目錄下是否有 module 是 .js extension，沒有的話再找 .json extension 如果有正確的 json 他就會載入，最後才會找 .node extension，需要預先編譯 Node add-on。

JSON 不需要 export 只需要正確的 JSON。

如果 module 是從 npm 安裝的話，你不提供路徑，只需要 module 名稱：

```js
var async = require('async');
```

Node 會找尋 node_modules 目錄下的 module，用 search hierarchy：

1. 先找應用程式目錄下是否有 node_modules 的資料夾。
2. 然後再往應用程式的上一層找是否有 node_modules 的資料夾。
3. 繼續找到 root 層`/node_modules`。
4. 最後，找安裝在 global 的 module。

Node 使用這樣的原因是要先找 local 的版本，然後再找 global 的版本，如果你嘗試使用新的版本 module，最好是安裝在 local 的地方。

```shell
npm install somemodule
```

上面的會最先被載入，而不是安裝在 global 的 module：

```shell
npm install -g somemodule
```

你可以看到 module 載入的 function：

```js
console.log(require.resolve('async'));
```

他會回傳 module 解析的位置：

```shell
/private/tmp/node_modules/async/dist/async.js
```

如果你提供資料夾當作 module 名稱，Node 會先找 package.json 的檔案，然後再找裡面屬性是`main`的路徑。

```json
{
    "name": "somemoduel",
    "main": "./lib/somemodule.js"
}
```

如果找不到 package.json 檔案，他載入 index.js 或是 index.node，如果全部都失敗，那就會出錯。

注意，caching module 是基於檔案名稱和路徑，如果你 cached 一個 global 的 module 然後載入了 local 的版本，那 local 版本的也會被 cached。

每個 module 會用 Module 物件所包裝，module 裡有`require` function，我們所使用的 global `require`會去呼叫 Module-specific function。`Module.require()` function 會呼叫另外的內部`Module._load()` function，會執行前面所說的功能。不同的是在 REPL 裡請求的時候，他會有自己處理的方法。

如果 module 是主要的 module，主要的 module 會在 command line 裡被呼叫，那他會被指派一個 global `require` 物件`require.main`。

```js
// test.js 
console.log(require)
```

你會看到`main`物件，Node 用檔名和路徑來找尋 module，以及應用程式的快取。

```shell
$ node test.js 
{ [Function: require]
  resolve: [Function: resolve],
  main: 
   Module {
     id: '.',
     exports: {},
     parent: null,
     filename: '/private/tmp/test.js',
     loaded: false,
     children: [],
     paths: 
      [ '/private/tmp/node_modules',
        '/private/node_modules',
        '/node_modules' ] },
  extensions: { '.js': [Function], '.json': [Function], '.node': [Function] },
  cache: 
   { '/private/tmp/test.js': 
      Module {
        id: '.',
        exports: {},
        parent: null,
        filename: '/private/tmp/test.js',
        loaded: false,
        children: [],
        paths: [Object] } } }
```

他並不會像瀏覽器的變數是全域的，因為他使用 function 把 scripts 包起來，所以不會有變數發生衝突：

```js
function (module, exports, __filename, ...) {}
```

換句話說，Node 是用匿名 function 來包裝 module，而且 expose 你所想要的 module，因為 module 屬性在要被使用錢都是由 module 名稱來的，他不會跟你的區域變數搞混。

### Sandboxing and the VM Module

在 Javascript 盡量避免使用`eval()`，因為他會執行在同一個 Javascript 的檔案裡，你可能會有一些未知的或是任意的 block，執行他前必須要小心的寫。

如果你想要在 Node 裡執行一段 Javascript，你可以使用 VM module，然後在沙盒裡面跑腳本。可以使用 VM 將 script 與本地環境隔離開來。

Script 可以被預先編譯，使用`vm.Script`。這裡有三個 function：第一個是`script.runInNewContext()`或`vm.runInNewContext()`其中之一，在新的文件中執行 script，而且 script 不能存取 local 和 global 的物件，所以需要傳入到 sandbox 中：

```js
// chap3-1.js
var vm = require('vm');

var sandbox = {
  process: 'this baby',
  require: 'that'
};

vm.runInNewContext('console.log(process);console.log(require)',sandbox);
```

```shell
$ node chap3-1.js 

evalmachine.<anonymous>:1
console.log(process);console.log(require)
^
ReferenceError: console is not defined
    at evalmachine.<anonymous>:1:1
    at ContextifyScript.Script.runInContext (vm.js:35:29)
    at ContextifyScript.Script.runInNewContext (vm.js:41:15)
    at Object.exports.runInNewContext (vm.js:72:17)
    at Object.<anonymous> (/Users/akiicat/Github/LearningNode2/chap3/chap3-1.js:8:4)
    at Module._compile (module.js:570:32)
    at Object.Module._extensions..js (module.js:579:10)
    at Module.load (module.js:487:32)
    at tryModuleLoad (module.js:446:12)
    at Function.Module._load (module.js:438:3)
```

sandbox 裡面包含了兩個 global 的值。不過他會出錯，因為他並不知道`console`是什麼，所以需要再傳入 console：

```js
// chap3-2.js
var vm = require('vm');

var sandbox = {
  process: 'this baby',
  require: 'that',
  console: console
};

vm.runInNewContext('console.log(process);console.log(require)',sandbox);
```

```shell
$ node chap3-2.js 
this baby
that
```

這預設是用來建立全新的 script，如果你想要存取 global 或其他物件的話，可以使用`runInThisContext()`，將 global 物件傳入，local 物件無法傳入。

```js
// example3-1.js
var vm = require('vm');

global.count1 = 100;
var count2 = 100;

var txt = 'if (count1 === undefined) var count1 = 0; count1++;' +
          'if (count2 === undefined) var count2 = 0; count2++;' +
          'console.log(count1); console.log(count2);';

var script = new vm.Script(txt);
script.runInThisContext({filename: 'count.vm'});

console.log(count1);
console.log(count2);
```

```shell
$ node example3-1.js 
101
1
101
100
```

如果不在裡面宣告`count2`的話，他會出錯，因為他沒辦法存取到應用程式的區域變數

```js
// chap3-3.js
var vm = require('vm');

global.count1 = 100;
var count2 = 100;

var txt = 'count1++;' +
          'count2++;' +
          'console.log(count1); console.log(count2);';

var script = new vm.Script(txt, {filename: 'count.vm'});

try {
  script.runInThisContext();
} catch(err) {
   console.log(err.stack);
}
```

```shell
$ node chap3-3.js 
ReferenceError: count2 is not defined
    at count.vm:1:10
    at ContextifyScript.Script.runInThisContext (vm.js:25:33)
    at Object.<anonymous> (/Users/akiicat/Github/LearningNode2/chap3/chap3-3.js:13:10)
    at Module._compile (module.js:570:32)
    at Object.Module._extensions..js (module.js:579:10)
    at Module.load (module.js:487:32)
    at tryModuleLoad (module.js:446:12)
    at Function.Module._load (module.js:438:3)
    at Module.runMain (module.js:604:10)
    at run (bootstrap_node.js:394:7)
```

會顯示錯誤是因為，選項裡面的`displayErrors`參數預設是`true`，其他的參數像是`filename`和`timeout`，`timeout`單位是微秒，如果超過執行時間會被終止，然後丟出錯誤。`filename`則是在追蹤錯誤的時候會顯示的名字，如果你想要追蹤 Script 物件的話，就要在 Script 物件傳入，而不是在`runInThisContext`的時候。

- `filename`
- `displayerrors`
- `timeout`

錯誤紀錄裡面可以看到檔名`count.vm`，用於追蹤錯誤。

如果我們把 script 寫在應用程式的外面，然後載入執行它：

```js
// script.js
if (count1 === undefined) var count1 = 0; count1++;
if (count2 === undefined) var count2 = 0; count2++;
console.log(count1); console.log(count2);
```

```js
// chap3-4.js
var vm = require('vm');
var fs = require('fs');

global.count1 = 100;
var count2 = 100;

var script = new vm.Script(fs.readFileSync('script.js','utf8'));
script.runInThisContext({filename: 'count.vm'});

console.log(count1);
console.log(count2);
```

我們需要在這裡把檔案載入，不能在 script.js 裡面載入檔案，因為`require`在裡面不能使用，global 的變數和 function 都不能在裡面使用。

最後 sandbox function `runInContext()`，可以使用 VM 物件區呼叫他，他可以傳入 sandbox 裡面包含 contextualized：

```js
// chap3-5.js
var vm = require('vm');
var util = require('util');

var sandbox = {
     count1 : 1
    };

vm.createContext(sandbox);
if (vm.isContext(sandbox)) console.log('contextualized');

vm.runInContext('count1++; counter=true;',sandbox,
                {filename: 'context.vm'});

console.log(util.inspect(sandbox));
```

```shell
$ node chap3-5.js 
contextualized
{ count1: 2, counter: true }
```

`runInContext()` function 支援三種選項，在`runInThisContext()`和`runInNewContext()`所支援的選項。Script 和 VM 最大的不同點是，Script 物件會預先編譯程式碼，你要傳入你所建立的檔案，而不是 function call 裡的其中一個選項。

## An In-Depth Exploration of NPM

安裝 npm

```shell
npm install npm -g
npm help npm
```

你可以選擇安裝 module 是裝在 global 還是 local，預設是安裝在 local 的，他會在 node_modules 路徑裡。

```shell
npm install request
```

他不只會安裝`request`也會連她依賴的 module 一起安裝。

如果想要裝在 global，可以加上`-g`或是`--global`：

```shell
npm install request -g
```

安裝 package 壓縮檔

```shell
npm install http://somecompany.com/somemodule.tgz
```

指定版本：

```shell
npm install modulename@0.1
```

檢查是否有新的 module，然後更新她：

```shell
npm update
npm update modulename
```

更新 npm 自己：

```shell
npm install npm -g
```

列出過時的 package

```shell
npm outdated
```

列出 package：`list`, `ls`, `la`, `ll`

```shell
npm ls
```

```shell
$ npm la async

│ /private/tmp
│ 
└── async@2.6.1 
    Higher-order functions and common patterns for asynchronous code
    git+https://github.com/caolan/async.git
    https://caolan.github.io/async/
```

直接安裝 module 的依賴套件：

```shell
npm install -d
```

從 github 安裝：

```shell
npm install http://github.com/visionmedia/express/tarball/master
```

如果從 github 安裝需要注意他可能會覆蓋掉你之前所使用的版本。

查看 global 的 module：

```shell
npm ls -g
```

設定檔

```shell
$ npm config list
; cli configs
scope = ""
user-agent = "npm/4.0.3 node/v6.9.1 darwin x64"

; node bin location = /usr/local/bin/node
; cwd = /private/tmp
; HOME = /Users/akiicat
; "npm config ls -l" to show all defaults.
```

列出所有預設的參數：

```shell
npm config ls -l
```

修改參數：

```shell
npm config delete keyname
npm config set keyname value
```

或是直接修改：

```shell
npm config edit
```

搜尋 package：

```shell
npm search html5 parser
```

建立 package.json 檔，加了`--yes`就不會跳出一堆問題問你：

```shell
npm init --yes
```

之後安裝 module 時候，加上`--save-dev`，他會把安裝的東西紀錄在 package.json 裡：

```shell
npm install request --save-dev
```

`devDependencies`欄位記錄我們安裝的套件，而`dependencies`是依賴的套件。

```shell
$ cat package.json 
{
  "name": "tmp",
  "version": "1.0.0",
  "description": "",
  "main": "test.js",
  "dependencies": {
    "async": "^2.6.1"
  },
  "devDependencies": {
    "request": "^2.87.0"
  },
  "scripts": {
    "test": "echo \"Error: no test specified\" && exit 1"
  },
  "keywords": [],
  "author": "",
  "license": "ISC"
}
```

如果要自動存入到 package.json 的話，可以修改 npmrc 檔案，他會在每個使用者的`~/.npmrc`、每個專案下的`/path/project/.npmrc`、global `$PREFIX/etc/npmrc`、或是內建的設定檔`/path/to/npm/npmrc`，或輸入：

```shell
npm config set save=true
npm config set save-exact=true
```

`--save`會自動將 package 存入到 package.json，`--save-exact`儲存確切的版本，而不會儲存一個範圍。

npm 有很多不同的設定可以參考 [npm documentaion](https://docs.npmjs.com/misc/config)。

## Creating and Publishing Your Own Node Module

就如同瀏覽器的 Javascript 你可能會想要把不同的 Javscript 分開到不同的檔案，這樣才能重複使用它。

### Creating a Module

我們有個`concatArray`：

```js
function concatArray(str, array) {
  return array.map(function(element) {
       return str + ' ' + element;
  });
};
```

為了要變成 Javascript library，我們用`exports`物件匯出他：

```js
// arrayfunctions.js
exports.concatArray = function(str, array) {
  return array.map(function(element) {
       return str + ' ' + element;
  });
};
```

要在 Node 應用程式裡使用`concatArray`，需要匯入 library：

```js
// chap3-7.js
var newArray = require ('./arrayfunctions.js');

console.log(newArray.concatArray('hello', ['test1','test2']));
```

你可以建立 contructor 或 function 的 module，然後將他匯出

### Packaging an Entire Directory

你可以將你的 module 拆成很多的 Javascript 的檔案，Node 可以載入所有路徑裡的內容，只要你用兩種方式組織他：

第一個方法是提供 package.json 檔案，裡面有路徑的資訊，他需要包含兩個東西，`name`和`main`：

```json
{
  "name" : "mylibrary",
  "main" : "./mymodule/mylibrary.js"
}
```

- `name`：module 的名字
- `main`：module 的進入點

第二種方法是在目錄下的`index.js`或`index.node`的檔案，如果你有很多檔案要匯出的話，index.js 看把你所想要匯出的東西包裝好，或是你檔案很大想要拆成更小的檔案。

### Preparing Your Module for Publication

包裝 [npm package](https://docs.npmjs.com/files/package.json) 和他的[建議欄位](http://wiki.commonjs.org/wiki/Packages/1.0#Package_Descriptor_File)

- `name` - The name of the package—required
- `description` - The package description
- `version` - The current version conforming to semantic version requirements—required 
- `keywords` - An array of search terms
- `maintainers` - An array of package maintainers (includes name, email, and website) contributors An array of package 
- `contributors` - (includes name, email, and website)
- `bugs` - The URL where bugs can be submitted
- `licenses` - An array of licenses
- `repository` - The package respository
- `dependencies` - Prerequisite packages and their version numbers

只有`name`和`version`是必須的欄位，雖然大多的欄位都是建議填的，你可以輸入：

```shell
npm init
```

他會以對話的方式填寫欄位。

我們建立一個物件叫`InputChecker`檢查 command 的資料，在第二章節為了使用`EventEmitter`，現在我們修改一下物件讓他變成 module，然後把`InputChecker`匯出：

```js
// index.js
var util = require('util');
var eventEmitter = require('events').EventEmitter;
var fs = require('fs');

exports.InputChecker = InputChecker;

function InputChecker(name, file) {
   this.name = name;
   this.writeStream = fs.createWriteStream('./' + file + '.txt',
      {'flags' : 'a',
       'encoding' : 'utf8',
       'mode' : 0666});
};

util.inherits(InputChecker,eventEmitter);
InputChecker.prototype.check = function (input) {
  var command = input.toString().trim().substr(0,3);
  if (command == 'wr:') {
     this.emit('write',input.substr(3,input.length));
  } else if (command == 'en:') {
     this.emit('end');
  } else {
     this.emit('echo',input);
  }
};
```

然後執行`npm init`：

```json
// package.json
{
  "name": "inputcheck",
  "version": "1.0.0",
  "description": "Looks for and implements commands from input",
  "main": "index.js",
  "scripts": {
    "test": "node test/test.js"
  },
  "keywords": [
    "command",
    "check"
  ],
  "author": "Shelley Powers",
  "license": "ISC"
}
```

因為`npm init`不會把依賴的套件加入，除非執行`npm install --save`，因為`InputChecker`並沒有依賴的套件，所以可以將欄位留空。

接下來加入測試的程式碼，確認他可以運行：

```js
// test/test.js
var inputChecker = require('inputcheck').InputChecker;

// testing new object and event handling
var ic = new inputChecker('Shelley','output');

ic.on('write', function(data) {
   this.writeStream.write(data, 'utf8');
});

ic.addListener('echo', function( data) {
   console.log(this.name + ' wrote ' + data);
});

ic.on('end', function() {
   process.exit();
});

process.stdin.resume();
process.stdin.setEncoding('utf8');
process.stdin.on('data', function(input) {
    ic.check(input);
});
```

我們在 package.json 裡面有加入：

```json
  "scripts": {
    "test": "node test/test.js"
  }
```

這個測試不是最原始的測試方法，因為沒有使用強大的 Node testing environment，這裡只是要 demo 他是如何運作的：

```shell
npm test
```

### Publishing the Module

[Developer Guide](https://docs.npmjs.com/misc/developers)

在 package.json 還有一些需要的欄位`directories`，他是一串資料夾，像是 test 或是 doc：

```json
{
    "directories": {
        "doc": ".",
        "test": "test",
        "example": "examples"
    }
}
```

在 publishing 之前，建議先測試安裝看看：

```shell
npm install . -g
```

正確之後加入使用者，會需要輸入使用者和密碼：

```shell
npm adduser
```

然後在 publish：

```shell
npm publish
```

如果有重要的東西不想被推上去的話可以加入`.npmignore`的檔案。

### Discovering Node Modules and Three Must-Have Modules

[npm website](https://www.npmjs.com/)

[最多星星的套件](https://www.npmjs.com/browse/star)

### Better Callback Management with Async

解決 pyramid of doom 的方法，最常使用的是 Async 套件，他會變得更線性，更容易管理：

- `waterfall`：function 會依序地被呼叫，他的結果一依序的傳入下一個 callback。
- `series`：function 會依序地被呼叫，不過他結果不會傳入下一個 callback，而是會把結果傳入回傳的 array 中。
- `parallel`：function 同時被呼叫，每個 callback 的結果會存入回傳的 array 中。
- `whilst`：重複的呼叫一個 function，當初步測試回傳 false 或發生錯誤時，呼叫最後一次回調。
- `queue`：同時呼叫 function 直到上限，新的 function 會在後面等待，直到有 function 結束他在會執行。
- `until`：重複呼叫一個 function，當後處理測試回傳 false 或發生錯誤時，才呼叫最後一個 callback。
- `auto`：function 根據需求被呼叫，每個 function 接收之前 callback 的結果。
- `iterator`：每個 function 都會呼叫下一個 function，並可以單獨訪問下一個 iterator。
- `apply`：帶有變數的 continuation function 與其他 control flow function 結合使用。
- `nextTick`：基於`process.nextTick`，在 event loop 的下一個 loop 中呼叫 callback。

Async 套件提供管理的功能，像是如果有很多的`forEach``map``filter`以及有效的功能。然而我們比較有興趣的功能是處理 control flow。

用 npm 安裝 Async，可以依照需求加上`-g`、`--save`、`--save-dev`：

```shell
npm install async
```

我們在第二章有個例子很適合使用`waterfall`，使用`fs.readFile`開檔讀檔，然後替換文字，寫入檔案。注意每個步驟的 callback function：

```js
// example3-5.js
var fs = require('fs'),
    async = require('async');

async.waterfall([
   function readData(callback) {
      fs.readFile('./data/data1.txt', 'utf8', function(err, data){
           callback(err,data);
       });
   },
   function modify(text, callback) {
      var adjdata=text.replace(/somecompany\.com/g,'burningbird.net');
      callback(null, adjdata);
   },
   function writeData(text, callback) {
       fs.writeFile('./data/data1.txt', text, function(err) {
          callback(err,text);
       });
   }
], function (err, result) {
      if (err) {
        console.log(err.message);
      } else {
        console.log(result);
      }
});
```

`async.waterfall`需要兩個參數：一個 array 和一個最終的 callback function。每個非同步的 function 都是 array 裡面的一個 element，而且每個 function 的參數都需要有一個 callback，它允許我們串連非同步的 callback，不會層層的疊在塗一個 callback funcion 裡。

當你看到你的 callback function 都是正常處理他自己的東西，我們沒有特別處理他的錯誤，因為如果其中一個環節出錯的話，他會一路把錯誤傳到最後的 function，最後的 callback 就是我們會看到的結果。

建議使用命名 function，這樣方便 debug。

另一個第二章的例子：

```js
// example3-6.js
var fs = require('fs'),
    async = require('async'),
    _dir = './data/';

var writeStream = fs.createWriteStream('./log.txt',
      {'flags' : 'a',
       'encoding' : 'utf8',
       'mode' : 0666});

var file = 'data1.txt';

fs.readdir (_dir, function (err, files) {
   files.forEach(function(file) {
       processItem(file);
   });
});

function processItem(file) {
   async.waterfall([
      function checkFile(callback) {
        fs.stat(_dir + file, function(err, stats) {
          callback(err, stats, file);
        });
      },
      function readData(stats, file, callback) {
        if (stats.isFile())
          fs.readFile(_dir + file, 'utf8', function(err, data){
            callback(err,file,data);
          });
      },
      function modify(file, text, callback) {
        var adjdata=text.replace(/somecompany\.com/g,'burningbird.net');
          callback(null, file, adjdata);
      },
      function writeData(file, text, callback) {
        fs.writeFile(_dir + file, text, function(err) {
          callback(err,file);
        });
      },
      function logChange(file, callback) {
        writeStream.write('changed ' + file + '\n', 'utf8', 
          function(err) {
            callback(err);
          })
      }
    ], function (err, result) {
        if (err) 
           console.log(err);
    }); 
}
```

我們不必在每個 callback 檢查錯誤，因為 Async 已經幫我們處理好了，他會一路傳到最後的 function。

其他 Async control flow 方法，像是`async.parallel`和`async.serial`以類似的方式執行，其中一組 task 作為第一個參數，最後一個 callback 為第二個參數。儘管如此，他們如何處理非同步的方式仍然與您所期望的不同。

分別打開三個檔案寫入東西：

```js
// example3-7.js
var fs = require('fs'),
    async = require('async');

async.parallel({
   data1 : function (callback) {
      fs.readFile('./data/fruit1.txt', 'utf8', function(err, data){
           callback(err,data);
       });
   },
   data2 : function (callback) {
      fs.readFile('./data/fruit2.txt', 'utf8', function(err, data){
           callback(err,data);
       });
   },
   data3 : function readData3(callback) {
      fs.readFile('./data/fruit3.txt', 'utf8', function(err, data){
           callback(err,data);
       });
   },


}, function (err, result) {
      if (err) {
         console.log(err.message);
      } else {
         console.log(result);
      }
});
```

```shell
$ node example3-7.js 
{ data1: 'apples\n', data2: 'oranges\n', data3: 'peaches\n' }
```

他回傳的結果是一組 array。

### Command-Line magic with Commander

用於分析 command line 的參數

安裝

```shell
npm install commander
```

`option`可以設定 flag：

```js
// options.js
var program = require('commander');

program
   .version ('0.0.1')
   .option ('-s, --source [web site]', 'Source web site')
   .option ('-f, --file [file name]', 'File name')
   .parse(process.argv);

console.log(program.source);
console.log(program.file);
```

兩個預設的參數

- version
- help

```shell
$ node options.js -h

  Usage: options [options]

  Options:

    -V, --version            output the version number
    -s, --source [web site]  Source web site
    -f, --file [file name]   File name
    -h, --help               output usage information

$ node options.js -V
0.0.1

$ node options.js -s s -f f
s
f

$ node options.js -sf sf
true
sf
```

他也支援參數合併在一起`-sf`，如果有需要傳入東西的話，就需要再做而外的處理。

如果是可變的參數，你需要有不同的 keyword 讓他辨別不同的指令。

```js
// options2.js
var program = require('commander');

program
  .version('0.0.1')
  .command('keyword <keywork> [otherKeywords...]')
  .action(function (keyword, otherKeywords) {
    console.log('keyword %s', keyword);
    if (otherKeywords) {
      otherKeywords.forEach(function (oKey) {
        console.log('keyword %s', oKey);
      });
    }
  });

program.parse(process.argv);
```

```shell
$ node options2.js keyword one two three
keyword one
keyword two
keyword three
```

### The Ubiquitous Underscore

安裝 underscore：

```shell
npm install underscore
```

他提供很好用的 function

```js
var _ = require('underscore');
_.each(['apple','cherry'], function (fruit) { console.log(fruit); });
```

因為他使用`_`接住套件，跟 jQuery 的`$`很像，不過在 REPL 中的話`_`有其他意思，可以用其他的變數取代掉

```js
var us = require('underscore');
us.each(['apple','cherry'], function(fruit) { console.log(fruit); });
```

underscore 為 array、collection、function、objec、chaining 和一般操作提供了擴展功能。幸運的是，還有很棒的 [documentation](https://underscorejs.org/)

例子 mixin，他可以自定義字串合併，且有不同的 function name。

```js
> var us = require('underscore');
> us.mixin({
... betterWithNode: function(str) {
..... return str + ' is better with Node';
..... }
... });
> console.log(us.betterWithNode('chocolate'));
chocolate is better with Node
```

underscore 並不是最高使用率，還有另外一個是 [lodash](https://lodash.com/docs)。