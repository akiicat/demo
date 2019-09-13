# Node.js Essential Patterns

non-blocking 的方法幾乎可以保證它的效能會勝過 always-blocking 的情境。

這個章節會介紹 callback 跟 event emitter：

## The callback pattern

Callback 是 reactor pattern 的 handler 實體化。Callback 是 function，他會被用來呼叫和傳遞運算結果，這就是我們需要處理非同步的操作，他們會取代了會確實回傳 `return` 的指令。JavaScript 是代表 callback 最好的語言，因為 function 是首要的 class 物件，而且可以輕易指派變數、傳入參數、回傳 function 給另一個呼叫的 function、或儲存在 data structure 裡面。另一個理想實作 callback 的構造是 **closures**，有了 closures，我們可以參照 function 所建立的環境，不論 callback 什麼時候被呼叫，我們都能維持非同步運算的順序。

如果你要複習一下 closures：https://developer.mozilla.org/en-US/docs/Web/JavaScript/Closures

### The Continuation-passing style

在 JavaScript 裡面，callback 是一個 function，可以當作參數傳入到另一個 function，而且也可以當作結過 return 回來。在 functional programming 裡面，傳送結果的方法稱作為 **coninuation-passing style (CPS)**，這是觀念很普遍，function 不一定會跟非同步運算聯繫，實際上，他會把它當作參數傳到另一個 function (callback)，而不是直接回傳結果。

#### 同步的 Continuation-passing style

直接看例子會比較清楚這個概念：

```js
function add(a, b) { 
  return a + b; 
} 
```

這個在同步的寫法來說沒麼特別的，用 `return` 回傳值，這種方法又稱作 **direct style**，這個方法也等同於下面這個 continuation-passing style 的方法：

```js
function add(a, b, callback) { 
  callback(a + b); 
} 
```

這個 `add()` 是同步的 CPS function，當 callback 完成的時候，他會立即回傳值：

```js
console.log('before'); 
add(1, 2, result => console.log('Result: ' + result)); 
console.log('after'); 
```

輸出的結果：

```
before
Result: 3
after
```

#### 非同步 Continuation-passing style

這個 `add()` 是非同步：

```js
function additionAsync(a, b, callback) { 
  setTimeout(() => callback(a + b), 100); 
} 
```

我們使用 `setTimeout()` 模擬非同步 callback 的呼叫，運行順序如下：

```js
console.log('before'); 
additionAsync(1, 2, result => console.log('Result: ' + result)); 
console.log('after'); 
```

執行結果：

```js
before
after
Result: 3
```

因為 `setTimeout()` 會觸發非同步的操作，他不需要等待 callback 執行，他會立即的回傳結果，把控制權交回給 `additionAsync()`，這個在 Node.js 裡的特性非常關鍵，只要當非同步的請求送出時，他就會把控制權交回 event，允許 queue 裡面有新的事件被執行。

當非同步的運算完成時，他會開始執行 callback，執行是從 **Event Loop** 開始執行。

#### Non-continuation-passing style callbacks

在有些時候，把 callback 當作參數，會讓我們去猜說這個 function 是非同步還是使用 continuation-passing style：

```js
const result = [1, 5, 7].map(element => element - 1); 
console.log(result); // [0, 4, 6]
```

很明顯的，這個 callback 是用來 iterate 每一個 array 裡面的 element，他並不會把運算的結果當作參數傳入 callback。事實上，由於使用 direct style，他會立即回傳同步的結果，在 API 文件裡有寫說 callback 的意圖。

### 同步或非同步？

在同步與非同步之間，最大的影響是整個應用程式的正確性跟效率，接下來會看一些有陷阱在裡面的範例，應該要避免建立前後矛盾和易於混淆的應用程序。

#### An unpredictable function

其中一個危險的情況是一個 API 裡面，條件式的一邊是同步，另一邊是非同步：

```js
const fs = require('fs'); 
const cache = {}; 
function inconsistentRead(filename, callback) { 
  if(cache[filename]) { 
    //invoked synchronously 
    callback(cache[filename]);   
  } else { 
    //asynchronous function 
    fs.readFile(filename, 'utf8', (err, data) => { 
      cache[filename] = data; 
      callback(data); 
    }); 
  } 
} 
```

