#include <gtest/gtest.h>
#include "rest-api/Handler.hpp"
#include "test/rest-api/MockDataExchange.hpp"
#include "test/rest-api/MutexHandlerTestBase.hpp"

using testing::AtLeast;
using testing::ByMove;
using testing::ByRef;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;
using testing::Sequence;

class SfxHandlerTest : public MutexHandlerTestBase
{
};

TEST_F(SfxHandlerTest, GetEmpty)
{
    NiceMock<MockSfxExchange> library;
    auto handler = RestApi::Handler::createSfxHandler(library, getMutex());
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, "[]");
}

TEST_F(SfxHandlerTest, GetOne)
{
    NiceMock<MockSfxExchange> library;
    ON_CALL(library, getSounds()).WillByDefault(
        Return(
            std::vector<RestApi::SfxWrapper>{
                RestApi::SfxWrapper{
                    "Id",
                    "File",
                    true,
                    false,
                    0.5f,
                    1.0f,
                    2.0f,
                    4.0f
                }
            }));
    auto handler = RestApi::Handler::createSfxHandler(library, getMutex());
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": \"Id\",\n"
        "        \"file\": \"File\",\n"
        "        \"loaded\": true,\n"
        "        \"positional\": false,\n"
        "        \"roll-off\": 0.5,\n"
        "        \"volume\": 1.0,\n"
        "        \"max-distance\": 2.0,\n"
        "        \"duration\": 4.0\n"
        "    }\n"
        "]");
}

TEST_F(SfxHandlerTest, GetMany)
{
    NiceMock<MockSfxExchange> library;
    ON_CALL(library, getSounds()).WillByDefault(
        Return(
            std::vector<RestApi::SfxWrapper>{
                RestApi::SfxWrapper{
                    "MyId_1",
                    "MyFile_1",
                    true,
                    true,
                    1.0f,
                    2.0f,
                    0.0f,
                    std::nullopt
                },
                RestApi::SfxWrapper{
                    "MyId_2",
                    "MyFile_2",
                    false,
                    false,
                    11.0f,
                    2.5f,
                    8.0f,
                    0.5f
                },
            }));
    auto handler = RestApi::Handler::createSfxHandler(library, getMutex());
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": \"MyId_1\",\n"
        "        \"file\": \"MyFile_1\",\n"
        "        \"loaded\": true,\n"
        "        \"positional\": true,\n"
        "        \"roll-off\": 1.0,\n"
        "        \"volume\": 2.0,\n"
        "        \"max-distance\": 0.0,\n"
        "        \"duration\": null\n"
        "    },\n"
        "    {\n"
        "        \"id\": \"MyId_2\",\n"
        "        \"file\": \"MyFile_2\",\n"
        "        \"loaded\": false,\n"
        "        \"positional\": false,\n"
        "        \"roll-off\": 11.0,\n"
        "        \"volume\": 2.5,\n"
        "        \"max-distance\": 8.0,\n"
        "        \"duration\": 0.5\n"
        "    }\n"
        "]");
}

TEST_F(SfxHandlerTest, GetById)
{
    NiceMock<MockSfxExchange> library;
    ON_CALL(library, getSounds()).WillByDefault(
        Return(
            std::vector<RestApi::SfxWrapper>{
                RestApi::SfxWrapper{
                    "THE_ONE",
                    "THE_FILE_ONE",
                    false,
                    true,
                    8.0f,
                    9.0f,
                    4.0f,
                    std::nullopt
                },
                RestApi::SfxWrapper{
                    "THE_TWO",
                    "THE_FILE_TWO",
                    true,
                    false,
                    25.0f,
                    7.5f,
                    11.0f,
                    std::nullopt
                },
            }));
    auto handler = RestApi::Handler::createSfxHandler(library, getMutex());
    auto [status, result] = handler->handleGet("THE_ONE");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"THE_ONE\",\n"
        "    \"file\": \"THE_FILE_ONE\",\n"
        "    \"loaded\": false,\n"
        "    \"positional\": true,\n"
        "    \"roll-off\": 8.0,\n"
        "    \"volume\": 9.0,\n"
        "    \"max-distance\": 4.0,\n"
        "    \"duration\": null\n"
        "}");
    std::tie(status, result) = handler->handleGet("THE_TWO");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"THE_TWO\",\n"
        "    \"file\": \"THE_FILE_TWO\",\n"
        "    \"loaded\": true,\n"
        "    \"positional\": false,\n"
        "    \"roll-off\": 25.0,\n"
        "    \"volume\": 7.5,\n"
        "    \"max-distance\": 11.0,\n"
        "    \"duration\": null\n"
        "}");
}

