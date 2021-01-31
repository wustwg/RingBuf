//
// Created by wg on 2021/1/30.
//

#include <cassert>
#include <algorithm>
#include <cstring>
#include "RingBuf.h"

RingBuf::RingBuf(ssize_t size) : mSize(size) {
    assert(size > 0);
    mBuf = new char[size];
    mRIndex = 0;
    mWIndex = 0;
    mPending = 0;
}

ssize_t RingBuf::write(char *buf, ssize_t size) {
    ssize_t maxCopyLen = std::min(mSize - mPending, size);
    assert(maxCopyLen >= 0);
    if (maxCopyLen == 0) {
        return 0;
    }
    ssize_t maxCopyLenTmp = maxCopyLen;
    while (maxCopyLenTmp > 0) {
        ssize_t copyLen = std::min(mSize - mWIndex, maxCopyLenTmp);
        memcpy(mBuf + mWIndex, buf + maxCopyLen - maxCopyLenTmp, copyLen);
        mPending += copyLen;
        maxCopyLenTmp -= copyLen;
        mWIndex = (mWIndex + copyLen) % mSize;
    }

    return maxCopyLen;
}

ssize_t RingBuf::read(char *buf, ssize_t size) {
    ssize_t maxCopyLen = std::min(mPending, size);
    assert(maxCopyLen >= 0);
    if (maxCopyLen == 0) {
        return 0;
    }
    ssize_t maxCopyLenTmp = maxCopyLen;
    while (maxCopyLenTmp > 0) {
        ssize_t copyLen = std::min(mSize - mRIndex, maxCopyLenTmp);
        memcpy(buf + maxCopyLen - maxCopyLenTmp, mBuf + mRIndex, copyLen);
        mPending -= copyLen;
        assert(mPending >= 0);
        maxCopyLenTmp -= copyLen;
        mRIndex = (mRIndex + copyLen) % mSize;
    }

    return maxCopyLen;
}

bool RingBuf::empty() {
    return mPending == mSize;
}

ssize_t RingBuf::getWriteIoVec(struct iovec *out, ssize_t size) {
    ssize_t maxCopyLen = mSize - mPending;
    assert(maxCopyLen >= 0);
    if (maxCopyLen == 0) {
        return 0;
    }
    ssize_t maxCopyLenTmp = maxCopyLen;
    ssize_t i = 0, tmpWindex = mWIndex;
    while (maxCopyLenTmp > 0 && i < size) {
        ssize_t copyLen = std::min(mSize - tmpWindex, maxCopyLenTmp);
        out[i].iov_base = mBuf + tmpWindex;
        out[i].iov_len = copyLen;
        maxCopyLenTmp -= copyLen;
        tmpWindex = (tmpWindex + copyLen) % mSize;
        i++;
    }
    return maxCopyLen;
}

ssize_t RingBuf::dataCount() {
    return mPending;
}

ssize_t RingBuf::freeCount() {
    return mSize - mPending;
}

ssize_t RingBuf::getReadIoVec(struct iovec *out, ssize_t size) {
    assert(mPending >= 0);
    if (mPending == 0) {
        return 0;
    }
    ssize_t maxCopyLenTmp = mPending;
    ssize_t i = 0, tmpRindex = mRIndex;
    while (maxCopyLenTmp > 0 && i < size) {
        ssize_t copyLen = std::min(mSize - tmpRindex, maxCopyLenTmp);
        out[i].iov_base = mBuf + tmpRindex;
        out[i].iov_len = copyLen;
        maxCopyLenTmp -= copyLen;
        tmpRindex = (tmpRindex + copyLen) % mSize;
        i++;
    }
    return mPending;
}

ssize_t RingBuf::readv(struct iovec *out, int size) {
    struct iovec src[2]{};
    auto len = getReadIoVec(src, 2);
    if (len == 0) {
        return 0;
    }
    auto rsize = copyIoVec(src, 2, out, size);
    setRreadSize(rsize);
    return rsize;
}

ssize_t RingBuf::writev(struct iovec *src, int size) {
    struct iovec out[2]{};
    auto len = getWriteIoVec(out, 2);
    if (len == 0) {
        return 0;
    }
    auto wsize = copyIoVec(src, size, out, 2);
    setWriteSize(wsize);
    return wsize;
}

ssize_t RingBuf::copyIoVec(struct iovec *src, int srcSize, struct iovec *dst, int dstSize) {
    // 分别表示i j 对应的已拷贝的数据长度
    ssize_t copyLenI = 0, copyLenJ = 0;
    // i表示src的下标，j表示dst的下标
    // copyLenS copyLenD 表示ij对应的是 当前可拷贝的长度。
    for (int i = 0, j = 0, copyLenS = src[i].iov_len, copyLenD = dst[j].iov_len; i < srcSize && j < dstSize;) {
        assert(copyLenS >= 0);
        assert(copyLenD >= 0);
        if (src[i].iov_base == nullptr || copyLenS == 0) {
            i++;
            continue;
        }
        if (src[j].iov_base == nullptr || copyLenD == 0) {
            j++;
            continue;
        }
        ssize_t minLen = std::min(copyLenS, copyLenD);
        memcpy((char *) dst[j].iov_base + copyLenJ, (char *) src[i].iov_base + copyLenI, minLen);
        copyLenS -= minLen;
        copyLenD -= minLen;
    }
    return 0;
}

void RingBuf::setWriteSize(ssize_t ssize) {
    assert(ssize >= 0);
    mPending += ssize;
    mWIndex = (mWIndex + ssize) % mSize;
}

void RingBuf::setRreadSize(ssize_t ssize) {
    mPending -= ssize;
    assert(mPending >= 0);
    mRIndex = (mRIndex + ssize) % mSize;
}

RingBuf::~RingBuf() {
    delete mBuf;
    mBuf = nullptr;
}