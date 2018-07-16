# Networking, Sockets, and Security

Node 應用程式的核心基於兩個主要的元件：networks 和 security。再講 network 不能不提到 socket。

把 network 跟 security 放在一起是因為當你一到獨立的機器上，最重要的就是安全性。每當你做完一小塊東西的時候，第一個問題就是，這個東西安全嗎。

## Servers, Streams, and Sockets

大多數的 Node core API 是建立監聽特別型態通訊的服務。像是 HTTP module 建立 web server 監聽 HTTP request。其他套件可以建立 Transmission Control Protocol (TCP) 伺服器、 Transport Layer Security (TLS) 伺服器、 User Datagram Protocol (UDP) socket。

### Socket and Streams

在通訊中，socket 是一個末端，network socket 是在不同電腦的兩個應用程式透過網路溝通的末端。data flows 資料流

在 socket 裡稱作為 stream。在 stream 裡的 data 可以透過 buffer 二進位的檔案傳送，或是 unicode 的 stream。傳送這兩種型態的資料叫做 packet：部分的資料分成相似大小。有特別型態的封包 finish packet (FIN)，透過 socket 去通知這個傳送已經完成了。

想像有兩個在透過對講機在講話，對講機是末端，末端是通訊中的 socket，當兩個或多人想要互相說話的時候，他們把無線電調到相同的頻率，然後當一個人想要跟另外一個人通訊的時候，他按下對講機的按鈕，以某種識別方式連線到正確的人，他們也使用一些字 over 通知對方他們不在講話，而變成監聽模式，其他人按下按對講機的按鈕，回答收到 acknowledges，然後再說一次 over 通知他已將講完話了。這種通訊會持續到一個人發出 over and out，意外著這段通訊已經結束了，同時間只有一個人可以講話。

對講機通訊 stream 就如同我們所知的 half-duplex，因為通訊在同一個時間只能單方向的傳遞。full-duplex 通訊 stream 允許雙向的通訊。

這種觀念應用在 Node stream，我們在第六章有運行了 half-duplex 跟 full-duplex stream。half-duplex stream 的範例是 stream 被使用來讀檔和寫檔，stream 支援 readable 介面跟 writable 介面，但是不能同時使用。full-duplex 的範例是 zlib 壓縮 stream，允許同時間讀跟寫。

### TCP Sockets and Servers

對於大多的網路應用程式中，TCP 提供了通訊的平台，像是網頁服務跟 email。它在 client 和 server sockets 間提供了一種可靠傳輸資料的方式。TCP 提供了一些基本建設，讓應用層能居於這基本建設上。

我們可以建立 TCP server 和 client 就如同 HTTP 一樣，不過有些不同，當我們建立 TCP server，不用傳入`requestListener`到 server creation function，function 會傳入 request 和 response 物件， TCP callback function 的參數是一個 socket 的實利，可以同時傳送和接收。

如何建立 TCP server，一旦 server socket 被建立，他會監聽兩個事件：當資料接收跟當 client 關閉連線的時候，他會印出收到的訊息：

```js
// example7-1.js
var net = require('net');
const PORT = 8124;

var server = net.createServer(function(conn) {
   console.log('connected');

   conn.on('data', function (data) {
      console.log(data + ' from ' + conn.remoteAddress + ' ' +
        conn.remotePort);
      conn.write('Repeating: ' + data);
   });

   conn.on('close', function() {
        console.log('client closed connection');
   });

}).listen(PORT);

server.on('listening', function() {
    console.log('listening on ' + PORT);
});

server.on('error', function(err){
  if (err.code == 'EADDRINUSE') {
    console.warn('Address in use, retrying...');
    setTimeout(() => {
      server.close();
      server.listen(PORT);
    }, 1000);
  }
  else {
    console.error(err);
  }
});

```

TCP server 也可以掛`listening`事件跟`error`事件，我們用`console.log()`把訊息印出來。然而因為`listen()`事件是非同步的，訊息會在 listen 事件發生前印出來。相反，你可以在`listen`的 callback function 中加入一個消息，或者你可以在這裡做我正在做的事情，它將 event handler 附加到`listening`事件並正確地提供回饋。

