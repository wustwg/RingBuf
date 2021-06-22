//
// Created by wg on 2021/1/30.
//

#ifndef RINGBUF_RINGBUF_H
#define RINGBUF_RINGBUF_H


#include <cstdlib>
#include <sys/uio.h>

class RingBuf {
public:
    /**
     *
     * @param size 缓冲区大小。
     */
    explicit RingBuf(ssize_t size);

    ~RingBuf();

    /**
     *
     * @param buf
     * @param size
     * @return 返回写的数据大小
     */
    ssize_t write(const char *buf, ssize_t size);

    ssize_t read(char *buf, ssize_t size);

    ssize_t peek(char *buf, ssize_t size) const ;

    ssize_t writev(struct iovec *src, int size);

    ssize_t readv(struct iovec *out, int size);

    /**
     * 获取缓冲区的可写iovec
     * @param out
     * @param size 大于0不超过2
     * @return 可写空间的总大小
     */
    ssize_t getWriteIoVec(struct iovec *out, ssize_t size);

    /**
     * 拷贝数据之后调用，表示已经拷贝了多少数据到缓冲区中；配合 getWriteIoVec 使用
     * @param ssize 数据大小
     */
    void setWriteSize(ssize_t ssize);

    /**
     * 获取当前缓冲区可读部分的iovec
     * @param out
     * @param size
     * @return
     */
    ssize_t getReadIoVec(struct iovec *out, ssize_t size);

    /**
     * 结合getReadIoVec使用的，如获取当前RingBuf的iovec之后，将其拷贝到别的RingBuf了，这时还要告诉原来的RingBuf被读取了多少数据
     * @param ssize 多少数据被读取了
     */
    void setReadSize(ssize_t ssize);

    static ssize_t copyIoVec(struct iovec *src, int srcSize, struct iovec *dst, int dstSize);

    bool empty() const;

    /**
     * 获取已写入的数据大小
     * @return
     */
    ssize_t dataCount() const;

    /**
     * 获取剩余空间大小
     * @return
     */
    ssize_t freeCount() const;

private:
    /**
     * 取摩操作，类似与%的作用，但是是用辗转相除法. left % right
     * @param left
     * @param right
     * @return
     */
    static ssize_t myMod(ssize_t left, ssize_t right);
    /**
     * 存储数据的buf指针
     */
    char *mBuf;

    /**
     * 写数据的起始下标
     */
    ssize_t mWIndex;
    /**
     * 读数据的起始下标
     */
    ssize_t mRIndex;
    /**
     * 存储空间大小
     */
    const ssize_t mSize;
    /**
     * 已写入的总数据量
     */
    ssize_t mPending;
};


#endif //RINGBUF_RINGBUF_H
