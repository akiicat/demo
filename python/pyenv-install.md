# pyenv install

[pyenv Github](https://github.com/pyenv/pyenv)

## Mac

### install

```shell
brew update
brew install pyenv
```

### env path

```shell
# .bash_profile
if command -v pyenv 1>/dev/null 2>&1; then
  eval "$(pyenv init -)"
fi
```

```
# 安裝
pyenv install 3.6.4
# 查看所有版本
pyenv versions
# 選擇預設版本：會在 ~/.pyenv/version 記錄 global 的版本
pyenv global 3.6.4
# 選擇版本：會在當前資料夾下建立 .python_version 檔案，裡面寫著 version
pyenv local 3.6.4
```

### upgrade

```
brew upgrade pyenv
```