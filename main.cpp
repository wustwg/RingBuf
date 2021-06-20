#include <iostream>
#include <cassert>
#include "RingBuf.h"
#include <chrono>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <condition_variable>

void testCopy() {
    RingBuf ringBuf(8000);
    std::mutex mutex{};
    std::condition_variable conditionVariable{};
    bool isReadOver = false;
    // 负责拷贝数据到缓冲区
    std::thread thread1([&]() {
        printf("thread1 tid:%d.\n", gettid());
        auto fd = open("/home/wg/CLionProjects/ringbuf/陈翔六点半之民间高手.mp4", O_NONBLOCK | O_RDONLY);
//        auto fd = open("/home/wg/CLionProjects/ringbuf/a.txt", O_NONBLOCK | O_RDONLY);
        if (fd <= 0) {
            printf("open file failed.error:%s\n", strerror(errno));
            return;
        }
        ssize_t len = 0;
        uint64_t totalLen = 0;
        std::unique_lock<std::mutex> uniqueLock{mutex};
        while (true) {
            struct iovec src[2]{};
            auto ret = ringBuf.getWriteIoVec(src, 2);
            if (ret == 0) {
                // 不可写了
//                printf("will wait tid:%d.\n", gettid());
                conditionVariable.notify_all();
                conditionVariable.wait(uniqueLock);
//                printf("wait over tid:%d.\n", gettid());
                ret = ringBuf.getWriteIoVec(src, 2);
                assert(ret > 0);
            }
            len = readv(fd, src, 2);
            if (len <= 0) {
                isReadOver = true;
                // 拷贝完成
                printf("copy file over,totalLen:%lu\n", totalLen);
                conditionVariable.notify_all();
                break;
            }
            ringBuf.setWriteSize(len);
            totalLen += len;
        }
        printf("读取结束。");
        close(fd);
    });

    std::thread thread2([&]() {
        printf("thread2 tid:%d.\n", gettid());
        RingBuf ringBuf2(2346);
        auto fd = open("陈翔六点半之民间高手2.mp4", O_NONBLOCK | O_WRONLY | O_CREAT);
//        auto fd = open("a.txt", O_NONBLOCK | O_WRONLY | O_CREAT);
        if (fd <= 0) {
            printf("open file2 failed.error:%s\n", strerror(errno));
            return;
        }
        ssize_t len = 0;
        uint64_t totalLen = 0;
        std::unique_lock<std::mutex> uniqueLock{mutex};
        while (true) {
            int i = 0;

            struct iovec src[2]{};
            auto ret = ringBuf.getReadIoVec(src, 2);
            if (ret > 0) {
                int ret2 = ringBuf2.writev(src, 2);
                ringBuf.setRreadSize(ret2);
                assert(ret - ringBuf.dataCount() == ret2);
                totalLen += ret2;
            }

            char buf[134]{};
            while (i ++ < 11) {
                len = ringBuf2.read(buf, sizeof(buf));
                if (len == 0) {
                    break;
                }
                auto ret = write(fd, buf, len);
                assert(ret == len);
                totalLen += len;
            }
            if (isReadOver && i < 11) {
                printf("copy file2 over1,totalLen:%lu\n", totalLen);
                break;
            }
            if (!isReadOver) {
                conditionVariable.notify_all();
                conditionVariable.wait(uniqueLock);
            }

           /* struct iovec src[2]{};
            auto ret = ringBuf.getReadIoVec(src, 2);
            if (ret == 0) {
                if (isReadOver) {
                    printf("copy file2 over1,totalLen:%lu\n", totalLen);
                    break;
                }
                conditionVariable.notify_all();
                // 不可读了
//                printf("will wait tid:%d.\n", gettid());
                conditionVariable.wait(uniqueLock);
//                printf("wait over tid:%d.\n", gettid());
//                ret = ringBuf.getReadIoVec(src, 2);
//                if (status == std::cv_status::timeout) {
//                    printf("copy file2 over,totalLen:%lu\n", totalLen);
//                    break;
//                }
            }
            len = writev(fd, src, 2);
            assert(len == ret);
            ringBuf.setRreadSize(len);
            totalLen += len;*/
        }
        printf("拷贝结束");
        close(fd);
    });

    thread1.join();
    thread2.join();
}

int main() {
    std::cout << "Hello, World!" << std::endl;
//    RingBuf buf(8000);
//    char buf2[] = {"aa889988888888887766665555555555555555555555666"};
//    char buf3[1500]{};
////    auto ret = buf.write(buf2, sizeof(buf2));
////    printf("write len:%ld\n", ret);
////    ret = buf.read(buf3, sizeof(buf3));
////    printf("write len:%ld buf3:%s left:%ld\n", ret, buf3, buf.dataCount());
//    int i = 0;
//    uint64_t len1 = 0, len2 = 0;
    using namespace std::chrono;
    auto start = steady_clock::now();
//    while (i++ < 2000000) {
//        auto ret = buf.write(buf2, sizeof(buf2));
//        len1 += ret;
//        ssize_t len3;
//        while ((len3 = buf.read(buf3, sizeof(buf3))) > 0) {
//            len2 += len3;
//        }
//    }
    testCopy();
//    printf("write and read len1:%lu len2:%lu cost:%ldus\n", len1, len2,
//           (duration_cast<microseconds>(steady_clock::now() - start)).count());
    printf("write and read cost:%ldus\n",
           (duration_cast<microseconds>(steady_clock::now() - start)).count());
//    assert(len2 == len1);

    return 0;
}