TEST_F(SfxHandlerTest, Put)
{
    NiceMock<MockSfxExchange> library;
    ON_CALL(library, getSounds()).WillByDefault(
        Return(
            std::vector<RestApi::SfxWrapper>{
                RestApi::SfxWrapper{
                    "ID_1",
                    "FILE_1",
                    false,
                    false,
                    0.0f,
                    5.0f,
                    2.5f,
                    1.0f
                },
                RestApi::SfxWrapper{
                    "ID_2",
                    "FILE_2",
                    true,
                    true,
                    10.0f,
                    8.0f,
                    1.5f,
                    2.0f
                },
            }));
    Sequence sequenceUnzipFunction;
    EXPECT_CALL(library, getUnzipFunction()).Times(1).InSequence(sequenceUnzipFunction).WillOnce([] {
        return [](const std::string& data, const std::filesystem::path& directory) -> std::filesystem::path {
            EXPECT_EQ(data, "zip_1");
            EXPECT_EQ(directory, "D_1");
            return "/the/first/directory/1";
        };
    });
    EXPECT_CALL(library, getUnzipFunction()).Times(1).InSequence(sequenceUnzipFunction).WillOnce([] {
        return [](const std::string& data, const std::filesystem::path& directory) -> std::filesystem::path {
            EXPECT_EQ(data, "2nd ZIP file");
            EXPECT_EQ(directory, "SECOND_DIRECTORY");
            return "/second";
        };
    });
    Sequence sequenceDirectory;
    EXPECT_CALL(library, getDirectory()).Times(1).InSequence(sequenceDirectory).WillOnce(Return("D_1"));
    EXPECT_CALL(library, getDirectory()).Times(1).InSequence(sequenceDirectory).WillOnce(Return("SECOND_DIRECTORY"));
    Sequence sequenceNewTrack;
    EXPECT_CALL(library, loadNewSfx(std::filesystem::path("/the/first/directory/1"))).Times(1).InSequence(sequenceNewTrack).WillOnce(Return("ID_1"));
    EXPECT_CALL(library, loadNewSfx(std::filesystem::path("/second"))).Times(1).InSequence(sequenceNewTrack).WillOnce(Return("ID_2"));
    auto handler = RestApi::Handler::createSfxHandler(library, getMutex());
    auto [status, result] = handler->handlePutZip("zip_1");
    EXPECT_EQ(status, RestApi::STATUS_CODE::CREATED);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"ID_1\",\n"
        "    \"file\": \"FILE_1\",\n"
        "    \"loaded\": false,\n"
        "    \"positional\": false,\n"
        "    \"roll-off\": 0.0,\n"
        "    \"volume\": 5.0,\n"
        "    \"max-distance\": 2.5,\n"
        "    \"duration\": 1.0\n"
        "}");
    std::tie(status, result) = handler->handlePutZip("2nd ZIP file");
    EXPECT_EQ(status, RestApi::STATUS_CODE::CREATED);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"ID_2\",\n"
        "    \"file\": \"FILE_2\",\n"
        "    \"loaded\": true,\n"
        "    \"positional\": true,\n"
        "    \"roll-off\": 10.0,\n"
        "    \"volume\": 8.0,\n"
        "    \"max-distance\": 1.5,\n"
        "    \"duration\": 2.0\n"
        "}");
}

TEST_F(SfxHandlerTest, Delete)
{
    NiceMock<MockSfxExchange> library;
    InSequence sequence;
    EXPECT_CALL(library, remove("MySfx1")).Times(1);
    EXPECT_CALL(library, remove("sound_abc")).Times(1);
    auto handler = RestApi::Handler::createSfxHandler(library, getMutex());
    EXPECT_EQ(handler->handleDelete("MySfx1").first, RestApi::STATUS_CODE::NO_CONTENT);
    EXPECT_EQ(handler->handleDelete("sound_abc").first, RestApi::STATUS_CODE::NO_CONTENT);
}

