首先编译运行：

```console
$ make
$ ./web-server -c web-server.conf
```

测试：

```console
$ wget 127.0.0.1:10086
...
$ wget 127.0.0.1:10086/index.html
...
$ wget 127.0.0.1:10086/firefox.jpg
...
```
