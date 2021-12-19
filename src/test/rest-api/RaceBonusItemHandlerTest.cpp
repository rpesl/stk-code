#include <gtest/gtest.h>
#include "rest-api/Handler.hpp"
#include "test/rest-api/MockDataExchange.hpp"

using testing::ByMove;
using testing::ByRef;
using testing::Sequence;
using testing::InSequence;
using testing::Return;
using testing::ReturnRef;
using testing::NiceMock;

class RaceBonusItemHandlerTest : public testing::Test
{
};

TEST_F(RaceBonusItemHandlerTest, ItemEmpty)
{
    std::mutex mutex;
    NiceMock<MockRaceBonusItemExchange> items;
    auto handler = RestApi::Handler::createRaceBonusItemHandler(items, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, "[]");
}

static std::unique_ptr<NiceMock<MockBonusItemExchange>> createMockItemExchange(
    size_t id,
    const RestApi::Position& position,
    const std::string& type,
    const std::optional<std::string>& originalType,
    int ticksUntilReturn,
    const std::optional<int>& usedUpCounter)
{
    auto item = std::make_unique<NiceMock<MockBonusItemExchange>>();
    ON_CALL(*item, getId()).WillByDefault(Return(id));
    ON_CALL(*item, getPosition()).WillByDefault(Return(position));
    ON_CALL(*item, getType()).WillByDefault(Return(type));
    ON_CALL(*item, getOriginalType()).WillByDefault(Return(originalType));
    ON_CALL(*item, getTicksUntilReturn()).WillByDefault(Return(ticksUntilReturn));
    ON_CALL(*item, getUsedUpCounter()).WillByDefault(Return(usedUpCounter));
    return item;
}

TEST_F(RaceBonusItemHandlerTest, ItemSingle)
{
    auto item = createMockItemExchange(
        1,
        {1.0f, 2.0f, 3.0f},
        "MyType",
        "OriginalType",
        5,
        13);
    std::vector<std::unique_ptr<RestApi::BonusItemWrapper>> items;
    items.push_back(std::move(item));
    NiceMock<MockRaceBonusItemExchange> trackItems;
    EXPECT_CALL(trackItems, getItems())
        .Times(1)
        .WillOnce(Return(ByMove(std::move(items))));
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceBonusItemHandler(trackItems, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": 1,\n"
        "        \"position\": [\n"
        "            1.0,\n"
        "            2.0,\n"
        "            3.0\n"
        "        ],\n"
        "        \"type\": \"MyType\",\n"
        "        \"original-type\": \"OriginalType\",\n"
        "        \"ticks-until-return\": 5,\n"
        "        \"used-up-counter\": 13\n"
        "    }\n"
        "]");
}

TEST_F(RaceBonusItemHandlerTest, ItemMany)
{
    auto item0 = createMockItemExchange(
        2,
        {5.0f, 3.0f, 8.0f},
        "TYPE",
        std::nullopt,
        1,
        std::nullopt);
    auto item1 = createMockItemExchange(
        4,
        {0.5f, 1.0f, 0.0f},
        "ABC",
        "Original",
        0,
        std::nullopt);
    std::vector<std::unique_ptr<RestApi::BonusItemWrapper>> items;
    items.push_back(std::move(item0));
    items.push_back(std::move(item1));
    NiceMock<MockRaceBonusItemExchange> trackItems;
    EXPECT_CALL(trackItems, getItems())
        .Times(1)
        .WillOnce(Return(ByMove(std::move(items))));
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceBonusItemHandler(trackItems, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": 2,\n"
        "        \"position\": [\n"
        "            5.0,\n"
        "            3.0,\n"
        "            8.0\n"
        "        ],\n"
        "        \"type\": \"TYPE\",\n"
        "        \"original-type\": null,\n"
        "        \"ticks-until-return\": 1,\n"
        "        \"used-up-counter\": null\n"
        "    },\n"
        "    {\n"
        "        \"id\": 4,\n"
        "        \"position\": [\n"
        "            0.5,\n"
        "            1.0,\n"
        "            0.0\n"
        "        ],\n"
        "        \"type\": \"ABC\",\n"
        "        \"original-type\": \"Original\",\n"
        "        \"ticks-until-return\": 0,\n"
        "        \"used-up-counter\": null\n"
        "    }\n"
        "]");
}