此外，我們也提供更複雜的 error handling，`error`有可能會因為 port 已經被使用發生錯誤而被呼叫，或是需要權限的 port。

當你建立 TCP socket，你可以傳入一些選項，`pauseOnConnect`和`allowHalfOpen`，兩個預設值都是`false`：

```json
{
  allowHalfOpen: false,
  pauseOnConnect: false
}
```

將`allowHalfOpen`設成`true`的話，socket 從 client 收到 FIN packet 的時候不會傳送 FIN。這會讓寫入的 socket 保持開啟，讀取的不會。為了關掉 socket，你必須使用`end()` function。設定`pauseOnConnect`為`true`允許建立連線，但是沒有任何數據被讀取，要開始讀取數據必須在 socket 上呼叫`resume()`。

TCP client 範例，資料透過 buffer 傳送，但我們可以使用`setEncoding()`來讀取`utf8`的字串。socket 的`write()`用來傳送資料。

```js
// example7-2.js
var net = require('net'); 
var client = new net.Socket(); 
client.setEncoding('utf8'); 

// connect to server 
client.connect ('8124','localhost', function () {
   console.log('connected to server'); 
   client.write('Who needs a browser to communicate?');
}); 

// when receive data, send to server 
process.stdin.on('data', function (data) { 
   client.write(data); 
}); 

// when receive data back, print to console 
client.on('data',function(data) { 
   console.log(data); 
}); 

// when server closed 
client.on('close',function() { 
   console.log('connection is closed'); 
});

client.on('error', function(e) {
   console.error(e);
});
```

使用 socket 的`remoteAddress`跟`remotePort`可以知道 IP address 跟 port。

```shell
$ node example7-1.js 
listening on 8124
connected
Who needs a browser to communicate? from ::ffff:127.0.0.1 59061
hi
 from ::ffff:127.0.0.1 59061
asdfasf
 from ::ffff:127.0.0.1 59061
client closed connection

$ node example7-2.js 
connected to server
Repeating: Who needs a browser to communicate?
hi
Repeating: hi
asdfasf
Repeating: asdfasf
```

連線會持續到其中一邊關掉程式，當 socket 收到`close`事件的時候會印出訊息，server 可服務多條連線，因為 function 是非同步的。

IPv4 對應到 IPv6 的話，前面的地址會以`::ffff`開頭。

```js
// example7-3.js
var net = require('net');
var fs = require('fs');

const unixsocket = '/tmp/learning.sock';

var server = net.createServer(function(conn) {
   console.log('connected');

   conn.on('data', function (data) {
      conn.write('Repeating: ' + data);
   });

   conn.on('close', function() {
        console.log('client closed connection');
   });

}).listen(unixsocket);

server.on('listening', function() {
    console.log('listening on ' + unixsocket);
});



// if exit and restart server, must unlink socket
server.on('error',function(err) {
   if (err.code == 'EADDRINUSE') {
      fs.unlink(unixsocket, function() {
          server.listen(unixsocket);
      });
   } else {
      console.log(err);
   }
});

process.on('uncaughtException', function (err) {
    console.log(err);
});
```

我們並不是綁定 TCP server 的 port，我們可以直接綁定 socket。我們修改一下範例，將新的 server 綁定在 unix socket 上，而不是 port。unix socket 是一個在你 server 上的路徑名稱，讀跟寫的權限可以用來控制應用程式的存取，這讓 Internet socket 更進階。

我也同時修改了 error handling ，如果應用程式重新啟動跟 socket 已經被使用了，要去解開 Unix socket。在你做一些突然的事情之前，你想確保沒有其他 client 正在使用 socket。

在`process`加上`uncaughtException`預防沒有接到例外時的處理。

