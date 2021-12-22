#include <gtest/gtest.h>
#include "rest-api/Handler.hpp"
#include "test/rest-api/MockDataExchange.hpp"

using testing::ByMove;
using testing::ByRef;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;
using testing::Sequence;

class RaceObjectHandlerTest : public testing::Test
{
};

TEST_F(RaceObjectHandlerTest, ObjectNotImplemented)
{
    std::mutex mutex;
    NiceMock<MockRaceObjectExchange> objects;
    auto handler = RestApi::Handler::createRaceObjectHandler(objects, mutex);
    auto [putStatusCode, putResult] = handler->handlePut("{}");
    EXPECT_EQ(putStatusCode, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(putResult, "null");
}

TEST_F(RaceObjectHandlerTest, ObjectEmpty)
{
    std::mutex mutex;
    NiceMock<MockRaceObjectExchange> objects;
    auto handler = RestApi::Handler::createRaceObjectHandler(objects, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, "[]");
}

static std::unique_ptr<MockObjectWrapper> createMockObjectExchange(
    size_t id,
    const std::string& name,
    const std::string& type,
    bool enabled,
    bool drivable,
    bool animated,
    const RestApi::Position& position,
    const RestApi::Position& centerPosition,
    const RestApi::Vector& rotation,
    const RestApi::Vector& scale,
    const std::string& lodGroup,
    const std::string& interaction,
    const std::function<std::unique_ptr<RestApi::LightWrapper>()>& light)
{
    auto object = std::make_unique<NiceMock<MockObjectWrapper>>();
    ON_CALL(*object, getId()).WillByDefault(Return(id));
    ON_CALL(*object, getName()).WillByDefault(Return(name));
    ON_CALL(*object, getType()).WillByDefault(Return(type));
    ON_CALL(*object, isEnabled()).WillByDefault(Return(enabled));
    ON_CALL(*object, isDrivable()).WillByDefault(Return(drivable));
    ON_CALL(*object, isAnimated()).WillByDefault(Return(animated));
    ON_CALL(*object, getPosition()).WillByDefault(Return(position));
    ON_CALL(*object, getCenterPosition()).WillByDefault(Return(centerPosition));
    ON_CALL(*object, getRotation()).WillByDefault(Return(rotation));
    ON_CALL(*object, getScale()).WillByDefault(Return(scale));
    ON_CALL(*object, getLodGroup()).WillByDefault(Return(lodGroup));
    ON_CALL(*object, getInteraction()).WillByDefault(Return(interaction));
    ON_CALL(*object, getChildren()).WillByDefault([] () -> std::vector<std::unique_ptr<RestApi::ObjectWrapper>> { return {}; });
    ON_CALL(*object, getMovableChildren()).WillByDefault([] () -> std::vector<std::unique_ptr<RestApi::ObjectWrapper>> { return {}; });
    ON_CALL(*object, getLight()).WillByDefault(light);
    ON_CALL(*object, getParticles()).WillByDefault([] { return nullptr; });
    return object;
}

static std::unique_ptr<MockLightWrapper> createMockLightWrapper(
    uint32_t color,
    float radius,
    float energy)
{
    auto light = std::make_unique<NiceMock<MockLightWrapper>>();
    ON_CALL(*light, getColor()).WillByDefault(Return(color));
    ON_CALL(*light, getRadius()).WillByDefault(Return(radius));
    ON_CALL(*light, getEnergy()).WillByDefault(Return(energy));
    return light;
}

TEST_F(RaceObjectHandlerTest, ObjectMany)
{
    std::mutex mutex;
    NiceMock<MockRaceObjectExchange> objects;
    auto createObjects = [] {
        std::vector<std::unique_ptr<RestApi::ObjectWrapper>> objects;
        objects.push_back(createMockObjectExchange(
            8,
            "Name1",
            "Type1",
            true,
            false,
            true,
            RestApi::Position{1.0f, 2.0f, 3.0f},
            RestApi::Position{2.0f, -2.0f, 5.0f},
            RestApi::Vector{0.5f, 1.0f, -0.5f},
            RestApi::Vector{5.0f, 8.0f, 6.0f},
            "LOD1",
            "Interaction1",
            [] { return nullptr; }
            ));
        objects.push_back(createMockObjectExchange(
            10,
            "Name2",
            "Type2",
            false,
            true,
            false,
            RestApi::Position{5.0f, 6.0f, 4.0f},
            RestApi::Position{8.0f, 2.0f, 5.0f},
            RestApi::Vector{6.0f, 5.0f, 7.0f},
            RestApi::Vector{1.0f, 10.0f, 4.0f},
            "LOD2",
            "Interaction2",
            [] { return createMockLightWrapper(1234567, 5.0f, 1.0f); }
        ));
        return objects;
    };
    ON_CALL(objects, getObjects()).WillByDefault(createObjects);
    auto handler = RestApi::Handler::createRaceObjectHandler(objects, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": 8,\n"
        "        \"name\": \"Name1\",\n"
        "        \"type\": \"Type1\",\n"
        "        \"enabled\": true,\n"
        "        \"drivable\": false,\n"
        "        \"animated\": true,\n"
        "        \"position\": [\n"
        "            1.0,\n"
        "            2.0,\n"
        "            3.0\n"
        "        ],\n"
        "        \"center\": [\n"
        "            2.0,\n"
        "            -2.0,\n"
        "            5.0\n"
        "        ],\n"
        "        \"rotation\": [\n"
        "            0.5,\n"
        "            1.0,\n"
        "            -0.5\n"
        "        ],\n"
        "        \"scale\": [\n"
        "            5.0,\n"
        "            8.0,\n"
        "            6.0\n"
        "        ],\n"
        "        \"lod-group\": \"LOD1\",\n"
        "        \"interaction\": \"Interaction1\",\n"
        "        \"children\": [],\n"
        "        \"movable-children\": []\n"
        "    },\n"
        "    {\n"
        "        \"id\": 10,\n"
        "        \"name\": \"Name2\",\n"
        "        \"type\": \"Type2\",\n"
        "        \"enabled\": false,\n"
        "        \"drivable\": true,\n"
        "        \"animated\": false,\n"
        "        \"position\": [\n"
        "            5.0,\n"
        "            6.0,\n"
        "            4.0\n"
        "        ],\n"
        "        \"center\": [\n"
        "            8.0,\n"
        "            2.0,\n"
        "            5.0\n"
        "        ],\n"
        "        \"rotation\": [\n"
        "            6.0,\n"
        "            5.0,\n"
        "            7.0\n"
        "        ],\n"
        "        \"scale\": [\n"
        "            1.0,\n"
        "            10.0,\n"
        "            4.0\n"
        "        ],\n"
        "        \"lod-group\": \"LOD2\",\n"
        "        \"interaction\": \"Interaction2\",\n"
        "        \"light\": {\n"
        "            \"color\": 1234567,\n"
        "            \"energy\": 1.0,\n"
        "            \"radius\": 5.0\n"
        "        },\n"
        "        \"children\": [],\n"
        "        \"movable-children\": []\n"
        "    }\n"
        "]");
    std::tie(status, result) = handler->handleGet("0");
    EXPECT_EQ(status, RestApi::STATUS_CODE::NOT_FOUND);
    std::tie(status, result) = handler->handleGet("5");
    EXPECT_EQ(status, RestApi::STATUS_CODE::NOT_FOUND);
    std::tie(status, result) = handler->handleGet("10");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": 10,\n"
        "    \"name\": \"Name2\",\n"
        "    \"type\": \"Type2\",\n"
        "    \"enabled\": false,\n"
        "    \"drivable\": true,\n"
        "    \"animated\": false,\n"
        "    \"position\": [\n"
        "        5.0,\n"
        "        6.0,\n"
        "        4.0\n"
        "    ],\n"
        "    \"center\": [\n"
        "        8.0,\n"
        "        2.0,\n"
        "        5.0\n"
        "    ],\n"
        "    \"rotation\": [\n"
        "        6.0,\n"
        "        5.0,\n"
        "        7.0\n"
        "    ],\n"
        "    \"scale\": [\n"
        "        1.0,\n"
        "        10.0,\n"
        "        4.0\n"
        "    ],\n"
        "    \"lod-group\": \"LOD2\",\n"
        "    \"interaction\": \"Interaction2\",\n"
        "    \"light\": {\n"
        "        \"color\": 1234567,\n"
        "        \"energy\": 1.0,\n"
        "        \"radius\": 5.0\n"
        "    },\n"
        "    \"children\": [],\n"
        "    \"movable-children\": []\n"
        "}");
    std::tie(status, result) = handler->handleGet("8");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": 8,\n"
        "    \"name\": \"Name1\",\n"
        "    \"type\": \"Type1\",\n"
        "    \"enabled\": true,\n"
        "    \"drivable\": false,\n"
        "    \"animated\": true,\n"
        "    \"position\": [\n"
        "        1.0,\n"
        "        2.0,\n"
        "        3.0\n"
        "    ],\n"
        "    \"center\": [\n"
        "        2.0,\n"
        "        -2.0,\n"
        "        5.0\n"
        "    ],\n"
        "    \"rotation\": [\n"
        "        0.5,\n"
        "        1.0,\n"
        "        -0.5\n"
        "    ],\n"
        "    \"scale\": [\n"
        "        5.0,\n"
        "        8.0,\n"
        "        6.0\n"
        "    ],\n"
        "    \"lod-group\": \"LOD1\",\n"
        "    \"interaction\": \"Interaction1\",\n"
        "    \"children\": [],\n"
        "    \"movable-children\": []\n"
        "}");
}