class RaceSfxHandlerTest : public testing::Test
{
};

TEST_F(RaceSfxHandlerTest, GetEmpty)
{
    std::mutex mutex;
    NiceMock<MockRaceSfxExchange> sfx;
    auto handler = RestApi::Handler::createRaceSfxHandler(sfx, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
    "{\n"
    "    \"sfx-allowed\": false,\n"
    "    \"master-volume\": 0.0,\n"
    "    \"listener\": {\n"
    "        \"position\": [\n"
    "            0.0,\n"
    "            0.0,\n"
    "            0.0\n"
    "        ],\n"
    "        \"direction\": [\n"
    "            0.0,\n"
    "            0.0,\n"
    "            0.0\n"
    "        ],\n"
    "        \"up\": [\n"
    "            0.0,\n"
    "            0.0,\n"
    "            0.0\n"
    "        ]\n"
    "    },\n"
    "    \"sounds\": []\n"
    "}");
}

static std::unique_ptr<MockSfxSoundWrapper> createMockSfxSoundWrapper(
    const std::string& sound,
    const std::string& status,
    bool loop,
    float volume,
    float pitch,
    float playTime,
    const std::optional<RestApi::Position>& position)
{
    auto sfx = std::make_unique<NiceMock<MockSfxSoundWrapper>>();
    ON_CALL(*sfx, getSound()).WillByDefault(Return(sound));
    ON_CALL(*sfx, getStatus()).WillByDefault(Return(status));
    ON_CALL(*sfx, inLoop()).WillByDefault(Return(loop));
    ON_CALL(*sfx, getVolume()).WillByDefault(Return(volume));
    ON_CALL(*sfx, getPitch()).WillByDefault(Return(pitch));
    ON_CALL(*sfx, getPlayTime()).WillByDefault(Return(playTime));
    ON_CALL(*sfx, getPosition()).WillByDefault(Return(position));
    return sfx;
}

TEST_F(RaceSfxHandlerTest, GetOne)
{
    std::mutex mutex;
    NiceMock<MockRaceSfxExchange> sfx;
    ON_CALL(sfx, getListenerPosition()).WillByDefault(Return(RestApi::Position{1.0f, 2.0f, 3.0f}));
    ON_CALL(sfx, getListenerDirection()).WillByDefault(Return(RestApi::Position{5.0f, -2.0f, 8.0f}));
    ON_CALL(sfx, getListenerUpDirection()).WillByDefault(Return(RestApi::Position{-1.0f, 14.0f, -5.0f}));
    ON_CALL(sfx, getMasterVolume()).WillByDefault(Return(0.25f));
    ON_CALL(sfx, isSfxAllowed()).WillByDefault(Return(true));
    ON_CALL(sfx, getAllSounds()).WillByDefault([] {
        std::vector<std::unique_ptr<RestApi::SfxSoundWrapper>> result;
        result.emplace_back(createMockSfxSoundWrapper("sound", "status", true, 0.5f, 0.75f, 12.5f, RestApi::Position{-15.0f, 8.0f, 12.0f}));
        return result;
    });
    auto handler = RestApi::Handler::createRaceSfxHandler(sfx, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"sfx-allowed\": true,\n"
        "    \"master-volume\": 0.25,\n"
        "    \"listener\": {\n"
        "        \"position\": [\n"
        "            1.0,\n"
        "            2.0,\n"
        "            3.0\n"
        "        ],\n"
        "        \"direction\": [\n"
        "            5.0,\n"
        "            -2.0,\n"
        "            8.0\n"
        "        ],\n"
        "        \"up\": [\n"
        "            -1.0,\n"
        "            14.0,\n"
        "            -5.0\n"
        "        ]\n"
        "    },\n"
        "    \"sounds\": [\n"
        "        {\n"
        "            \"sound\": \"sound\",\n"
        "            \"status\": \"status\",\n"
        "            \"loop\": true,\n"
        "            \"volume\": 0.5,\n"
        "            \"pitch\": 0.75,\n"
        "            \"play-time\": 12.5,\n"
        "            \"position\": [\n"
        "                -15.0,\n"
        "                8.0,\n"
        "                12.0\n"
        "            ]\n"
        "        }\n"
        "    ]\n"
        "}");
}

