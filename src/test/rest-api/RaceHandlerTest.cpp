#include <gtest/gtest.h>
#include "rest-api/Handler.hpp"
#include "test/rest-api/MockDataExchange.hpp"
#include "test/rest-api/MutexHandlerTestBase.hpp"

using testing::ByMove;
using testing::ByRef;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;
using testing::Sequence;

class RaceHandlerTest : public MutexHandlerTestBase
{
};

TEST_F(RaceHandlerTest, NotImplemented)
{
    NiceMock<MockRaceExchange> race;
    auto handler = RestApi::Handler::createRaceHandler(race, getMutex());
    auto [putStatusCode, putResult] = handler->handlePut("{}");
    EXPECT_EQ(putStatusCode, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(putResult, "null");
    auto [deleteStatusCode, deleteResult] = handler->handleDelete("{}");
    EXPECT_EQ(deleteStatusCode, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(deleteResult, "null");
}

TEST_F(RaceHandlerTest, GetNoRace)
{
    NiceMock<MockRaceExchange> race;
    ON_CALL(race, getStatus).WillByDefault(Return("NONE"));
    auto handler = RestApi::Handler::createRaceHandler(race, getMutex());
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"status\": \"NONE\",\n"
        "    \"race\": null\n"
        "}");
}

TEST_F(RaceHandlerTest, GetRace)
{
    NiceMock<MockRaceExchange> race;
    ON_CALL(race, getStatus).WillByDefault(Return("RACE"));
    ON_CALL(race, isActive).WillByDefault(Return(true));
    ON_CALL(race, getId).WillByDefault(Return(5));
    ON_CALL(race, getTrackName).WillByDefault(Return("track_name"));
    ON_CALL(race, getMajorRaceMode).WillByDefault(Return("major_mode"));
    ON_CALL(race, getMinorRaceMode).WillByDefault(Return("minor_mode"));
    ON_CALL(race, getDifficulty).WillByDefault(Return("difficulty"));
    ON_CALL(race, getClockType).WillByDefault(Return("clock_type"));
    ON_CALL(race, getTime).WillByDefault(Return(1.5f));
    auto handler = RestApi::Handler::createRaceHandler(race, [] () {return nullptr;});
    auto EXPECTED_RESPONSE =
        "{\n"
        "    \"status\": \"RACE\",\n"
        "    \"race\": {\n"
        "        \"id\": 5,\n"
        "        \"track\": \"track_name\",\n"
        "        \"major-race-mode\": \"major_mode\",\n"
        "        \"minor-race-mode\": \"minor_mode\",\n"
        "        \"difficulty\": \"difficulty\",\n"
        "        \"clock-type\": \"clock_type\",\n"
        "        \"time\": 1.5\n"
        "    }\n"
        "}";
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, EXPECTED_RESPONSE);
    handler = RestApi::Handler::createRaceHandler(race, getMutex());
    std::tie(status, result) = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, EXPECTED_RESPONSE);
}

TEST_F(RaceHandlerTest, GetRaceById)
{
    NiceMock<MockRaceExchange> race;
    ON_CALL(race, getStatus).WillByDefault(Return("MyStatus"));
    ON_CALL(race, isActive).WillByDefault(Return(false));
    InSequence sequence;
    auto handler = RestApi::Handler::createRaceHandler(race, getMutex());
    EXPECT_CALL(race, getId()).Times(1).WillOnce(Return(7));
    EXPECT_CALL(race, getRaceResults(5)).Times(1).WillOnce(Return("Result 1"));
    auto [status, result] = handler->handleGet("5");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, "Result 1");
    EXPECT_CALL(race, getId()).Times(1).WillOnce(Return(11));
    auto [statusCurrentRace, resultCurrent] = handler->handleGet("11");
    EXPECT_EQ(statusCurrentRace, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        resultCurrent,
        "{\n"
        "    \"status\": \"MyStatus\",\n"
        "    \"race\": null\n"
        "}");
}

TEST_F(RaceHandlerTest, GetKartsEmpty)
{
    NiceMock<MockKartModelExchange> karts;
    auto handler = RestApi::Handler::createKartModelHandler(karts, getMutex());
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, "[]");
    EXPECT_EQ(handler->handleGet("id").first, RestApi::STATUS_CODE::NOT_FOUND);
}

