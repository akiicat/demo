# Git

## Git diff

```shell
git diff # working area 的 diff
git diff <file> # 某個檔案的 diff
git diff --staged # stage 狀態的 diff
git difftool # 用 vim 開啟 diff
git diff HEAD~2 HEAD # 兩個 commit 的差異
```



## Git log

```shell
# 格式
# 9791961 Akiicat 6 days ago - Initial Data
git log --pretty=format:"%h %an %ar - %s"

# 搜尋 log 訊息
git log --grep="message"

# 一行格式
git log --oneline --graph
git log --all --decorate --oneline --graph

# 顯示所有 commit 的差異
git log -p
git log -p -n 2 # 限制數量 2 個最新的 commit
```



## Git show

顯示 HEAD 的詳細訊息

```shell
git show
git show <commit-hash>
```



## Git Branch

```shell
git branch -r # 顯示遠程分支，本地不會顯示
git branch -va # `-a` 顯示本地加遠端的分支 `-v` 包含 HEAD commit 的訊息
```



## Git Revert

`revert` 跟 `reset` 很像，不過 `revert` 會保留紀錄繼續 commit 往下前進，使用時記得讓 working 區域保持乾淨

```shell
# 原始
# A -> B -> C

# A -> B
git reset HEAD~1 --hard

# A -> B -> C -> B
git revert HEAD~1 --no-edit
```



## Git Checkout

```shell
# 在 merge 過後發生衝突
# 可以直接使用 checkout 來修正衝突
git checkout --ours staging.txt
git checkout --theirs staging.txt
```



## Git Commit

```shell
git commit --no-edit # Git 預設幫你輸入文字
```



## Git pull 

```shell
# pull = fetch + merge
# --no-edit 預設 merge 文字訊息
# 54cccee (HEAD, master) Merge branch 'master' of /s/remote-project/1
git pull --no-edit origin master
```



## Git Bisect

Binary Search，通常用來 Debug 用，找出第一個弄壞程式碼的 commit 

```shell
git bisect start # 初始化
git bisect start <good> <bad> # 給定範圍，可以是 tag 或 commit

git bisect bad # 目前是壞的
git bisect good HEAD~5 # 在五個之前的 commit 是好的

# 這時候會用 Binary Search 縮小範圍，只需要判斷當前的 commit 是好的還是壞的就行
git bisect bad
git bisect good 

git bisect reset # 重設

# 也可以用來標記 tag 來標記範圍
git bisect bad 1.2.9 
git bisect good 1.2.8

# 可以使用測試下去跑，系統回傳 0 是 good，系統回傳 1 是 bad
git bisect run <shell>
```



## Git Blame

```shell
git blame -L 6,8 list.html # 限制行數
```



## Git Cherry Pick

用法跟 `merge` 一樣，不過他可以指定某個 commit，發生衝突可以使用 `checkout`，衝突後繼續使用 `continue` 參數繼續，或者使用 abort 停止合併

```shell
git cherry-pick <branch~n>
git cherry-pick new_branch~1 
git cherry-pick --continue
git cherry-pick --abort
```



## Git Rebase

```shel
git rebase --interactive HEAD~2
git rebase --interactive --root
```

