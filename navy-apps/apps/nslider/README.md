# NSlider

1. copy slides.pdf to `slides/`  将pdf复制到`slides/`目录下面
2. run `convert.sh`  运行`convert.sh` 
3. modify the variable `N` in `src/main.cpp` to the total number of the slides 修改在`src/main.cpp`中的变量`N`为幻灯片总数

## Error report

If you see the following error, refer to [here][sf] for solution.  如果您看到以下错误，请参考[这里][sf]以获取解决方案。
```
convert-im6.q16: attempt to perform an operation not allowed by the security policy `PDF' @ error/constitute.c/IsCoderAuthorized/408.
翻译：convert-im6.q16: 尝试执行安全策略 'PDF' 不允许的操作 @ error/constitute.c/IsCoderAuthorized/408.
```

[sf]: https://stackoverflow.com/questions/52998331/imagemagick-security-policy-pdf-blocking-conversion