TEST_F(RaceHandlerTest, GetKartsOne)
{
    NiceMock<MockKartModelExchange> karts;
    ON_CALL(karts, getAvailableKarts).WillByDefault(
        Return(
            std::vector<RestApi::KartModelWrapper>{
                RestApi::KartModelWrapper{
                    "kart_id",
                    "kart_name",
                    0.5f,
                    1.0f,
                    2.0f,
                    0.25f
    }}));
    auto handler = RestApi::Handler::createKartModelHandler(karts, getMutex());
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": \"kart_id\",\n"
        "        \"name\": \"kart_name\",\n"
        "        \"mass\": 0.5,\n"
        "        \"engine-max-speed\": 1.0,\n"
        "        \"acceleration-efficiency\": 2.0,\n"
        "        \"nitro-consumption\": 0.25\n"
        "    }\n"
        "]");
    EXPECT_EQ(handler->handleGet("id").first, RestApi::STATUS_CODE::NOT_FOUND);
    auto [statusById, resultById] = handler->handleGet("kart_id");
    EXPECT_EQ(statusById, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        resultById,
        "{\n"
        "    \"id\": \"kart_id\",\n"
        "    \"name\": \"kart_name\",\n"
        "    \"mass\": 0.5,\n"
        "    \"engine-max-speed\": 1.0,\n"
        "    \"acceleration-efficiency\": 2.0,\n"
        "    \"nitro-consumption\": 0.25\n"
        "}");
}

TEST_F(RaceHandlerTest, GetKartsMany)
{
    NiceMock<MockKartModelExchange> karts;
    ON_CALL(karts, getAvailableKarts).WillByDefault(
        Return(
            std::vector<RestApi::KartModelWrapper>{
                RestApi::KartModelWrapper{
                    "id1",
                    "name1",
                    5.0f,
                    8.0f,
                    12.0f,
                    1.0f
                },
                RestApi::KartModelWrapper{
                    "id2",
                    "name2",
                    8.0f,
                    4.0f,
                    3.0f,
                    2.0f
                }}));
    auto handler = RestApi::Handler::createKartModelHandler(karts, getMutex());
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": \"id1\",\n"
        "        \"name\": \"name1\",\n"
        "        \"mass\": 5.0,\n"
        "        \"engine-max-speed\": 8.0,\n"
        "        \"acceleration-efficiency\": 12.0,\n"
        "        \"nitro-consumption\": 1.0\n"
        "    },\n"
        "    {\n"
        "        \"id\": \"id2\",\n"
        "        \"name\": \"name2\",\n"
        "        \"mass\": 8.0,\n"
        "        \"engine-max-speed\": 4.0,\n"
        "        \"acceleration-efficiency\": 3.0,\n"
        "        \"nitro-consumption\": 2.0\n"
        "    }\n"
        "]");
    EXPECT_EQ(handler->handleGet("id").first, RestApi::STATUS_CODE::NOT_FOUND);
    std::tie(status, result) = handler->handleGet("id1");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"id1\",\n"
        "    \"name\": \"name1\",\n"
        "    \"mass\": 5.0,\n"
        "    \"engine-max-speed\": 8.0,\n"
        "    \"acceleration-efficiency\": 12.0,\n"
        "    \"nitro-consumption\": 1.0\n"
        "}");
    std::tie(status, result) = handler->handleGet("id2");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"id2\",\n"
        "    \"name\": \"name2\",\n"
        "    \"mass\": 8.0,\n"
        "    \"engine-max-speed\": 4.0,\n"
        "    \"acceleration-efficiency\": 3.0,\n"
        "    \"nitro-consumption\": 2.0\n"
        "}");
}