TEST_F(RaceSfxHandlerTest, GetMany)
{
    std::mutex mutex;
    NiceMock<MockRaceSfxExchange> sfx;
    ON_CALL(sfx, getListenerPosition()).WillByDefault(Return(RestApi::Position{5.0f, 8.0f, 1.0f}));
    ON_CALL(sfx, getListenerDirection()).WillByDefault(Return(RestApi::Position{11.0f, 12.0f, 13.0f}));
    ON_CALL(sfx, getListenerUpDirection()).WillByDefault(Return(RestApi::Position{24.0f, -2.0f, -8.0f}));
    ON_CALL(sfx, getMasterVolume()).WillByDefault(Return(1.0f));
    ON_CALL(sfx, isSfxAllowed()).WillByDefault(Return(false));
    ON_CALL(sfx, getAllSounds()).WillByDefault([] {
        std::vector<std::unique_ptr<RestApi::SfxSoundWrapper>> result;
        result.emplace_back(createMockSfxSoundWrapper("SOUND_1", "STATUS_1", false, 0.125f, 0.25f, 1.5f, RestApi::Position{5.0f, 0.0f, 15.0f}));
        result.emplace_back(createMockSfxSoundWrapper("SOUND_2", "STATUS_2", true, 0.75f, 0.875f, 21.5f, RestApi::Position{15.0f, 11.0f, 24.0f}));
        return result;
    });
    auto handler = RestApi::Handler::createRaceSfxHandler(sfx, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"sfx-allowed\": false,\n"
        "    \"master-volume\": 1.0,\n"
        "    \"listener\": {\n"
        "        \"position\": [\n"
        "            5.0,\n"
        "            8.0,\n"
        "            1.0\n"
        "        ],\n"
        "        \"direction\": [\n"
        "            11.0,\n"
        "            12.0,\n"
        "            13.0\n"
        "        ],\n"
        "        \"up\": [\n"
        "            24.0,\n"
        "            -2.0,\n"
        "            -8.0\n"
        "        ]\n"
        "    },\n"
        "    \"sounds\": [\n"
        "        {\n"
        "            \"sound\": \"SOUND_1\",\n"
        "            \"status\": \"STATUS_1\",\n"
        "            \"loop\": false,\n"
        "            \"volume\": 0.125,\n"
        "            \"pitch\": 0.25,\n"
        "            \"play-time\": 1.5,\n"
        "            \"position\": [\n"
        "                5.0,\n"
        "                0.0,\n"
        "                15.0\n"
        "            ]\n"
        "        },\n"
        "        {\n"
        "            \"sound\": \"SOUND_2\",\n"
        "            \"status\": \"STATUS_2\",\n"
        "            \"loop\": true,\n"
        "            \"volume\": 0.75,\n"
        "            \"pitch\": 0.875,\n"
        "            \"play-time\": 21.5,\n"
        "            \"position\": [\n"
        "                15.0,\n"
        "                11.0,\n"
        "                24.0\n"
        "            ]\n"
        "        }\n"
        "    ]\n"
        "}");
}

