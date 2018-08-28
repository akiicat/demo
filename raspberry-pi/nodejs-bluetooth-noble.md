# Use nodejs bluetooth noble

環境是 raspberry pi 3 B+。

nodejs 環境裡，可以讓 nodejs 使用 bluetooth 的 adapter。

## 環境安裝

安裝藍牙相關的 library：

```shell
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install bluetooth bluez libbluetooth-dev libudev-dev
```

## Nodejs 安裝

這邊是用 raspberry pi 3 B+，預設版本是 4.8.2 版，注意不能使用 nvm 安裝，因為 noble 他會使用系統的 nodejs，不會去抓 nvm 的 nodejs：

[node 安裝](https://nodejs.org/en/download/package-manager/#debian-and-ubuntu-based-linux-distributions)

```shell
curl -sL https://deb.nodesource.com/setup_8.x | sudo -E bash -
sudo apt-get install -y nodejs
```

## 建立新專案

```shell
mkdir my-app
cd my-app
npm init -y
```

### 安裝 nobel

```shell
npm install noble --save
npm install node-beacon-scanner --save
```

### bluetooth scanning

在專案裡面建立一個新的檔案叫做 `index.js`，然後複製下面的程式碼到 index.js 裡面：

```js
// my-app/index.js
const Noble = require("noble");
const BeaconScanner = require("node-beacon-scanner");

var scanner = new BeaconScanner();

scanner.onadvertisement = (advertisement) => {
    var beacon = advertisement["iBeacon"];
    beacon.rssi = advertisement["rssi"];
    console.log(JSON.stringify(beacon, null, "    "))
};

scanner.startScan().then(() => {
    console.log("Scanning for BLE devices...")  ;
}).catch((error) => {
    console.error(error);
});
```

執行：

```shell
$ sudo node index.js 
Scanning for BLE devices...
{
    "uuid": "50765CB7-D9EA-4E21-99A4-FA879613A492",
    "major": 48658,
    "minor": 54441,
    "txPower": 206,
    "rssi": -90
}
```

## Trouble Shoot

上一次執行完成後，沒有把藍牙關掉的話，下一次執行的時候將會出錯，如果壞掉的話，記得要重新開啟：

```shell
sudo systemctl restart bluetooth
```

或是在每一次執行程式碼的時候，先將之前執行過的藍芽關掉，再重新打開：

```js
noble.stopScanning();
noble.startScanning([], false, function(err) { ... })
```

