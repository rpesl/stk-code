#pragma once
#include <gmock/gmock.h>
#include "rest-api/RaceObserver.hpp"

class MockRaceResultLoader : public RestApi::RaceResultLoader
{
public:
    MOCK_METHOD(std::optional<size_t>, getLatestId, (), (const, override));
    MOCK_METHOD(std::string, get, (size_t), (const, override));
    MOCK_METHOD(void, store, (size_t, const std::string&), (override));
};
