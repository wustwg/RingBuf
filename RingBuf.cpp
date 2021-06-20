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

ssize_t RingBuf::write(const char *buf, ssize_t size) {
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
        mWIndex = myMod(mWIndex + copyLen, mSize);
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
        mRIndex = myMod(mRIndex + copyLen, mSize);
    }

    return maxCopyLen;
}

bool RingBuf::empty() const {
    return mPending == mSize;
}

ssize_t RingBuf::getWriteIoVec(struct iovec *out, ssize_t size) {
    memset(out, 0, sizeof(iovec) * size);

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
        tmpWindex = myMod(tmpWindex + copyLen, mSize);
        i++;
    }
    return maxCopyLen;
}

ssize_t RingBuf::dataCount() const {
    return mPending;
}

ssize_t RingBuf::freeCount() const {
    return mSize - mPending;
}

ssize_t RingBuf::getReadIoVec(struct iovec *out, ssize_t size) {
    memset(out, 0, sizeof(iovec) * size);

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
        tmpRindex = myMod(tmpRindex + copyLen, mSize);
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
    setReadSize(rsize);
    return rsize;
}

ssize_t RingBuf::writev(struct iovec *src, int size) {
    struct iovec out[2]{};
    auto len = getWriteIoVec(out, 2);
    if (len == 0) {
        return 0;
    }
    auto wSize = copyIoVec(src, size, out, 2);
    setWriteSize(wSize);
    return wSize;
}

ssize_t RingBuf::copyIoVec(struct iovec *src, int srcSize, struct iovec *dst, int dstSize) {
    // 分别表示i j 对应的已拷贝的数据长度
    ssize_t copyLenI = 0, copyLenJ = 0;
    ssize_t totalCopyLen = 0;
    // i表示src的下标，j表示dst的下标
    for (int i = 0, j = 0; i < srcSize && j < dstSize;) {
        assert(src[i].iov_len >= copyLenI);
        assert(dst[j].iov_len >= copyLenJ);
        if (src[i].iov_base == nullptr || src[i].iov_len - copyLenI == 0) {
            i++;
            copyLenI = 0;
            continue;
        }
        if (src[j].iov_base == nullptr || dst[j].iov_len - copyLenJ == 0) {
            j++;
            copyLenJ = 0;
            continue;
        }
        ssize_t minCopyLen = std::min(src[i].iov_len - copyLenI, dst[j].iov_len - copyLenJ);
        memcpy((char *) dst[j].iov_base + copyLenJ, (char *) src[i].iov_base + copyLenI, minCopyLen);
        copyLenI += minCopyLen;
        copyLenJ += minCopyLen;
        totalCopyLen += minCopyLen;
    }
    return totalCopyLen;
}

void RingBuf::setWriteSize(ssize_t ssize) {
    mPending += ssize;
    assert(mPending <= mSize);
    mWIndex = myMod(mWIndex + ssize, mSize);
}

void RingBuf::setReadSize(ssize_t ssize) {
    mPending -= ssize;
    assert(mPending >= 0);
    mRIndex = myMod(mRIndex + ssize, mSize);
}

RingBuf::~RingBuf() {
    delete mBuf;
    mBuf = nullptr;
}

ssize_t RingBuf::myMod(ssize_t left, ssize_t right) {
    while (left >= right) {
        left -= right;
    }
    return left;
}
