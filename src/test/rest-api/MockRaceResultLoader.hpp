#pragma once
#include <gmock/gmock.h>
#include "rest-api/RaceObserver.hpp"

class MockRaceResultLoader : public RestApi::RaceResultLoader
{
public:
    MOCK_METHOD(std::optional<uint64_t>, getLatestId, (), (const, override));
    MOCK_METHOD(std::string, get, (uint64_t), (const, override));
    MOCK_METHOD(void, store, (uint64_t, const std::string&), (override));
};
