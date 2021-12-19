#include <gtest/gtest.h>
#include "rest-api/Handler.hpp"
#include "test/rest-api/MockDataExchange.hpp"

using testing::ByMove;
using testing::ByRef;
using testing::Return;
using testing::ReturnRef;
using testing::NiceMock;

class RaceMaterialHandlerTest : public testing::Test
{
};

TEST_F(RaceMaterialHandlerTest, MaterialNotImplemented)
{
    std::mutex mutex;
    NiceMock<MockRaceMaterialExchange> raceMaterialExchange;
    auto handler = RestApi::Handler::createRaceMaterialHandler(raceMaterialExchange, mutex);
    auto [putStatusCode, putResult] = handler->handlePut("{}");
    EXPECT_EQ(putStatusCode, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(putResult, "null");
    auto [deleteStatusCode, deleteResult] = handler->handleDelete("{}");
    EXPECT_EQ(deleteStatusCode, RestApi::STATUS_CODE::NOT_FOUND);
    EXPECT_EQ(deleteResult, "null");
}

TEST_F(RaceMaterialHandlerTest, MaterialEmpty)
{
    std::mutex mutex;
    NiceMock<MockRaceMaterialExchange> raceMaterialExchange;
    ON_CALL(raceMaterialExchange, getMaterials()).WillByDefault(Return(ByMove(std::vector<std::unique_ptr<RestApi::MaterialWrapper>>())));
    auto handler = RestApi::Handler::createRaceMaterialHandler(raceMaterialExchange, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(result, "[]");
}

static std::unique_ptr<NiceMock<MockMaterialWrapper>> createMockMaterialExchange(
    const std::string& name,
    const std::string& path,
    std::optional<char> mirrorAxis,
    bool belowSurface,
    bool fallingEffect,
    bool driveReset,
    bool surface,
    bool jumpTexture,
    bool gravity,
    bool ignore,
    bool highTireAdhesion,
    bool textureCompression,
    const std::string& collisionReaction,
    const std::optional<std::string>& collisionParticles,
    std::optional<std::reference_wrapper<const RestApi::ParticleWrapper>> particlesOnDrive,
    std::optional<std::reference_wrapper<const RestApi::ParticleWrapper>> particlesOnSkid,
    bool uClamp,
    bool vClamp,
    const std::vector<float>& randomHueSettings,
    int slowDownTicks,
    float slowDownMaxSpeedFraction,
    bool zipper,
    float zipperMinSpeed,
    float zipperMaxSpeedIncrease,
    float zipperDuration,
    float zipperSpeedGain,
    float zipperFadeOutTime,
    float zipperEngineForce,
    const std::optional<std::string>& sfxName,
    float sfxMinSpeed,
    float sfxMaxSpeed,
    float sfxMinPitch,
    float sfxMaxPitch,
    float sfxPitchPerSpeed,
    const std::optional<std::string>& alphaMask,
    bool colorizable,
    float colorizationFactor,
    const std::string& colorizationMask,
    const std::string& shaderName,
    const std::optional<std::string>& uVTwoTexture,
    const std::array<std::string, 6>& samplerPath)
{
    auto material = std::make_unique<NiceMock<MockMaterialWrapper>>();
    ON_CALL(*material, getName()).WillByDefault(Return(name));
    ON_CALL(*material, getPath()).WillByDefault(Return(path));
    ON_CALL(*material, getMirrorAxis()).WillByDefault(Return(mirrorAxis));
    ON_CALL(*material, isBelowSurface()).WillByDefault(Return(belowSurface));
    ON_CALL(*material, hasFallingEffect()).WillByDefault(Return(fallingEffect));
    ON_CALL(*material, isDriveReset()).WillByDefault(Return(driveReset));
    ON_CALL(*material, isSurface()).WillByDefault(Return(surface));
    ON_CALL(*material, isJumpTexture()).WillByDefault(Return(jumpTexture));
    ON_CALL(*material, hasGravity()).WillByDefault(Return(gravity));
    ON_CALL(*material, isIgnore()).WillByDefault(Return(ignore));
    ON_CALL(*material, hasHighTireAdhesion()).WillByDefault(Return(highTireAdhesion));
    ON_CALL(*material, hasTextureCompression()).WillByDefault(Return(textureCompression));
    ON_CALL(*material, getCollisionReaction()).WillByDefault(Return(collisionReaction));
    ON_CALL(*material, getCollisionParticles()).WillByDefault(Return(collisionParticles));
    ON_CALL(*material, getParticlesOnDrive()).WillByDefault(Return(particlesOnDrive));
    ON_CALL(*material, getParticlesOnSkid()).WillByDefault(Return(particlesOnSkid));
    ON_CALL(*material, hasUClamp()).WillByDefault(Return(uClamp));
    ON_CALL(*material, hasVClamp()).WillByDefault(Return(vClamp));
    ON_CALL(*material, getRandomHueSettings()).WillByDefault(Return(randomHueSettings));
    ON_CALL(*material, getSlowDownTicks()).WillByDefault(Return(slowDownTicks));
    ON_CALL(*material, getSlowDownMaxSpeedFraction()).WillByDefault(Return(slowDownMaxSpeedFraction));
    ON_CALL(*material, isZipper()).WillByDefault(Return(zipper));
    ON_CALL(*material, getZipperMinSpeed()).WillByDefault(Return(zipperMinSpeed));
    ON_CALL(*material, getZipperMaxSpeedIncrease()).WillByDefault(Return(zipperMaxSpeedIncrease));
    ON_CALL(*material, getZipperDuration()).WillByDefault(Return(zipperDuration));
    ON_CALL(*material, getZipperSpeedGain()).WillByDefault(Return(zipperSpeedGain));
    ON_CALL(*material, getZipperFadeOutTime()).WillByDefault(Return(zipperFadeOutTime));
    ON_CALL(*material, getZipperEngineForce()).WillByDefault(Return(zipperEngineForce));
    ON_CALL(*material, getSfxName()).WillByDefault(Return(sfxName));
    ON_CALL(*material, getSfxMinSpeed()).WillByDefault(Return(sfxMinSpeed));
    ON_CALL(*material, getSfxMaxSpeed()).WillByDefault(Return(sfxMaxSpeed));
    ON_CALL(*material, getSfxMinPitch()).WillByDefault(Return(sfxMinPitch));
    ON_CALL(*material, getSfxMaxPitch()).WillByDefault(Return(sfxMaxPitch));
    ON_CALL(*material, getSfxPitchPerSpeed()).WillByDefault(Return(sfxPitchPerSpeed));
    ON_CALL(*material, getAlphaMask()).WillByDefault(Return(alphaMask));
    ON_CALL(*material, isColorizable()).WillByDefault(Return(colorizable));
    ON_CALL(*material, getColorizationFactor()).WillByDefault(Return(colorizationFactor));
    ON_CALL(*material, getColorizationMask()).WillByDefault(Return(colorizationMask));
    ON_CALL(*material, getShaderName()).WillByDefault(Return(shaderName));
    ON_CALL(*material, getUVTwoTexture()).WillByDefault(Return(uVTwoTexture));
    ON_CALL(*material, getSamplerPath()).WillByDefault(Return(samplerPath));
    return material;
}

TEST_F(RaceMaterialHandlerTest, MaterialSingle)
{
    auto material = createMockMaterialExchange(
        "MaterialName",
        "/abc/efg",
        'U',
        true,
        false,
        true,
        false,
        true,
        false,
        true,
        false,
        true,
        "NORMAL",
        std::nullopt,
        std::nullopt,
        std::nullopt,
        true,
        false,
        {2.0f,1.0f,3.0f},
        8,
        0.5f,
        true,
        1.0f,
        2.0f,
        23.0f,
        2.0f,
        1.0f,
        5.0f,
        "SFX_NAME",
        0.5f,
        2.0f,
        1.0f,
        3.0f,
        0.5f,
        "ALPHA",
        true,
        2.0f,
        "MASK",
        "alphablend",
        std::nullopt,
        {"/sampler1", "/sampler/2", "/3/sampler/3/"}
    );
    auto materials = std::vector<std::unique_ptr<RestApi::MaterialWrapper>>();
    materials.push_back(std::move(material));
    auto raceMaterialExchange = NiceMock<MockRaceMaterialExchange>();
    EXPECT_CALL(raceMaterialExchange, getMaterials())
        .Times(1)
        .WillOnce(Return(ByMove(std::move(materials))));
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceMaterialHandler(raceMaterialExchange, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"texture\": {\n"
        "            \"name\": \"MaterialName\",\n"
        "            \"path\": \"/abc/efg\"\n"
        "        },\n"
        "        \"mirror-axis-when-reverse\": \"U\",\n"
        "        \"below-surface\": true,\n"
        "        \"falling-effect\": false,\n"
        "        \"surface\": false,\n"
        "        \"drive-reset\": true,\n"
        "        \"jump-texture\": true,\n"
        "        \"gravity\": false,\n"
        "        \"ignore\": true,\n"
        "        \"high-tire-adhesion\": false,\n"
        "        \"texture-compression\": true,\n"
        "        \"collision\": {\n"
        "            \"collision-reaction-value\": \"NORMAL\",\n"
        "            \"particles\": null\n"
        "        },\n"
        "        \"particles\": {\n"
        "            \"on-drive\": null,\n"
        "            \"on-skid\": null\n"
        "        },\n"
        "        \"clamp\": {\n"
        "            \"u\": true,\n"
        "            \"v\": false\n"
        "        },\n"
        "        \"hue\": [\n"
        "            2.0,\n"
        "            1.0,\n"
        "            3.0\n"
        "        ],\n"
        "        \"slowdown\": {\n"
        "            \"ticks\": 8,\n"
        "            \"max-speed-fraction\": 0.5\n"
        "        },\n"
        "        \"zipper\": {\n"
        "            \"min-speed\": 1.0,\n"
        "            \"max-speed-increase\": 2.0,\n"
        "            \"duration\": 23.0,\n"
        "            \"speed-gain\": 2.0,\n"
        "            \"fade-out-time\": 1.0,\n"
        "            \"engine-force\": 5.0\n"
        "        },\n"
        "        \"sfx\": {\n"
        "            \"name\": \"SFX_NAME\",\n"
        "            \"speed\": {\n"
        "                \"min\": 0.5,\n"
        "                \"max\": 2.0\n"
        "            },\n"
        "            \"pitch\": {\n"
        "                \"min\": 1.0,\n"
        "                \"max\": 3.0,\n"
        "                \"per-speed\": 0.5\n"
        "            }\n"
        "        },\n"
        "        \"alpha-mask\": \"ALPHA\",\n"
        "        \"colorization\": {\n"
        "            \"factor\": 2.0,\n"
        "            \"mask\": \"MASK\"\n"
        "        },\n"
        "        \"shader\": {\n"
        "            \"name\": \"alphablend\",\n"
        "            \"uv-two-texture\": null,\n"
        "            \"sampler-path\": [\n"
        "                \"/sampler1\",\n"
        "                \"/sampler/2\",\n"
        "                \"/3/sampler/3/\",\n"
        "                \"\",\n"
        "                \"\",\n"
        "                \"\"\n"
        "            ]\n"
        "        }\n"
        "    }\n"
        "]");
}

TEST_F(RaceMaterialHandlerTest, MaterialMany)
{
    auto material0 = createMockMaterialExchange(
        "material_0",
        "/material_0",
        'V',
        false,
        true,
        false,
        true,
        false,
        true,
        false,
        true,
        false,
        "RESCUE",
        "particle_name",
        std::nullopt,
        std::nullopt,
        false,
        true,
        {0.5f,0.0f},
        0,
        100.0f,
        false,
        1.0f,
        2.0f,
        3.0f,
        4.0f,
        5.0f,
        6.0f,
        std::nullopt,
        1.0f,
        2.0f,
        3.0f,
        4.0f,
        5.0f,
        "mask",
        false,
        1.0f,
        "abc",
        "solid",
        "/second_texture",
        {}
    );
    auto material1 = createMockMaterialExchange(
        "material_1",
        "/material_1",
        std::nullopt,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        "PUSH_BACK",
        std::nullopt,
        std::nullopt,
        std::nullopt,
        false,
        false,
        {2.0f,1.0f,3.0f},
        1,
        2.0f,
        true,
        2.0f,
        3.0f,
        4.0f,
        5.0f,
        6.0f,
        7.0f,
        "sound",
        1.0f,
        10.0f,
        5.0f,
        15.0f,
        2.0f,
        "AlphaMask",
        true,
        1.0f,
        "colorization",
        "shader",
        std::nullopt,
        {"/material_0", "/material_1", "/material_2", "/material_3", "/material_4", "/material_5"}
    );
    auto materials = std::vector<std::unique_ptr<RestApi::MaterialWrapper>>();
    materials.push_back(std::move(material0));
    materials.push_back(std::move(material1));
    auto raceMaterialExchange = NiceMock<MockRaceMaterialExchange>();
    EXPECT_CALL(raceMaterialExchange, getMaterials())
        .Times(1)
        .WillOnce(Return(ByMove(std::move(materials))));
    std::mutex mutex;
    auto handler = RestApi::Handler::createRaceMaterialHandler(raceMaterialExchange, mutex);
    auto [status, result] = handler->handleGet();
    EXPECT_EQ(status, RestApi::STATUS_CODE::OK);
    EXPECT_EQ(
        result,
        "[\n"
        "    {\n"
        "        \"texture\": {\n"
        "            \"name\": \"material_0\",\n"
        "            \"path\": \"/material_0\"\n"
        "        },\n"
        "        \"mirror-axis-when-reverse\": \"V\",\n"
        "        \"below-surface\": false,\n"
        "        \"falling-effect\": true,\n"
        "        \"surface\": true,\n"
        "        \"drive-reset\": false,\n"
        "        \"jump-texture\": false,\n"
        "        \"gravity\": true,\n"
        "        \"ignore\": false,\n"
        "        \"high-tire-adhesion\": true,\n"
        "        \"texture-compression\": false,\n"
        "        \"collision\": {\n"
        "            \"collision-reaction-value\": \"RESCUE\",\n"
        "            \"particles\": \"particle_name\"\n"
        "        },\n"
        "        \"particles\": {\n"
        "            \"on-drive\": null,\n"
        "            \"on-skid\": null\n"
        "        },\n"
        "        \"clamp\": {\n"
        "            \"u\": false,\n"
        "            \"v\": true\n"
        "        },\n"
        "        \"hue\": [\n"
        "            0.5,\n"
        "            0.0\n"
        "        ],\n"
        "        \"slowdown\": {\n"
        "            \"ticks\": 0,\n"
        "            \"max-speed-fraction\": 100.0\n"
        "        },\n"
        "        \"zipper\": null,\n"
        "        \"sfx\": null,\n"
        "        \"alpha-mask\": \"mask\",\n"
        "        \"colorization\": null,\n"
        "        \"shader\": {\n"
        "            \"name\": \"solid\",\n"
        "            \"uv-two-texture\": \"/second_texture\",\n"
        "            \"sampler-path\": [\n"
        "                \"\",\n"
        "                \"\",\n"
        "                \"\",\n"
        "                \"\",\n"
        "                \"\",\n"
        "                \"\"\n"
        "            ]\n"
        "        }\n"
        "    },\n"
        "    {\n"
        "        \"texture\": {\n"
        "            \"name\": \"material_1\",\n"
        "            \"path\": \"/material_1\"\n"
        "        },\n"
        "        \"mirror-axis-when-reverse\": null,\n"
        "        \"below-surface\": false,\n"
        "        \"falling-effect\": false,\n"
        "        \"surface\": false,\n"
        "        \"drive-reset\": false,\n"
        "        \"jump-texture\": false,\n"
        "        \"gravity\": false,\n"
        "        \"ignore\": false,\n"
        "        \"high-tire-adhesion\": false,\n"
        "        \"texture-compression\": false,\n"
        "        \"collision\": {\n"
        "            \"collision-reaction-value\": \"PUSH_BACK\",\n"
        "            \"particles\": null\n"
        "        },\n"
        "        \"particles\": {\n"
        "            \"on-drive\": null,\n"
        "            \"on-skid\": null\n"
        "        },\n"
        "        \"clamp\": {\n"
        "            \"u\": false,\n"
        "            \"v\": false\n"
        "        },\n"
        "        \"hue\": [\n"
        "            2.0,\n"
        "            1.0,\n"
        "            3.0\n"
        "        ],\n"
        "        \"slowdown\": {\n"
        "            \"ticks\": 1,\n"
        "            \"max-speed-fraction\": 2.0\n"
        "        },\n"
        "        \"zipper\": {\n"
        "            \"min-speed\": 2.0,\n"
        "            \"max-speed-increase\": 3.0,\n"
        "            \"duration\": 4.0,\n"
        "            \"speed-gain\": 5.0,\n"
        "            \"fade-out-time\": 6.0,\n"
        "            \"engine-force\": 7.0\n"
        "        },\n"
        "        \"sfx\": {\n"
        "            \"name\": \"sound\",\n"
        "            \"speed\": {\n"
        "                \"min\": 1.0,\n"
        "                \"max\": 10.0\n"
        "            },\n"
        "            \"pitch\": {\n"
        "                \"min\": 5.0,\n"
        "                \"max\": 15.0,\n"
        "                \"per-speed\": 2.0\n"
        "            }\n"
        "        },\n"
        "        \"alpha-mask\": \"AlphaMask\",\n"
        "        \"colorization\": {\n"
        "            \"factor\": 1.0,\n"
        "            \"mask\": \"colorization\"\n"
        "        },\n"
        "        \"shader\": {\n"
        "            \"name\": \"shader\",\n"
        "            \"uv-two-texture\": null,\n"
        "            \"sampler-path\": [\n"
        "                \"/material_0\",\n"
        "                \"/material_1\",\n"
        "                \"/material_2\",\n"
        "                \"/material_3\",\n"
        "                \"/material_4\",\n"
        "                \"/material_5\"\n"
        "            ]\n"
        "        }\n"
        "    }\n"
        "]");
}