TEST_F(RaceHandlerTest, PutKart)
{
    NiceMock<MockKartModelExchange> karts;
    ON_CALL(karts, getAvailableKarts).WillByDefault(
        Return(
            std::vector<RestApi::KartModelWrapper>{
                RestApi::KartModelWrapper{
                    "MyId_1",
                    "MyName_1",
                    7.0f,
                    0.5f,
                    5.0f,
                    11.0f
                },
                RestApi::KartModelWrapper{
                    "MyId_2",
                    "MyName_2",
                    8.0f,
                    6.0f,
                    12.0f,
                    1.5f
                }}));
    Sequence sequenceUnzipFunction;
    EXPECT_CALL(karts, getUnzipFunction()).Times(1).InSequence(sequenceUnzipFunction).WillOnce([] {
        return [](const std::string& data, const std::filesystem::path& directory) -> std::filesystem::path {
            EXPECT_EQ(data, "Test TEST Test");
            EXPECT_EQ(directory, "MyDirectory");
            return "/abc/def";
        };
    });
    EXPECT_CALL(karts, getUnzipFunction()).Times(1).InSequence(sequenceUnzipFunction).WillOnce([] {
        return [](const std::string& data, const std::filesystem::path& directory) -> std::filesystem::path {
            EXPECT_EQ(data, "DATA_2");
            EXPECT_EQ(directory, "DIRECTORY_2");
            return "/123";
        };
    });
    Sequence sequenceDirectory;
    EXPECT_CALL(karts, getDirectory()).Times(1).InSequence(sequenceDirectory).WillOnce(Return("MyDirectory"));
    EXPECT_CALL(karts, getDirectory()).Times(1).InSequence(sequenceDirectory).WillOnce(Return("DIRECTORY_2"));
    Sequence sequenceNewTrack;
    EXPECT_CALL(karts, loadNewKart(std::filesystem::path("/abc/def"))).Times(1).InSequence(sequenceNewTrack).WillOnce(Return("MyId_2"));
    EXPECT_CALL(karts, loadNewKart(std::filesystem::path("/123"))).Times(1).InSequence(sequenceNewTrack).WillOnce(Return("MyId_1"));
    auto handler = RestApi::Handler::createKartModelHandler(karts, getMutex());
    auto [status, result] = handler->handlePutZip("Test TEST Test");
    EXPECT_EQ(status, RestApi::STATUS_CODE::CREATED);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"MyId_2\",\n"
        "    \"name\": \"MyName_2\",\n"
        "    \"mass\": 8.0,\n"
        "    \"engine-max-speed\": 6.0,\n"
        "    \"acceleration-efficiency\": 12.0,\n"
        "    \"nitro-consumption\": 1.5\n"
        "}");
    std::tie(status, result) = handler->handlePutZip("DATA_2");
    EXPECT_EQ(status, RestApi::STATUS_CODE::CREATED);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"MyId_1\",\n"
        "    \"name\": \"MyName_1\",\n"
        "    \"mass\": 7.0,\n"
        "    \"engine-max-speed\": 0.5,\n"
        "    \"acceleration-efficiency\": 5.0,\n"
        "    \"nitro-consumption\": 11.0\n"
        "}");
}

TEST_F(RaceHandlerTest, DeleteKart)
{
    NiceMock<MockKartModelExchange> karts;
    InSequence sequence;
    EXPECT_CALL(karts, remove("addon_beastie")).Times(1);
    EXPECT_CALL(karts, remove("addon_abc")).Times(1);
    auto handler = RestApi::Handler::createKartModelHandler(karts, getMutex());
    EXPECT_EQ(handler->handleDelete("addon_beastie").first, RestApi::STATUS_CODE::NO_CONTENT);
    EXPECT_EQ(handler->handleDelete("addon_abc").first, RestApi::STATUS_CODE::NO_CONTENT);
}

TEST_F(RaceHandlerTest, GetTracksEmpty)
{
    NiceMock<MockTrackModelExchange> tracks;
    auto handler = RestApi::Handler::createTrackModelHandler(tracks, getMutex());
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, "[]");
    EXPECT_EQ(handler->handleGet("id").first, RestApi::STATUS_CODE::NOT_FOUND);
}