TEST_F(RaceBonusItemHandlerTest, SelectItem)
{
    NiceMock<MockRaceBonusItemExchange> trackItems;
    auto createItems = [] {
        auto item0 = createMockItemExchange(
            0,
            {1.0f, 10.0f, 0.5f},
            "1",
            "A",
            1,
            std::nullopt);
        auto item1 = createMockItemExchange(
            2,
            {3.0f, 4.0f, 1.0f},
            "2",
            std::nullopt,
            2,
            56);
        std::vector<std::unique_ptr<RestApi::BonusItemWrapper>> items;
        items.push_back(std::move(item0));
        items.emplace_back(nullptr);
        items.push_back(std::move(item1));
        return items;
    };
    EXPECT_CALL(trackItems, getItems()).WillRepeatedly(createItems);
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceBonusItemHandler(trackItems, mutex);
    auto [status, result] = handler->handleGet("0");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": 0,\n"
        "    \"position\": [\n"
        "        1.0,\n"
        "        10.0,\n"
        "        0.5\n"
        "    ],\n"
        "    \"type\": \"1\",\n"
        "    \"original-type\": \"A\",\n"
        "    \"ticks-until-return\": 1,\n"
        "    \"used-up-counter\": null\n"
        "}");
    std::tie(status, result) = handler->handleGet("1");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, "null");
    std::tie(status, result) = handler->handleGet("2");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": 2,\n"
        "    \"position\": [\n"
        "        3.0,\n"
        "        4.0,\n"
        "        1.0\n"
        "    ],\n"
        "    \"type\": \"2\",\n"
        "    \"original-type\": null,\n"
        "    \"ticks-until-return\": 2,\n"
        "    \"used-up-counter\": 56\n"
        "}");
    std::tie(status, result) = handler->handleGet("3");
    EXPECT_EQ(status, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(result, "null");
    std::tie(status, result) = handler->handleGet("abc");
    EXPECT_EQ(status, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(result, "null");
}

TEST_F(RaceBonusItemHandlerTest, PostAll)
{
    std::mutex mutex;
    NiceMock<MockRaceBonusItemExchange> items;
    auto handler = RestApi::Handler::createRaceBonusItemHandler(items, mutex);
    auto [status, result] = handler->handlePost("{}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::NOT_FOUND);
}

TEST_F(RaceBonusItemHandlerTest, PostById)
{
    std::mutex mutex;
    NiceMock<MockRaceBonusItemExchange> items;
    auto createItems = [] (const std::function<void(MockBonusItemExchange&, MockBonusItemExchange&)>& setExpectations) {
        auto item0 = createMockItemExchange(
            8,
            {1.0f, 10.0f, 0.5f},
            "1",
            "A",
            1,
            std::nullopt);
        auto item1 = createMockItemExchange(
            24,
            {3.0f, 4.0f, 1.0f},
            "2",
            std::nullopt,
            2,
            56);
        setExpectations(*item0, *item1);
        std::vector<std::unique_ptr<RestApi::BonusItemWrapper>> items;
        items.push_back(std::move(item0));
        items.emplace_back(nullptr);
        items.push_back(std::move(item1));
        return items;
    };
    Sequence sequence;
    EXPECT_CALL(items, getItems()).Times(1).InSequence(sequence).WillRepeatedly([&createItems] {
        return createItems([](MockBonusItemExchange& item0, MockBonusItemExchange&) {
            EXPECT_CALL(item0, setType("newType")).Times(1);
            EXPECT_CALL(item0, setOriginalType(std::optional<std::string>())).Times(1);
            EXPECT_CALL(item0, setTicksUntilReturn(12)).Times(1);
            EXPECT_CALL(item0, setUsedUpCounter(std::optional<int>(37))).Times(1);
        });
    });
    EXPECT_CALL(items, getItems()).Times(1).InSequence(sequence).WillRepeatedly([&createItems] {
        return createItems([](MockBonusItemExchange&, MockBonusItemExchange& item1) {
            EXPECT_CALL(item1, setType("T2")).Times(1);
            EXPECT_CALL(item1, setOriginalType(std::optional<std::string>("O2"))).Times(1);
            EXPECT_CALL(item1, setTicksUntilReturn(34)).Times(1);
            EXPECT_CALL(item1, setUsedUpCounter(std::optional<int>())).Times(1);
        });
    });
    EXPECT_CALL(items, getItems()).Times(1).InSequence(sequence).WillRepeatedly([&createItems] {
        return createItems([](MockBonusItemExchange&, MockBonusItemExchange& item1) {
            EXPECT_CALL(item1, setType("T3")).Times(1);
        });
    });
    EXPECT_CALL(items, getItems()).Times(1).InSequence(sequence).WillRepeatedly([&createItems] {
        return createItems([](MockBonusItemExchange&, MockBonusItemExchange& item1) {
            EXPECT_CALL(item1, setType("T4")).Times(1);
            EXPECT_CALL(item1, setOriginalType(std::optional<std::string>("O4"))).Times(1);
        });
    });
    EXPECT_CALL(items, getItems()).Times(1).InSequence(sequence).WillRepeatedly([&createItems] {
        return createItems([](MockBonusItemExchange&, MockBonusItemExchange& item1) {
            EXPECT_CALL(item1, setType("T5")).Times(1);
            EXPECT_CALL(item1, setOriginalType(std::optional<std::string>("O5"))).Times(1);
            EXPECT_CALL(item1, setTicksUntilReturn(5)).Times(1);
        });
    });
    EXPECT_CALL(items, getItems()).Times(2).InSequence(sequence).WillRepeatedly([&createItems] {
        return createItems([](MockBonusItemExchange&, MockBonusItemExchange&) {});
    });
    auto handler = RestApi::Handler::createRaceBonusItemHandler(items, mutex);
    auto [status, result] = handler->handlePost(
        "0",
        "{"
        "   \"type\": \"newType\","
        "   \"original-type\": null,"
        "   \"ticks-until-return\": 12,"
        "   \"used-up-counter\": 37"
        "}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    std::tie(status, result) = handler->handlePost(
        "2",
        "{"
        "   \"type\": \"T2\","
        "   \"original-type\": \"O2\","
        "   \"ticks-until-return\": 34,"
        "   \"used-up-counter\": null"
        "}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    std::tie(status, result) = handler->handlePost(
        "2",
        "{"
        "   \"type\": \"T3\""
        "}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    std::tie(status, result) = handler->handlePost(
        "2",
        "{"
        "   \"type\": \"T4\","
        "   \"original-type\": \"O4\""
        "}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    std::tie(status, result) = handler->handlePost(
        "2",
        "{"
        "   \"type\": \"T5\","
        "   \"original-type\": \"O5\","
        "   \"ticks-until-return\": 5"
        "}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    std::tie(status, result) = handler->handlePost(
        "1",
        "{}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::NOT_FOUND);
    std::tie(status, result) = handler->handlePost(
        "3",
        "{}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::NOT_FOUND);
}

TEST_F(RaceBonusItemHandlerTest, Delete)
{
    NiceMock<MockRaceBonusItemExchange> items;
    InSequence sequence;
    EXPECT_CALL(items, remove(1)).Times(1);
    EXPECT_CALL(items, remove(0)).Times(1);
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceBonusItemHandler(items, mutex);
    EXPECT_EQ(handler->handleDelete("1").first, RestApi::STATUS_CODE::NO_CONTENT);
    EXPECT_EQ(handler->handleDelete("0").first, RestApi::STATUS_CODE::NO_CONTENT);
    EXPECT_THROW(handler->handleDelete("abc"), std::invalid_argument);
}

MATCHER_P(ItemIs, other, "Match items")
{
    return arg.position == other.position
           && arg.type == other.type
           && arg.originalType == other.originalType
           && arg.ticksUntilReturn == other.ticksUntilReturn
           && arg.usedUpCounter == other.usedUpCounter;
}

TEST_F(RaceBonusItemHandlerTest, Put)
{
    std::mutex mutex;
    NiceMock<MockRaceBonusItemExchange> items;
    Sequence sequence;
    EXPECT_CALL(items, add(ItemIs(
        RestApi::NewBonusItem{
            {5.0f, 0.5f, -1.0f},
            "NewType",
            "NewOriginalType",
            12,
            std::nullopt})))
        .Times(1)
        .InSequence(sequence)
        .WillOnce([] {
            return createMockItemExchange(
                5,
                {10.0f, 1.0f, 100.0f},
                "TheType",
                "TheOriginalType",
                111,
                21);
        });
    EXPECT_CALL(items, add(ItemIs(
        RestApi::NewBonusItem{
            {1.0f, -2.0f, 3.0f},
            "T",
            std::nullopt,
            1,
            37})))
        .Times(1)
        .InSequence(sequence)
        .WillOnce([] {
            return createMockItemExchange(
                8,
                {-3.0f, 4.0f, 2.0f},
                "TypeT",
                std::nullopt,
                53,
                std::nullopt);
        });
    auto handler = RestApi::Handler::createRaceBonusItemHandler(items, mutex);
    auto [status, result] = handler->handlePut(
        "{"
        "   \"position\": [5.0, 0.5, -1.0],"
        "   \"type\": \"NewType\","
        "   \"original-type\": \"NewOriginalType\","
        "   \"ticks-until-return\": 12,"
        "   \"used-up-counter\": null"
        "}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::CREATED);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": 5,\n"
        "    \"position\": [\n"
        "        10.0,\n"
        "        1.0,\n"
        "        100.0\n"
        "    ],\n"
        "    \"type\": \"TheType\",\n"
        "    \"original-type\": \"TheOriginalType\",\n"
        "    \"ticks-until-return\": 111,\n"
        "    \"used-up-counter\": 21\n"
        "}");
    std::tie(status, result) = handler->handlePut(
        "{"
        "   \"position\": [1.0, -2.0, 3.0],"
        "   \"type\": \"T\","
        "   \"original-type\": null,"
        "   \"ticks-until-return\": 1,"
        "   \"used-up-counter\": 37"
        "}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::CREATED);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": 8,\n"
        "    \"position\": [\n"
        "        -3.0,\n"
        "        4.0,\n"
        "        2.0\n"
        "    ],\n"
        "    \"type\": \"TypeT\",\n"
        "    \"original-type\": null,\n"
        "    \"ticks-until-return\": 53,\n"
        "    \"used-up-counter\": null\n"
        "}");
}
