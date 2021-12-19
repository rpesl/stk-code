#include <gtest/gtest.h>
#include "rest-api/Handler.hpp"
#include "test/rest-api/MockDataExchange.hpp"

using testing::ByMove;
using testing::ByRef;
using testing::Return;
using testing::ReturnRef;
using testing::NiceMock;

class ChecklineHandlerTest : public testing::Test
{
};

TEST_F(ChecklineHandlerTest, ChecklineNotImplemented)
{
    NiceMock<MockRaceChecklineExchange> checkline;
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceChecklineHandler(checkline, mutex);
    auto [postStatusCode, postResult] = handler->handlePost("{}");
    EXPECT_EQ(postStatusCode, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(postResult, "null");
    auto [putStatusCode, putResult] = handler->handlePut("{}");
    EXPECT_EQ(putStatusCode, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(putResult, "null");
    auto [deleteStatusCode, deleteResult] = handler->handleDelete("{}");
    EXPECT_EQ(deleteStatusCode, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(deleteResult, "null");
}

TEST_F(ChecklineHandlerTest, ChecklineEmpty)
{
    NiceMock<MockRaceChecklineExchange> checkline;
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceChecklineHandler(checkline, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, "[]");
    EXPECT_EQ(handler->handleGet("0").first, RestApi::STATUS_CODE::NOT_FOUND);
}

TEST_F(ChecklineHandlerTest, ChecklineSingle)
{
    NiceMock<MockRaceChecklineExchange> checkline;
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceChecklineHandler(checkline, mutex);
    std::vector<RestApi::ChecklineWrapper> checklines;
    checklines.push_back(RestApi::ChecklineWrapper{
        1,
        "KIND",
        true,
        std::pair<RestApi::Position, RestApi::Position>{{1.0f, 2.0f, 3.0f}, {12.0f, 0.5f, -10.0f}},
        std::nullopt,
        {2, 5},
        {10, 8},
        {true, false, false}});
    ON_CALL(checkline, getChecklines()).WillByDefault(Return(checklines));
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": 1,\n"
        "        \"kind\": \"KIND\",\n"
        "        \"active-at-reset\": true,\n"
        "        \"position\": [\n"
        "            [\n"
        "                1.0,\n"
        "                2.0,\n"
        "                3.0\n"
        "            ],\n"
        "            [\n"
        "                12.0,\n"
        "                0.5,\n"
        "                -10.0\n"
        "            ]\n"
        "        ],\n"
        "        \"other-ids\": [\n"
        "            10,\n"
        "            8\n"
        "        ],\n"
        "        \"same-group\": [\n"
        "            2,\n"
        "            5\n"
        "        ],\n"
        "        \"karts\": [\n"
        "            0\n"
        "        ]\n"
        "    }\n"
        "]");
    EXPECT_EQ(handler->handleGet("5").first, RestApi::STATUS_CODE::NOT_FOUND);
    std::tie(status, result) = handler->handleGet("1");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": 1,\n"
        "    \"kind\": \"KIND\",\n"
        "    \"active-at-reset\": true,\n"
        "    \"position\": [\n"
        "        [\n"
        "            1.0,\n"
        "            2.0,\n"
        "            3.0\n"
        "        ],\n"
        "        [\n"
        "            12.0,\n"
        "            0.5,\n"
        "            -10.0\n"
        "        ]\n"
        "    ],\n"
        "    \"other-ids\": [\n"
        "        10,\n"
        "        8\n"
        "    ],\n"
        "    \"same-group\": [\n"
        "        2,\n"
        "        5\n"
        "    ],\n"
        "    \"karts\": [\n"
        "        0\n"
        "    ]\n"
        "}");
}

TEST_F(ChecklineHandlerTest, ChecklineMany)
{
    NiceMock<MockRaceChecklineExchange> checkline;
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceChecklineHandler(checkline, mutex);
    std::vector<RestApi::ChecklineWrapper> checklines;
    checklines.push_back(RestApi::ChecklineWrapper{
        0,
        "MyKind1",
        false,
        std::nullopt,
        true,
        {1},
        {},
        {false, true}});
    checklines.push_back(RestApi::ChecklineWrapper{
        5,
        "MyKind2",
        true,
        std::pair<RestApi::Position, RestApi::Position>{{5.0f, -2.0f, 0.5f}, {0.0f, -13.0f, 21.0f}},
        false,
        {},
        {11},
        {false}});
    ON_CALL(checkline, getChecklines()).WillByDefault(Return(checklines));
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": 0,\n"
        "        \"kind\": \"MyKind1\",\n"
        "        \"active-at-reset\": false,\n"
        "        \"ignore-height\": true,\n"
        "        \"other-ids\": [],\n"
        "        \"same-group\": [\n"
        "            1\n"
        "        ],\n"
        "        \"karts\": [\n"
        "            1\n"
        "        ]\n"
        "    },\n"
        "    {\n"
        "        \"id\": 5,\n"
        "        \"kind\": \"MyKind2\",\n"
        "        \"active-at-reset\": true,\n"
        "        \"ignore-height\": false,\n"
        "        \"position\": [\n"
        "            [\n"
        "                5.0,\n"
        "                -2.0,\n"
        "                0.5\n"
        "            ],\n"
        "            [\n"
        "                0.0,\n"
        "                -13.0,\n"
        "                21.0\n"
        "            ]\n"
        "        ],\n"
        "        \"other-ids\": [\n"
        "            11\n"
        "        ],\n"
        "        \"same-group\": [],\n"
        "        \"karts\": []\n"
        "    }\n"
        "]");
    EXPECT_EQ(handler->handleGet("1").first, RestApi::STATUS_CODE::NOT_FOUND);
    std::tie(status, result) = handler->handleGet("0");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": 0,\n"
        "    \"kind\": \"MyKind1\",\n"
        "    \"active-at-reset\": false,\n"
        "    \"ignore-height\": true,\n"
        "    \"other-ids\": [],\n"
        "    \"same-group\": [\n"
        "        1\n"
        "    ],\n"
        "    \"karts\": [\n"
        "        1\n"
        "    ]\n"
        "}");
    std::tie(status, result) = handler->handleGet("5");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": 5,\n"
        "    \"kind\": \"MyKind2\",\n"
        "    \"active-at-reset\": true,\n"
        "    \"ignore-height\": false,\n"
        "    \"position\": [\n"
        "        [\n"
        "            5.0,\n"
        "            -2.0,\n"
        "            0.5\n"
        "        ],\n"
        "        [\n"
        "            0.0,\n"
        "            -13.0,\n"
        "            21.0\n"
        "        ]\n"
        "    ],\n"
        "    \"other-ids\": [\n"
        "        11\n"
        "    ],\n"
        "    \"same-group\": [],\n"
        "    \"karts\": []\n"
        "}");
}

