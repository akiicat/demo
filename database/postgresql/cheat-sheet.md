# Postgresql cheat sheet

```sh
-c                   # 執行單行 SQL 指令
-h, --host=HOSTNAME  # 伺服器的 host
-p, --port=PORT      # 伺服器的 port
-U, --username=NAME  # 指定某個使用者
-W, --password       # 彈出對話框輸入密碼
-d, --dbname=NAME    # 指定某個資料庫
-f, --file=NAME      # 執行某個檔案的 SQL 指令
-l, --list           # 列出可以使用的資料庫
-v NAME=VALUE        # 設定變數 NAME = VALUE
-X                   # 進入 psql 時，不會載入 `~/.psqlrc` 的設定檔
-E                   # 進入 psql 執行 '\' 開頭的指令，會顯示 SQL 語法
-e, --exit-on-error  # 錯誤時離開，預設錯誤時會繼續進行
```

##### -v 參數的使用法法

設定變數 NAME = VALUE

```sh
psql -v v1=12 -v v2="'Hello World'"
```

然後用 :v1, :v2 取得變數

```sql
select * from table_1 where id = :v1;
```

# 互動介面

## 基本

```sh
\h  # 列出所有 SQL 的指令
\?  # psql 的指令說明
\q  # 離開
```

## 一般

```sh
\c[onnect] [DBNAME|- USER|- HOST|- PORT|-]  # 連線到某個資料庫
\cd [DIR]                                   # 移動到某個資料夾
\encoding [ENCODING]                        # 顯示或設定編碼
\h [NAME]                                   # 說明某個指令，* 代表全部的指令
\set [NAME [VALUE]]                         # 顯示或設定參數
\unset [NAME]                               # 移除某個參數
\timing                                     # 顯示執行時間，預設關閉
\! [COMMAND]                                # 執行 shell 指令
```

## 暫存指令

Query Buffer 是一個暫時儲存指令的地方

```sh
\e [FILE]  # 編輯 Query Buffer 或是某個檔案
\g [FILE]  # 執行 Query Buffer 內的東西，然後將結果存到某個檔案裡面
\p         # 顯示 Query Buffer 裡面有什麼東西
\r         # 清除 Query Buffer
\w FILE    # 把 Query Buffer 存成一個檔案
```

## Input / Output

```sh
\i FILE    # 執行某個檔案的 SQL 指令
\o [FILE]  # 接下來的輸出結果都存到檔案裡面
\echo      # 印出字串到螢幕上
\qecho     # 將印出的字串存到 \o 的檔案裡面
```

### 比較 \echo 與 \qecho

```
dbname=# \o oupu.txt 
dbname=# \echo hi
hi
dbname=# \qecho qq
dbname=# \q
```

```sh
cat oupu.txt
# => qq
```

## Information

- 加上 + 顯示詳細資料
- PARTTERN: __schemas__.__name__

```sh
\l              # 列出資料庫
\d [NAME]       # 顯示 public 的 tables 或顯示某個 table 的狀態
\dt [PATTERN]   # 列出 tables
\di [PATTERN]   # 列出 indexes tables
\ds [PATTERN]   # 列出 sequences tables
\dv [PATTERN]   # 列出 views tables
\dS [PATTERN]   # 列出 system tables
\dp [PATTERN]   # 列出 table, view, and sequence access privileges

\dg [PATTERN]   # 列出群組
\du [PATTERN]   # 列出使用者
\dn [PATTERN]   # 列出 schemas

\da [PATTERN]   # 列出 aggregate functions
\db [PATTERN]   # 列出 tablespaces
\dc [PATTERN]   # 列出 conversions
\dC             # 列出 casts
\dd [PATTERN]   # 列出物件註解
\dD [PATTERN]   # 列出 domains
\df [PATTERN]   # 列出 functions

\dF [PATTERN]   # 列出 text search configurations
\dFd [PATTERN]  # 列出 text search dictionaries
\dFt [PATTERN]  # 列出 text search templates
\dFp [PATTERN]  # 列出 text search parsers
\do [NAME]      # 列出 operators
\dT [PATTERN]   # 列出 data Type
\dl             # 列出 large object 等同於 \lo_list
```

## Formatting

```sh
\a           # 對齊功能，預設開
\C [STRING]  # 設定 table 的 title
\f [STRING]  # 顯示或設定分隔爛位的符號
\H           # 以 html 的格式輸出，預設關
\T [STRING]  # html 輸出的時候，設定最外層 <table> 裡的屬性 (id or class...)
\t           # 只顯示內容，欄位標題都不顯示，預設關
\x           # 顯示的時候將每個 row 都展開來，預設關
```

## CheatSheet

- [cheatsheet](http://www.postgresonline.com/downloads/special_feature/postgresql83_psql_cheatsheet.pdf)