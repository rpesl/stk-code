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

class MusicHandlerTest : public MutexHandlerTestBase
{
};

TEST_F(MusicHandlerTest, GetEmpty)
{
    std::mutex mutex;
    NiceMock<MockMusicExchange> library;
    auto handler = RestApi::Handler::createMusicHandler(library, getMutex());
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, "[]");
}

TEST_F(MusicHandlerTest, GetOne)
{
    std::mutex mutex;
    NiceMock<MockMusicExchange> library;
    ON_CALL(library, getAllMusic()).WillByDefault(
        Return(
            std::vector<RestApi::MusicWrapper>{
                RestApi::MusicWrapper{
                    "MyId",
                    "MyTitle",
                    "MyComposer",
                    "MyFilename",
                    "MyFastFilename"
                }
            }));
    auto handler = RestApi::Handler::createMusicHandler(library, getMutex());
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": \"MyId\",\n"
        "        \"title\": \"MyTitle\",\n"
        "        \"composer\": \"MyComposer\",\n"
        "        \"filename\": \"MyFilename\",\n"
        "        \"fast-filename\": \"MyFastFilename\"\n"
        "    }\n"
        "]");
}

TEST_F(MusicHandlerTest, GetMany)
{
    std::mutex mutex;
    NiceMock<MockMusicExchange> library;
    ON_CALL(library, getAllMusic()).WillByDefault(
        Return(
            std::vector<RestApi::MusicWrapper>{
                RestApi::MusicWrapper{
                    "Id1",
                    "Title1",
                    "Composer1",
                    "Filename1",
                    "FastFilename1"
                },
                RestApi::MusicWrapper{
                    "Id2",
                    "Title2",
                    "Composer2",
                    "Filename2",
                    std::nullopt
                },
            }));
    auto handler = RestApi::Handler::createMusicHandler(library, getMutex());
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": \"Id1\",\n"
        "        \"title\": \"Title1\",\n"
        "        \"composer\": \"Composer1\",\n"
        "        \"filename\": \"Filename1\",\n"
        "        \"fast-filename\": \"FastFilename1\"\n"
        "    },\n"
        "    {\n"
        "        \"id\": \"Id2\",\n"
        "        \"title\": \"Title2\",\n"
        "        \"composer\": \"Composer2\",\n"
        "        \"filename\": \"Filename2\",\n"
        "        \"fast-filename\": null\n"
        "    }\n"
        "]");
}

