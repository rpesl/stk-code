#pragma once
#include <gtest/gtest.h>
#include "rest-api/Handler.hpp"

class MutexHandlerTestBase : public testing::Test
{
protected:
    RestApi::GetMutex getMutex();

private:
    std::mutex mutex_;
};
