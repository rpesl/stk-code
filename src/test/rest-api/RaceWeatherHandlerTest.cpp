#include <gtest/gtest.h>
#include "rest-api/Handler.hpp"
#include "test/rest-api/MockDataExchange.hpp"

using testing::ByMove;
using testing::ByRef;
using testing::Return;
using testing::ReturnRef;
using testing::NiceMock;

class RaceWeatherHandlerTest : public testing::Test
{
};

TEST_F(RaceWeatherHandlerTest, WeatherNotImplemented)
{
    NiceMock<MockRaceWeatherExchange> weatherExchange;
    auto handler = RestApi::Handler::createRaceWeatherHandler(weatherExchange);
    auto [putStatusCode, putResult] = handler->handlePut("{}");
    EXPECT_EQ(putStatusCode, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(putResult, "null");
    auto [deleteStatusCode, deleteResult] = handler->handleDelete("{}");
    EXPECT_EQ(deleteStatusCode, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(deleteResult, "null");
}

TEST_F(RaceWeatherHandlerTest, WeatherSunny)
{
    NiceMock<MockRaceWeatherExchange> weatherExchange;
    NiceMock<MockParticleWrapper> particles;
    ON_CALL(weatherExchange, getLightning()).WillByDefault(Return(false));
    ON_CALL(weatherExchange, getSound()).WillByDefault(Return(""));
    ON_CALL(weatherExchange, getSkyColor()).WillByDefault(Return(0x12345678));
    ON_CALL(weatherExchange, getParticles()).WillByDefault(Return(std::nullopt));
    auto handler = RestApi::Handler::createRaceWeatherHandler(weatherExchange);
    auto [statusCode, result] = handler->handleGet();
    EXPECT_EQ(statusCode, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"sky-color\": 305419896,\n"
        "    \"sound\": \"\",\n"
        "    \"particles\": null,\n"
        "    \"lightning\": false\n"
        "}");
}

TEST_F(RaceWeatherHandlerTest, WeatherRain)
{
    NiceMock<MockRaceWeatherExchange> weatherExchange;
    ON_CALL(weatherExchange, getLightning()).WillByDefault(Return(true));
    ON_CALL(weatherExchange, getSound()).WillByDefault(Return("rain"));
    ON_CALL(weatherExchange, getSkyColor()).WillByDefault(Return(0xFFFF0000));
    auto particles = createMockParticleWrapper(
        "AbC",
        {1.0f, 2.0f},
        {-1.0f, 1.0f},
        {100.0f, 1000.0f},
        3,
        "MyShape123",
        {1, 14},
        {1.0f, 2.0f, 3.0f},
        3.0f,
        5,
        {4.0f, 5.0f, 6.0f},
        4,
        {-0.5f, 0.5f},
        true,
        false,
        true
    );
    std::optional<std::reference_wrapper<const RestApi::ParticleWrapper>> optionalParticles = *particles;
    ON_CALL(weatherExchange, getParticles()).WillByDefault(Return(optionalParticles));
    auto handler = RestApi::Handler::createRaceWeatherHandler(weatherExchange);
    auto [statusCode, result] = handler->handleGet();
    EXPECT_EQ(statusCode, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"sky-color\": 4294901760,\n"
        "    \"sound\": \"rain\",\n"
        "    \"particles\": {\n"
        "        \"name\": \"AbC\",\n"
        "        \"size\": {\n"
        "            \"min\": 1.0,\n"
        "            \"max\": 2.0\n"
        "        },\n"
        "        \"rate\": {\n"
        "            \"min\": -1.0,\n"
        "            \"max\": 1.0\n"
        "        },\n"
        "        \"lifetime\": {\n"
        "            \"min\": 100.0,\n"
        "            \"max\": 1000.0\n"
        "        },\n"
        "        \"fadeout-time\": 3,\n"
        "        \"shape\": \"MyShape123\",\n"
        "        \"material\": null,\n"
        "        \"color\": {\n"
        "            \"min\": 1,\n"
        "            \"max\": 14\n"
        "        },\n"
        "        \"box-size\": [\n"
        "            1.0,\n"
        "            2.0,\n"
        "            3.0\n"
        "        ],\n"
        "        \"sphere-radius\": 3.0,\n"
        "        \"angle-spread\": 5,\n"
        "        \"velocity\": [\n"
        "            4.0,\n"
        "            5.0,\n"
        "            6.0\n"
        "        ],\n"
        "        \"emission-decay-rate\": 4,\n"
        "        \"scale-affector\": [\n"
        "            -0.5,\n"
        "            0.5\n"
        "        ],\n"
        "        \"flips\": true,\n"
        "        \"vertical-particles\": false,\n"
        "        \"randomize-initial-y\": true\n"
        "    },\n"
        "    \"lightning\": true\n"
        "}");
}