TEST_F(MusicHandlerTest, GetById)
{
    std::mutex mutex;
    NiceMock<MockMusicExchange> library;
    ON_CALL(library, getAllMusic()).WillByDefault(
        Return(
            std::vector<RestApi::MusicWrapper>{
                RestApi::MusicWrapper{
                    "The_1",
                    "Title_1",
                    "Composer_1",
                    "Filename_1",
                    std::nullopt
                },
                RestApi::MusicWrapper{
                    "The_2",
                    "Title_2",
                    "Composer_2",
                    "Filename_2",
                    "FastFilename_2"
                },
            }));
    auto handler = RestApi::Handler::createMusicHandler(library, getMutex());
    auto [status, result] = handler->handleGet("The_1");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"The_1\",\n"
        "    \"title\": \"Title_1\",\n"
        "    \"composer\": \"Composer_1\",\n"
        "    \"filename\": \"Filename_1\",\n"
        "    \"fast-filename\": null\n"
        "}");
    std::tie(status, result) = handler->handleGet("The_2");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"The_2\",\n"
        "    \"title\": \"Title_2\",\n"
        "    \"composer\": \"Composer_2\",\n"
        "    \"filename\": \"Filename_2\",\n"
        "    \"fast-filename\": \"FastFilename_2\"\n"
        "}");
    EXPECT_EQ(handler->handleGet("The_0").first, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(handler->handleGet("The_3").first, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(handler->handleGet("abc").first, RestApi::STATUS_CODE::NOT_FOUND);
}

TEST_F(MusicHandlerTest, Put)
{
    std::mutex mutex;
    NiceMock<MockMusicExchange> library;
    ON_CALL(library, getAllMusic()).WillByDefault(
        Return(
            std::vector<RestApi::MusicWrapper>{
                RestApi::MusicWrapper{
                    "id_1",
                    "title_1",
                    "composer_1",
                    "filename_1",
                    std::nullopt
                },
                RestApi::MusicWrapper{
                    "id_2",
                    "SECOND_TITLE",
                    "COMPOSER",
                    "THE_FILENAME",
                    "THE_FAST_FILENAME"
                },
            }));
    Sequence sequenceUnzipFunction;
    EXPECT_CALL(library, getUnzipFunction()).Times(1).InSequence(sequenceUnzipFunction).WillOnce([] {
        return [](const std::string& data, const std::filesystem::path& directory) -> std::filesystem::path {
            EXPECT_EQ(data, "FIRST_ZIP");
            EXPECT_EQ(directory, "FIRST_DIRECTORY");
            return "/directory/1";
        };
    });
    EXPECT_CALL(library, getUnzipFunction()).Times(1).InSequence(sequenceUnzipFunction).WillOnce([] {
        return [](const std::string& data, const std::filesystem::path& directory) -> std::filesystem::path {
            EXPECT_EQ(data, "zip 2");
            EXPECT_EQ(directory, "D2");
            return "/2";
        };
    });
    Sequence sequenceDirectory;
    EXPECT_CALL(library, getDirectory()).Times(1).InSequence(sequenceDirectory).WillOnce(Return("FIRST_DIRECTORY"));
    EXPECT_CALL(library, getDirectory()).Times(1).InSequence(sequenceDirectory).WillOnce(Return("D2"));
    Sequence sequenceNewTrack;
    EXPECT_CALL(library, loadNewMusic(std::filesystem::path("/directory/1"))).Times(1).InSequence(sequenceNewTrack).WillOnce(Return("id_1"));
    EXPECT_CALL(library, loadNewMusic(std::filesystem::path("/2"))).Times(1).InSequence(sequenceNewTrack).WillOnce(Return("id_2"));
    auto handler = RestApi::Handler::createMusicHandler(library, getMutex());
    auto [status, result] = handler->handlePutZip("FIRST_ZIP");
    EXPECT_EQ(status, RestApi::STATUS_CODE::CREATED);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"id_1\",\n"
        "    \"title\": \"title_1\",\n"
        "    \"composer\": \"composer_1\",\n"
        "    \"filename\": \"filename_1\",\n"
        "    \"fast-filename\": null\n"
        "}");
    std::tie(status, result) = handler->handlePutZip("zip 2");
    EXPECT_EQ(status, RestApi::STATUS_CODE::CREATED);    EXPECT_EQ(
        result,
        "{\n"
        "    \"id\": \"id_2\",\n"
        "    \"title\": \"SECOND_TITLE\",\n"
        "    \"composer\": \"COMPOSER\",\n"
        "    \"filename\": \"THE_FILENAME\",\n"
        "    \"fast-filename\": \"THE_FAST_FILENAME\"\n"
        "}");
}

TEST_F(MusicHandlerTest, Delete)
{
    std::mutex mutex;
    NiceMock<MockMusicExchange> library;
    InSequence sequence;
    EXPECT_CALL(library, remove("music_123")).Times(1);
    EXPECT_CALL(library, remove("MyMusicTrack")).Times(1);
    auto handler = RestApi::Handler::createMusicHandler(library, getMutex());
    EXPECT_EQ(handler->handleDelete("music_123").first, RestApi::STATUS_CODE::NO_CONTENT);
    EXPECT_EQ(handler->handleDelete("MyMusicTrack").first, RestApi::STATUS_CODE::NO_CONTENT);
}

class RaceMusicHandlerTest : public testing::Test
{
};