TEST_F(RaceSfxHandlerTest, GetById)
{
    std::mutex mutex;
    NiceMock<MockRaceSfxExchange> sfx;
    ON_CALL(sfx, getAllSounds()).WillByDefault([] {
        std::vector<std::unique_ptr<RestApi::SfxSoundWrapper>> result;
        result.emplace_back(createMockSfxSoundWrapper("id_1", "1", true, 1.0f, 0.75f, 0.5f, RestApi::Position{4.0f, 7.0f, 5.0f}));
        result.emplace_back(createMockSfxSoundWrapper("id_2", "2", false, 0.0f, 1.0f, 4.0f, RestApi::Position{-1.0f, -1.5f, 4.0f}));
        return result;
    });
    auto handler = RestApi::Handler::createRaceSfxHandler(sfx, mutex);
    auto [status, result] = handler->handleGet("1");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"sound\": \"id_2\",\n"
        "    \"status\": \"2\",\n"
        "    \"loop\": false,\n"
        "    \"volume\": 0.0,\n"
        "    \"pitch\": 1.0,\n"
        "    \"play-time\": 4.0,\n"
        "    \"position\": [\n"
        "        -1.0,\n"
        "        -1.5,\n"
        "        4.0\n"
        "    ]\n"
        "}");
    std::tie(status, result) = handler->handleGet("0");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"sound\": \"id_1\",\n"
        "    \"status\": \"1\",\n"
        "    \"loop\": true,\n"
        "    \"volume\": 1.0,\n"
        "    \"pitch\": 0.75,\n"
        "    \"play-time\": 0.5,\n"
        "    \"position\": [\n"
        "        4.0,\n"
        "        7.0,\n"
        "        5.0\n"
        "    ]\n"
        "}");
    EXPECT_EQ(handler->handleGet("3").first, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(handler->handleGet("abc").first, RestApi::STATUS_CODE::NOT_FOUND);
}

TEST_F(RaceSfxHandlerTest, Post)
{
    std::mutex mutex;
    NiceMock<MockRaceSfxExchange> sfx;
    ON_CALL(sfx, getListenerPosition()).WillByDefault(Return(RestApi::Position{-2.0f, 1.0f, 2.0f}));
    ON_CALL(sfx, getListenerDirection()).WillByDefault(Return(RestApi::Position{4.0f, 2.0f, 3.0f}));
    ON_CALL(sfx, getListenerUpDirection()).WillByDefault(Return(RestApi::Position{0.0f, 1.0f, -1.0f}));
    ON_CALL(sfx, getMasterVolume()).WillByDefault(Return(0.875f));
    ON_CALL(sfx, isSfxAllowed()).WillByDefault(Return(true));
    constexpr const char* EXPECTED_RESULT =
        "{\n"
        "    \"sfx-allowed\": true,\n"
        "    \"master-volume\": 0.875,\n"
        "    \"listener\": {\n"
        "        \"position\": [\n"
        "            -2.0,\n"
        "            1.0,\n"
        "            2.0\n"
        "        ],\n"
        "        \"direction\": [\n"
        "            4.0,\n"
        "            2.0,\n"
        "            3.0\n"
        "        ],\n"
        "        \"up\": [\n"
        "            0.0,\n"
        "            1.0,\n"
        "            -1.0\n"
        "        ]\n"
        "    },\n"
        "    \"sounds\": []\n"
        "}";
    Sequence volumeSequence;
    Sequence sfxSequence;
    EXPECT_CALL(sfx, setMasterVolume(0.5f)).Times(1).InSequence(volumeSequence);
    EXPECT_CALL(sfx, setSfxAllowed(true)).Times(1).InSequence(sfxSequence);
    auto handler = RestApi::Handler::createRaceSfxHandler(sfx, mutex);
    auto [status, result] = handler->handlePost(R"({ "sfx-allowed": true, "master-volume": 0.5 })");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, EXPECTED_RESULT);
    EXPECT_CALL(sfx, setSfxAllowed(false)).Times(1).InSequence(sfxSequence);
    std::tie(status, result) = handler->handlePost(R"({ "sfx-allowed": false })");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, EXPECTED_RESULT);
    EXPECT_CALL(sfx, setMasterVolume(1.0f)).Times(1).InSequence(volumeSequence);
    std::tie(status, result) = handler->handlePost(R"({ "master-volume": 1.0 })");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, EXPECTED_RESULT);
}

