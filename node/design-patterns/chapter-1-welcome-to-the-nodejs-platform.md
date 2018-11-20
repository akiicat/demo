# Welcome to the Node.js Platform

下載範例[程式碼](https://github.com/PacktPublishing/Node.js_Design_Patterns_Second_Edition_Code)

## Node.js philosophy

你可以到[這裡](https://en.wikipedia.org/wiki/List_of_software_development_philosophies)看所有軟體開發的原則

### Small core

Node.js 有一個最小的功能集，在功能集外面其餘的部分稱作為 **userland** 或是 **userspace**，這麼一來可以快速的實驗且演化新的功能，而且讓功能集保持在最小的狀況下，也能方便的維護。

### Small modules

小 module 的原則是從 Unix 系統來的，有兩個原則：

- Small is beautiful
- Make each program do one thing well

小 module 通常會遵守：

- Easier to understand and use
- Simpler to test and maintain
- Perfect to share with the browser

這樣可以讓小的程式碼容易重複使用且分享給其他人，全新的 **Don't Repeat Yourself (DRY)** 的原則

### Small surface area

把小 module 中裡面最常使用有功能 expose 給外面的人使用，不僅更加清楚這個 API 的功能，而且可以重複使用，

### Simplicity and pragmatism

你有聽過 **Keep It Simple, Stupid (KISS)** 的原則：

> "Simplicity is the ultimate sophistication" -- Leonardo da Vinci

Richard P. Gabriel 是創照出了這個詞 "worse is better" 來描述 model，因此在設計越少越簡單的功能是一個很好的選擇

> "The design must be simple, both in implementation and interface. It is more important for the implementation to be simple than the interface. Simplicity is the most important consideration in a design."

設計簡單會讓我們很好維護與了解。

這個原則不只在 Node.js 還可以用在 JavaScript。使用簡單的 function, object, class 在現實生活中很常見，在純的物件導向中，會把現實世界的問題認為是完美的來解決，但真實的世界是，我們只是在把軟體趨近於現實，我們很快就會遇到更複雜的問題，很難再完美的建立龐大的程式碼，與之後的維護。

## ES6 語法

在使用之前要在前面加上 `use strict`，不然有些會出錯，後面都不會加上這行，你要自己加。

因為它定義在 ECMAScript 2015 標準裡面，簡寫成 ES2015 或是 ES6，讓 JavaScript 變得更加彈性且更有樂趣。

後面使用的 Node.js 版本是 version 6。

### Let 跟 Const

在以前，Javascript 只提供 function scope 跟 global scope 來控制生命週期和可用的變數，舉例來說，如果你在 `if` 判斷式裡面定義一個參數，在判斷式的外面一樣可以被存取，不管這行有沒有執行：

```js
if (false) { 
   var x = "hello"; 
} 
console.log(x); 
```

這樣執行的話不會發生任何錯誤，只會獲得 `undefined` 的結果，這個行為也導致了很多的問題發生，所以 ES2015 為什麼要創造 `let` 這個關鍵字，讓變數只在 scope 裡面。我們把 `var` 改成 `let`：

```js
if (false) { 
   let x = "hello"; 
} 
console.log(x); 
```

你會獲得 `ReferenceError: x is not defined` 這個錯誤，因為我們試圖獲得被定義在另一個 block 的變數。

讓 `let` 更有意義的用法，是在 loop 裡面定義暫時的變數：

```js
for (let i=0; i < 10; i++) { 
  // do something here 
} 
console.log(i); 
```

在上面的例子我們會獲得 `ReferenceError: i is not defined` 的錯誤，`let` 讓我們更安全的程式碼，因為我們不想要存取其他的 scope 裡面的變數，而且我們可以得知錯誤訊息，避免淺在的 bug。

ES2015 也介紹了 `const` 這個關鍵字，讓我們宣告常數：

```js
const x = 'This will never change'; 
x = '...'; 
```

試圖改變常數的話會獲得 `TypeError: Assignment to constant variable.` 的錯誤。

另外你可以這樣用：

```js
const x = {}; 
x.name = 'John'; 
```

我們更改的是 object 裡面的屬性，並不是取代換掉新的 object，所以不會發生錯誤，不果改成這樣，重新綁定新的值給變數的話，就會發生錯誤了：

```js
x = null; // This will fail
```

用在保護一個變數時使用，防止這個變數重新宣告：

```js
const path = require('path'); 
// .. do stuff with the path module 
let path = './some/path'; // this will fail 
```

如果你想要建立一個永遠不變的 object，這些方法還不夠，你需要使用到 ES5 的 `Object.freeze()` 的[方法](https://developer.mozilla.org/it/docs/Web/JavaScript/Reference/Global_Objects/Object/freeze) ，或是 `deep-freeze` [module](https://www.npmjs.com/package/deep-freeze)。

### Array function

這是一個 callback 變得簡潔的 function，像是我們 filter 的範例：

```js
const numbers = [2, 6, 7, 8, 1]; 
const even = numbers.filter(function(x) { 
  return x%2 === 0; 
});
```

可以寫成 array function 的語法：

```js
const numbers = [2, 6, 7, 8, 1]; 
const even = numbers.filter(x => x%2 === 0); 
```

只留下參數跟代表 function 的 `=>` 箭頭符號，有多個參數的話，你必須要用小括號刮起來，用逗點隔開，如果沒有參數的話，也需要寫小括號 `() => {...}`，如果你程式只有一行的話，不需要大括號跟 `return`，多行的話兩個都要寫：

```js
const numbers = [2, 6, 7, 8, 1]; 
const even = numbers.filter(x => { 
  if (x%2 === 0) { 
    console.log(x + ' is even!'); 
    return true; 
  } 
}); 
```

另一個功能是，箭頭符號會自動綁定 `this`，像是：

```js
function DelayedGreeter(name) { 
  this.name = name; 
} 
 
DelayedGreeter.prototype.greet = function() { 
  setTimeout( function cb() { 
    console.log('Hello ' + this.name); 
  }, 500); 
}; 
 
const greeter = new DelayedGreeter('World'); 
greeter.greet(); // will print "Hello undefined" 
```

他會印出 `undefined`，因為在 `greet` 的 scope 跟 `function cb()` 的 scope 不同，所以他會獲得 `undefiend`，在以前唯一能修改這個問題的方法只有使用 `bind` 方法來綁定：

```js
DelayedGreeter.prototype.greet = function() { 
  setTimeout( (function cb() { 
    console.log('Hello' + this.name); 
  }).bind(this), 500); 
}; 
```

但因為有箭頭 function，他會包含在同一個詞彙範圍裡面 lexical scope，我們可以直接使用箭頭 function 當作 callback，來解決這個方法：

```js
DelayedGreeter.prototype.greet = function() { 
  setTimeout( () => console.log('Hello' + this.name), 500); 
}; 
```

這是一個很便利的功能，也讓程式碼變得更簡潔也更直接。

### Class 語法

ES2015 引用了一個新的語法，來利用原型繼承，讓更多從物件導向來的開發者像是 Java 或 C# 能更熟悉語法。這個新的語法並沒有改變原有 JavaScript 運行的功能，他仍然是使用 function 的 protoype，對開發者來說只是一個便利且易讀的語法，最重要且需要了解的是它只是個糖衣語法。

首先是使用 prototype-based 方法的 `Person` function：

```js
function Person(name, surname, age) { 
  this.name = name; 
  this.surname = surname; 
  this.age = age; 
} 
 
Person.prototype.getFullName = function() { 
  return this.name + '' + this.surname; 
}; 
 
Person.older = function(person1, person2) { 
  return (person1.age >= person2.age) ? person1 : person2; 
}; 
```

我們寫了一個 prototype function 可以讓我們更簡單的拿到 person 的全名，和一個可以比較兩個人年紀的 `older` function，如果使用 ES2015 的語法：

```js
class Person {
  constructor (name, surname, age) {
    this.name = name;
    this.surname = surname;
    this.age = age;
  }

  getFullName () {
    return this.name + ' ' + this.surname;
  }

  static older (person1, person2) {
    return (person1.age >= person2.age) ? person1 : person2;
  }
}
```

會變得更容易閱讀且更直接，且更明確的說明哪裡是 `constructor` 跟 `static` 的 `older` function。

這兩個可以互相變化，但另一個功能是，新的語法能夠使用 `extend` 擴增 `Person` 的 prototype 還有 `super` 這個字：

```js
class PersonWithMiddlename extends Person { 
  constructor (name, middlename, surname, age) { 
    super(name, surname, age); 
    this.middlename = middlename; 
  } 
 
  getFullName () { 
    return this.name + '' + this.middlename + '' + this.surname; 
  } 
} 
```

值得注意的是，這個行為跟其他物件導向的非常相似，我們可以 extend 一個 class，然後使用 `super` 去呼叫 parent 的 constructor，跟複寫 `getFullName` 的方法。

### 增強 Object literals

ES2015 增強的文字的語法，比較在 object 裡面比較方便指派變數的方法，還有 setter 跟 getter 的方法。

```js
const x = 22; 
const y = 17; 
const obj = { x, y }; 
```

`x` 跟 `y` 分別是 `22` 跟 `17`，相對的也可以使用 function：

```js
module.exports = { 
  square (x) { 
    return x * x; 
  }, 
  cube (x) { 
    return x * x * x; 
  } 
}; 
```

這個例子可以計算屬性的名字：

```js
const namespace = '-webkit-'; 
const style = { 
  [namespace + 'box-sizing'] : 'border-box', 
  [namespace + 'box-shadow'] : '10px10px5px #888888' 
}; 
```

這樣可以分別使用 `-webkit-box-sizing` 跟 `-webkit-box-shadow`。

setter 跟 getter 的使用方法，直接看例子：

```js
const person = { 
  name : 'George', 
  surname : 'Boole', 
 
  get fullname () { 
    return this.name + '' + this.surname; 
  }, 
 
  set fullname (fullname) { 
    let parts = fullname.split(''); 
    this.name = parts[0]; 
    this.surname = parts[1]; 
  } 
}; 
 
console.log(person.fullname); // "George Boole" 
console.log(person.fullname = 'Alan Turing'); // "Alan Turing" 
console.log(person.name); // "Alan" 
```

這便定義了三個變數 `name`, `surface`, `fullname`，你可以計算屬性值，像是使用方法像是正常的屬性一樣。

### Map 和 Set collections

多了新的 `Map` prototype，可以更安全、更彈性、更直觀的使用：

```js
const profiles = new Map(); 
profiles.set('twitter', '@adalovelace'); 
profiles.set('facebook', 'adalovelace'); 
profiles.set('googleplus', 'ada'); 
 
profiles.size; // 3 
profiles.has('twitter'); // true 
profiles.get('twitter'); // "@adalovelace" 
profiles.has('youtube'); // false 
profiles.delete('facebook'); 
profiles.has('facebook'); // false 
profiles.get('facebook'); // undefined 
for (const entry of profiles) { 
  console.log(entry); 
} 
```

它提供一些方便的方法，像是 `set`, `get`, `has`, `delete`, `size`，我們可以使用 `for...of` 語法，拿到每一個 entry，每一個 entry 會有第一個屬性跟值，這個介面非常直觀。

有趣的是，我們可以把 function 當作 Map 的 key，在一般的物件幾乎不可能達成，因為大部分的 key 會把它轉換成字串，這個是一個新的功能，像是，我們可以建立 micro testing framework：

```js
const tests = new Map(); 
tests.set(() => 2+2, 4); 
tests.set(() => 2*2, 4); 
tests.set(() => 2/2, 1); 
 
for (const entry of tests) { 
  console.log((entry[0]() === entry[1]) ? 'PASS' : 'FAIL'); 
} 
```

如同你所見的，我們把 function 當作 key，把預期的值當作 value，順序是以他們 insert 的順序，不能保證他會有一定的順序。

`Set` prototype 讓我們建構有順序且唯一的 values。

```js
const s = new Set([0, 1, 2, 3]); 
s.add(3); // will not be added 
s.size; // 4 
s.delete(0); 
s.has(0); // false 
 
for (const entry of s) { 
  console.log(entry); 
} 
```

這個跟 `Map` 很像，有 `add`, `has`, `delete`, `size` 的方法，每個 entry 都是一個值，同樣的，裡面可以放 object 跟 function。

### WeakMap 和 WeakSet collections

ES2015 定義了 weak 版本的 `Map` 和 `Set` 的 prototype，分別叫做 `WeakMap` 跟 `WeakSet`。

`WeakＭap` 跟 `Map` 有兩個不同的地方：

- 你沒辦法 iterate 整個 entries
- 他只允許 object 當作 keys

不同的功能是，當你沒有東西指到這個參數上的時候，他就會清掉裡面的東西 garbage collect。這個東西通常用來儲存 metadata，然後在正常運行的時候會刪除的物件：

```js
let obj = {}; 
const map = new WeakMap(); 
map.set(obj, {key: "some_value"}); 
console.log(map.get(obj)); // {key: "some_value"} 
obj = undefined; // now obj and the associated data in the map  
                 // will be cleaned up in the next gc cycle 
```

可以透過 `map.get(obj)` 獲得值，最後清理指派新的值給 object，就會 garbage collect 裡面的值。

WeakSet 同樣只能存在 object 裡面跟不能被 iterated，而且也會 garbage collect。

```js
let obj1= {key: "val1"}; 
let obj2= {key: "val2"}; 
const set= new WeakSet([obj1, obj2]); 
console.log(set.has(obj1)); // true 
obj1= undefined; // now obj1 will be removed from the set 
console.log(set.has(obj1)); // false 
```

`WeakMap`, `WeakSet` 跟 `Map`, `Set` 並沒有誰比較好，只是看你用在哪個地方。

### Template literals

新的符號 **`** 來建立 string，最大的不同點是，你可以在字串裡面使用 `${expression}` 插入表達式，而且字串可以重複的被使用很多次

```js
const name = "Leonardo"; 
const interests = ["arts", "architecture", "science", "music",  
                   "mathematics"]; 
const birth = { year : 1452, place : 'Florence' }; 
const text = `${name} was an Italian polymath
 interested in many topics such as
 ${interests.join(', ')}.He was born
 in ${birth.year} in ${birth.place}.`; 
console.log(text); 
```

最後他印出來的字會像是這樣：

```txt
Leonardo was an Italian polymath
 interested in many topics such as
 arts, architecture, science, music, mathematics.He was born
 in 1452 in Florence.
```

### 其他 ES2015 的功能

- Promise (Chapter 4, *Asynchronous Control Flow Patterns with ES2015*)
- Default function parameters
- Rest parameters
- Spread operator
- Destructuring
- `new.target` (we will talk about this in Chapter 2, *Node.js Essential Patterns*)
- Proxy (we will talk about this in Chapter 6, *Design Patterns*)
- Reflect
- Symbols

更多新的功能可以參考：

https://nodejs.org/en/docs/es6/

## The reactor pattern

reactor pattern 是 Node.js 非同步的核心，我們會經過 pattern 後的觀念，像是 single-threaded 的架構、non-blocking I/O，我們會看到整個 Node.js 是怎麼建構的。

### I/O is slow

在基本的所有電腦的運算裡面，I/O 是最慢的，存取 RAM 的速度是 nanoseconds (10E-9 秒)，存取磁碟或網路上的資料是 milliseconds (10E-3 秒)。傳送速度也是有差的，RAM 可以到 GB/s，磁碟或是網路只能到 MB/s 優化可以到 GB/s。在 CPU 裡面，I/O 運算不昂貴，但送出請求跟完成運算之間會有延遲，而且還要考慮到人為的因素，會比存取磁碟的速度慢上很多。

### Blocking I/O

在傳統的 blocking I/O 程式裡面，關於 I/O 請求的 function call 會阻止正在執行的 thread，直到運算完成，這可能只需要幾個 milliseconds，或是磁碟存取，甚至使用者行為需要更長的時間。這個 pseudocode 會 blocking thread 不是 blocking socket：

```pseudocode
//blocks the thread until the data is available 
data = socket.read(); 
//data is available 
print(data); 
```

web server 在同一個 thread 裡面裡面使用 blocking I/O 處理多個 connection 是很瑣碎的事，因為每個在 socket 上的 I/O 運算都會阻擋到其他的程序，所以在傳統並行處理的方法是新開一個 thread 或 process (或是重複使用同一個 pool)，如此一來，即使有一個 I/O connection 被 block 住了，也不會影響到其他的 request，因為他們分別在不同的 thread。

我們為每一個運算都開一個 thread 看起來是很好的選擇，很不幸的是，thread 在系統資源裡面並不便宜，他會消耗記憶體跟會有內容交換的問題發生 context switches，所以為每一個 connection 建立一個 thread，並不是一個最好的選擇。

### Non-blocking I/O

大多數作業系統支援另一種存取資源的機制，叫做 non-blocking I/O。在這個 operating mode 裡面，system call 一定會立即回傳，不需要等待資料讀取跟寫入，如果沒有回傳結果的話，function 會回傳一個 predefined constant，意思是目前這個時間點沒有可用的回傳資料。

舉例來說，Unix 作業系統有一個 function `fcntl()` 是用來操作，可以把存在的 file descriptor 從 operating mode 改成 non-blocking (用 `O_NONBLOCK` flag)，一但資源是在 non-blocking mode 的狀態，任何 read 的運算會回傳 `EAGAIN`，以防資源還沒有準備好讀取任何數據。

大多數基本存取 non-blocking I/O 的 pattern 是用迴圈輪詢 (poll) 每個資源，直到某些資料回傳，這個叫做 **busy-waiting**。下面這個 pseudocode 是使用 non-blocking I/O 跟 polling loop：

```pseudocode
socketA, pipeB; 
watchedList.add(socketA, FOR_READ);                     //[1] 
watchedList.add(pipeB, FOR_READ); 
while(events = demultiplexer.watch(watchedList)) {      //[2] 
  //event loop 
  foreach(event in events) {                            //[3] 
    //This read will never block and will always return data 
    data = event.resource.read(); 
    if(data === RESOURCE_CLOSED) 
      //the resource was closed, remove it from the watched list 
      demultiplexer.unwatch(event.resource); 
    else 
      //some actual data was received, process it 
      consumeData(data); 
  } 
} 
```

你可以看到，在這個簡單的技術下，在同一個 thread 已經可以處理不同的資源，但是不夠有效率，loop 會消耗大量的 CPU 資源，大多數的時間是不可用的，Polling 的演算法會消耗大量的 CPU 時間。

### Event demultiplexing

毫無疑問的，Busy-waiting 絕對不是處理 non-blocking 資源理想的技術，大多數作業系統提供有效率處理 concurrent, non-blocking 資源的機制，這個機制又稱作 **synchronous event demultiplexer** 或是 **event notification interface**，這個元件會收集和 queue 住 I/O 事件，這些事件是一組被 watched 的資源，會一直 block 事件到新的事件可以執行的時候。這個 pseudocode 是普遍的 synchronous event demultiplexer，會讀取兩個資源：

```pseudocode
socketA, pipeB; 
watchedList.add(socketA, FOR_READ);                     //[1] 
watchedList.add(pipeB, FOR_READ); 
while(events = demultiplexer.watch(watchedList)) {      //[2] 
  //event loop 
  foreach(event in events) {                            //[3] 
    //This read will never block and will always return data 
    data = event.resource.read(); 
    if(data === RESOURCE_CLOSED) 
      //the resource was closed, remove it from the watched list 
      demultiplexer.unwatch(event.resource); 
    else 
      //some actual data was received, process it 
      consumeData(data); 
  } 
} 
```

1. 把資源加到 data structure 裡面，每個項目都會聯繫一個運算，我們例子是 `read`。
2. event notifier 會 watch 一群被設定的資源，這個 call 是同步的，而且會 block 直到被 watched 的資源準備 `read`。如果有事件發生，event demultiplexer 會回傳這個事件的 call，這一組事件將會被執行。
3. 由 event demultiplexer 回傳的 event 將會被執行。在這個時間點，聯繫每個 event 的資源保證準備 `read`，而且不會被其他運算 block 住。當所有 event 被執行完，event demultiplexer 又會再次 block，等到新的 event 能夠再次被執行，這個東西稱作為 **event loop**。

現在用單一個 thread 可以處理很多個 I/O 運算，而且不需要用到 busy-waiting。

使用單一個 thread 並不會影響我們運算 I/O 的能力，tasks 會散佈在時間上，而不是橫跨不同的 thread，這樣可以讓 thread 的閒置時間最小化。選擇這個 model 的好處不只這些，單一個 thread 對開發者處理並行的程序來說也是好的。之後我們會看到要如何處以 race condition 跟多個 thread 要如何依序執行。

## 介紹 reactor pattern

reactor pattern 對於前面的章節來說是一個特殊的演算法。在他背後主要的想法是讓每一個 handler (在 Node.js 裡面代表 callback function) 聯繫每一個 I/O 運算，只要 event loop 執行 event 就會呼叫 I/O 運算。

![](https://www.safaribooksonline.com/library/view/nodejs-design-patterns/9781785885587/graphics/B05259_01_03.jpg)

1. Application 藉由提交一個 request 給 **Event Demultiplexer** 會產生新的 I/O operation。同時應用程式也會指派一個 handler，當 operation 完成的時候會去呼叫 handler。提交一個新請求給 **Event Demultiplexer** 是 non-blocking call，他會立即把控制權交給 application。
2. 當一組的 I/O operarion 完成的時候，**Event Demultiplexer** 推送一個新的事件給 **Event Queue**。
3. 在這個時候，**Event Loop** 會依序跑過每一個 **Event Queue**。
4. 對於每個 event，會去呼叫聯繫的 handler。
5. handler 是 application code 裡的一小部分，當 handler (**5a**) 執行完成的時候會把操作權接回給 **Event Loop**。然而，新的非同步 operation 也可能在執行 handler (**5b**) 的時候發出請求，導致控制權交回給 **Event Loop** 之前，新的 operation 會被加到 **Event Demultiplexer** (**1**)。
6. 當執行所有 **Event Queue** 裡面的項目，loop 會在一次 block 在 **Event Demultiplexer**，當有新的事件可用的時候，**Event Demultiplexer** 將會觸發另一個 cycle。

現在非同步的行為已經很清楚了：application 會在某一個時間點即時存取資源 (without blocking)，和提供另一個時間點會被呼叫的 handler。

如果 Event Demultiplexer 裡面沒有其他的運算，而且 **Event Queue** 沒有其他事件要執行的話，Node.js application 會自動退出。

我們現在可以定義 Node.js 的核心：

**Pattern (reactor)** 通過 block 來處理 I/O，直到從一組 observed 的資源中獲得新事件，然後將每個事件分派給關聯的 handler 程序來處理。

### The non-blocking I/O engine of Node.js-libuv 

每個系統都有他自己的 **Event Demultiplexer**：Linux 上的 epoll、Mac OS X 上的 kqueue、Windows 上的 **I/O Completion Port (IOCP)** API。此外，取決於不同的 resouce type，也會有不同的行為，即使在相同的系統。舉例來說，在 Unix 裡面，一般的 filesystem 不支援 non-blocking operation，所以為了模擬 non-blocking 的行為，我們有必要在 Event Loop 外面分別使用不同的 thread。在橫跨所有不兼容的作業系統中，需要一個更高的抽象層專門處理 Event Demultiplexer。這就是為什麼 Node.js core 的團隊建立了一個 C library 叫做 **libuv**，有了這個東西，能夠讓 Node.js 兼容於大多數主要的平台，而且正規化不同 resource type non-blocking 的行為。libuv 代表 Node.js 最底層的 I/O engine。

此外，將底層的 system call 抽象化，libuv 也實現了 reactor pattern，因此可以提供 API，像是建立 event loops、管理 event queue、運行非同步 I/O operations 跟 queuing 其他任務的 type。

學習 libuv 的資源：http://nikhilm.github.io/uvbook/

### The recipe for Node.js

reactor pattern 跟 libuv 是建立 Node.js block 的基礎，但是我們需要以下三個元件來建立完整的 platform：

- 負責用來包裝和揭發 libuv 和底層功能的 bindings 給 JavaScript。
- V8 是由 Google 用來開發 Chrome 瀏覽器 JavaScript 的引擎。這是為什麼 Node.js 非常快且有效率。V8 宣稱有革命性的設計，速度與記憶體管理的效率。
- JavaScript library 的核心 (又稱作 **node-core**)，屬於較高層的 Node.js API。

這張圖代表 Node.js 最終的架構：

![](https://www.safaribooksonline.com/library/view/nodejs-design-patterns/9781785885587/graphics/B05259_01_04.jpg)