TEST_F(RaceMusicHandlerTest, MusicNotImplemented)
{
    std::mutex mutex;
    NiceMock<MockRaceMusicExchange> music;
    auto handler = RestApi::Handler::createRaceMusicHandler(music, mutex);
    auto [deleteStatusCode, deleteResult] = handler->handleDelete("{}");
    EXPECT_EQ(deleteStatusCode, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(deleteResult, "null");
}

TEST_F(RaceMusicHandlerTest, MusicEmpty)
{
    std::mutex mutex;
    NiceMock<MockRaceMusicExchange> music;
    auto handler = RestApi::Handler::createRaceMusicHandler(music, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"enabled\": false,\n"
        "    \"volume\": 0.0,\n"
        "    \"music\": null\n"
        "}");
}

TEST_F(RaceMusicHandlerTest, MusicPlaying)
{
    std::mutex mutex;
    NiceMock<MockRaceMusicExchange> music;
    ON_CALL(music, isEnabled()).WillByDefault(Return(true));
    ON_CALL(music, getMasterMusicGain()).WillByDefault(Return(0.5f));
    ON_CALL(music, getMusic()).WillByDefault(Return(RestApi::MusicWrapper{
        "MusicId",
        "MusicTitle",
        "MusicComposer",
        "MusicFile",
        std::nullopt
    }));
    auto handler = RestApi::Handler::createRaceMusicHandler(music, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"enabled\": true,\n"
        "    \"volume\": 0.5,\n"
        "    \"music\": {\n"
        "        \"id\": \"MusicId\",\n"
        "        \"title\": \"MusicTitle\",\n"
        "        \"composer\": \"MusicComposer\",\n"
        "        \"filename\": \"MusicFile\",\n"
        "        \"fast-filename\": null\n"
        "    }\n"
        "}");
}

TEST_F(RaceMusicHandlerTest, MusicPlayingFast)
{
    std::mutex mutex;
    NiceMock<MockRaceMusicExchange> music;
    ON_CALL(music, isEnabled()).WillByDefault(Return(true));
    ON_CALL(music, getMasterMusicGain()).WillByDefault(Return(4.0f));
    ON_CALL(music, getMusic()).WillByDefault(Return(RestApi::MusicWrapper{
        "A",
        "B",
        "C",
        "D",
        "E"
    }));
    auto handler = RestApi::Handler::createRaceMusicHandler(music, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"enabled\": true,\n"
        "    \"volume\": 4.0,\n"
        "    \"music\": {\n"
        "        \"id\": \"A\",\n"
        "        \"title\": \"B\",\n"
        "        \"composer\": \"C\",\n"
        "        \"filename\": \"D\",\n"
        "        \"fast-filename\": \"E\"\n"
        "    }\n"
        "}");
}

TEST_F(RaceMusicHandlerTest, Post)
{
    std::mutex mutex;
    NiceMock<MockRaceMusicExchange> music;
    Sequence sequenceGet;
    Sequence sequenceSet;
    EXPECT_CALL(music, isEnabled()).Times(1).InSequence(sequenceGet).WillOnce(Return(true));
    EXPECT_CALL(music, getMasterMusicGain()).Times(1).InSequence(sequenceGet).WillOnce(Return(1.0f));
    EXPECT_CALL(music, getMusic()).Times(1).InSequence(sequenceGet).WillOnce(Return(RestApi::MusicWrapper{
        "MUSIC",
        "TITLE",
        "COMPOSE",
        "FILE",
        std::nullopt
    }));
    EXPECT_CALL(music, setEnabled(true)).Times(1).InSequence(sequenceSet);
    EXPECT_CALL(music, setMasterMusicGain(0.5f)).Times(1).InSequence(sequenceSet);
    EXPECT_CALL(music, setMusic(std::optional<std::string>("MUSIC_ID"))).Times(1).InSequence(sequenceSet);
    auto handler = RestApi::Handler::createRaceMusicHandler(music, mutex);
    auto [status, result] = handler->handlePost(R"({"enabled": true, "volume": 0.5, "music": "MUSIC_ID"})");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"enabled\": true,\n"
        "    \"volume\": 1.0,\n"
        "    \"music\": {\n"
        "        \"id\": \"MUSIC\",\n"
        "        \"title\": \"TITLE\",\n"
        "        \"composer\": \"COMPOSE\",\n"
        "        \"filename\": \"FILE\",\n"
        "        \"fast-filename\": null\n"
        "    }\n"
        "}");
    EXPECT_CALL(music, isEnabled()).Times(1).InSequence(sequenceGet).WillOnce(Return(false));
    EXPECT_CALL(music, getMasterMusicGain()).Times(1).InSequence(sequenceGet).WillOnce(Return(0.0f));
    EXPECT_CALL(music, getMusic()).Times(1).InSequence(sequenceGet).WillOnce(Return(std::nullopt));
    EXPECT_CALL(music, setMusic(std::optional<std::string>())).Times(1).InSequence(sequenceSet);
    std::tie(status, result) = handler->handlePost(R"({"music": null})");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"enabled\": false,\n"
        "    \"volume\": 0.0,\n"
        "    \"music\": null\n"
        "}");
    EXPECT_CALL(music, isEnabled()).Times(1).InSequence(sequenceGet).WillOnce(Return(true));
    EXPECT_CALL(music, getMasterMusicGain()).Times(1).InSequence(sequenceGet).WillOnce(Return(0.25f));
    EXPECT_CALL(music, getMusic()).Times(1).InSequence(sequenceGet).WillOnce(Return(std::nullopt));
    EXPECT_CALL(music, setEnabled(true)).Times(1).InSequence(sequenceSet);
    EXPECT_CALL(music, setMasterMusicGain(0.5f)).Times(1).InSequence(sequenceSet);
    std::tie(status, result) = handler->handlePost(R"({"enabled": true, "volume": 0.5})");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "{\n"
        "    \"enabled\": true,\n"
        "    \"volume\": 0.25,\n"
        "    \"music\": null\n"
        "}");
}