`cache` 變數會記住所有已經讀取的檔案，這只是個範例，我們並沒有做任何錯誤的處理，危險的地方是，在沒有 cache 的文件裡面 `fs.readFile()` 會非同步執行，但是有 cache 的文件就會以同步執行。

### Unleashing Zalgo

我們看一下要如何使用難以預測的 function，前面寫的 `inconsistentRead()` 這個 function：

```js
function createFileReader(filename) {
  const listeners = [];
  inconsistentRead(filename, value => {
    listeners.forEach(listener => listener(value));
  });

  return {
    onDataReady: listener => listeners.push(listener)
  };
} 
```

當前面的 function 被呼叫後，接著會建立一個含有 notifier 的物件，讓我們對每個檔案做讀取的操作，讀取之後，所有的 listener 會被同時呼叫。執行讀取的 function 是 `inconsistentRead()`，接著使用 `createFileReader()` function：

```js
const reader1 = createFileReader('data.txt'); 
reader1.onDataReady(data => { 
  console.log('First call data: ' + data); 
 
  //...sometime later we try to read again from 
  //the same file 
  const reader2 = createFileReader('data.txt'); 
  reader2.onDataReady( data => { 
    console.log('Second call data: ' + data); 
  }); 
}); 
```

輸出結果：

```shell
First call data: some data
```

如同你看到的，第二次的讀檔沒有被呼叫，原因有：

- 在建立 `reader1` 的時候，我們的 `inconsistentRead()` function 的行為是非同步的，因為我們沒有任何 cache，因此我們有時間註冊我們的 listener，當讀取結束之後，listener 會在 event loop 的下一個 cycle 被呼叫。
- Reader2 是在 event loop cycle 裡面被建立的，cache 裡面已經有同樣的檔案，因此，`inconsistentRead()` 會是同步執行的，所以他的 callback 將會立即被呼叫，也代表在呼叫 reader2 的時候會同步的呼叫，然而我們在建立 reader2 之後註冊 listener，所以 listener 不會被呼叫。

這個 `inconsistentRead()` function 非常的不穩定。

這個 bug 在現實的 application 裡面會變得非常複雜且難以辨識。

Zalgo 是網路用法用於形容不詳的東西：http://blog.izs.me/post/59142742143/designing-apis-for-asynchrony

### 使用同步 APIs

重上面的例子所學到的教訓，對於每個 API 需要明確的定義：不是同步就是非同步。

要修改這個 bug 的方法是讓 `inconsistentRead()` 這個 function 變成全部同步的，這是可型的，因為 Node.js 提供同步 I/O 的操作，舉例來說，我們可以使用 `fs.readFileSync()` 來替換掉非同步的部分：

```js
const fs = require('fs'); 
const cache = {}; 
function consistentReadSync(filename) { 
  if(cache[filename]) { 
    return cache[filename];   
  } else { 
    cache[filename] = fs.readFileSync(filename, 'utf8'); 
    return cache[filename]; 
  } 
} 
```

我們把整個 function 都轉換成 direct style，如果是同步的話，沒有什麼理由要使用 continuation-passing style。甚至我們可以說，實作同步 API 最好的方法是用 direct style 實現，不只不會被混淆，而且更有效率。

>  單純同步的 function 使用 direct style 會比較好

記住 API 從 CPS 改成 direct style，或是從非同步改成同步，或相反都需要將程式碼做改變，像是我們的例子，我們需要完全的把 `createFileReader()` API 的介面做改變，讓他能在同步的環境運行。

使用同步 API 有兩個注意事項：

- 對於某些特別的 function，不一定能使用同步的 API。
- 同步的 API 會 block 住 event loop，而且會暫停同時發出的請求，這會破壞 JavaScript concurrency 的模組，而且會讓應用程式變慢。

在這個 `consistentReadSync()` function 裡面，blocking event loop 的風險已經減輕了，因為每一次每個檔案會呼叫同步 I/O 的 API。在存取小量的檔案，這個 function 不會對 event loop 產生影響，但是萬一有很多檔案同時請求的時候呢？使用同步的 I/O 就不是個有效率的方法了。所以你可以看妳情況選擇你要用的方法。