在 unlink socket 前，你可以檢查是否有其他 server 在使用 socket：[Stack Overflow](https://stackoverflow.com/questions/16178239/gracefully-shutdown-unix-socket-server-on-nodejs-running-under-forever)

unix socket client 應用程式：

```js
// example7-4.js
var net = require('net');
var client = new net.Socket();
client.setEncoding('utf8');

// connect to server
client.connect ('/tmp/learning.sock', function () {
   console.log('connected to server');
   client.write('Who needs a browser to communicate?');
});

// when receive data, send to server
process.stdin.on('data', function (data) {
   client.write(data);
});

// when receive data back, print to console
client.on('data',function(data) {
   console.log(data);
});

// when server closed
client.on('close',function() {
   console.log('connection is closed');
});

client.on('error', function(e) {
   console.error(e);
});
```

### UDP/Datagram Socket

TCP 需要在通訊的兩個末端進行溝通。UDP 是沒有連線的 protocol。因為這個原因，UDP 是不可靠的而且比 TCP 更強大。另外一方面，普遍來說 UDP 的速度比 TCP 更快，讓更多即時的需求的更受歡迎，像是透過網路協定傳送聲音 Voice over Internet Protocol (VoIP) 的技術，TCP 連接要求可能會對信號的 quality 產生負面影響。

Node core 支援兩種型態的 socket。

UDP 套件是`dgram`

```js
require('dgram');
```

使用`createSocket`方法建立 UDP socket，傳入 socket 的型態`udp4`或`udp6`到 callback function。不像 TCP 可以傳送字串，UDP 必須傳送 buffer。

```js
// example7-5.js
var dgram = require('dgram');

var client = dgram.createSocket("udp4");

process.stdin.on('data', function (data) {
   console.log(data.toString('utf8'));
   client.send(data, 0, data.length, 8124, "localhost",
      function (err, bytes) {
        if (err)
          console.error('error: ' + err);
        else
          console.log('successful');
   });
});
```

上面的範例是 UDP client，在裡面資料會透過`process.stdin`存取，然後透過 UDP socket 傳送，注意我們不必設定 string 的 encoding，因為他是只能透過 buffer 傳，然後`process.stdin`的資料就是 buffer。我們要做的就是把 buffer 轉換成 stream，使用`toString()`，最後用`console.log()`印出來。

```js
// example7-6.js
var dgram = require('dgram');

var server = dgram.createSocket("udp4");

server.on ("message", function(msg, rinfo) {
   console.log("Message: " + msg + " from " + rinfo.address + ":"
                + rinfo.port);
});

server.bind(8124);
```

上面的範例是 UDP server，比 client 更間單。server 所做的是建立 socket，綁定特定的 port，然後監聽`message`事件。當訊息來了的時候，應用程式會印出 IP 和 port，不需要使用 encoding，印為他會自動將 buffer 轉換成 string。

我們不用綁定 socket 到 port 上，沒有這個綁定，socket 將會企圖監聽每個 port。

不管在 client 或是在 server 都沒有`close`的方法，在 client 和 server 沒有連線需要維護，只需要傳送和接收資料的能力就行。

## Guards at the Gate

在網頁應用程式中，安全性用來確保人們不應該存取到應用程式的伺服器。

### Setting Up TLS/SSL

Secure 是在 client 和 server 之間的溝通防止篡改，他會在Secure Sockets Layer (SSL) 之上，他升級之後就是 Transport Lyaer Security (TLS). TLS/SSL 為 HTTPS 提供了底層的加密。然而，在我們開發 HTTPS 前，我們還有些環境設定要做。

TLS/SSL 連線提供 client 和 server 之間的 handshake。在 handshake 的期間，client 會讓 server 知道是他支援哪種安全性。server 選一個 function 然後傳送會包含 public key 的 SSL certificate，client 確認這個 certificate 然後產生一個隨機的數字，使用收到的 server key 將數字加密傳回 server，server 用 private key 解開那個數字，這樣用來確保安全的通訊。

要讓這個能運作，我們必須先產生 public key 跟 private key 以及 certificate，在 production 系統，certificate 必須由 trusted authority 簽署，像是 domain registrar，但我們開發的目的是你可以使用 self-signed certificate。在瀏覽器使用會發出警告，但因為正在開發的網頁不會讓使用者接觸到，應該不會是問題。

OpenSSL 是用來產生必要的檔案，如果你是用 Linux 那麼他已經安裝好了：

```shell
openssl genrsa -des3 -out site.key 4196
```

這個指令產生 private key，用 Triple-DES 加密，然後以 privacy-enhanced mail (PEM) 格式儲存，讓 ASCII 可以讀它。

他會跳出密碼叫你輸入，你在下個地方建立 certificate-signing request (CSR) 會使用到。

當你產生 CSR，他會詢問你剛建立的密碼，然後還會問一堆問題，這裡面最重要的問題是 Common Name，這是在問你網站的 hostname，提供你應用程式的 hostname。

```shell
openssl req -new -key site.key -out site.csr
```

private key 想要一個 passphrase，問題是每次你開啟 server 你都要輸入 passphrase，每次都要輸入在 production 上會是個問題。先將檔案命名：

```shell
mv site.key site.key.org
openssl rsa -in site.key.org -out site.key
```

如果你移除了 passphrase，確保你的 server 是安全的，而且只能讓信任的使用者讀取。

下一步是產生 self-signed certificate，他會維持 365 天：

```shell
openssl x509 -req -days 365 -in site.csr -signkey site.key -out final.crt
```

現在你有足夠的東西來建立 TLS/SSL 和 HTTPS。

- site.key：沒有 passphrase 的 private key
- site.key.org：有 passphrase 的 private key
- site.csr：public key
- final.crt：certification

### Working with HTTPS

網頁內如果會詢問使用者 login 和信用卡資訊的話，應該要有 HTTPS。如果沒有的話，你的資料會在公開的地方傳輸，很容易被挖出來。HTTPS 是 HTTP 協定的變形，HTTP 結合 SSL 確保網頁是傳送的資料是加密的，且資料收到的時候是完整的，不會有任何篡改。

HTTPS 使用 443 port。

基本的 HTTPS server，他只會傳送 Hello World 的訊息：

```js
// example7-7.js
var   fs = require("fs"),
      https = require("https");

var privateKey = fs.readFileSync('site.key').toString();
var certificate = fs.readFileSync('final.crt').toString();

var options = {
   key: privateKey,
   cert: certificate
};

https.createServer(options, function(req,res) {
   res.writeHead(200);
   res.end("Hello Secure World\n");
}).listen(443);
```

public key 和 certificate 是公開的，他們的資料是可以同步讀取。資料傳入`https.createServer`的第一個參數，callback function 跟 http 一樣。

如果跑在 443 port 要用 sudo 權限下去執行，或是改成其他 port。

存取網頁會跳出安全性警告，因為是使用 self-signed certificate。使用 trusted certificate authority 的話可以消除這個不安全的訊息。你應該要在網站所有頁面都要使用 HTTPS 而不是只有在付款頁面跟密碼頁面。

### The Crypto Module

Node 提供套件可以用來加密 Crypto，是一個 OpenSSL 功能的介面。他把 OpenSSL 的 hash、HMAC、cipher、decipher、sign、verfiy function 包裝起來，讓 Node 可以非常簡單去使用，前提是假設大家都懂 OpenSSL。

- [OpenSSL Document](https://www.openssl.org/docs/)
- [Openssl Cookbook](https://www.feistyduck.com/books/openssl-cookbook/)

Crypto 套件使用 OpenSSL 的 hash 的功能建立密碼，hash 是使用 checksum 來儲存值。

你可以使用 Crypto 套件的`createHash`方法來建立 password hash 然後儲存在資料庫，會使用`sha1`演算法加密：

```js
var hashpassword = crypto.createHash('sha1')
                         .update(password)
                         .digest('hex');
```

digest encoding 設成 16 進位，編碼預設是二進位和 base64。

儲存加密過的密碼會比儲存原文來的好，但如果你的 encryption key 被偷的話，密碼還是有可能會被破解，儲存 hash 會必較好一點，如果忘記密碼的話應該要重設密碼，而不是回復密碼。

rainbow table 是一個簡單的從每個可能組合的字當中預先 hash 過的 table。如果你有一個密碼，你確定你沒有辦法破解他，序列的字元中有 rainbow table 某個地方可以更間單決定你的 password。

rainbow table 的方法是使用 salt，會產生唯一的值，附加在尚未加密的密碼前面，他可以是單一的值，這個值必須安全的儲存在 server 上，最好的方法是為每一個使用者產生一個唯一的值，當然，salt 也有可能同時被偷，但仍然還是要有人企圖去破解密碼，然後產生 ranbox table，可以為每個密碼產生非常複雜的密碼。

這是個簡單的程式，可以把 username 和 password 傳入 command-line 參數，產生密碼 hash，然後儲存在 MySQL 的 資料庫裡。

```shell
npm install node-mysql
```

建立 table：

```mysql
CREATE TABLE user (userid INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(userid),
username VARCHAR(400) NOT NULL, passwordhash VARCHAR(400) NOT NULL,
salt DOUBLE NOT NULL );
```



```js
// example7-8.js
var mysql = require('mysql'),
    crypto = require('crypto');

var connection = mysql.createConnection({
   host: 'localhost',
   user: 'username',
   password: 'password'
  });

connection.connect();

connection.query('USE nodedatabase');

var username = process.argv[2];
var password = process.argv[3];

var salt = Math.round((Date.now() * Math.random())) + '';

var hashpassword = crypto.createHash('sha512')
                   .update(salt + password, 'utf8')
                   .digest('hex');
// create user record
connection.query('INSERT INTO user ' +
   'SET username = ?, passwordhash = ?, salt = ?',
   [username, hashpassword, salt], function(err, result) {
      if (err) console.error(err);
      connection.end();
});
```

salt 是由日期所產生的亂數，附加到未加密的密碼前面，然後用 sha512 產生 hash，然後使用者的資料會存到資料庫裡：

```shell
$ node example7-8.js akiicat password

mysql> select * from user;
+--------+----------+----------------------------------------------------------------------------------------------------------------------------------+-------------+
| userid | username | passwordhash                                                                                                                     | salt        |
+--------+----------+----------------------------------------------------------------------------------------------------------------------------------+-------------+
|      1 | akiicat  | f1a6199a4ef3cde1dce2b65b20dc7b414e4fcba19ecb82e6a4af5e53645c204635936356f5c4b3f499e0f7a122a9533373d871d5f9973e6a3c469a84243ba6d1 | 45366791958 |
+--------+----------+----------------------------------------------------------------------------------------------------------------------------------+-------------+
1 row in set (0.00 sec)
```

測試重新加密過後是否會在產生相同 hash：

```js
// example7-9.js
var mysql = require('mysql'),
    crypto = require('crypto');

var connection = mysql.createConnection({
   user: 'username',
   password: 'userpass'
  });

connection.query('USE nodedatabase');

var username = process.argv[2];
var password = process.argv[3];

connection.query('SELECT passwordhash, salt FROM user WHERE username = ?',
   [username], function(err, result, fields) {
   if (err) return console.error(err);

   var newhash = crypto.createHash('sha512')
                 .update(result[0].salt + password, 'utf8')
                 .digest('hex');

   if (result[0].passwordhash === newhash) {
      console.log("OK, you're cool");
   } else {
      console.log("Your password is wrong. Try again.");
   }
   connection.end();
});
```

```shell
$ node example7-9.js akiicat password
OK, you're cool

$ node example7-9.js akiicat wrong_password
Your password is wrong. Try again.
```

crypto hash 也可以使用 duplex stream：

```js
// chap7-1.js
var crypto = require('crypto');
var fs = require('fs');
var hash = crypto.createHash('sha256');
hash.setEncoding('hex');

var input = fs.createReadStream('main.txt');
var output = fs.createWriteStream('mainhash.txt');

input.pipe(hash).pipe(output);
```

```shell
$ cat main.txt 
This is the file we're using for testing

$ node chap7-1.js 

$ cat mainhash.txt 
64f2db3e0573ce3a7a0e1bed3db3f568f364d511f58a305eb1650d6e73f23739
```

你可以為每一個檔案建立 hash，傳輸完成後檢查 hash 是否相同來確定檔案相同。

你也可以使用`md5`演算法，產生 MD5 的 checksum，比較快，比較熱門，但是不安全。

```js
var hash = crypto.createHash('md5');
```

