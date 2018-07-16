# Node and ES6

[Node documentation ES6 的更動](https://nodejs.org/en/docs/es6/)

## Strict Mode

自從 ECMAScript 5 以後 Javascript 已經加入了的 strict mode，但他會直接影響到 ES6 功能的使用：

如果你在最前面加入這行的話，就會使用 strict mode：

```js
"use strict";
```

你可以使用單引號或雙引號。

還有其他方法可以使用 strict mode，像是`--strict_mode`的標籤，但我建議不要使用這個。因為強制使用 strict mode 可能會讓其他的套件產生錯誤，或是看不到的後過，在你想要使用的應用程式裡面使用就行了。

strict mode 會影響你的程式碼，如果你沒有在使用之前定義變數的話，他會跳出錯誤，或是 function 只能宣告一次，或是你不能在相同層級的`eval`裡面使用變數，等。但在這節想要表達的是，你不能在 strict mode 裡面使用 octal literals。

在之前的章節，設定檔案的權限，你不能使用 octal literal。

```js
// chap9-1.js
"use strict";

var fs = require('fs');

fs.open('./new.txt','a+',0o666, function(err, fd) {
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

如果你這邊使用：

```js
fs.open('./new.txt','a+',0666, function(err, fd) {
    ...
}
```

在 strict mode 下會跳出錯誤：

```shell
$ node chap9-1.js 
/Users/akiicat/Github/LearningNode2/chap9/chap9-1.js:5
fs.open('./new.txt','a+',0666, function(err, fd) {
                         ^^^^
```

但如果設定成`0o666`，strict mode 就能夠運行。

為了使用某些 ES6 extension，strict mode 是必要的。如果你想要使用 ES6 class，你就必須使用 strict mode。如果你想要使用`let`，你也必須使用 strict mode。

### Discussing Octal Literals

如果你想要探索 octal literal 的變化，推薦閱讀 [ES Discuss thread](https://esdiscuss.org/topic/octal-literals-have-their-uses-you-unix-haters-skip-this-one)

## let and const

在以前限制 Javascript 應用程式的是沒辦法在 block 層級裡面宣告變數。其中一個最受歡迎的 ES6 是`let` statement，有了`let`我們可以在 block 裡面宣告變數，而且他的範圍只在 block 裡，如果我們使用`var`的話，block 外面也可以存取：

```js
if (true) {
  var sum = 100;
}

console.log(sum);  // 100
```

```js
// chap9-2.js
"use strict";

if (true) {
  let sum = 100;
}

console.log(sum);  
```

```shell
$ node chap9-2.js 
/Users/akiicat/Github/LearningNode2/chap9/chap9-2.js:7
console.log(sum);
            ^
ReferenceError: sum is not defined
```

因為我們使用`let`所以應用程式必須使用 strict mode。(好像沒有也可以？)

除了 block-level 的範圍，`let`跟`var`不同的地方是參數宣告，在任何 statement 被執行前，`var`會 hoisted 在執行範圍的最上層。

```js
console.log(test);
var test;
```

印出來的結果會是`undefined`，不會有 runtime error 發生。

```js
"use strict";

console.log(test);
let test;
```

如果使用`let`的結果會產生錯誤：

```shell
console.log(test);
            ^
ReferenceError: test is not defined
```

你應該全部都使用`let`嗎？有些 programmer 會說要，有些人說不用。你也可以同時使用然後限制`var`變數的使用，變數必須應用程式或 function-level 的範圍，然後在 block-level 範圍只使用`let`。

從`let`繼續下去，`const` statement 的宣告只允許 read-only 的參照值。如果值是原始的值，那他就不會變了。如果值是物件，那你不能再指派新的物件或是其他的值，不過你可以改變物件裡面的屬性。

如果你試圖要改變`const`的值，他會失敗，而且保持沈默：(現在好像會跳出錯誤)

```js
const MY_VAL = 10;

MY_VAL = 100;

console.log(MY_VAL); // 10
```

```shell
MY_VAL = 100;
       ^

TypeError: Assignment to constant variable.
```

值得注意的是`const`是 value reference，如果你只派一個新的 array 或物件到`const`，你會改變物件或 array 內部的成員。(不過現在好像會跳出錯誤)

```js
// chap9-3.js
const test = ['one','two','three'];

const test2 = {apples : 1, peaches: 2};

// test = test2; //fails

test[0] = test2.peaches;

test2.apples = test[2];

console.log(test); // [ 2, 'two', 'three' ]
console.log(test2); // { apples: 'three', peaches: 2 }
```

```shell
$ node chap9-3.js 
[ 2, 'two', 'three' ]
{ apples: 'three', peaches: 2 }
```

不幸的是，他可能會讓我們搞混`const`的行為，你可以使用`Object.freeze()`讓物件不變。將第一行改成：

```js
const test = Object.freeze(['one','two','three']);
```

```shell
$ node chap9-3.js 
[ 'one', 'two', 'three' ]
{ apples: 'three', peaches: 2 }
```

當指派一個物件到 const 時無法預防他的屬性被修改，它可以意味著一定程度的語義，且這個語意用來告訴另一個 coder，這個項目不會在 reassigned 新的值了。

[const 深入討論](https://mathiasbynens.be/notes/es6-const)

`const`跟`let`一樣都只能在 block-level 範圍，但不像`let`需要 strict mode (現在 let 好像也不用)

## Arrow Functions

arrow function 有兩件事：首先，他提供簡單的符號。建立簡單的 server：

```js
http.createServer(function (req, res) {
    res.writeHead(200);
    res.write('Hello');
    res.end();
}).listen(8124);
```

使用 array function：

```js
http.createServer((req, res) => {
    res.writeHead(200);
    res.write('Hello');
    res.end();
}).listen(8124);
```

`function` 的字消失了，而 fat arrow `=>`箭頭代表匿名函數：

```js
var decArray = [23, 255, 122, 5, 16, 99];
var hexArray = decArray.map(function(element) {
    return element.toString(16);    
})

console.log(hexArray); // [ '17', 'ff', '7a', '5', '10', '63' ]
```

可以簡化成：

```js
 // chap9-4.js
var decArray = [23, 255, 122, 5, 16, 99];
var hexArray = decArray.map(element => element.toString(16));

console.log(hexArray); // ["17", "ff", "7a", "5", "10", "63"]
```

`return`也可以被省略。

array function 不只會省略符號，他還會重新定義`this`的符號。原本的 function 會定義他自己`this`的值。下面的範例會得到`undefined`：

```js
// chap9-5.js
function NewObj(name) {
   this.name = name;
}

NewObj.prototype.doLater = function() {
   setTimeout(function() {
      console.log(this.name);  // undefined
   }, 1000);
};

var obj = new NewObj('shelley');
obj.doLater();
```

這是因為`this`是定義在`setTimeout`裡面，他是另一個物件的 contructor。我們會拿到錯`setTimeout`的`this`。

把他傳入到`self`的參數傳入就可以了：

```js
// chap9-5.js
function NewObj(name) {
   this.name = name;
}

NewObj.prototype.doLater = function() {
   var self = this;
   setTimeout(function() {
      console.log(self.name); // 'shelley'
   }, 1000);
};

var obj = new NewObj('shelley');
obj.doLater();
```

arrow function 會讓`this`的值是在這個括號範圍內的值：

```js
function NewObj(name) {
   this.name = name;
}

NewObj.prototype.doLater = function() {
   setTimeout(() => {
      console.log(this.name); // 'shelley'
   }, 1000);
};

var obj = new NewObj('shelley');
obj.doLater();
```

array function 有一些詭異的用法，像是如何回傳空的物件還有參數：[這篇](https://developer.ibm.com/node/2015/09/21/an-introduction-to-javascript-es6-arrow-functions/)在討論這些詭異的方法，以及要如何運作。

## Classes

Javascript 現在支援 class。在第三張有是用 class 建立`InputChecker`：

```js
// example2-7.js
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
  let command = input.trim().substr(0,3);
  if (command == 'wr:') {
     this.emit('write',input.substr(3,input.length));
  } else if (command == 'en:') {
     this.emit('end');
  } else {
     this.emit('echo',input);
  }
};

...
```

他可以在物件裡面鑲嵌`check()`的 function，為了使用 ES6 的 class，要使用 strict mode。(好像不用加？)

不用再使用`util.inherits()`繼承 prototype 的方法，從 superconstructor 繼承到 contructor，這個新的 class 會拓展`EventEmitter`物件的方法。為了建立且初始化新的物件我家了 constructor，在 constructor 裡，我`super()`呼叫父層的 class。

```js
// class.js
'use strict';

const util = require('util');
const eventEmitter = require('events').EventEmitter;
const fs = require('fs');

class InputChecker extends eventEmitter {

   constructor(name, file) {
      super()
      this.name = name;

      this.writeStream = fs.createWriteStream('./' + file + '.txt',
         {'flags' : 'a',
          'encoding' : 'utf8',
          'mode' : 0o666});
      }

   check (input) {
      let command = input.toString().trim().substr(0,3);
      if (command == 'wr:') {
        this.emit('write',input.substr(3,input.length));
      } else if (command == 'en:') {
        this.emit('end');
      } else {
        this.emit('echo',input);
      }
   }
};

exports.InputChecker = InputChecker;
```

我們並沒有使用 prototype 物件加入`check()`方法，而是這個方法會直接包含在 class 裡面，定義`check()`並不用`var`跟`function`，你只需要定義方法。所有的邏輯運作都跟原本的物件一樣或非常接近了。

```js
// chap9-6.js
var InputChecker = require('./class').InputChecker;

// testing new object and event handling
var ic = new InputChecker('Shelley','output');

ic.on('write', function (data) {
   this.writeStream.write(data, 'utf8');
});
ic.addListener('echo', function( data) {
   console.log(this.name + ' wrote ' + data);
});

ic.on('end', function() {
   process.exit();
});

process.stdin.setEncoding('utf8');
process.stdin.on('data', function(input) {
    ic.check(input);
});
```

上面的程式碼，我沒有使用 strict mode，因為在套件裡面已經定義了 strict mode。

[class reference](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Classes)

## Promises with Bluebird

在早期階段的 Node，他們在討論要使用 callbakc 還是 promise，最後 callbakc 贏了。

promise 現在已經是 ES6 的一部分了，這邊會使用 promise 的套件 Bluebird。

另外一個使用 Bluebire 的原因是效能，[作者解釋](https://softwareengineering.stackexchange.com/questions/278778/why-are-native-es6-promises-slower-and-more-memory-intensive-than-bluebird)

不用在 nested callbacks，ES6 promise 功能是 braning，一個 branch 處理成功的分支，另一個 branch 處理失敗的分支。Node filesystem 的應用程式可以拿來做 demo，把它轉換成 promise 的結構。

開檔、讀檔、修改、然後寫到另一個檔案：

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

你可以看到這個結構有兩層，讀檔然後寫入檔案。

現在我們使用 Bluebird 簡化他。

我使用的是 Bluebire `promisifyAll()`的 function 他用來轉換成 promise。而不在使用`readFile()`，然後我們會使用`readFileAsync()`，這是一個支援 promies 的 funciton：

```js
// chap9-7.js
var promise = require('bluebird');
var fs = promise.promisifyAll(require('fs'));

fs.readFileAsync('./apples.txt','utf8')
   .then(function(data) {
      var adjData = data.replace(/apple/g, 'orange');
      return fs.writeFileAsync('./oranges.txt', adjData);
   })
   .catch(function(error) {
      console.error(error);
   });
```

在範例中，如果順利開檔的話會由`then()` funcion 來處理，發生錯誤的話會由`catch()`來處理。讀取成功後，會用呼叫`writeFile()`、`writeFileAsync()`寫入檔案，在前面的例子，我們知道`writeFile`會回傳錯誤，但現在錯誤會由`catch()` function 來處理。

雖然 nested function 並沒有很大，你可以看到 promise 版本的程式碼變得多乾淨，你也可以發現 nested 的問題被解決了，特別是不管有任何什麼問題、多少數量的呼叫，都會只有一個 error-handling 來解決。

那如果更複雜的問題呢？我修改下面的程式碼，加上了建立目錄，在程式碼你會有兩個`then()` function，建立資料夾是使用`mkdirAsync()`，會在程序的最後回傳，這是讓 promise 運作的關鍵，因為傳入他會被附加在下一個`then()` function 的前面。修該過後的資料會傳入 promise function 然後寫入。不管任何錯誤，都會由`catch()`來處理：

```js
// chap9-8.js
var promise = require('bluebird');
var fs = promise.promisifyAll(require('fs'));

fs.readFileAsync('./apples.txt','utf8')
   .then(function(data) {
      var adjData = data.replace(/apple/g, 'orange');
      return fs.mkdirAsync('./fruit/');
   })
   .then(function(adjData) {
      return fs.writeFileAsync('./fruit/oranges.txt', adjData);
   })
   .catch(function(error) {
      console.error(error);
   });
```

假如我們要處理資料夾裡面所有檔案的 array 時，像是當我們使用 File System function `readdir()`獲得資料夾內的內容，要該如何處理？

這裡使用 array-handling function 像是`map()`來處理，回傳的程式碼會轉換成 array，裡面的每個檔案都會被打開過，然後修改檔案內容，存到另一個資料夾下的相同檔名，裡面的 error 處理讀寫錯誤，外面的 error 處理資料夾的存取：

```js
// chap9-9.js
var promise = require('bluebird');
var fs = promise.promisifyAll(require('fs'));

fs.readdirAsync('./apples/').map(filename => {
   fs.readFileAsync('./apples/'+filename,'utf8')
      .then(function(data) {
         var adjData = data.replace(/apple/g, 'orange');
         return fs.writeFileAsync('./oranges/'+filename, adjData);
      })
      .catch(function(error) {
         console.error(error);
      })
   })
   .catch(function(error) {
      console.error(error);
   })

```

