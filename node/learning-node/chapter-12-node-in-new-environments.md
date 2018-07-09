# Node in New Environments

[Learning Node.js for Mobile Application Development](https://www.packtpub.com/web-development/learning-nodejs-mobile-application-development)

## Samsung IoT and GPIO

Samsung 建立了很多不同的 Node 稱作 IoT.js，還有 IoT 科技稱作 JerryScript。

IoT.js 是由像是 Buffer、HTTP、Net、File System 套件所組長的，不支援像是 Crypto 的套件，還有新的功能：GPIO，他代表硬體的介面，連接應用程式跟硬體。

GPIO 全名是：general-purpose input/output。可以用來控制硬體的 input 跟 output。

使用 IoT.js，初始化 GPIO 物件然後呼叫 function 像是：`gpio.setPen()` 的參數，pin 腳的值、`in` `out` `none`、選擇性參數、callback。`gpio.writePin()`的參數 pin 值、boolean 值、callback。

## Node for Microcontrollers and Microcomputers

### Fritzing

- 模擬 Arduino app [Fritzing](http://fritzing.org/home/)
- [如何使用麵包板](https://learn.sparkfun.com/tutorials/how-to-use-a-breadboard)
- [計算正確的電阻值](https://hardwarefun.com/tutorials/calculating-correct-resistor-value-to-protect-arduino-pin)
- [我用 Arduino 控制 LED 需要什麼電阻](https://electronics.stackexchange.com/questions/32990/do-i-really-need-resistors-when-controlling-leds-with-arduino)

### Node and Arduino

- [安裝 Arduino 軟體](https://www.arduino.cc/en/Main/Software)
- [安裝說明](https://www.arduino.cc/en/Guide/HomePage)

有一個 Node 框架叫做 [Johnny-Five](http://johnny-five.io/) 提供寫 programming 的方法：

```shell
npm install johnny-fiv e
```

讓 Arduino 第 13 個 pin 腳閃爍：

```js
// ardblink.js
var five = require("johnny-five");
var board = new five.Board();

board.on("ready", function() {
  var led = new five.Led(13);
  led.blink(500);
});
```

控制燈泡：

```js
// chap12-1.js
var five = require("johnny-five");
var board = new five.Board();

board.on("ready", function() {
  console.log("Ready event. Repl instance auto-initialized!");

  var led = new five.Led(11);

  this.repl.inject({
    // Allow limited on/off control access to the
    // Led instance from the REPL.
    on: function() {
      led.on();
    },
    off: function() {
      led.off();
    },
strobe: function() {
    led.strobe(1000);
},
pulse: function() {
    led.pulse({
           easing: "linear",
           duration: 3000,
           cuePoints: [0, 0.2, 0.4, 0.6, 0.8, 1],
           keyFrames: [0, 10, 0, 50, 0, 255],
           onstop: function() {
               console.log("Animation stopped");
           }
        });
},
stop: function() {
    led.stop();
},
fade: function() {
    led.fadeIn();
    
}, 
fadeOut: function() {
    led.fadeOut();
}
  });
});
```

```shell
node fancyblinking.js
```

- [Real-Time Temperature Logging with Arduino, Node, and Plotly](http://www.instructables.com/id/Real-Time-Temperature-Logging-With-Arduino-NodeJS-/)
- [The Arduino Experimenter’s Guide to NodeJs has a whole host of projects to try, and provides the finished code](http://node-ardx.org/)
- [Controlling a MotorBoat using Arduino and Node • As an alternative to Johnny-Five, try out the Cylon Arduino module ](https://www.sitepoint.com/controlling-a-motorbot-using-arduino-and-node-js/)
- [Arduino Node.js RC Car Driven with the HTML5 Gamepad API](https://cylonjs.com/)
- [How to Control Philips Hue Lights from an Arduino (and Add a Motion Sensor)](https://www.makeuseof.com/tag/control-philips-hue-lights-arduino-and-motion-sensor/)

### Node and Raspberry Pi 2

```shell
npm install johnny-five raspi-io
```

跟 Arduino 的例子一樣：

```js
// piblink.js
var five = require("johnny-five");
var Raspi = require("raspi-io");
var board = new five.Board({
  io: new Raspi()
});

board.on("ready", function() {
  var led = new five.Led("P1-13");
  led.blink();
});
```

```js
var five = require("johnny-five");
var Raspi = require("raspi-io");
var board = new five.Board({
    io: new Raspi()
});

board.on("ready", function() {
  console.log("Ready event. Repl instance auto-initialized!");

  var led = new five.Led("P1-12");

  this.repl.inject({
    // Allow limited on/off control access to the
    // Led instance from the REPL.
    on: function() {
      led.on();
    },
    off: function() {
      led.off();
    },
strobe: function() {
    led.strobe(1000);
},
pulse: function() {
    led.pulse({
           easing: "linear",
           duration: 3000,
           cuePoints: [0, 0.2, 0.4, 0.6, 0.8, 1],
           keyFrames: [0, 10, 0, 50, 0, 255],
           onstop: function() {
               console.log("Animation stopped");
           }
        });
},
stop: function() {
    led.stop();
},
fade: function() {
    led.fadeIn();
    
}, 
fadeOut: function() {
    led.fadeOut();
}
  });
});
```

- [Easy Node.js and WebSockets LED Controller for Raspberry Pi](https://www.digikey.com/en/resources/conversion-calculators/conversion-calculator-resistor-color-code-4-band)
- [Home Monitoring with Raspberry Pi and Node](https://www.hackster.io/andreioros/home-monitoring-with-raspberry-pi-and-node-js-8ec795)
- [Heimcontrol.js: Home Automation with Raspberry Pi and Node](https://ni-c.github.io/heimcontrol.js/get-started.html)
- [Build Your Own Smart TV Using RaspberryPi, NodeJS, and Socket.io](https://www.codementor.io/donald/tutorial-build-your-own-smart-tv-using-raspberrypi-nodejs-and-socket-io-8sdcaalbg)
- [Building a Garage Door Opener with Node and MQTT—Using an Intel Edison](https://blog.risingstack.com/getting-started-with-nodejs-and-mqtt/)
- [Amazon’s Guide to Make Your Own Raspberry Pi Powered Alexa Device](https://www.techworm.net/2016/03/amazons-guide-make-raspberry-pi-powered-alexa-device.html)