class QuadHandlerTest : public testing::Test
{
};

TEST_F(QuadHandlerTest, QuadNotImplemented)
{
    NiceMock<MockRaceQuadExchange> quad;
    auto handler = RestApi::Handler::createRaceQuadHandler(quad);
    auto [postStatusCode, postResult] = handler->handlePost("{}");
    EXPECT_EQ(postStatusCode, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(postResult, "null");
    auto [putStatusCode, putResult] = handler->handlePut("{}");
    EXPECT_EQ(putStatusCode, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(putResult, "null");
    auto [deleteStatusCode, deleteResult] = handler->handleDelete("{}");
    EXPECT_EQ(deleteStatusCode, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(deleteResult, "null");
}

TEST_F(QuadHandlerTest, QuadEmpty)
{
    NiceMock<MockRaceQuadExchange> quad;
    auto handler = RestApi::Handler::createRaceQuadHandler(quad);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, "[]");
    EXPECT_EQ(handler->handleGet("0").first, RestApi::STATUS_CODE::NOT_FOUND);
}

TEST_F(QuadHandlerTest, QuadSingle)
{
    NiceMock<MockRaceQuadExchange> quad;
    auto handler = RestApi::Handler::createRaceQuadHandler(quad);
    std::vector<RestApi::QuadWrapper> quads;
    quads.push_back(RestApi::QuadWrapper{
        11,
        true,
        false,
        true,
        {
            RestApi::Position{12.0f, 13.0f, 14.0f},
            RestApi::Position{2.0f, -2.0f, 1.0f},
            RestApi::Position{5.0f, 1.0f, 11.0f},
            RestApi::Position{8.0f, -15.0f, 13.0f}
        },
        {1.0f, 2.0f},
        {1, 2, 24}
    });
    ON_CALL(quad, getQuads()).WillByDefault(Return(quads));
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": 11,\n"
        "        \"ignore\": true,\n"
        "        \"invisible\": false,\n"
        "        \"ai-ignore\": true,\n"
        "        \"position\": [\n"
        "            [\n"
        "                12.0,\n"
        "                13.0,\n"
        "                14.0\n"
        "            ],\n"
        "            [\n"
        "                2.0,\n"
        "                -2.0,\n"
        "                1.0\n"
        "            ],\n"
        "            [\n"
        "                5.0,\n"
        "                1.0,\n"
        "                11.0\n"
        "            ],\n"
        "            [\n"
        "                8.0,\n"
        "                -15.0,\n"
        "                13.0\n"
        "            ]\n"
        "        ],\n"
        "        \"height-testing\": {\n"
        "            \"min\": 1.0,\n"
        "            \"max\": 2.0\n"
        "        },\n"
        "        \"successors\": [\n"
        "            1,\n"
        "            2,\n"
        "            24\n"
        "        ]\n"
        "    }\n"
        "]");
    EXPECT_EQ(handler->handleGet("5").first, RestApi::STATUS_CODE::NOT_FOUND);
    std::tie(status, result) = handler->handleGet("11");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": 11,\n"
        "    \"ignore\": true,\n"
        "    \"invisible\": false,\n"
        "    \"ai-ignore\": true,\n"
        "    \"position\": [\n"
        "        [\n"
        "            12.0,\n"
        "            13.0,\n"
        "            14.0\n"
        "        ],\n"
        "        [\n"
        "            2.0,\n"
        "            -2.0,\n"
        "            1.0\n"
        "        ],\n"
        "        [\n"
        "            5.0,\n"
        "            1.0,\n"
        "            11.0\n"
        "        ],\n"
        "        [\n"
        "            8.0,\n"
        "            -15.0,\n"
        "            13.0\n"
        "        ]\n"
        "    ],\n"
        "    \"height-testing\": {\n"
        "        \"min\": 1.0,\n"
        "        \"max\": 2.0\n"
        "    },\n"
        "    \"successors\": [\n"
        "        1,\n"
        "        2,\n"
        "        24\n"
        "    ]\n"
        "}");
}