static std::unique_ptr<MockObjectWrapper> createMockObjectExchange(
    size_t id,
    std::unique_ptr<NiceMock<MockLightWrapper>>&& light = nullptr)
{
    auto object = std::make_unique<NiceMock<MockObjectWrapper>>();
    ON_CALL(*object, getId()).WillByDefault(Return(id));
    ON_CALL(*object, getLight()).WillByDefault(Return(ByMove(std::move(light))));
    return object;
}

TEST_F(RaceObjectHandlerTest, Post)
{
    NiceMock<MockRaceObjectExchange> objects;
    auto createObjects = [] (
        const std::function<void(MockObjectWrapper&, MockObjectWrapper&)>& setExpectations,
        std::unique_ptr<NiceMock<MockLightWrapper>>&& light0 = nullptr,
        std::unique_ptr<NiceMock<MockLightWrapper>>&& light1 = nullptr) {
        auto object0 = createMockObjectExchange(1);
        auto object1 = createMockObjectExchange(7);
        ON_CALL(*object0, getLight()).WillByDefault(Return(ByMove(std::move(light0))));
        ON_CALL(*object1, getLight()).WillByDefault(Return(ByMove(std::move(light1))));
        setExpectations(*object0, *object1);
        std::vector<std::unique_ptr<RestApi::ObjectWrapper>> objects;
        objects.push_back(std::move(object0));
        objects.push_back(std::move(object1));
        return objects;
    };
    Sequence sequence;
    EXPECT_CALL(objects, getObjects()).Times(1).InSequence(sequence).WillRepeatedly([&createObjects] {
        return createObjects([](MockObjectWrapper& object0, MockObjectWrapper&) {
            EXPECT_CALL(object0, setPosition(RestApi::Position{-5.0f, 1.25f, -0.5f})).Times(1);
            EXPECT_CALL(object0, setEnabled(false)).Times(1);
        });
    });
    EXPECT_CALL(objects, getObjects()).Times(1).InSequence(sequence).WillRepeatedly([&createObjects] {
        return createObjects([](MockObjectWrapper& object0, MockObjectWrapper&) {
            EXPECT_CALL(object0, setPosition(RestApi::Position{1.0f, 2.0f, -4.0f})).Times(1);
        });
    });
    EXPECT_CALL(objects, getObjects()).Times(1).InSequence(sequence).WillRepeatedly([&createObjects] {
        return createObjects([](MockObjectWrapper&, MockObjectWrapper& object1) {
            EXPECT_CALL(object1, setEnabled(true)).Times(1);
        });
    });
    EXPECT_CALL(objects, getObjects()).Times(1).InSequence(sequence).WillRepeatedly([&createObjects] {
        auto light = std::make_unique<NiceMock<MockLightWrapper>>();
        EXPECT_CALL(*light, setColor(654321)).Times(1);
        return createObjects([](MockObjectWrapper& object0, MockObjectWrapper&) {
            EXPECT_CALL(object0, setEnabled(false)).Times(1);
        }, std::move(light));
    });
    EXPECT_CALL(objects, getObjects()).Times(1).InSequence(sequence).WillRepeatedly([&createObjects] {
        auto light = std::make_unique<NiceMock<MockLightWrapper>>();
        EXPECT_CALL(*light, setEnergy(8.0f)).Times(1);
        EXPECT_CALL(*light, setRadius(4.0f)).Times(1);
        return createObjects([](MockObjectWrapper&, MockObjectWrapper&) {}, nullptr, std::move(light));
    });
    EXPECT_CALL(objects, getObjects()).Times(3).InSequence(sequence).WillRepeatedly([&createObjects] {
        return createObjects([](MockObjectWrapper&, MockObjectWrapper&) {});
    });
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceObjectHandler(objects, mutex);
    auto [status, result] = handler->handlePost(
        "1",
        "{"
        "   \"position\": [\n"
        "       -5.0,\n"
        "       1.25,\n"
        "       -0.5\n"
        "   ],\n"
        "   \"enabled\": false"
        "}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    std::tie(status, result) = handler->handlePost(
        "1",
        "{"
        "   \"position\": [\n"
        "       1.0,\n"
        "       2.0,\n"
        "       -4.0\n"
        "   ]\n"
        "}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    std::tie(status, result) = handler->handlePost(
        "7",
        "{"
        "   \"enabled\": true"
        "}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    std::tie(status, result) = handler->handlePost(
        "1",
        "{"
        "   \"enabled\": false,"
        "   \"color\": 654321\n"
        "}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    std::tie(status, result) = handler->handlePost(
        "7",
        "{"
        "   \"energy\": 8.0,\n"
        "   \"radius\": 4.0\n"
        "}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    std::tie(status, result) = handler->handlePost(
        "7",
        "{"
        "}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    std::tie(status, result) = handler->handlePost(
        "0",
        "{}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::NOT_FOUND);
    std::tie(status, result) = handler->handlePost(
        "3",
        "{}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::NOT_FOUND);
}

TEST_F(RaceObjectHandlerTest, Delete)
{
    NiceMock<MockRaceObjectExchange> objects;
    InSequence sequence;
    EXPECT_CALL(objects, remove(1)).Times(1);
    EXPECT_CALL(objects, remove(0)).Times(1);
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceObjectHandler(objects, mutex);
    EXPECT_EQ(handler->handleDelete("1").first, RestApi::STATUS_CODE::NO_CONTENT);
    EXPECT_EQ(handler->handleDelete("0").first, RestApi::STATUS_CODE::NO_CONTENT);
    EXPECT_THROW(handler->handleDelete("abc"), std::invalid_argument);
}