TEST_F(RaceSfxHandlerTest, PostById)
{
    std::mutex mutex;
    NiceMock<MockRaceSfxExchange> sfx;
    auto createSounds = [] (const std::function<void(MockSfxSoundWrapper&, MockSfxSoundWrapper&)>& setExpectations)
    {
        auto sfx0 = createMockSfxSoundWrapper("ID0", "STATUS-0", false, 1.0f, 0.5f, 1.5f, RestApi::Position{1.0f, 2.0f, 0.0f});
        auto sfx1 = createMockSfxSoundWrapper("ID1", "STATUS-1", false, 0.125f, 0.25f, 10.0f, RestApi::Position{1.0f, 1.5f, -0.5f});
        setExpectations(*sfx0, *sfx1);
        std::vector<std::unique_ptr<RestApi::SfxSoundWrapper>> result;
        result.emplace_back(std::move(sfx0));
        result.emplace_back(std::move(sfx1));
        return result;
    };
    Sequence sequence;
    EXPECT_CALL(sfx, getAllSounds()).Times(1).InSequence(sequence).WillOnce([&createSounds] {
        return createSounds([] (MockSfxSoundWrapper& sfx, MockSfxSoundWrapper&) {
            EXPECT_CALL(sfx, setStatus("NEW_STATUS"));
            EXPECT_CALL(sfx, setLoop(false));
            EXPECT_CALL(sfx, setVolume(0.75f));
            EXPECT_CALL(sfx, setPitch(0.5f));
            EXPECT_CALL(sfx, setPosition(RestApi::Position{18.0f, 9.0f, -5.0f}));
        });
    });
    auto handler = RestApi::Handler::createRaceSfxHandler(sfx, mutex);
    auto [status, result] = handler->handlePost("0", R"({ "status": "NEW_STATUS", "loop": false, "volume": 0.75, "pitch": 0.5, "position": [18.0, 9.0, -5.0] })");
    constexpr const char* ID0_RESULT =
        "{\n"
        "    \"sound\": \"ID0\",\n"
        "    \"status\": \"STATUS-0\",\n"
        "    \"loop\": false,\n"
        "    \"volume\": 1.0,\n"
        "    \"pitch\": 0.5,\n"
        "    \"play-time\": 1.5,\n"
        "    \"position\": [\n"
        "        1.0,\n"
        "        2.0,\n"
        "        0.0\n"
        "    ]\n"
        "}";
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, ID0_RESULT);
    EXPECT_CALL(sfx, getAllSounds()).Times(1).InSequence(sequence).WillOnce([&createSounds] {
        return createSounds([] (MockSfxSoundWrapper&, MockSfxSoundWrapper& sfx) {
            EXPECT_CALL(sfx, setStatus("OTHER"));
            EXPECT_CALL(sfx, setLoop(true));
            EXPECT_CALL(sfx, setVolume(1.0f));
            EXPECT_CALL(sfx, setPitch(0.125f));
            EXPECT_CALL(sfx, setPosition(RestApi::Position{5.0f, 8.0f, 7.5f}));
        });
    });
    std::tie(status, result) = handler->handlePost("1", R"({ "status": "OTHER", "loop": true, "volume": 1.0, "pitch": 0.125, "position": [5.0, 8.0, 7.5] })");
    constexpr const char* ID1_RESULT =
        "{\n"
        "    \"sound\": \"ID1\",\n"
        "    \"status\": \"STATUS-1\",\n"
        "    \"loop\": false,\n"
        "    \"volume\": 0.125,\n"
        "    \"pitch\": 0.25,\n"
        "    \"play-time\": 10.0,\n"
        "    \"position\": [\n"
        "        1.0,\n"
        "        1.5,\n"
        "        -0.5\n"
        "    ]\n"
        "}";
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, ID1_RESULT);
    EXPECT_CALL(sfx, getAllSounds()).Times(1).InSequence(sequence).WillOnce([&createSounds] {
        return createSounds([] (MockSfxSoundWrapper&, MockSfxSoundWrapper& sfx) {
            EXPECT_CALL(sfx, setStatus("MyStatus"));
            EXPECT_CALL(sfx, setLoop(true));
            EXPECT_CALL(sfx, setVolume(0.125f));
            EXPECT_CALL(sfx, setPitch(0.5f));
        });
    });
    std::tie(status, result) = handler->handlePost("1", R"({ "status": "MyStatus", "loop": true, "volume": 0.125, "pitch": 0.5 })");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, ID1_RESULT);
    EXPECT_CALL(sfx, getAllSounds()).Times(1).InSequence(sequence).WillOnce([&createSounds] {
        return createSounds([] (MockSfxSoundWrapper& sfx, MockSfxSoundWrapper&) {
            EXPECT_CALL(sfx, setLoop(false));
            EXPECT_CALL(sfx, setPitch(1.0f));
            EXPECT_CALL(sfx, setPosition(RestApi::Position{1.0f, -3.0f, 0.0f}));
        });
    });
    std::tie(status, result) = handler->handlePost("0", R"({ "loop": false, "pitch": 1.0, "position": [1.0, -3.0, 0.0] })");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, ID0_RESULT);
    EXPECT_CALL(sfx, getAllSounds()).Times(1).InSequence(sequence).WillOnce([&createSounds] {
        return createSounds([] (MockSfxSoundWrapper& sfx, MockSfxSoundWrapper&) {
            EXPECT_CALL(sfx, setVolume(0.75f));
        });
    });
    std::tie(status, result) = handler->handlePost("0", R"({ "volume": 0.75 })");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, ID0_RESULT);
    EXPECT_CALL(sfx, getAllSounds()).Times(AtLeast(1)).InSequence(sequence).WillOnce([&createSounds] {
        return createSounds([] (MockSfxSoundWrapper&, MockSfxSoundWrapper&) {});
    });
    EXPECT_EQ(handler->handlePost("2", R"({})").first, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(handler->handlePost("abc", R"({})").first, RestApi::STATUS_CODE::NOT_FOUND);
}