TEST_F(RaceHandlerTest, GetTracksOne)
{
    NiceMock<MockTrackModelExchange> tracks;
    ON_CALL(tracks, getAvailableTracks).WillByDefault(
        Return(
            std::vector<RestApi::TrackModelWrapper>{
                RestApi::TrackModelWrapper{
                    "track_id",
                    "track_name",
                    {"group1", "group2"},
                    true,
                    false,
                    true
                }}));
    auto handler = RestApi::Handler::createTrackModelHandler(tracks, getMutex());
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": \"track_id\",\n"
        "        \"name\": \"track_name\",\n"
        "        \"modes\": {\n"
        "            \"race\": true,\n"
        "            \"soccer\": false,\n"
        "            \"arena\": true\n"
        "        },\n"
        "        \"groups\": [\n"
        "            \"group1\",\n"
        "            \"group2\"\n"
        "        ]\n"
        "    }\n"
        "]");
    EXPECT_EQ(handler->handleGet("id").first, RestApi::STATUS_CODE::NOT_FOUND);
    std::tie(status, result) = handler->handleGet("track_id");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"track_id\",\n"
        "    \"name\": \"track_name\",\n"
        "    \"modes\": {\n"
        "        \"race\": true,\n"
        "        \"soccer\": false,\n"
        "        \"arena\": true\n"
        "    },\n"
        "    \"groups\": [\n"
        "        \"group1\",\n"
        "        \"group2\"\n"
        "    ]\n"
        "}");
}

TEST_F(RaceHandlerTest, GetTracksMany)
{
    NiceMock<MockTrackModelExchange> tracks;
    ON_CALL(tracks, getAvailableTracks).WillByDefault(
        Return(
            std::vector<RestApi::TrackModelWrapper>{
                RestApi::TrackModelWrapper{
                    "id1",
                    "name1",
                    {"group"},
                    false,
                    true,
                    false
                },
                RestApi::TrackModelWrapper{
                    "id2",
                    "name2",
                    {},
                    false,
                    false,
                    false
                }}));
    auto handler = RestApi::Handler::createTrackModelHandler(tracks, getMutex());
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": \"id1\",\n"
        "        \"name\": \"name1\",\n"
        "        \"modes\": {\n"
        "            \"race\": false,\n"
        "            \"soccer\": true,\n"
        "            \"arena\": false\n"
        "        },\n"
        "        \"groups\": [\n"
        "            \"group\"\n"
        "        ]\n"
        "    },\n"
        "    {\n"
        "        \"id\": \"id2\",\n"
        "        \"name\": \"name2\",\n"
        "        \"modes\": {\n"
        "            \"race\": false,\n"
        "            \"soccer\": false,\n"
        "            \"arena\": false\n"
        "        },\n"
        "        \"groups\": []\n"
        "    }\n"
        "]");
    EXPECT_EQ(handler->handleGet("id").first, RestApi::STATUS_CODE::NOT_FOUND);
    std::tie(status, result) = handler->handleGet("id1");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"id1\",\n"
        "    \"name\": \"name1\",\n"
        "    \"modes\": {\n"
        "        \"race\": false,\n"
        "        \"soccer\": true,\n"
        "        \"arena\": false\n"
        "    },\n"
        "    \"groups\": [\n"
        "        \"group\"\n"
        "    ]\n"
        "}");
    std::tie(status, result) = handler->handleGet("id2");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"id2\",\n"
        "    \"name\": \"name2\",\n"
        "    \"modes\": {\n"
        "        \"race\": false,\n"
        "        \"soccer\": false,\n"
        "        \"arena\": false\n"
        "    },\n"
        "    \"groups\": []\n"
        "}");
}

