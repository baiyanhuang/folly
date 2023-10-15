#include "folly/executors/GlobalExecutor.h"
#include "folly/executors/thread_factory/ThreadFactory.h"
#include "folly/io/async/DelayedDestructionBase.h"
#include "folly/logging/LogLevel.h"
#include "folly/system/ThreadId.h"
#include "folly/system/ThreadName.h"

#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/executors/ThreadedExecutor.h>
#include <folly/futures/Future.h>
#include <folly/lang/Align.h>
#include <folly/logging/xlog.h>
#include <folly/synchronization/LifoSem.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <tuple>

using namespace folly;
using namespace std;

#include <folly/futures/Future.h>

TEST(ThenInlineBug, good_on_return_value) {
    uint64_t threadId1 = 0;
    folly::via(folly::getGlobalCPUExecutor(), [&threadId1]() {
        threadId1 = folly::getCurrentThreadID();
        XLOG(INFO, "lambda 0");
        return true;
    }).thenInline([&threadId1](auto &&) {
        auto threadId2 = folly::getCurrentThreadID();
        XLOG(INFO, "lambda 1");
        ASSERT_EQ(threadId1, threadId2);
    }).wait();
}

TEST(ThenInlineBug, bug_on_return_future) {
    uint64_t threadId1 = 0;
    folly::via(folly::getGlobalCPUExecutor(), [&threadId1]() {
        threadId1 = folly::getCurrentThreadID();
        XLOG(INFO, "lambda 0");
        return folly::makeFuture(true);
    }).thenInline([&threadId1](auto &&) {
        auto threadId2 = folly::getCurrentThreadID();
        XLOG(INFO, "lambda 1");
        ASSERT_EQ(threadId1, threadId2);
    }).wait();
}