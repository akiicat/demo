# Git tutorial

### 檢視目前設定

```
git config --list
```

### 設定 username 及 email

設定的東西放在 `~/.gitconfig`，或者直接修改檔案。

```
git config --global user.name "akiicat"
git config --global user.email "aaaa1379@gmail.com"
```

### 便利的設定 -- 別名

[more information](https://github.com/kaochenlong/eddie-dotfiles/blob/master/.gitconfig)

```
# ~/.gitconfig
[alias]
  co = checkout
  br = branch
  aa = add --all
```

### 查看現有的 branch

檢視合併過的 branch

```sh
git branch
git branch --merged
```

### 切換 branch 如果不存在會自動建立

```sh
git checkout -b add_user
```

### 在 branch commit 東西

```sh
git status
git add -A
git reset HEAD
git commit -am "add user"
```

### merge branch 到 master

- --no-ff: 分支會有小耳朵的線圖

```sh
git checkout master
git merge add_user
git merge add_user --no-ff
```

### 刪除 branch

```sh
# 已經 merge 過的 branch
git branch -d add_user
# 尚未 merge 過的 branch
git branch -D add_user
```

### 回到先前的版本，且推回去

```sh
git reset --hard <old-commit-id>
git push -f origin branch
```

可以使用 reflog 你所輸入的指令，然後可以回到你之前所輸入的指令，或者把刪掉的節點撿回來

```sh
git reflog
git reset s1f09xc7 --hard
git cherry-pick 823520ed
```

### 回到 untrack 的狀態

不管有沒有 commit 過都會回到 untrack 的狀態，當檔案不在 repo 裡面的時候才會使用。

```
git rm --cached <file>
git rm -r --cached <dir>
```

### 修改最後一次 commit 的內容

```
git commit --amend
```

### 把 commit 併到上一次的 commit

```
git add -A
git commit --amend -m "..."
```

### 提交空白目錄

因為 git 不能提交空白的目錄，慣例會在檔案中放 `.keep` 或是 `.gitkeep` 的空白檔案。

### 檢視提交紀錄

一行精簡版與檢視單一檔案的歷史紀錄。

```
git log
git log --graph --oneline
git log -p <file>
```

### 檢視每行程式碼的紀錄

```
git blame <file>
```

### 讓檔案或目錄回到最近一次 commit 的狀態

不小心將某個檔案刪除了

```
git checkout hello.rb
git checkout .
```

### 取消最後一次提交 -- 復原至最後一次提交

- soft: 變回 untracked 的狀態
- hard: 變回最後一次 commit 的狀態

```
git reset HEAD^
git reset HEAD^ --hard
```

### 重置所有 add 的東西

```
git add -A
git reset HEAD
```

### 刪除 untracked 的檔案

已經存在修改過後尚未 commit 的檔案不會被刪除。

```
git clean -f
git clean -df
```

### rebase

### 新增、檢視標籤

tag 預設不會被 push 上去

推上特定標籤、推上所有標籤。

```
git tag
git tag 1.0.0
git push origin 1.0.0
git push origin --tags
```

### 遠端節點

檢視、新增與刪除

```
git remote -v
git remote add origin git@github.com:akiicat/akiicat.github.io.git
git remote rm origin
```

### 上傳

上傳 master 分支至 origin 節點

```
git push origin master
git push origin <branch>
```

### 刪除遠端分支

```
git push origin :<branch>
git push origin :add_comments
```

### 下載更新

git pull = git fetch + git merge

```
git pull origin master
```

### 暫存

工作做到一半，有急事需先暫停手邊的工作。

把目前狀態存下來、把最後一次 stash 拿出來用、檢視 stash 、刪除最後一次的 stash。

```
git stash save
git stash apply
git stash list
git stash drop
```

### 針對每個節點刪除特定檔案

不小心把帳號密碼提交上去

```
git filter-branch --tree-filter "rm -f config/password.txt"
```

### 合併某個特定節點

移花接木

```
git cherry-pick <commit-id>
git cherry-pick <commit-id> --edit # 編輯訊息
git cherry-pick <commit-id> --no-commit # 不合併
```

### 更新 fork 過的檔按

fork 後會停留在當初 fork 的狀態，如果要獲得更新過的專案：

新增原來 repo 的遠端節點、取得該節點檔案、合併。

```
git remote add upstream <origin-repo>
git fetch upstream
git merge upstream/master
```

### git flow

- [ihower git flow](https://ihower.tw/blog/archives/5140)
- [Branching Workflows](https://git-scm.com/book/en/v2/Git-Branching-Branching-Workflows)
- [A successful Git branching model](http://nvie.com/posts/a-successful-git-branching-model/)

```sh
# 初始化 git flow
git flow init

# 新增 feature "sign_up"
git flow feature start sign_up

# 完成 feature "sign_up"
git flow feature finish sign_up
```

### gitignore

只在對之後進來的檔案才有 filter 的效果，之前進來就沒有用了，但可以使用下面的方法拿掉

```sh
git rm --cached <file>
```