TEST_F(RaceHandlerTest, PutTrack)
{
    NiceMock<MockTrackModelExchange> tracks;
    ON_CALL(tracks, getAvailableTracks).WillByDefault(
        Return(
            std::vector<RestApi::TrackModelWrapper>{
                RestApi::TrackModelWrapper{
                    "My_1",
                    "My_Name_1",
                    {},
                    false,
                    false,
                    false
                },
                RestApi::TrackModelWrapper{
                    "My_2",
                    "My_Name_2",
                    {},
                    true,
                    true,
                    true
                }}));
    Sequence sequenceUnzipFunction;
    EXPECT_CALL(tracks, getUnzipFunction()).Times(1).InSequence(sequenceUnzipFunction).WillOnce([] {
        return [](const std::string& data, const std::filesystem::path& directory) -> std::filesystem::path {
            EXPECT_EQ(data, "Test TEST Test");
            EXPECT_EQ(directory, "MyDirectory");
            return "/abc/def";
        };
    });
    EXPECT_CALL(tracks, getUnzipFunction()).Times(1).InSequence(sequenceUnzipFunction).WillOnce([] {
        return [](const std::string& data, const std::filesystem::path& directory) -> std::filesystem::path {
            EXPECT_EQ(data, "DATA_2");
            EXPECT_EQ(directory, "DIRECTORY_2");
            return "/123";
        };
    });
    Sequence sequenceDirectory;
    EXPECT_CALL(tracks, getDirectory()).Times(1).InSequence(sequenceDirectory).WillOnce(Return("MyDirectory"));
    EXPECT_CALL(tracks, getDirectory()).Times(1).InSequence(sequenceDirectory).WillOnce(Return("DIRECTORY_2"));
    Sequence sequenceNewTrack;
    EXPECT_CALL(tracks, loadNewTrack(std::filesystem::path("/abc/def"))).Times(1).InSequence(sequenceNewTrack).WillOnce(Return("My_2"));
    EXPECT_CALL(tracks, loadNewTrack(std::filesystem::path("/123"))).Times(1).InSequence(sequenceNewTrack).WillOnce(Return("My_1"));
    auto handler = RestApi::Handler::createTrackModelHandler(tracks, getMutex());
    auto [status, result] = handler->handlePutZip("Test TEST Test");
    EXPECT_EQ(status, RestApi::STATUS_CODE::CREATED);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"My_2\",\n"
        "    \"name\": \"My_Name_2\",\n"
        "    \"modes\": {\n"
        "        \"race\": true,\n"
        "        \"soccer\": true,\n"
        "        \"arena\": true\n"
        "    },\n"
        "    \"groups\": []\n"
        "}");
    std::tie(status, result) = handler->handlePutZip("DATA_2");
    EXPECT_EQ(status, RestApi::STATUS_CODE::CREATED);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"My_1\",\n"
        "    \"name\": \"My_Name_1\",\n"
        "    \"modes\": {\n"
        "        \"race\": false,\n"
        "        \"soccer\": false,\n"
        "        \"arena\": false\n"
        "    },\n"
        "    \"groups\": []\n"
        "}");
}

TEST_F(RaceHandlerTest, DeleteTrack)
{
    NiceMock<MockTrackModelExchange> tracks;
    InSequence sequence;
    EXPECT_CALL(tracks, remove("addon_track")).Times(1);
    EXPECT_CALL(tracks, remove("addon_123")).Times(1);
    auto handler = RestApi::Handler::createTrackModelHandler(tracks, getMutex());
    EXPECT_EQ(handler->handleDelete("addon_track").first, RestApi::STATUS_CODE::NO_CONTENT);
    EXPECT_EQ(handler->handleDelete("addon_123").first, RestApi::STATUS_CODE::NO_CONTENT);
}

TEST_F(RaceHandlerTest, PostInvalidJson)
{
    NiceMock<MockRaceExchange> race;
    auto handler = RestApi::Handler::createRaceHandler(race, getMutex());
    EXPECT_THROW(handler->handlePost(R"({"status: "EXIT"})"), std::invalid_argument);
}

TEST_F(RaceHandlerTest, PostInvalidStatus)
{
    NiceMock<MockRaceExchange> race;
    auto handler = RestApi::Handler::createRaceHandler(race, getMutex());
    EXPECT_THROW(handler->handlePost(R"({"status": "ABC"})"), std::invalid_argument);
    EXPECT_THROW(handler->handlePost(R"({"status": 1})"), std::invalid_argument);
}

TEST_F(RaceHandlerTest, PostInvalidMissingStatus)
{
    NiceMock<MockRaceExchange> race;
    auto handler = RestApi::Handler::createRaceHandler(race, getMutex());
    EXPECT_THROW(handler->handlePost(R"({})"), std::invalid_argument);
}

