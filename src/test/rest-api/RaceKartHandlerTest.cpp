#include <gtest/gtest.h>
#include "rest-api/Handler.hpp"
#include "test/rest-api/MockDataExchange.hpp"

using testing::ByMove;
using testing::ByRef;
using testing::InSequence;
using testing::Return;
using testing::ReturnRef;
using testing::NiceMock;

class RaceKartHandlerTest : public testing::Test
{
};

TEST_F(RaceKartHandlerTest, KartNotImplemented)
{
    std::mutex mutex;
    NiceMock<MockRaceKartExchange> karts;
    auto handler = RestApi::Handler::createRaceKartHandler(karts, mutex);
    auto [putStatusCode, putResult] = handler->handlePut("{}");
    EXPECT_EQ(putStatusCode, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(putResult, "null");
    auto [deleteStatusCode, deleteResult] = handler->handleDelete("{}");
    EXPECT_EQ(deleteStatusCode, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(deleteResult, "null");
}

TEST_F(RaceKartHandlerTest, KartEmpty)
{
    std::mutex mutex;
    NiceMock<MockRaceKartExchange> karts;
    auto handler = RestApi::Handler::createRaceKartHandler(karts, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, "[]");
}

static std::unique_ptr<NiceMock<MockKartWrapper>> createMockKartExchange(
    size_t id,
    int rank,
    const std::string& controller,
    const std::string& ident,
    uint32_t color,
    const std::string& type,
    const std::vector<std::string>& groups,
    const std::optional<std::string>& engineSfxType,
    const std::optional<std::string>& skidSound,
    float friction,
    float frictionSlip,
    const std::string& terrainImpulseType,
    float speed,
    float maxSpeed,
    std::optional<float> minBoostSpeed,
    RestApi::Vector velocity,
    const std::vector<RestApi::SpeedIncrease>& speedIncrease,
    const std::vector<RestApi::SpeedDecrease>& speedDecrease,
    RestApi::Position position,
    RestApi::Position frontPosition,
    bool jumping,
    bool flying,
    bool nearGround,
    bool onGround,
    float pitch,
    float roll,
    float lean,
    float leanMax,
    const std::string& handicapLevel,
    bool boostedAI,
    bool blockedByPlunger,
    bool shielded,
    bool squashed,
    bool eliminated,
    bool ghostKart,
    bool inRescue,
    const std::optional<std::string>& skidding,
    bool skiddingBonusReady,
    float skiddingFactor,
    float maxSkidding,
    float steer,
    float steerAngle,
    float acceleration,
    bool braking,
    bool fire,
    bool lookBack,
    const std::string& skidControl,
    float collisionImpulse,
    float collisionImpulseTime,
    float restitution,
    float collectedEnergy,
    float maxNitro,
    int8_t minNitroConsumption,
    float consumptionPerTick,
    bool nitroActivated,
    const std::optional<RestApi::Attachment>& attachment,
    const std::optional<RestApi::PowerUp>& powerUp,
    const std::string& minimapIconPath,
    std::optional<float> finishTime)
{
    auto kart = std::make_unique<NiceMock<MockKartWrapper>>();
    ON_CALL(*kart, getId()).WillByDefault(Return(id));
    ON_CALL(*kart, getRank()).WillByDefault(Return(rank));
    ON_CALL(*kart, getController()).WillByDefault(Return(controller));
    ON_CALL(*kart, getIdent()).WillByDefault(Return(ident));
    ON_CALL(*kart, getColor()).WillByDefault(Return(color));
    ON_CALL(*kart, getType()).WillByDefault(Return(type));
    ON_CALL(*kart, getGroups()).WillByDefault(Return(groups));
    ON_CALL(*kart, getEngineSfxType()).WillByDefault(Return(engineSfxType));
    ON_CALL(*kart, getSkidSound()).WillByDefault(Return(skidSound));
    ON_CALL(*kart, getFriction()).WillByDefault(Return(friction));
    ON_CALL(*kart, getFrictionSlip()).WillByDefault(Return(frictionSlip));
    ON_CALL(*kart, getTerrainImpulseType()).WillByDefault(Return(terrainImpulseType));
    ON_CALL(*kart, getSpeed()).WillByDefault(Return(speed));
    ON_CALL(*kart, getMaxSpeed()).WillByDefault(Return(maxSpeed));
    ON_CALL(*kart, getMinBoostSpeed()).WillByDefault(Return(minBoostSpeed));
    ON_CALL(*kart, getVelocity()).WillByDefault(Return(velocity));
    ON_CALL(*kart, getSpeedIncrease()).WillByDefault(Return(speedIncrease));
    ON_CALL(*kart, getSpeedDecrease()).WillByDefault(Return(speedDecrease));
    ON_CALL(*kart, getPosition()).WillByDefault(Return(position));
    ON_CALL(*kart, getFrontPosition()).WillByDefault(Return(frontPosition));
    ON_CALL(*kart, isJumping()).WillByDefault(Return(jumping));
    ON_CALL(*kart, isFlying()).WillByDefault(Return(flying));
    ON_CALL(*kart, isNearGround()).WillByDefault(Return(nearGround));
    ON_CALL(*kart, isOnGround()).WillByDefault(Return(onGround));
    ON_CALL(*kart, getPitch()).WillByDefault(Return(pitch));
    ON_CALL(*kart, getRoll()).WillByDefault(Return(roll));
    ON_CALL(*kart, getLean()).WillByDefault(Return(lean));
    ON_CALL(*kart, getLeanMax()).WillByDefault(Return(leanMax));
    ON_CALL(*kart, getHandicapLevel()).WillByDefault(Return(handicapLevel));
    ON_CALL(*kart, isBoostedAI()).WillByDefault(Return(boostedAI));
    ON_CALL(*kart, isBlockedByPlunger()).WillByDefault(Return(blockedByPlunger));
    ON_CALL(*kart, isShielded()).WillByDefault(Return(shielded));
    ON_CALL(*kart, isSquashed()).WillByDefault(Return(squashed));
    ON_CALL(*kart, isEliminated()).WillByDefault(Return(eliminated));
    ON_CALL(*kart, isGhostKart()).WillByDefault(Return(ghostKart));
    ON_CALL(*kart, isInRescue()).WillByDefault(Return(inRescue));
    ON_CALL(*kart, getSkidding()).WillByDefault(Return(skidding));
    ON_CALL(*kart, isSkiddingBonusReady()).WillByDefault(Return(skiddingBonusReady));
    ON_CALL(*kart, getSkiddingFactor()).WillByDefault(Return(skiddingFactor));
    ON_CALL(*kart, getMaxSkidding()).WillByDefault(Return(maxSkidding));
    ON_CALL(*kart, getSteer()).WillByDefault(Return(steer));
    ON_CALL(*kart, getMaxSteerAngle()).WillByDefault(Return(steerAngle));
    ON_CALL(*kart, getAcceleration()).WillByDefault(Return(acceleration));
    ON_CALL(*kart, isBraking()).WillByDefault(Return(braking));
    ON_CALL(*kart, doesFire()).WillByDefault(Return(fire));
    ON_CALL(*kart, doesLookBack()).WillByDefault(Return(lookBack));
    ON_CALL(*kart, getSkidControl()).WillByDefault(Return(skidControl));
    ON_CALL(*kart, getCollisionImpulse()).WillByDefault(Return(collisionImpulse));
    ON_CALL(*kart, getCollisionImpulseTime()).WillByDefault(Return(collisionImpulseTime));
    ON_CALL(*kart, getRestitution()).WillByDefault(Return(restitution));
    ON_CALL(*kart, getCollectedEnergy()).WillByDefault(Return(collectedEnergy));
    ON_CALL(*kart, getMaxNitro()).WillByDefault(Return(maxNitro));
    ON_CALL(*kart, getMinNitroConsumption()).WillByDefault(Return(minNitroConsumption));
    ON_CALL(*kart, getConsumptionPerTick()).WillByDefault(Return(consumptionPerTick));
    ON_CALL(*kart, hasNitroActivated()).WillByDefault(Return(nitroActivated));
    ON_CALL(*kart, getAttachment()).WillByDefault(Return(attachment));
    ON_CALL(*kart, getPowerUp()).WillByDefault(Return(powerUp));
    ON_CALL(*kart, getMinimapIconPath()).WillByDefault(Return(minimapIconPath));
    ON_CALL(*kart, getFinishTime()).WillByDefault(Return(finishTime));
    return kart;
}

TEST_F(RaceKartHandlerTest, KartSingle)
{
    auto kart = createMockKartExchange(
        1,
        2,
        "myController",
        "MyIdent",
        12345,
        "MyType",
        {"Group1", "Group2"},
        "MyEngineSfx",
        "MySkidSound",
        2.0f,
        3.0f,
        "MyTerrainImpulseType",
        2.0f,
        5.0f,
        4.0f,
        {1.0f, 2.0f, 3.0f},
        {
            RestApi::SpeedIncrease{"SpeedIncrease1", true, std::nullopt, 1.0f, 2.0f},
            RestApi::SpeedIncrease{"SpeedIncrease2", false, std::nullopt, 1.5f, 2.5f},
            RestApi::SpeedIncrease{"SpeedIncrease3", true, 5, 3.0f, 4.0f}
        },
        {
            RestApi::SpeedDecrease{"SpeedDecrease1", true, std::nullopt, 0.5f},
            RestApi::SpeedDecrease{"SpeedDecrease2", false, std::nullopt, 2.0f},
            RestApi::SpeedDecrease{"SpeedDecrease3", true, 15, 1.0f}
        },
        {5.0f, 15.0f, 10.0f},
        {-5.0f, -15.0f, 18.0f},
        true,
        false,
        true,
        false,
        11.0f,
        12.0f,
        13.0f,
        14.0f,
        "MyHandicap",
        false,
        true,
        false,
        true,
        false,
        true,
        false,
        "MySkidding",
        true,
        9.0f,
        10.0f,
        -11.0f,
        -12.0f,
        -13.0f,
        false,
        true,
        false,
        "MySkidControl",
        -1.0f,
        2.0f,
        30.0f,
        4.0f,
        55.0f,
        63,
        80.0f,
        true,
        RestApi::Attachment{"MyAttachment", 2},
        RestApi::PowerUp{"MyPowerUp", 3},
        "MyIconPath",
        101.0f);
    std::vector<std::unique_ptr<RestApi::KartWrapper>> karts;
    karts.push_back(std::move(kart));
    NiceMock<MockRaceKartExchange> raceKarts;
    EXPECT_CALL(raceKarts, getKarts())
        .Times(1)
        .WillOnce(Return(ByMove(std::move(karts))));
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceKartHandler(raceKarts, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": 1,\n"
        "        \"rank\": 2,\n"
        "        \"controller\": \"myController\",\n"
        "        \"characteristics\": {\n"
        "            \"ident\": \"MyIdent\",\n"
        "            \"color\": 12345,\n"
        "            \"type\": \"MyType\",\n"
        "            \"groups\": [\n"
        "                \"Group1\",\n"
        "                \"Group2\"\n"
        "            ],\n"
        "            \"engine-sfx\": \"MyEngineSfx\",\n"
        "            \"skid-sound\": \"MySkidSound\",\n"
        "            \"friction\": 2.0,\n"
        "            \"friction-slip\": 3.0,\n"
        "            \"terrain-impulse-type\": \"MyTerrainImpulseType\"\n"
        "        },\n"
        "        \"speed\": {\n"
        "            \"current\": 2.0,\n"
        "            \"max\": 5.0,\n"
        "            \"min-boost-speed\": 4.0,\n"
        "            \"velocity\": [\n"
        "                1.0,\n"
        "                2.0,\n"
        "                3.0\n"
        "            ],\n"
        "            \"increase\": [\n"
        "                {\n"
        "                    \"kind\": \"SpeedIncrease1\",\n"
        "                    \"value\": 1.0,\n"
        "                    \"engine-force\": 2.0\n"
        "                },\n"
        "                {\n"
        "                    \"kind\": \"SpeedIncrease3\",\n"
        "                    \"time-left\": 5,\n"
        "                    \"value\": 3.0,\n"
        "                    \"engine-force\": 4.0\n"
        "                }\n"
        "            ],\n"
        "            \"decrease\": [\n"
        "                {\n"
        "                    \"kind\": \"SpeedDecrease1\",\n"
        "                    \"fraction\": 0.5\n"
        "                },\n"
        "                {\n"
        "                    \"kind\": \"SpeedDecrease3\",\n"
        "                    \"time-left\": 15,\n"
        "                    \"fraction\": 1.0\n"
        "                }\n"
        "            ]\n"
        "        },\n"
        "        \"position\": {\n"
        "            \"current\": [\n"
        "                5.0,\n"
        "                15.0,\n"
        "                10.0\n"
        "            ],\n"
        "            \"front\": [\n"
        "                -5.0,\n"
        "                -15.0,\n"
        "                18.0\n"
        "            ],\n"
        "            \"jumping\": true,\n"
        "            \"flying\": false,\n"
        "            \"near-ground\": true,\n"
        "            \"on-ground\": false,\n"
        "            \"pitch\": 11.0,\n"
        "            \"roll\": 12.0,\n"
        "            \"lean\": 13.0,\n"
        "            \"lean-max\": 14.0\n"
        "        },\n"
        "        \"status\": {\n"
        "            \"handicap\": \"MyHandicap\",\n"
        "            \"boosted-ai\": false,\n"
        "            \"blocked-by-plunger\": true,\n"
        "            \"shielded\": false,\n"
        "            \"squashed\": true,\n"
        "            \"eliminated\": false,\n"
        "            \"ghost\": true,\n"
        "            \"rescue\": false\n"
        "        },\n"
        "        \"skidding\": {\n"
        "            \"status\": \"MySkidding\",\n"
        "            \"ready\": true,\n"
        "            \"factor\": 9.0,\n"
        "            \"max\": 10.0\n"
        "        },\n"
        "        \"control\": {\n"
        "            \"steer\": -11.0,\n"
        "            \"max-steer\": -12.0,\n"
        "            \"acceleration\": -13.0,\n"
        "            \"braking\": false,\n"
        "            \"fire\": true,\n"
        "            \"look-back\": false,\n"
        "            \"skid-control\": \"MySkidControl\"\n"
        "        },\n"
        "        \"collision\": {\n"
        "            \"impulse\": -1.0,\n"
        "            \"time\": 2.0,\n"
        "            \"restitution\": 30.0\n"
        "        },\n"
        "        \"nitro\": {\n"
        "            \"collected\": 4.0,\n"
        "            \"max\": 55.0,\n"
        "            \"min-ticks\": 63,\n"
        "            \"consumption-per-tick\": 80.0,\n"
        "            \"activated\": true\n"
        "        },\n"
        "        \"attachment\": {\n"
        "            \"type\": \"MyAttachment\",\n"
        "            \"ticks\": 2\n"
        "        },\n"
        "        \"power-up\": {\n"
        "            \"type\": \"MyPowerUp\",\n"
        "            \"count\": 3\n"
        "        },\n"
        "        \"icon\": null,\n"
        "        \"minimap-icon\": \"MyIconPath\",\n"
        "        \"shadow\": null,\n"
        "        \"ground\": null,\n"
        "        \"result\": {\n"
        "            \"finished\": true,\n"
        "            \"time\": 101.0\n"
        "        }\n"
        "    }\n"
        "]");
}

static std::vector<std::unique_ptr<RestApi::KartWrapper>> createManyKarts()
{

    auto kart0 = createMockKartExchange(
        2,
        3,
        "controller1",
        "ident1",
        1,
        "type1",
        {"group1"},
        "engine1",
        "skid1",
        3.0f,
        4.0f,
        "terrain1",
        3.0f,
        6.0f,
        5.0f,
        {4.0f, 3.0f, 2.0f},
        {
            RestApi::SpeedIncrease{"increase1", false, std::nullopt, 1.0f, 2.0f},
            RestApi::SpeedIncrease{"increase2", true, 8, 17.0f, 5.0f},
            RestApi::SpeedIncrease{"increase3", false, 5, 3.0f, 4.0f},
            RestApi::SpeedIncrease{"increase4", true, std::nullopt, 1.5f, 2.5f}
        },
        {
            RestApi::SpeedDecrease{"decrease1", false, std::nullopt, 0.5f},
            RestApi::SpeedDecrease{"decrease2", true, 16, 0.5f}
        },
        {6.0f, 16.0f, 11.0f},
        {-4.0f, -14.0f, 19.0f},
        false,
        true,
        false,
        true,
        12.0f,
        13.0f,
        14.0f,
        15.0f,
        "handicap1",
        true,
        false,
        true,
        false,
        true,
        false,
        true,
        "skidding1",
        false,
        10.0f,
        11.0f,
        -10.0f,
        -9.0f,
        -12.0f,
        true,
        false,
        true,
        "skid_control1",
        0.0f,
        3.0f,
        31.0f,
        5.0f,
        56.0f,
        64,
        81.0f,
        false,
        RestApi::Attachment{"attachment1", 4},
        RestApi::PowerUp{"power_up1", 1},
        "/icon_path/1",
        102.0f);
    auto kart1 = createMockKartExchange(
        5,
        4,
        "controller2",
        "ident2",
        3,
        "type2",
        {},
        std::nullopt,
        std::nullopt,
        4.0f,
        6.0f,
        "terrain2",
        5.0f,
        8.0f,
        std::nullopt,
        {4.0f, 6.0f, 5.0f},
        {
            RestApi::SpeedIncrease{"increase1", false, std::nullopt, 2.0f, 1.0f},
        },
        {
            RestApi::SpeedDecrease{"decrease1", false, std::nullopt, 2.5f},
        },
        {8.0f, 18.0f, 13.0f},
        {-2.0f, -12.0f, 21.0f},
        false,
        false,
        false,
        false,
        17.0f,
        16.0f,
        15.0f,
        14.0f,
        "handicap2",
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        std::nullopt,
        false,
        -12.0f,
        -11.0f,
        8.0f,
        9.0f,
        10.0f,
        false,
        false,
        false,
        "skid_control2",
        2.0f,
        5.0f,
        29.0f,
        3.0f,
        58.0f,
        13,
        84.0f,
        false,
        std::nullopt,
        std::nullopt,
        "/icon_path/2",
        std::nullopt);
    std::vector<std::unique_ptr<RestApi::KartWrapper>> karts;
    karts.push_back(std::move(kart0));
    karts.push_back(std::move(kart1));
    return karts;
}

TEST_F(RaceKartHandlerTest, KartMany)
{
    NiceMock<MockRaceKartExchange> raceKarts;
    auto karts = createManyKarts();
    EXPECT_CALL(raceKarts, getKarts())
        .Times(1)
        .WillOnce(Return(ByMove(std::move(karts))));
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceKartHandler(raceKarts, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"id\": 2,\n"
        "        \"rank\": 3,\n"
        "        \"controller\": \"controller1\",\n"
        "        \"characteristics\": {\n"
        "            \"ident\": \"ident1\",\n"
        "            \"color\": 1,\n"
        "            \"type\": \"type1\",\n"
        "            \"groups\": [\n"
        "                \"group1\"\n"
        "            ],\n"
        "            \"engine-sfx\": \"engine1\",\n"
        "            \"skid-sound\": \"skid1\",\n"
        "            \"friction\": 3.0,\n"
        "            \"friction-slip\": 4.0,\n"
        "            \"terrain-impulse-type\": \"terrain1\"\n"
        "        },\n"
        "        \"speed\": {\n"
        "            \"current\": 3.0,\n"
        "            \"max\": 6.0,\n"
        "            \"min-boost-speed\": 5.0,\n"
        "            \"velocity\": [\n"
        "                4.0,\n"
        "                3.0,\n"
        "                2.0\n"
        "            ],\n"
        "            \"increase\": [\n"
        "                {\n"
        "                    \"kind\": \"increase2\",\n"
        "                    \"time-left\": 8,\n"
        "                    \"value\": 17.0,\n"
        "                    \"engine-force\": 5.0\n"
        "                },\n"
        "                {\n"
        "                    \"kind\": \"increase4\",\n"
        "                    \"value\": 1.5,\n"
        "                    \"engine-force\": 2.5\n"
        "                }\n"
        "            ],\n"
        "            \"decrease\": [\n"
        "                {\n"
        "                    \"kind\": \"decrease2\",\n"
        "                    \"time-left\": 16,\n"
        "                    \"fraction\": 0.5\n"
        "                }\n"
        "            ]\n"
        "        },\n"
        "        \"position\": {\n"
        "            \"current\": [\n"
        "                6.0,\n"
        "                16.0,\n"
        "                11.0\n"
        "            ],\n"
        "            \"front\": [\n"
        "                -4.0,\n"
        "                -14.0,\n"
        "                19.0\n"
        "            ],\n"
        "            \"jumping\": false,\n"
        "            \"flying\": true,\n"
        "            \"near-ground\": false,\n"
        "            \"on-ground\": true,\n"
        "            \"pitch\": 12.0,\n"
        "            \"roll\": 13.0,\n"
        "            \"lean\": 14.0,\n"
        "            \"lean-max\": 15.0\n"
        "        },\n"
        "        \"status\": {\n"
        "            \"handicap\": \"handicap1\",\n"
        "            \"boosted-ai\": true,\n"
        "            \"blocked-by-plunger\": false,\n"
        "            \"shielded\": true,\n"
        "            \"squashed\": false,\n"
        "            \"eliminated\": true,\n"
        "            \"ghost\": false,\n"
        "            \"rescue\": true\n"
        "        },\n"
        "        \"skidding\": {\n"
        "            \"status\": \"skidding1\",\n"
        "            \"ready\": false,\n"
        "            \"factor\": 10.0,\n"
        "            \"max\": 11.0\n"
        "        },\n"
        "        \"control\": {\n"
        "            \"steer\": -10.0,\n"
        "            \"max-steer\": -9.0,\n"
        "            \"acceleration\": -12.0,\n"
        "            \"braking\": true,\n"
        "            \"fire\": false,\n"
        "            \"look-back\": true,\n"
        "            \"skid-control\": \"skid_control1\"\n"
        "        },\n"
        "        \"collision\": {\n"
        "            \"impulse\": 0.0,\n"
        "            \"time\": 3.0,\n"
        "            \"restitution\": 31.0\n"
        "        },\n"
        "        \"nitro\": {\n"
        "            \"collected\": 5.0,\n"
        "            \"max\": 56.0,\n"
        "            \"min-ticks\": 64,\n"
        "            \"consumption-per-tick\": 81.0,\n"
        "            \"activated\": false\n"
        "        },\n"
        "        \"attachment\": {\n"
        "            \"type\": \"attachment1\",\n"
        "            \"ticks\": 4\n"
        "        },\n"
        "        \"power-up\": {\n"
        "            \"type\": \"power_up1\",\n"
        "            \"count\": 1\n"
        "        },\n"
        "        \"icon\": null,\n"
        "        \"minimap-icon\": \"/icon_path/1\",\n"
        "        \"shadow\": null,\n"
        "        \"ground\": null,\n"
        "        \"result\": {\n"
        "            \"finished\": true,\n"
        "            \"time\": 102.0\n"
        "        }\n"
        "    },\n"
        "    {\n"
        "        \"id\": 5,\n"
        "        \"rank\": 4,\n"
        "        \"controller\": \"controller2\",\n"
        "        \"characteristics\": {\n"
        "            \"ident\": \"ident2\",\n"
        "            \"color\": 3,\n"
        "            \"type\": \"type2\",\n"
        "            \"groups\": [],\n"
        "            \"engine-sfx\": null,\n"
        "            \"skid-sound\": null,\n"
        "            \"friction\": 4.0,\n"
        "            \"friction-slip\": 6.0,\n"
        "            \"terrain-impulse-type\": \"terrain2\"\n"
        "        },\n"
        "        \"speed\": {\n"
        "            \"current\": 5.0,\n"
        "            \"max\": 8.0,\n"
        "            \"min-boost-speed\": null,\n"
        "            \"velocity\": [\n"
        "                4.0,\n"
        "                6.0,\n"
        "                5.0\n"
        "            ],\n"
        "            \"increase\": [],\n"
        "            \"decrease\": []\n"
        "        },\n"
        "        \"position\": {\n"
        "            \"current\": [\n"
        "                8.0,\n"
        "                18.0,\n"
        "                13.0\n"
        "            ],\n"
        "            \"front\": [\n"
        "                -2.0,\n"
        "                -12.0,\n"
        "                21.0\n"
        "            ],\n"
        "            \"jumping\": false,\n"
        "            \"flying\": false,\n"
        "            \"near-ground\": false,\n"
        "            \"on-ground\": false,\n"
        "            \"pitch\": 17.0,\n"
        "            \"roll\": 16.0,\n"
        "            \"lean\": 15.0,\n"
        "            \"lean-max\": 14.0\n"
        "        },\n"
        "        \"status\": {\n"
        "            \"handicap\": \"handicap2\",\n"
        "            \"boosted-ai\": false,\n"
        "            \"blocked-by-plunger\": false,\n"
        "            \"shielded\": false,\n"
        "            \"squashed\": false,\n"
        "            \"eliminated\": false,\n"
        "            \"ghost\": false,\n"
        "            \"rescue\": false\n"
        "        },\n"
        "        \"skidding\": {\n"
        "            \"status\": null,\n"
        "            \"ready\": false,\n"
        "            \"factor\": -12.0,\n"
        "            \"max\": -11.0\n"
        "        },\n"
        "        \"control\": {\n"
        "            \"steer\": 8.0,\n"
        "            \"max-steer\": 9.0,\n"
        "            \"acceleration\": 10.0,\n"
        "            \"braking\": false,\n"
        "            \"fire\": false,\n"
        "            \"look-back\": false,\n"
        "            \"skid-control\": \"skid_control2\"\n"
        "        },\n"
        "        \"collision\": {\n"
        "            \"impulse\": 2.0,\n"
        "            \"time\": 5.0,\n"
        "            \"restitution\": 29.0\n"
        "        },\n"
        "        \"nitro\": {\n"
        "            \"collected\": 3.0,\n"
        "            \"max\": 58.0,\n"
        "            \"min-ticks\": 13,\n"
        "            \"consumption-per-tick\": 84.0,\n"
        "            \"activated\": false\n"
        "        },\n"
        "        \"attachment\": null,\n"
        "        \"power-up\": null,\n"
        "        \"icon\": null,\n"
        "        \"minimap-icon\": \"/icon_path/2\",\n"
        "        \"shadow\": null,\n"
        "        \"ground\": null,\n"
        "        \"result\": {\n"
        "            \"finished\": false,\n"
        "            \"time\": null\n"
        "        }\n"
        "    }\n"
        "]");
}

TEST_F(RaceKartHandlerTest, SelectKart)
{
    auto createKart = [] {
        auto kart = createMockKartExchange(
            14,
            1,
            "controller",
            "ident",
            15,
            "type",
            {},
            "engine",
            "skid",
            3.0f,
            4.0f,
            "terrain",
            3.0f,
            6.0f,
            5.0f,
            {4.0f, 3.0f, 2.0f},
            {},
            {},
            {6.0f, 16.0f, 11.0f},
            {-4.0f, -14.0f, 19.0f},
            true,
            true,
            true,
            true,
            12.0f,
            13.0f,
            14.0f,
            15.0f,
            "handicap",
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            "skidding",
            true,
            10.0f,
            11.0f,
            -10.0f,
            -9.0f,
            -12.0f,
            true,
            true,
            true,
            "skid_control",
            0.0f,
            3.0f,
            31.0f,
            5.0f,
            56.0f,
            64,
            81.0f,
            true,
            RestApi::Attachment{"attachment", 7},
            RestApi::PowerUp{"power_up", 2},
            "/icon_path",
            std::nullopt);
        std::vector<std::unique_ptr<RestApi::KartWrapper>> karts;
        karts.push_back(std::move(kart));
        return karts;
    };
    NiceMock<MockRaceKartExchange> raceKarts;
    EXPECT_CALL(raceKarts, getKarts()).WillRepeatedly(createKart);
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceKartHandler(raceKarts, mutex);
    auto [status0, result0] = handler->handleGet("0");
    EXPECT_EQ(status0, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(result0, "null");
    auto [status1, result1] = handler->handleGet("14");
    EXPECT_EQ(status1, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result1,
        "{\n"
        "    \"id\": 14,\n"
        "    \"rank\": 1,\n"
        "    \"controller\": \"controller\",\n"
        "    \"characteristics\": {\n"
        "        \"ident\": \"ident\",\n"
        "        \"color\": 15,\n"
        "        \"type\": \"type\",\n"
        "        \"groups\": [],\n"
        "        \"engine-sfx\": \"engine\",\n"
        "        \"skid-sound\": \"skid\",\n"
        "        \"friction\": 3.0,\n"
        "        \"friction-slip\": 4.0,\n"
        "        \"terrain-impulse-type\": \"terrain\"\n"
        "    },\n"
        "    \"speed\": {\n"
        "        \"current\": 3.0,\n"
        "        \"max\": 6.0,\n"
        "        \"min-boost-speed\": 5.0,\n"
        "        \"velocity\": [\n"
        "            4.0,\n"
        "            3.0,\n"
        "            2.0\n"
        "        ],\n"
        "        \"increase\": [],\n"
        "        \"decrease\": []\n"
        "    },\n"
        "    \"position\": {\n"
        "        \"current\": [\n"
        "            6.0,\n"
        "            16.0,\n"
        "            11.0\n"
        "        ],\n"
        "        \"front\": [\n"
        "            -4.0,\n"
        "            -14.0,\n"
        "            19.0\n"
        "        ],\n"
        "        \"jumping\": true,\n"
        "        \"flying\": true,\n"
        "        \"near-ground\": true,\n"
        "        \"on-ground\": true,\n"
        "        \"pitch\": 12.0,\n"
        "        \"roll\": 13.0,\n"
        "        \"lean\": 14.0,\n"
        "        \"lean-max\": 15.0\n"
        "    },\n"
        "    \"status\": {\n"
        "        \"handicap\": \"handicap\",\n"
        "        \"boosted-ai\": true,\n"
        "        \"blocked-by-plunger\": true,\n"
        "        \"shielded\": true,\n"
        "        \"squashed\": true,\n"
        "        \"eliminated\": true,\n"
        "        \"ghost\": true,\n"
        "        \"rescue\": true\n"
        "    },\n"
        "    \"skidding\": {\n"
        "        \"status\": \"skidding\",\n"
        "        \"ready\": true,\n"
        "        \"factor\": 10.0,\n"
        "        \"max\": 11.0\n"
        "    },\n"
        "    \"control\": {\n"
        "        \"steer\": -10.0,\n"
        "        \"max-steer\": -9.0,\n"
        "        \"acceleration\": -12.0,\n"
        "        \"braking\": true,\n"
        "        \"fire\": true,\n"
        "        \"look-back\": true,\n"
        "        \"skid-control\": \"skid_control\"\n"
        "    },\n"
        "    \"collision\": {\n"
        "        \"impulse\": 0.0,\n"
        "        \"time\": 3.0,\n"
        "        \"restitution\": 31.0\n"
        "    },\n"
        "    \"nitro\": {\n"
        "        \"collected\": 5.0,\n"
        "        \"max\": 56.0,\n"
        "        \"min-ticks\": 64,\n"
        "        \"consumption-per-tick\": 81.0,\n"
        "        \"activated\": true\n"
        "    },\n"
        "    \"attachment\": {\n"
        "        \"type\": \"attachment\",\n"
        "        \"ticks\": 7\n"
        "    },\n"
        "    \"power-up\": {\n"
        "        \"type\": \"power_up\",\n"
        "        \"count\": 2\n"
        "    },\n"
        "    \"icon\": null,\n"
        "    \"minimap-icon\": \"/icon_path\",\n"
        "    \"shadow\": null,\n"
        "    \"ground\": null,\n"
        "    \"result\": {\n"
        "        \"finished\": false,\n"
        "        \"time\": null\n"
        "    }\n"
        "}");
    auto [status3, result3] = handler->handleGet("TEST");
    EXPECT_EQ(status3, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(result3, "null");
}

TEST_F(RaceKartHandlerTest, InvalidPostManyKarts)
{
    NiceMock<MockRaceKartExchange> raceKarts;
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceKartHandler(raceKarts, mutex);
    EXPECT_EQ(handler->handlePost("{}").first, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(handler->handlePost("5", "{}").first, RestApi::STATUS_CODE::NOT_FOUND);
}

TEST_F(RaceKartHandlerTest, PostKartType)
{
    NiceMock<MockRaceKartExchange> raceKarts;
    auto karts = createManyKarts();
    auto& kart = dynamic_cast<MockKartWrapper&>(*karts[0]);
    InSequence sequence;
    EXPECT_CALL(raceKarts, getKarts())
        .Times(1)
        .WillOnce(Return(ByMove(std::move(karts))));
    EXPECT_CALL(kart, setKart("beastie")).Times(1);
    EXPECT_CALL(raceKarts, getKarts()).Times(1).WillOnce(createManyKarts);
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceKartHandler(raceKarts, mutex);
    auto [status, result] = handler->handlePost(
        "2",
        R"({"characteristics": {"ident": "beastie"}})");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
}

MATCHER_P(AttachmentIs, other, "Match attachments")
{
    if (arg.has_value() && other.has_value())
    {
        return arg->type == other->type && arg->ticks == other->ticks;
    }
    return arg.has_value() == other.has_value();
}

TEST_F(RaceKartHandlerTest, PostNewAttachment)
{
    NiceMock<MockRaceKartExchange> raceKarts;
    auto karts = createManyKarts();
    auto& kart = dynamic_cast<MockKartWrapper&>(*karts[1]);
    InSequence sequence;
    EXPECT_CALL(raceKarts, getKarts()).Times(1).WillOnce(createManyKarts);
    EXPECT_CALL(raceKarts, getKarts())
        .Times(1)
        .WillOnce(Return(ByMove(std::move(karts))));
    EXPECT_CALL(kart, setAttachment(AttachmentIs(std::optional<RestApi::Attachment>(RestApi::Attachment{"newAttachment", 10})))).Times(1);
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceKartHandler(raceKarts, mutex);
    auto [status, result] = handler->handlePost(
        "5",
        R"({"attachment": {"type": "newAttachment", "ticks": 10}})");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
}

TEST_F(RaceKartHandlerTest, PostRemoveAttachment)
{
    NiceMock<MockRaceKartExchange> raceKarts;
    auto karts = createManyKarts();
    auto& kart = dynamic_cast<MockKartWrapper&>(*karts[1]);
    InSequence sequence;
    EXPECT_CALL(raceKarts, getKarts()).Times(1).WillOnce(createManyKarts);
    EXPECT_CALL(raceKarts, getKarts())
        .Times(1)
        .WillOnce(Return(ByMove(std::move(karts))));
    EXPECT_CALL(kart, setAttachment(AttachmentIs(std::optional<RestApi::Attachment>()))).Times(1);
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceKartHandler(raceKarts, mutex);
    auto [status, result] = handler->handlePost(
        "5",
        R"({"attachment": null})");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
}

MATCHER_P(PowerUpIs, other, "Match attachments")
{
    if (arg.has_value() && other.has_value())
    {
        return arg->name == other->name && arg->count == other->count;
    }
    return arg.has_value() == other.has_value();
}

TEST_F(RaceKartHandlerTest, PostPowerUp)
{
    NiceMock<MockRaceKartExchange> raceKarts;
    auto karts = createManyKarts();
    auto& kart = dynamic_cast<MockKartWrapper&>(*karts[0]);
    InSequence sequence;
    EXPECT_CALL(raceKarts, getKarts()).Times(1).WillOnce(createManyKarts);
    EXPECT_CALL(raceKarts, getKarts())
        .Times(1)
        .WillOnce(Return(ByMove(std::move(karts))));
    EXPECT_CALL(kart, setPowerUp(PowerUpIs(std::optional<RestApi::PowerUp>(RestApi::PowerUp{"newPowerUp", 5})))).Times(1);
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceKartHandler(raceKarts, mutex);
    auto [status, result] = handler->handlePost(
        "2",
        R"({"power-up": {"name": "newPowerUp", "count": 5}})");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
}

TEST_F(RaceKartHandlerTest, PostRemovePowerUp)
{
    NiceMock<MockRaceKartExchange> raceKarts;
    auto karts = createManyKarts();
    auto& kart = dynamic_cast<MockKartWrapper&>(*karts[0]);
    InSequence sequence;
    EXPECT_CALL(raceKarts, getKarts()).Times(1).WillOnce(createManyKarts);
    EXPECT_CALL(raceKarts, getKarts())
        .Times(1)
        .WillOnce(Return(ByMove(std::move(karts))));
    EXPECT_CALL(kart, setPowerUp(PowerUpIs(std::optional<RestApi::PowerUp>()))).Times(1);
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceKartHandler(raceKarts, mutex);
    auto [status, result] = handler->handlePost(
        "2",
        R"({"power-up": null})");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
}

TEST_F(RaceKartHandlerTest, PostAttachmentAndPowerUp)
{
    NiceMock<MockRaceKartExchange> raceKarts;
    auto karts = createManyKarts();
    auto& kart = dynamic_cast<MockKartWrapper&>(*karts[0]);
    InSequence sequence;
    EXPECT_CALL(raceKarts, getKarts()).Times(1).WillOnce(createManyKarts);
    EXPECT_CALL(raceKarts, getKarts())
        .Times(1)
        .WillOnce(Return(ByMove(std::move(karts))));
    EXPECT_CALL(kart, setAttachment(AttachmentIs(std::optional<RestApi::Attachment>(RestApi::Attachment{"A", 2})))).Times(1);
    EXPECT_CALL(kart, setPowerUp(PowerUpIs(std::optional<RestApi::PowerUp>(RestApi::PowerUp{"P", 1})))).Times(1);
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceKartHandler(raceKarts, mutex);
    auto [status, result] = handler->handlePost(
        "2",
        R"({"attachment": {"type": "A", "ticks": 2}, "power-up": {"name": "P", "count": 1}})");
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
}