### 延後執行

為了修正 `inconsistentRead()` function 的另一個方法就是讓他變非同步，這個手法會讓同步裡面的 callback 被安排在未來執行，而不是當下的 event loop 裡面執行，在 Node.js 我們可以使用 `process.nextTick()` 延後執行的 function，讓它下一個 event loop 執行，這個功能非常簡單，讓 callback 當作參數，而且一定會放到 event queue 後面，然後在任何 pending I/O event 之前，而且會立即回傳結果。這個 callback 會在 event loop 運行之後被呼叫。

```js
const fs = require('fs'); 
const cache = {}; 
function consistentReadAsync(filename, callback) { 
  if(cache[filename]) { 
    process.nextTick(() => callback(cache[filename])); 
  } else { 
    //asynchronous function 
    fs.readFile(filename, 'utf8', (err, data) => { 
      cache[filename] = data; 
      callback(data); 
    }); 
  } 
} 
```

現在，在我們設計的 function 裡面，我們保證 callback 是非同步執行的。

另一個延後執行的 API 是 `setImmediate()`，他的目的非常相似，但是語意不同。`process.nextTick()` 的 callback 會在任何其他 I/O event 前面執行，但是 `setImmediate()` 會在任何 I/O event 後面執行。因為 `process.nextTick()` 會在已經 scheduled I/O 前面執行，所以有可能會導致不確定的 I/O starvation，舉例像是 recursive invocation，但是  `setImmediate()` 就不會發生這個問題。

> 使用 `process.nextTick()`，我們能保證 callback 一定是非同步執行的。

## Node.js callback 慣例

在 Node.js 裡面，continuation-passing API 和 callback 會遵守一些慣例，這些慣例用在 Node.js core API 裡面，同樣的也可以套用在你的應用程式裡面，所以了解這些慣例非常重要。

### Callback 放在最後面

在標準的慣例裡面，我們會把 callback 放在參數的最後面：

```js
fs.readFile(filename, [options], callback) 
```

即使有選擇性的參數，也是會把 callback 放在最後面，因為如果放在適當的位置會比較容易閱讀。

### 錯誤放在最前面

使用 callback 的時候，error 可以是任何型態，

在 CPS 模式底下，錯誤會放在 callback 的第一個參數，第二個參數開始才是傳遞其他的值，如果執行過後沒有錯誤的話，那麼第一個參數應該會是 `null` 或是 `undefined`：

```js
fs.readFile('foo.txt', 'utf8', (err, data) => { 
  if(err) 
    handleError(err); 
  else 
    processData(data); 
}); 
```

最好的實作方法就是檢查當下的錯誤，如果什麼都不做的話，會讓我們更難 debug、更難發現 single point of failure，另外一個重要的慣例：讓錯誤的型態必須是 `Error`，代表錯誤的參數不應該是字串或是數字。

### 傳遞錯誤

Direct Style 在同步裡面傳遞錯誤的方法是用 `throw`，這會直接離開發生錯誤的地方，直到他被 caught。

在非同步的 CPS 裡面，正確傳遞錯誤的方法就是把錯誤傳給下一個 callback，看起來會像這樣：

```js
const fs = require('fs'); 
function readJSON(filename, callback) { 
  fs.readFile(filename, 'utf8', (err, data) => { 
    let parsed; 
    if(err) 
      //propagate the error and exit the current function 
      return callback(err); 

    try { 
      //parse the file contents 
      parsed = JSON.parse(data); 
    } catch(err) { 
      //catch parsing errors 
      return callback(err); 
    } 
    //no errors, propagate just the data 
    callback(null, parsed); 
  }); 
}; 
```

注意我們想要傳遞錯誤與傳遞結果的細節，而且注意如果發生錯誤的時候會使用 `return`，這樣才能確保 callback 被呼叫後會立即的結束這個 function。

### 沒有被接到的 Exceptions

你可能會發現我們在 `readFile()` 裡面使用 `try...catch` 的 block 來包住 `JSON.parse()`，為了避免在 `fs.readFile()` 丟出錯誤。在非同步的 callback 裡面丟出錯誤，會導致這個 exception 跳離 event loop，而且不會傳遞到下一個 callback。

