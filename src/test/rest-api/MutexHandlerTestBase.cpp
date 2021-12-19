#include "test/rest-api/MutexHandlerTestBase.hpp"

RestApi::GetMutex MutexHandlerTestBase::getMutex()
{
    return [&] {
        return &mutex_;
    };
}
