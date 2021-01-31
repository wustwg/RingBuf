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
    ssize_t write(char *buf, ssize_t size);

    ssize_t read(char *buf, ssize_t size);

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

    ssize_t getReadIoVec(struct iovec *out, ssize_t size);

    void setRreadSize(ssize_t ssize);

    static ssize_t copyIoVec(struct iovec *src, int srcSize, struct iovec *dst, int dstSize);

    bool empty();

    /**
     * 获取已写入的数据大小
     * @return
     */
    ssize_t dataCount();

    /**
     * 获取剩余空间大小
     * @return
     */
    ssize_t freeCount();

private:
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
    const ssize_t mSize;
    /**
     * 已写入的总数据量
     */
    ssize_t mPending;
};


#endif //RINGBUF_RINGBUF_H