在 Node.js 裡面，這是不可復原的，應用程式會用 `stderr` 印出錯誤後立即停止。要 demo 的話，直接移掉 `try...catch` 的 block：

```js
const fs = require('fs'); 
function readJSONThrows(filename, callback) { 
  fs.readFile(filename, 'utf8', (err, data) => { 
    if(err) { 
      return callback(err); 
    } 
    //no errors, propagate just the data 
    callback(null, JSON.parse(data)); 
  }); 
}; 
```

然後傳入一個不是 JSON 格式的檔案進去測試：

```js
readJSONThrows('nonJSON.txt', err => console.log(err)); 
```

執行結果大概會像是這樣：

```shell
$ node readJSON.js 
SyntaxError: Unexpected token h in JSON at position 1
    at JSON.parse (<anonymous>)
    at fs.readFile (/Users/.../readJSON.js:13:21)
    at FSReqWrap.readFileAfterClose [as oncomplete] (fs.js:511:3)
```

如果你看到 stack trace 的錯誤訊息，可以看到錯誤的起始點是在 `fs.js` module。透過 event loop，原先的 API 已經完整的獨到資料回傳給 `fs.readFile()`，這個錯誤訊息表達了 exception 會從我們的 callback 遍歷整個 stack，然後直接進入到 event loop 裡面，最後被 caught and thrown 印到 console 上。

這也代表我們使用 `try...catch` 包住 `readJSONThrows()` 的話會沒有用，因為在 block 裡面的 stack 跟我們 callback 的 stack 的呼叫方法是不一樣的。下面這個是我們描述的反例：

```js
try { 
  readJSONThrows('nonJSON.txt', function(err, result) { 
    //... 
  }); 
} catch(err) { 
  console.log('This will not catch the JSON parsing exception'); 
} 
```

這個 catch 永遠不會接到錯誤，因為 callback 是非同步執行的

我們還有辦法在程式執行之前，執行一些動作，實際上，Node.js 有一個特別的 event 叫做 `uncaughtException`，會在離開城市前執行：

```js
process.on('uncaughtException', (err) => { 
  console.error('This will catch at last the ' + 
    'JSON parsing exception: ' + err.message); 
  // Terminates the application with 1 (error) as exit code: 
  // without the following line, the application would continue 
  process.exit(1); 
}); 
```

需要知道這個的原因是因為，帶有無法預期的 exception 離開程式代表我們不能保證城市的一致性，這可能會導致不可預期的錯誤。例如，可能會有不完整的 I/O request 正在運行，或是讓 closure 變得不一致，所以建議在 production 裡面，在接收到未預期的 exception 之後，無論如何都要離開城市。

## The module system and its patterns

module 是應用程式中最基本的一環，他會把不需要揭露的 private function 跟 private variable 隱藏起來，然後在後面執行。

### The revealing module pattern

其中一個在 JavaScript 裡面最主要的問題是沒有 namespacing，程式碼會跑在 global 的環境下，你所使用的參數有可能會感染到別人所寫的程式碼，解決這個問題的其中一個方法就是 revealing module pattern，程式碼長得像這樣：

```js
const module = (() => { 
  const privateFoo = () => {...}; 
  const privateBar = []; 
 
  const exported = { 
    publicFoo: () => {...}, 
    publicBar: () => {...} 
  }; 
 
  return exported; 
})(); 
console.log(module);
```

這個 pattern 使用了自我呼叫的 function (self-invoking function) 會建立 private scope，然後只需要匯出公開的部分。module 變數帶著我們匯出的 API，其餘部分就不能透過外面存取。這個 pattern 成為 Node.js module system 的基礎。

### 解釋 Node.js module

CommonJS 這個團體目的是要把 JavaScript 標準化，其中一個最有名的提案是 **CommonJS modules**，Node.js 就是建立在這個標準之上，然後再加上一些擴充的東西。要描述他如何運作的話，可以用 revealing module pattern 比喻，每個 module 會運行在 private scope，定義在 local 裡面的參數才不會污染 global namespace。

#### 自己寫一個 module loader

