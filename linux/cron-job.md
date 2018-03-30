# Cron Job

## 開啟

```shell
crontab -e
```

通常都需要管理員的權限，用 sudo 執行：

```shell
sudo crontab -e
```

## Crontab Lines

由左到右五個值分別由空白鍵分開，分別是：分、時、日、月、週

```shell
* * * * * <command>
- - - - -
| | | | |
| | | | ----- Day of week (0 - 7) (Sunday=0 or 7)
| | | ------- Month (1 - 12)
| | --------- Day of month (1 - 31)
| ----------- Hour (0 - 23)
------------- Minute (0 - 59)
```


每個 1月 1日星期一的 4:01 分執行：

```shell
01 04 1 1 1 <command>
```

星號 `*` 代表所有時間的意思。

每天 4:01分執行：

```shell
01 04 * * * <command>
```

逗號 `,` 可以指定多個值。
連接號 `-` 可以指定一個範圍的值。

每個一月和六月的 1 到 15 號 4 點或 5 點的 1 分或 31 分執行：

```shell
01,31 04,05 1-15 1,6 * <command>
```

每十分鐘執行：
```shell
*/10 * * * * <command>
0,10,20,30,40,50 * * * * <command>
```

Cron 有特殊的字串

|**string**|**meaning**|
|:-----:|:-----:|
|@reboot|Run once, at startup.|
|@yearly|Run once a year, "0 0 1 1 *".|
|@annually|(same as @yearly)|
|@monthly|Run once a month, "0 0 1 * *".|
|@weekly|Run once a week, "0 0 * * 0".|
|@daily|Run once a day, "0 0 * * *".|
|@midnight|(same as @daily)|
|@hourly|Run once an hour, "0 * * * *".|

```shell
@reboot <command>
```

## Crontab 參數

1. -l 參數：當前的 crontab 會顯示在標準輸出上
1. -r 參數：當前的 crontab 會被移除
1. -e 參數：編輯當前的 crontab 可以選擇編輯器環境



- [Cron How To](https://help.ubuntu.com/community/CronHowto)