TEST_F(RaceSfxHandlerTest, Put)
{
    std::mutex mutex;
    NiceMock<MockRaceSfxExchange> sfx;
    InSequence sequence;
    auto handler = RestApi::Handler::createRaceSfxHandler(sfx, mutex);
    EXPECT_CALL(sfx, add("TheSound"))
        .Times(1)
        .WillOnce([] {
            return createMockSfxSoundWrapper(
                "sound",
                "status",
                true,
                0.75f,
                1.0f,
                5.0f,
                RestApi::Position{1.0f, 5.0f, 2.0f});
        });
    auto [status, result] = handler->handlePut(R"({"sound": "TheSound"})");
    EXPECT_EQ(status, RestApi::STATUS_CODE::CREATED);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"sound\": \"sound\",\n"
        "    \"status\": \"status\",\n"
        "    \"loop\": true,\n"
        "    \"volume\": 0.75,\n"
        "    \"pitch\": 1.0,\n"
        "    \"play-time\": 5.0,\n"
        "    \"position\": [\n"
        "        1.0,\n"
        "        5.0,\n"
        "        2.0\n"
        "    ]\n"
        "}");
    EXPECT_CALL(sfx, add("sfx"))
        .Times(1)
        .WillOnce([] {
            return createMockSfxSoundWrapper(
                "SFX",
                "STATUS",
                false,
                0.875f,
                0.125f,
                11.0f,
                std::nullopt);
        });
    std::tie(status, result) = handler->handlePut(R"({"sound": "sfx"})");
    EXPECT_EQ(status, RestApi::STATUS_CODE::CREATED);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"sound\": \"SFX\",\n"
        "    \"status\": \"STATUS\",\n"
        "    \"loop\": false,\n"
        "    \"volume\": 0.875,\n"
        "    \"pitch\": 0.125,\n"
        "    \"play-time\": 11.0,\n"
        "    \"position\": null\n"
        "}");
    EXPECT_THROW(handler->handlePut(R"({})"), std::invalid_argument);
}

TEST_F(RaceSfxHandlerTest, Delete)
{
    std::mutex mutex;
    NiceMock<MockRaceSfxExchange> sfx;
    InSequence sequence;
    EXPECT_CALL(sfx, remove(1)).Times(1);
    EXPECT_CALL(sfx, remove(5)).Times(1);
    auto handler = RestApi::Handler::createRaceSfxHandler(sfx, mutex);
    EXPECT_EQ(handler->handleDelete("1").first, RestApi::STATUS_CODE::NO_CONTENT);
    EXPECT_EQ(handler->handleDelete("5").first, RestApi::STATUS_CODE::NO_CONTENT);
    EXPECT_THROW(handler->handleDelete("abc"), std::invalid_argument);
}
