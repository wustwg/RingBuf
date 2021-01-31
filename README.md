# RingBuf
A high performance RingBuf base by C++!
Support read/write,readv/writev.

# 环形缓冲区
一个基于C++实现的高效的环形缓冲区，代码数量很少，可根据自己的需求扩展接口。
支持read/write、readv/writev接口。

## 环形缓冲区的适用场景
1. 需要自己提供read/write接口
2. 同线程内，支持一边读一边写

## 注意事项
1. 接口非线程安全。