TEST_F(RaceHandlerTest, PostExitInvalidMember)
{
    NiceMock<MockRaceExchange> race;
    auto handler = RestApi::Handler::createRaceHandler(race, getMutex());
    EXPECT_THROW(handler->handlePost(R"({"status": "EXIT", "difficulty": "EXPERT"})"), std::invalid_argument);
}

TEST_F(RaceHandlerTest, PostPauseInvalidMember)
{
    NiceMock<MockRaceExchange> race;
    auto handler = RestApi::Handler::createRaceHandler(race, getMutex());
    EXPECT_THROW(handler->handlePost(R"({"status": "PAUSE", "difficulty": "EXPERT"})"), std::invalid_argument);
}

TEST_F(RaceHandlerTest, PostResumeInvalidMember)
{
    NiceMock<MockRaceExchange> race;
    auto handler = RestApi::Handler::createRaceHandler(race, getMutex());
    EXPECT_THROW(handler->handlePost(R"({"status": "RESUME", "difficulty": "EXPERT"})"), std::invalid_argument);
}

TEST_F(RaceHandlerTest, PostNewMissingMember)
{
    NiceMock<MockRaceExchange> race;
    auto handler = RestApi::Handler::createRaceHandler(race, getMutex());
    EXPECT_THROW(handler->handlePost(R"({"status": "NEW", "difficulty": "EXPERT"})"), std::invalid_argument);
}

TEST_F(RaceHandlerTest, PostNewInvalidMember)
{
    NiceMock<MockRaceExchange> race;
    auto handler = RestApi::Handler::createRaceHandler(race, getMutex());
    EXPECT_THROW(handler->handlePost(R"({"status": "NEW", "race":{"laps": 4, "track": "track", "kart": "kart", "ai-karts": [], "difficulty": "EASY", "reverse": false, "test": true}})"), std::invalid_argument);
    EXPECT_THROW(handler->handlePost(R"({"status": "NEW", "race":{"laps": 4, "track": "track", "kart": "kart", "ai-karts": [], "difficulty": "EASY", "reverse": false}, "test": true})"), std::invalid_argument);
}

TEST_F(RaceHandlerTest, PostExit)
{
    NiceMock<MockRaceExchange> race;
    ON_CALL(race, getStatus).WillByDefault(Return("exit status"));
    ON_CALL(race, isActive).WillByDefault(Return(false));
    EXPECT_CALL(race, stop).Times(1);
    EXPECT_CALL(race, pause).Times(0);
    EXPECT_CALL(race, resume).Times(0);
    auto handler = RestApi::Handler::createRaceHandler(race, getMutex());
    auto [status, result] = handler->handlePost(R"({"status": "EXIT"})");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"status\": \"exit status\",\n"
        "    \"race\": null\n"
        "}");
}

TEST_F(RaceHandlerTest, PostPause)
{
    NiceMock<MockRaceExchange> race;
    ON_CALL(race, getStatus).WillByDefault(Return("PAUSE_STATUS"));
    ON_CALL(race, isActive).WillByDefault(Return(true));
    ON_CALL(race, getId).WillByDefault(Return(0));
    ON_CALL(race, getTrackName).WillByDefault(Return("TRACK"));
    ON_CALL(race, getMajorRaceMode).WillByDefault(Return("MAJOR"));
    ON_CALL(race, getMinorRaceMode).WillByDefault(Return("MINOR"));
    ON_CALL(race, getDifficulty).WillByDefault(Return("HARD"));
    ON_CALL(race, getClockType).WillByDefault(Return("FORWARD"));
    ON_CALL(race, getTime).WillByDefault(Return(-0.5f));
    EXPECT_CALL(race, stop).Times(0);
    EXPECT_CALL(race, pause).Times(1);
    EXPECT_CALL(race, resume).Times(0);
    auto handler = RestApi::Handler::createRaceHandler(race, getMutex());
    auto[status, result] = handler->handlePost(R"({"status": "PAUSE"})");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"status\": \"PAUSE_STATUS\",\n"
        "    \"race\": {\n"
        "        \"id\": 0,\n"
        "        \"track\": \"TRACK\",\n"
        "        \"major-race-mode\": \"MAJOR\",\n"
        "        \"minor-race-mode\": \"MINOR\",\n"
        "        \"difficulty\": \"HARD\",\n"
        "        \"clock-type\": \"FORWARD\",\n"
        "        \"time\": -0.5\n"
        "    }\n"
        "}");
}

