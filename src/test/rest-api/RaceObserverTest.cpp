#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "rest-api/RaceObserver.hpp"
#include "test/rest-api/MockRaceResultLoader.hpp"

using testing::ByMove;
using testing::ByRef;
using testing::InSequence;
using testing::Return;
using testing::ReturnRef;
using testing::NiceMock;

class RaceObserverTest : public testing::Test
{
};

TEST_F(RaceObserverTest, StartStop)
{
    NiceMock<MockRaceResultLoader> loader;
    InSequence sequence;
    EXPECT_CALL(loader, getLatestId()).Times(1).WillOnce(Return(std::nullopt));
    EXPECT_CALL(loader, store(0, "abc 0")).Times(1);
    EXPECT_CALL(loader, store(0, "abc 1")).Times(1);
    size_t counter = 0;
    RestApi::RaceObserver observer(loader, [&counter] { return "abc " + std::to_string(counter++); });
    EXPECT_EQ(observer.getCurrentRaceId(), std::nullopt);
    observer.start();
    EXPECT_EQ(observer.getCurrentRaceId(), 0);
    observer.stop();
    EXPECT_EQ(observer.getCurrentRaceId(), std::nullopt);
}

TEST_F(RaceObserverTest, StartUpdateStop)
{
    NiceMock<MockRaceResultLoader> loader;
    InSequence sequence;
    EXPECT_CALL(loader, getLatestId()).Times(1).WillOnce(Return(9));
    EXPECT_CALL(loader, store(10, "TEST_15_TEST")).Times(1);
    EXPECT_CALL(loader, store(10, "TEST_16_TEST")).Times(1);
    EXPECT_CALL(loader, store(10, "TEST_17_TEST")).Times(1);
    size_t counter = 15;
    RestApi::RaceObserver observer(loader, [&counter] { return "TEST_" + std::to_string(counter++) + "_TEST"; });
    EXPECT_EQ(observer.getCurrentRaceId(), std::nullopt);
    observer.start();
    EXPECT_EQ(observer.getCurrentRaceId(), 10);
    observer.update();
    EXPECT_EQ(observer.getCurrentRaceId(), 10);
    observer.stop();
    EXPECT_EQ(observer.getCurrentRaceId(), std::nullopt);
}

TEST_F(RaceObserverTest, MultipleRaces)
{
    NiceMock<MockRaceResultLoader> loader;
    InSequence sequence;
    EXPECT_CALL(loader, getLatestId()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(loader, store(1, "COUNTER: -1")).Times(1);
    EXPECT_CALL(loader, store(1, "COUNTER: 0")).Times(1);
    EXPECT_CALL(loader, store(2, "COUNTER: 1")).Times(1);
    EXPECT_CALL(loader, store(3, "COUNTER: 2")).Times(1);
    EXPECT_CALL(loader, store(3, "COUNTER: 3")).Times(1);
    EXPECT_CALL(loader, store(3, "COUNTER: 4")).Times(1);
    EXPECT_CALL(loader, store(4, "COUNTER: 5")).Times(1);
    int counter = -1;
    RestApi::RaceObserver observer(loader, [&counter] { return "COUNTER: " + std::to_string(counter++); });
    EXPECT_EQ(observer.getCurrentRaceId(), std::nullopt);
    observer.start();
    EXPECT_EQ(observer.getCurrentRaceId(), 1);
    observer.update();
    EXPECT_EQ(observer.getCurrentRaceId(), 1);
    observer.start();
    EXPECT_EQ(observer.getCurrentRaceId(), 2);
    observer.start();
    EXPECT_EQ(observer.getCurrentRaceId(), 3);
    observer.update();
    EXPECT_EQ(observer.getCurrentRaceId(), 3);
    observer.stop();
    EXPECT_EQ(observer.getCurrentRaceId(), std::nullopt);
    observer.start();
    EXPECT_EQ(observer.getCurrentRaceId(), 4);
}