要解釋他是怎麼運作，我們從零開始建立一個類似的系統，接下來的程式碼會建立一個類似 `require()` 的功能。

我們先建立一個載入 module 內容的 function，裡面的東西會是 private scope，然後再呼叫他：

```js
function loadModule(filename, module, require) { 
  const wrappedSrc=`(function(module, exports, require) { 
      ${fs.readFileSync(filename, 'utf8')} 
    })(module, module.exports, require);`; 
  eval(wrappedSrc); 
} 
```

這段 module 的原始碼，實質上只是把一個 function 包進去。不同的點是他還有傳入一些參數：`module` ,`exports`, `require`。這邊可以注意的地方是，為什麼我們要把參數 `module.exports` 包裝成  `exports`，後面會講到，先記著一下。

這邊只是個例子，在真實的應用程式裡，很少你會需要自己 `eval()` 自己的程式碼。有一些功能像是 `eval()` 或是其他的 `vm` module 的 function (https://nodejs.org/api/vm.html)，使用上很容易會以錯誤的方式來寫，或是錯誤的 input，而且很有可能會讓系統產生 code injection 的攻擊。所以在使用的時候要非常的小心，或是避免使用它們。

接下來是實作 `require()` 的變數：

```js
const require = (moduleName) => { 
  console.log(`Require invoked for module: ${moduleName}`); 
  const id = require.resolve(moduleName);      //[1] 
  if(require.cache[id]) {                      //[2] 
    return require.cache[id].exports; 
  } 
 
  //module metadata 
  const module = {                             //[3] 
    exports: {}, 
    id: id 
  }; 
  //Update the cache 
  require.cache[id] = module;                  //[4] 
 
  //load the module 
  loadModule(id, module, require);             //[5] 
 
  //return exported variables 
  return module.exports;                       //[6] 
}; 
require.cache = {}; 
require.resolve = (moduleName) => { 
  /* resolve a full module id from the moduleName */ 
}; 
```

上面的程式碼，模擬了 `require()` 用來載入 module 的功能，上面的只是教學用的，並不是現實使用的 `require()`，不過對於知道 Node.js 的 module 是怎麼運作的、怎麼定義的、怎麼載入的，已經算很夠用了。

1. 首先，我們要先獲得 module 的全名，我們會稱這個全名叫做 `id`。`require.resolve()` 所使用的是 resolving algorithm。
2. 如果這個 module 已經被載入了，就會透過 cache 裡面的紀錄，立即回傳。
3. 如果這個 module 還沒有被載入，我們就會建立一個 `module` 的物件，裡面包含著空的 `exports` 屬性，在程式碼內的這個屬性，將會被匯出成 public API。
4. 然後把 `module` 加入 cache
5. 透過我們前面寫的 `loadModule()`，程式碼將會被 `eval()`，我們會傳入剛剛所建立空的 `module` 物件給 `require()`，透過操作和取代 `module.exports` 這個物件，我們會把它匯出成 public API。
6. 最後 `module.exports` 會包含著 module 的 public API 回傳給 caller。

如同你看到的，Node.js module system 後面的運作並沒有什麼魔法。裡面所有東西只是包裝我們建立的原始碼，跟包裝可以運行的環境。

#### 定義 module

在看我們自己寫的 `require()` 之前，我們要先知道如何定義一個 module：

```js
//load another dependency 
const dependency = require('./anotherModule'); 
 
//a private function 
function log() { 
  console.log(`Well done ${dependency.username}`); 
} 
 
//the API to be exported for public use 
module.exports.run = () => { 
  log(); 
}; 
```

基本要去記得的概念是在 module 裡面的所有東西都是 private 的，除非他把東西指派給變數 `module.exports`，這個變數的內容會被 `require()` 載入的時候 cache 住，然後回傳內容。

#### 定義 global

還有另一個參數的是 `global`，把東西指派到這個參數會自動變成全域範圍。注意，使用 global 的寫法的並不太好，很容易污染到程式碼，除非你知道要做什麼，不然盡量避免使用全域變數。

#### Module.exports vs exports

對於一些還不熟悉 Node.js 的人會疑惑 `module.exports` 和 `exports` 兩個之間有什麼不同，不過從前面的範例已經解釋得很清楚了 `exports` 只是 `module.exports` 的縮寫而已，本質上也只是一個 object。

所以我們可以把東西加入到這個 object 裡面：

```js
exports.hello = () => { 
  console.log('Hello'); 
} 
```

重新指派 `exports` 參數的話並不會有任何影響，因為他並不會改變 `module.exports` 的變數，他只會把參數指派給自己，所以這個方法是錯的：

```js
exports = () => { 
  console.log('Hello'); 
} 
```

如果我們只想要匯出一個 function、object 或是字串等等，必須重新指派 `module.exports` 變數：

```js
module.exports = () => { 
  console.log('Hello'); 
} 
```

#### Require function 是同步的

另一個重要的細節是，我們自己寫的 `require()` 是同步的，事實上，他會使用 simple direct style 回傳 module 的內容，而且不需要 callback，在真實 Node.js 的 `require()` function 也是如此，所以當我們指派給 `module.exports` 也必須是同步的，像是這段程式碼就是錯誤的：

```js
setTimeout(() => { 
  module.exports = function() {...}; 
}, 100); 
```

這個特性影響了我們在定義 module 的時候只能使用同步的方式，也是為什麼 core Node.js libraries 提供同步的 API，取代大多數人在使用的非同步。

如果要非同步 export 的 module 的話，到第九章 *Advanced Asynchronous Recipes* 再說。

曾今 Node.js 有過 `require()` 可以是非同步的，不過很快地就被移除了，因為比起其他的優勢來說，非同步的 `require()` 增加了複雜度以及初始化的時間。

#### The resolving algorithm

術語相依性地獄 *dependency hell* 是指應用程式在互相相依的狀態，但需要兼容不同的版本。藉由不同位置的 module 載入不同的版本的 modele，Node.js 優雅地解決了這個問題。這個功能用在 npm，同樣的也用在 `require` function 裡面的  resolving algorithm。

`resolve()` 需要輸入 module name (這邊叫做 `moduleName`) 當作輸入，他會回傳 module 的全名。用來載入程式碼跟便是唯一一個檔案都需要這個路徑。resolving algorithm 可以簡單地分成三個部分：

- **File modules**：如果 `moduleName` 是從 `/` 開始的話，代表他已經是絕對路徑了，所以直接回傳就可以了。如果是從 `./` 開始的話，會從當前的位置開始計算相對路徑。
- **Core modules**：如果 `moduleName` 不是從 `/` 或是 `./` 開始的話，那這個演算法會試著從 core Node.js modules 開始找。
- **Package modules**：如果一直都沒有找到的話，他會找當前路徑下是否有 `node_modules` 的資料夾，然後看裡面是否有我們需要的 module，如果沒有就會往上一層資料夾找是否有 `node_modules`，直到 root 路徑為止。

對於 `moduleName` 會對應以下的檔案：

- `<moduleName>.js`
- `<moduleName>/index.js`
- `<moduleName>/package.js` 裡面的 `main` 屬性的值

完整的 resolving algorithm 可以在這裡被找到：https://nodejs.org/api/modules.html#modules_all_together

`node_modules` 是 npm 安裝依賴套件的地方，每個 package 可以有他自己依賴 private 的套件，結構會像是這樣：

```
myApp 
├── foo.js 
└── node_modules 
    ├── depA 
    │   └── index.js 
    ├── depB 
    │   ├── bar.js 
    │   └── node_modules 
    │       └── depA 
    │           └── index.js 
    └── depC 
        ├── foobar.js 
        └── node_modules 
            └── depA 
                └── index.js 
```

像上面的例子，`myApp`, `depB`, `depC` 都需要依賴 `depA`，然而他們都有自己私人的版本，因為  `require('depA')` 是使用 resolving algorithm 在不同地方來獲得不同的 module。

resolving algorithm 是 Node.js 依賴管理的強大之處，而且能夠處理數百或是數千個 package，而且不會發生衝突或版本不相容的問題。

當我們在呼叫 `require()` 的時候就會使用到 resolving algorithm，然後如果需要的直接拿取某個 module 也可以呼叫 `require.resolve()`。

#### The module cache

每個 module 在第一次使用的時候會被載入和執行，之後如果一連串 `require()` 就會回傳 cached 的版本，從我們剛剛自己建立的 `require()` 裡面就能很清楚的看到，caching 對於效能來說是非常關鍵的，而且還有其他重要的意義：

- 這能夠讓 module 在互相依賴有循環的關係。
- 而且保證，相同的 modeul 會回傳相同的內容。

透過 `require.cache` 的參數，可以看到 module 的 cache，所以如果需要的話能夠直接存取它。如果你要刪除一個 cache 最常見的方法是，直接刪掉 `require.cache` 裡面的 key，在測試的時候非常好用，不過在正常環境裡使用的話會非常危險。

#### 循環依賴

很多人可能會注意到有循環依賴設計上的問題，在真實的應用程式裡，這是很有可能發生的，所以讓我們知道在 Node.js 裡面是怎麼運作的，會非常的有用。不過如果有看過我們寫的 `require()`，應該就會知道他是如何運作的，以及需要注意什麼。

假設我們有兩個 module 如下：

```js
// a.js
exports.loaded = false; 
const b = require('./b'); 
module.exports = { 
  bWasLoaded: b.loaded, 
  loaded: true 
}; 
```

```js
// b.js
exports.loaded = false; 
const a = require('./a'); 
module.exports = { 
  aWasLoaded: a.loaded, 
  loaded: true 
}; 
```

現在我們從 `main.js` 分別載入這兩個 module：

```js
const a = require('./a'); 
const b = require('./b'); 
console.log(a); 
console.log(b); 
```

前面的程式碼會印出這樣的結果：

```shell
$ node main.js 
{ bWasLoaded: true, loaded: true }
{ aWasLoaded: false, loaded: true }
```

這個結果顯示了循環依賴需要注意的地方。 當兩個 module 在 `main.js` 裡完全載入的時候，這時 `main.js` 在載入 `b.js` 的時候並不會完全載入 `a.js`，實際上，他的狀態已經是被 require 的狀態了。要檢驗他的話，可以在 `main.js` 裡面交換兩個 modules 的順序來驗證。

如果你有試過的話，你會發現順序是相反。不過在很大的程式裡面，會很難知道哪個 module 先載入。

## Module definition patterns

除了載入依賴的功能之外，Module system 也能是用來定義 API 的工具，對於設計 API 所產生的問題，最主要的因素是要平衡 private 跟 public 的功能。目標是用 information hiding, extensibility, code reuse 來增加程式碼的品質。

#### Named Exports

大多數揭露 public API 的方法是 **named exports**，他會包含我們想要變成 public 的屬性或物件，透過 `exports` 或是 `module.exports` 的方法，這樣匯出的物件會變成 container 或是 namespace，裡面會是相關的功能：

```js
// logger.js 
exports.info = (message) => { 
  console.log('info: ' + message); 
}; 
 
exports.verbose = (message) => { 
  console.log('verbose: ' + message); 
}; 
```

```js
// main.js 
const logger = require('./logger'); 
logger.info('This is an informational message'); 
logger.verbose('This is a verbose message'); 
```

CommonJS 規範允許使用 `exports` 揭露 public 的成員。因為 named exports 是跟 CommonJS 規範最接近的模式。後面會說明 `module.exports` 的使用。

### Exporting A Function

其中一個最流行定義 module patterns 的方式是讓 `modules.exports` 這個參數重新指派成一個 function，他優勢的地方是他只提供單一個功能，這讓 module 很容易了解和使用，而且他同時也遵守了 *small surface* 的原則，這種定義 module 的方式又被稱作為 **substack pattern**：

```js
//file logger.js 
module.exports = (message) => { 
  console.log(`info: ${message}`); 
}; 
```

這個 pattern 能夠把匯出的 function 當作 namespace，這是非常強大的功能，因為他給 module 單一而且明確的進入點。這個方法也可以匯出第二個或更多個 function：

```js
module.exports.verbose = (message) => { 
  console.log(`verbose: ${message}`); 
}; 
```

使用我們剛剛寫的程式：

```js
//file main.js 
const logger = require('./logger'); 
logger('This is an informational message'); 
logger.verbose('This is a verbose message'); 
```

雖然只匯出一個 function 看起來限制很多，但實際上，對大多數的 module，這個方法很適合強調單一功能，同時第二層的功能更不容易看見。Node.js 的模塊化鼓勵使用 **Single Responsibility Principle** (**SRP**)，每個 module 都有他自己單一的功能，而且每個 function 功能應該要被 module 封裝起來。

#### Pattern (substack)

會出一個 module 主要的功能，其他輔助的功能會在 function 的 namespace 裡面匯出。

### Exporting A Constructor

匯出 constructor 的 module 可以想像成匯出一個特殊 function 的 module。不同的地方是，我們允許使用者使用 constructor 建立一個新的 instance，而且有能夠新增 prototype 和產生新的 classes：

```js
//file logger.js 
function Logger(name) { 
  this.name = name; 
} 
 
Logger.prototype.log = function(message) { 
  console.log(`[${this.name}] ${message}`); 
}; 
 
Logger.prototype.info = function(message) { 
  this.log(`info: ${message}`); 
}; 
 
Logger.prototype.verbose = function(message) { 
  this.log(`verbose: ${message}`); 
}; 
 
module.exports = Logger; 
```

使用上面的 code：

```js
//file main.js 
const Logger = require('./logger'); 
const dbLogger = new Logger('DB'); 
dbLogger.info('This is an informational message'); 
const accessLogger = new Logger('ACCESS'); 
accessLogger.verbose('This is a verbose message'); 
```

或是使用 ES2015 匯出 class：

```js
class Logger { 
  constructor(name) { 
    this.name = name; 
  } 
 
  log(message) { 
    console.log(`[${this.name}] ${message}`); 
  } 
 
  info(message) { 
    this.log(`info: ${message}`); 
  } 
 
  verbose(message) { 
    this.log(`verbose: ${message}`); 
  } 
} 
 
module.exports = Logger; 
```

ES2015 的 class 只是 prototype 的糖衣語法，class module 的使用方法跟 prototype-based 的 module 完全一樣。

匯出 constructor 或 class 仍然會提供 module 單一的進入點，但它會比於 substack pattern 匯出更多 module 內部的東西，在拓展功能的時候也會比較有力。

這個 pattern 的變形，可以在呼叫的時候，不用使用 `new` 這個指令：

```js
function Logger(name) { 
  if(!(this instanceof Logger)) { 
    return new Logger(name); 
  } 
  this.name = name; 
}; 
```

這個小伎倆很簡單：他會檢查 `this` 是否存在而且 `this` 是否是 Logger 的 instance。如果結果是 false，代表他在乎叫 Logger() 的時候沒有使用 `new` ，所以我們會建立 `new` 的 instance，然後回傳。所以可以這樣使用：

```js
//file logger.js 
const Logger = require('./logger'); 
const dbLogger = Logger('DB'); 
accessLogger.verbose('This is a verbose message'); 
```

ES2015 有一個比較乾淨的方法，`new.target` 這個語法是從 Node.js 6 版本開始的。如果 function 使用 `new` 這個字呼叫的話，`new.target` 的屬性是在 function 裡面的 meta property，而且在運行的時候會被判斷為 true：

```js
function Logger(name) { 
  if(!new.target) { 
    return new LoggerConstructor(name); 
  } 
  this.name = name; 
} 
```

這個 code 的跟前一個相同，`new.target` 這個也是 ES2015 的糖衣語法。

### Exporting an Instance

利用 `require()` cache 的機制，可以間單的從 constructor 或 factory 建立有狀態的 instance，可以傳入不同的 modules 之間：

```js
//file logger.js 
function Logger(name) { 
  this.count = 0; 
  this.name = name; 
} 
Logger.prototype.log = function(message) { 
  this.count++; 
  console.log('[' + this.name + '] ' + message); 
}; 
module.exports = new Logger('DEFAULT'); 
```

可以這樣使用：

```js
//file main.js 
const logger = require('./logger'); 
logger.log('This is an informational message'); 
```

因為 module 已經被 cache 了，所以需要 `logger` 的每個 module 都會共享物件相同的狀態