TEST_F(RaceHandlerTest, PostResume)
{
    NiceMock<MockRaceExchange> race;
    ON_CALL(race, getStatus).WillByDefault(Return("STATUS1"));
    ON_CALL(race, isActive).WillByDefault(Return(true));
    ON_CALL(race, getId).WillByDefault(Return(2));
    ON_CALL(race, getTrackName).WillByDefault(Return("track1"));
    ON_CALL(race, getMajorRaceMode).WillByDefault(Return("major1"));
    ON_CALL(race, getMinorRaceMode).WillByDefault(Return("minor1"));
    ON_CALL(race, getDifficulty).WillByDefault(Return("easy1"));
    ON_CALL(race, getClockType).WillByDefault(Return("countdown1"));
    ON_CALL(race, getTime).WillByDefault(Return(1.0f));
    EXPECT_CALL(race, stop).Times(0);
    EXPECT_CALL(race, pause).Times(0);
    EXPECT_CALL(race, resume).Times(1);
    auto handler = RestApi::Handler::createRaceHandler(race, getMutex());
    auto[status, result] = handler->handlePost(R"({"status": "RESUME"})");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"status\": \"STATUS1\",\n"
        "    \"race\": {\n"
        "        \"id\": 2,\n"
        "        \"track\": \"track1\",\n"
        "        \"major-race-mode\": \"major1\",\n"
        "        \"minor-race-mode\": \"minor1\",\n"
        "        \"difficulty\": \"easy1\",\n"
        "        \"clock-type\": \"countdown1\",\n"
        "        \"time\": 1.0\n"
        "    }\n"
        "}");
}

MATCHER_P(RaceIs, other, "Match NewRaces")
{
    return arg.numberOfLaps == other.numberOfLaps
           && arg.track == other.track
           && arg.kart == other.kart
           && arg.aiKarts == other.aiKarts
           && arg.difficulty == other.difficulty
           && arg.reverse == other.reverse;
}

TEST_F(RaceHandlerTest, PostNew)
{
    NiceMock<MockRaceExchange> race;
    ON_CALL(race, getStatus).WillByDefault(Return("RACE"));
    ON_CALL(race, isActive).WillByDefault(Return(true));
    ON_CALL(race, getTrackName).WillByDefault(Return("TrackName"));
    ON_CALL(race, getMajorRaceMode).WillByDefault(Return("M1"));
    ON_CALL(race, getMinorRaceMode).WillByDefault(Return("M2"));
    ON_CALL(race, getDifficulty).WillByDefault(Return("D1"));
    ON_CALL(race, getClockType).WillByDefault(Return("C1"));
    ON_CALL(race, getTime).WillByDefault(Return(5.0f));
    const RestApi::NewRace newRace = {
        5,
        "MyTrack",
        "MyKart",
        {"AI1", "AI2"},
        "MyDifficulty",
        true
    };
    EXPECT_CALL(race, start(RaceIs(newRace))).Times(1);
    auto handler = RestApi::Handler::createRaceHandler(race, getMutex());
    auto [status, result] = handler->handlePost("{\n"
        "    \"status\": \"NEW\",\n"
        "    \"race\": {\n"
        "        \"laps\": 5,\n"
        "        \"track\": \"MyTrack\",\n"
        "        \"kart\": \"MyKart\",\n"
        "        \"ai-karts\": [\"AI1\", \"AI2\"],\n"
        "        \"difficulty\": \"MyDifficulty\",\n"
        "        \"reverse\": true\n"
        "    }\n"
        "}");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"status\": \"RACE\",\n"
        "    \"race\": {\n"
        "        \"id\": null,\n"
        "        \"track\": \"TrackName\",\n"
        "        \"major-race-mode\": \"M1\",\n"
        "        \"minor-race-mode\": \"M2\",\n"
        "        \"difficulty\": \"D1\",\n"
        "        \"clock-type\": \"C1\",\n"
        "        \"time\": 5.0\n"
        "    }\n"
        "}");
}