TEST_F(QuadHandlerTest, QuadMany)
{
    NiceMock<MockRaceQuadExchange> quad;
    auto handler = RestApi::Handler::createRaceQuadHandler(quad);
    std::vector<RestApi::QuadWrapper> quads;
    quads.push_back(RestApi::QuadWrapper{
        0,
        false,
        true,
        false,
        {
            RestApi::Position{1.0f, 3.0f, 4.0f},
            RestApi::Position{5.0f, 6.0f, 7.0f},
            RestApi::Position{11.0f, 21.0f, 8.0f},
            RestApi::Position{14.0f, 2.0f, 0.5f}
        },
        {-1.0f, 1.0f},
        {441}
    });
    quads.push_back(RestApi::QuadWrapper{
        1,
        false,
        false,
        false,
        {
            RestApi::Position{4.0f, 12.0f, 11.0f},
            RestApi::Position{-1.0f, 55.0f, 5.0f},
            RestApi::Position{13.0f, -0.5f, 10.0f},
            RestApi::Position{22.0f, 5.0f, 1.0f}
        },
        {0.5f, 22.0f},
        {}
    });
    ON_CALL(quad, getQuads()).WillByDefault(Return(quads));
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": 0,\n"
        "        \"ignore\": false,\n"
        "        \"invisible\": true,\n"
        "        \"ai-ignore\": false,\n"
        "        \"position\": [\n"
        "            [\n"
        "                1.0,\n"
        "                3.0,\n"
        "                4.0\n"
        "            ],\n"
        "            [\n"
        "                5.0,\n"
        "                6.0,\n"
        "                7.0\n"
        "            ],\n"
        "            [\n"
        "                11.0,\n"
        "                21.0,\n"
        "                8.0\n"
        "            ],\n"
        "            [\n"
        "                14.0,\n"
        "                2.0,\n"
        "                0.5\n"
        "            ]\n"
        "        ],\n"
        "        \"height-testing\": {\n"
        "            \"min\": -1.0,\n"
        "            \"max\": 1.0\n"
        "        },\n"
        "        \"successors\": [\n"
        "            441\n"
        "        ]\n"
        "    },\n"
        "    {\n"
        "        \"id\": 1,\n"
        "        \"ignore\": false,\n"
        "        \"invisible\": false,\n"
        "        \"ai-ignore\": false,\n"
        "        \"position\": [\n"
        "            [\n"
        "                4.0,\n"
        "                12.0,\n"
        "                11.0\n"
        "            ],\n"
        "            [\n"
        "                -1.0,\n"
        "                55.0,\n"
        "                5.0\n"
        "            ],\n"
        "            [\n"
        "                13.0,\n"
        "                -0.5,\n"
        "                10.0\n"
        "            ],\n"
        "            [\n"
        "                22.0,\n"
        "                5.0,\n"
        "                1.0\n"
        "            ]\n"
        "        ],\n"
        "        \"height-testing\": {\n"
        "            \"min\": 0.5,\n"
        "            \"max\": 22.0\n"
        "        },\n"
        "        \"successors\": []\n"
        "    }\n"
        "]");
    EXPECT_EQ(handler->handleGet("2").first, RestApi::STATUS_CODE::NOT_FOUND);
    std::tie(status, result) = handler->handleGet("0");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": 0,\n"
        "    \"ignore\": false,\n"
        "    \"invisible\": true,\n"
        "    \"ai-ignore\": false,\n"
        "    \"position\": [\n"
        "        [\n"
        "            1.0,\n"
        "            3.0,\n"
        "            4.0\n"
        "        ],\n"
        "        [\n"
        "            5.0,\n"
        "            6.0,\n"
        "            7.0\n"
        "        ],\n"
        "        [\n"
        "            11.0,\n"
        "            21.0,\n"
        "            8.0\n"
        "        ],\n"
        "        [\n"
        "            14.0,\n"
        "            2.0,\n"
        "            0.5\n"
        "        ]\n"
        "    ],\n"
        "    \"height-testing\": {\n"
        "        \"min\": -1.0,\n"
        "        \"max\": 1.0\n"
        "    },\n"
        "    \"successors\": [\n"
        "        441\n"
        "    ]\n"
        "}");
    std::tie(status, result) = handler->handleGet("1");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": 1,\n"
        "    \"ignore\": false,\n"
        "    \"invisible\": false,\n"
        "    \"ai-ignore\": false,\n"
        "    \"position\": [\n"
        "        [\n"
        "            4.0,\n"
        "            12.0,\n"
        "            11.0\n"
        "        ],\n"
        "        [\n"
        "            -1.0,\n"
        "            55.0,\n"
        "            5.0\n"
        "        ],\n"
        "        [\n"
        "            13.0,\n"
        "            -0.5,\n"
        "            10.0\n"
        "        ],\n"
        "        [\n"
        "            22.0,\n"
        "            5.0,\n"
        "            1.0\n"
        "        ]\n"
        "    ],\n"
        "    \"height-testing\": {\n"
        "        \"min\": 0.5,\n"
        "        \"max\": 22.0\n"
        "    },\n"
        "    \"successors\": []\n"
        "}");
}
