#pragma once
#include <gmock/gmock.h>
#include "rest-api/DataExchange.hpp"
// ---------------------------------------------------------------------------------------------------------------------
using FloatArray2 = std::array<float, 2>;
using FloatArray3 = std::array<float, 3>;
using StringArray6 = std::array<std::string, 6>;
// ---------------------------------------------------------------------------------------------------------------------
class MockParticleWrapper;
std::unique_ptr<testing::NiceMock<MockParticleWrapper>> createMockParticleWrapper(
    const std::string& name,
    RestApi::Interval<float> size,
    RestApi::Interval<float> rate,
    RestApi::Interval<float> lifetime,
    int fadeoutTime,
    const std::string& shape,
    RestApi::Interval<uint32_t> color,
    FloatArray3 boxSize,
    float sphereRadius,
    int angleSpread,
    FloatArray3 velocity,
    int emissionDecayRate,
    FloatArray2 scaleAffactor,
    bool flips,
    bool vertical,
    bool randomY
);
// ---------------------------------------------------------------------------------------------------------------------
// Game resources
// ---------------------------------------------------------------------------------------------------------------------
class MockRaceExchange : public RestApi::RaceExchange
{
public:
    MOCK_METHOD(std::optional<uint64_t>, getId, (), (const, override));
    MOCK_METHOD(std::string, getStatus, (), (const, override));
    MOCK_METHOD(bool, isActive, (), (const, override));
    MOCK_METHOD(std::optional<std::string>, getTrackName, (), (const, override));
    MOCK_METHOD(std::optional<std::string>, getMajorRaceMode, (), (const, override));
    MOCK_METHOD(std::optional<std::string>, getMinorRaceMode, (), (const, override));
    MOCK_METHOD(std::optional<std::string>, getDifficulty, (), (const, override));
    MOCK_METHOD(std::optional<std::string>, getClockType, (), (const, override));
    MOCK_METHOD(std::optional<float>, getTime, (), (const, override));
    MOCK_METHOD(std::optional<std::string>, getRaceResults, (size_t raceId), (const, override));
    MOCK_METHOD(void, start, (const RestApi::NewRace&), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(void, pause, (), (override));
    MOCK_METHOD(void, resume, (), (override));
};
// ---------------------------------------------------------------------------------------------------------------------
class MockKartModelExchange : public RestApi::KartModelExchange
{
public:
    MOCK_METHOD(std::function<std::filesystem::path(const std::string&, const std::filesystem::path&)>, getUnzipFunction, (), (override));
    MOCK_METHOD(std::vector<RestApi::KartModelWrapper>, getAvailableKarts, (), (const, override));
    MOCK_METHOD(std::filesystem::path, getDirectory, (), (const, override));
    MOCK_METHOD(std::string, loadNewKart, (const std::filesystem::path&), (override));
    MOCK_METHOD(void, remove, (const std::string&), (override));
};
// ---------------------------------------------------------------------------------------------------------------------
class MockMusicExchange : public RestApi::MusicExchange
{
public:
    MOCK_METHOD(std::function<std::filesystem::path(const std::string&, const std::filesystem::path&)>, getUnzipFunction, (), (override));
    MOCK_METHOD(std::vector<RestApi::MusicWrapper>, getAllMusic, (), (const, override));
    MOCK_METHOD(std::filesystem::path, getDirectory, (), (const, override));
    MOCK_METHOD(std::string, loadNewMusic, (const std::filesystem::path&), (override));
    MOCK_METHOD(void, remove, (const std::string&), (override));
};
// ---------------------------------------------------------------------------------------------------------------------
class MockSfxExchange : public RestApi::SfxExchange
{
public:
    MOCK_METHOD(std::function<std::filesystem::path(const std::string&, const std::filesystem::path&)>, getUnzipFunction, (), (override));
    MOCK_METHOD(std::vector<RestApi::SfxWrapper>, getSounds, (), (const, override));
    MOCK_METHOD(std::filesystem::path, getDirectory, (), (const, override));
    MOCK_METHOD(std::string, loadNewSfx, (const std::filesystem::path&), (override));
    MOCK_METHOD(void, remove, (const std::string&), (override));
};
// ---------------------------------------------------------------------------------------------------------------------
class MockTrackModelExchange : public RestApi::TrackModelExchange
{
public:
    MOCK_METHOD(std::function<std::filesystem::path(const std::string&, const std::filesystem::path&)>, getUnzipFunction, (), (override));
    MOCK_METHOD(std::vector<RestApi::TrackModelWrapper>, getAvailableTracks, (), (const, override));
    MOCK_METHOD(std::filesystem::path, getDirectory, (), (const, override));
    MOCK_METHOD(std::string, loadNewTrack, (const std::filesystem::path&), (override));
    MOCK_METHOD(void, remove, (const std::string&), (override));
};
// ---------------------------------------------------------------------------------------------------------------------
// Race resources
// ---------------------------------------------------------------------------------------------------------------------
class MockBonusItemExchange : public RestApi::BonusItemWrapper
{
public:
    MOCK_METHOD(uint64_t, getId, (), (const, override));
    MOCK_METHOD(RestApi::Position, getPosition, (), (const, override));
    MOCK_METHOD(std::string, getType, (), (const, override));
    MOCK_METHOD(std::optional<std::string>, getOriginalType, (), (const, override));
    MOCK_METHOD(int, getTicksUntilReturn, (), (const, override));
    MOCK_METHOD(std::optional<int>, getUsedUpCounter, (), (const, override));
    MOCK_METHOD(void, setType, (const std::string& type), (override));
    MOCK_METHOD(void, setOriginalType, (const std::optional<std::string>& type), (override));
    MOCK_METHOD(void, setTicksUntilReturn, (int ticks), (override));
    MOCK_METHOD(void, setUsedUpCounter, (std::optional<int> usedUpCounter), (override));
};
class MockRaceBonusItemExchange : public RestApi::RaceBonusItemExchange
{
public:
    MOCK_METHOD(std::vector<std::unique_ptr<RestApi::BonusItemWrapper>>, getItems, (), (const, override));
    MOCK_METHOD(std::unique_ptr<RestApi::BonusItemWrapper>, add, (const RestApi::NewBonusItem&), (override));
    MOCK_METHOD(void, remove, (size_t), (override));
};
// ---------------------------------------------------------------------------------------------------------------------
class MockRaceChecklineExchange : public RestApi::RaceChecklineExchange
{
public:
    MOCK_METHOD(std::vector<RestApi::ChecklineWrapper>, getChecklines, (), (const, override));
};
// ---------------------------------------------------------------------------------------------------------------------
class MockKartWrapper : public RestApi::KartWrapper
{
public:
    // Id
    MOCK_METHOD(uint64_t, getId, (), (const, override));

    // Rank
    MOCK_METHOD(int, getRank, (), (const, override));

    // Controller
    MOCK_METHOD(std::string, getController, (), (const, override));

    // Characteristics
    MOCK_METHOD(std::string, getIdent, (), (const, override));
    MOCK_METHOD(uint32_t, getColor, (), (const, override));
    MOCK_METHOD(std::string, getType, (), (const, override));
    MOCK_METHOD(std::vector<std::string>, getGroups, (), (const, override));
    MOCK_METHOD(std::optional<std::string>, getEngineSfxType, (), (const, override));
    MOCK_METHOD(std::optional<std::string>, getSkidSound, (), (const, override));
    MOCK_METHOD(float, getFriction, (), (const, override));
    MOCK_METHOD(float, getFrictionSlip, (), (const, override));
    MOCK_METHOD(std::string, getTerrainImpulseType, (), (const, override));
    MOCK_METHOD(void, setKart, (const std::string& kart), (override));

    // Speed
    MOCK_METHOD(float, getSpeed, (), (const, override));
    MOCK_METHOD(float, getMaxSpeed, (), (const, override));
    MOCK_METHOD(std::optional<float>, getMinBoostSpeed, (), (const, override));
    MOCK_METHOD(RestApi::Vector, getVelocity, (), (const, override));
    MOCK_METHOD(std::vector<RestApi::SpeedIncrease>, getSpeedIncrease, (), (const, override));
    MOCK_METHOD(std::vector<RestApi::SpeedDecrease>, getSpeedDecrease, (), (const, override));

    // Position
    MOCK_METHOD(RestApi::Position, getPosition, (), (const, override));
    MOCK_METHOD(RestApi::Position, getFrontPosition, (), (const, override));
    MOCK_METHOD(bool, isJumping, (), (const, override));
    MOCK_METHOD(bool, isFlying, (), (const, override));
    MOCK_METHOD(bool, isNearGround, (), (const, override));
    MOCK_METHOD(bool, isOnGround, (), (const, override));
    MOCK_METHOD(float, getPitch, (), (const, override));
    MOCK_METHOD(float, getRoll, (), (const, override));
    MOCK_METHOD(float, getLean, (), (const, override));
    MOCK_METHOD(float, getLeanMax, (), (const, override));

    // Status
    MOCK_METHOD(std::string, getHandicapLevel, (), (const, override));
    MOCK_METHOD(bool, isBoostedAI, (), (const, override));
    MOCK_METHOD(bool, isBlockedByPlunger, (), (const, override));
    MOCK_METHOD(bool, isShielded, (), (const, override));
    MOCK_METHOD(bool, isSquashed, (), (const, override));
    MOCK_METHOD(bool, isEliminated, (), (const, override));
    MOCK_METHOD(bool, isGhostKart, (), (const, override));
    MOCK_METHOD(bool, isInRescue, (), (const, override));

    // Skidding
    MOCK_METHOD(std::optional<std::string>, getSkidding, (), (const, override));
    MOCK_METHOD(bool, isSkiddingBonusReady, (), (const, override));
    MOCK_METHOD(float, getSkiddingFactor, (), (const, override));
    MOCK_METHOD(float, getMaxSkidding, (), (const, override));

    // Control
    MOCK_METHOD(float, getSteer, (), (const, override));
    MOCK_METHOD(float, getMaxSteerAngle, (), (const, override));
    MOCK_METHOD(float, getAcceleration, (), (const, override));
    MOCK_METHOD(bool, isBraking, (), (const, override));
    MOCK_METHOD(bool, doesFire, (), (const, override));
    MOCK_METHOD(bool, doesLookBack, (), (const, override));
    MOCK_METHOD(std::string, getSkidControl, (), (const, override));

    // Collision
    MOCK_METHOD(float, getCollisionImpulse, (), (const, override));
    MOCK_METHOD(float, getCollisionImpulseTime, (), (const, override));
    MOCK_METHOD(float, getRestitution, (), (const, override));

    // Nitro
    MOCK_METHOD(float, getCollectedEnergy, (), (const, override));
    MOCK_METHOD(float, getMaxNitro, (), (const, override));
    MOCK_METHOD(int8_t, getMinNitroConsumption, (), (const, override));
    MOCK_METHOD(float, getConsumptionPerTick, (), (const, override));
    MOCK_METHOD(bool, hasNitroActivated, (), (const, override));

    // Attachment
    MOCK_METHOD(std::optional<RestApi::Attachment>, getAttachment, (), (const, override));
    MOCK_METHOD(void, setAttachment, (const std::optional<RestApi::Attachment>&), (override));

    // Power-Up
    MOCK_METHOD(std::optional<RestApi::PowerUp>, getPowerUp, (), (const, override));
    MOCK_METHOD(void, setPowerUp, (const std::optional<RestApi::PowerUp>&), (override));

    // Icon
    MOCK_METHOD(std::optional<std::string>, getIcon, (), (const, override));

    // Minimap
    MOCK_METHOD(std::string, getMinimapIconPath, (), (const, override));

    // Shadow
    MOCK_METHOD(std::optional<std::string>, getShadowMaterial, (), (const, override));

    // Ground
    MOCK_METHOD(std::optional<std::string>, getGround, (), (const, override));

    // Result
    MOCK_METHOD(std::optional<float>, getFinishTime, (), (const, override));
};
class MockRaceKartExchange : public RestApi::RaceKartExchange
{
public:
    MOCK_METHOD(std::vector<std::unique_ptr<RestApi::KartWrapper>>, getKarts, (), (const, override));
};
// ---------------------------------------------------------------------------------------------------------------------
class MockParticleWrapper : public RestApi::ParticleWrapper
{
public:
    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(RestApi::Interval<float>, getSize, (), (const, override));
    MOCK_METHOD(RestApi::Interval<float>, getRate, (), (const, override));
    MOCK_METHOD(RestApi::Interval<float>, getLifetime, (), (const, override));
    MOCK_METHOD(int, getFadeoutTime, (), (const, override));
    MOCK_METHOD(std::string, getShape, (), (const, override));
    MOCK_METHOD(std::optional<std::string>, getMaterial, (), (const, override));
    MOCK_METHOD(RestApi::Interval<uint32_t>, getColor, (), (const, override));
    MOCK_METHOD(FloatArray3, getBoxSize, (), (const, override));
    MOCK_METHOD(float, getSphereRadius, (), (const, override));
    MOCK_METHOD(int, getAngleSpread, (), (const, override));
    MOCK_METHOD(FloatArray3, getVelocity, (), (const, override));
    MOCK_METHOD(int, getEmissionDecayRate, (), (const, override));
    MOCK_METHOD(FloatArray2, getScaleAffactor, (), (const, override));
    MOCK_METHOD(bool, hasFlips, (), (const, override));
    MOCK_METHOD(bool, isVertical, (), (const, override));
    MOCK_METHOD(bool, hasRandomInitialY, (), (const, override));
};
class MockMaterialWrapper : public RestApi::MaterialWrapper
{
public:
    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(std::string, getPath, (), (const, override));
    MOCK_METHOD(std::optional<char>, getMirrorAxis, (), (const, override));
    MOCK_METHOD(bool, isBelowSurface, (), (const, override));
    MOCK_METHOD(bool, hasFallingEffect, (), (const, override));
    MOCK_METHOD(bool, isDriveReset, (), (const, override));
    MOCK_METHOD(bool, isSurface, (), (const, override));
    MOCK_METHOD(bool, isJumpTexture, (), (const, override));
    MOCK_METHOD(bool, hasGravity, (), (const, override));
    MOCK_METHOD(bool, isIgnore, (), (const, override));
    MOCK_METHOD(bool, hasHighTireAdhesion, (), (const, override));
    MOCK_METHOD(bool, hasTextureCompression, (), (const, override));
    MOCK_METHOD(std::string, getCollisionReaction, (), (const, override));
    MOCK_METHOD(std::optional<std::string>, getCollisionParticles, (), (const, override));
    MOCK_METHOD(std::optional<std::reference_wrapper<const RestApi::ParticleWrapper>>, getParticlesOnDrive, (), (const, override));
    MOCK_METHOD(std::optional<std::reference_wrapper<const RestApi::ParticleWrapper>>, getParticlesOnSkid, (), (const, override));
    MOCK_METHOD(bool, hasUClamp, (), (const, override));
    MOCK_METHOD(bool, hasVClamp, (), (const, override));
    MOCK_METHOD(std::vector<float>, getRandomHueSettings, (), (const, override));
    MOCK_METHOD(int, getSlowDownTicks, (), (const, override));
    MOCK_METHOD(float, getSlowDownMaxSpeedFraction, (), (const, override));
    MOCK_METHOD(bool, isZipper, (), (const, override));
    MOCK_METHOD(float, getZipperMinSpeed, (), (const, override));
    MOCK_METHOD(float, getZipperMaxSpeedIncrease, (), (const, override));
    MOCK_METHOD(float, getZipperDuration, (), (const, override));
    MOCK_METHOD(float, getZipperSpeedGain, (), (const, override));
    MOCK_METHOD(float, getZipperFadeOutTime, (), (const, override));
    MOCK_METHOD(float, getZipperEngineForce, (), (const, override));
    MOCK_METHOD(std::optional<std::string>, getSfxName, (), (const, override));
    MOCK_METHOD(float, getSfxMinSpeed, (), (const, override));
    MOCK_METHOD(float, getSfxMaxSpeed, (), (const, override));
    MOCK_METHOD(float, getSfxMinPitch, (), (const, override));
    MOCK_METHOD(float, getSfxMaxPitch, (), (const, override));
    MOCK_METHOD(float, getSfxPitchPerSpeed, (), (const, override));
    MOCK_METHOD(std::optional<std::string>, getAlphaMask, (), (const, override));
    MOCK_METHOD(bool, isColorizable, (), (const, override));
    MOCK_METHOD(float, getColorizationFactor, (), (const, override));
    MOCK_METHOD(std::string, getColorizationMask, (), (const, override));
    MOCK_METHOD(std::string, getShaderName, (), (const, override));
    MOCK_METHOD(std::optional<std::string>, getUVTwoTexture, (), (const, override));
    MOCK_METHOD(StringArray6, getSamplerPath, (), (const, override));
};
class MockRaceMaterialExchange : public RestApi::RaceMaterialExchange
{
public:
    MOCK_METHOD(std::vector<std::unique_ptr<RestApi::MaterialWrapper>>, getMaterials, (), (const, override));
};
// ---------------------------------------------------------------------------------------------------------------------
class MockRaceMusicExchange : public RestApi::RaceMusicExchange
{
public:
    MOCK_METHOD(bool, isEnabled, (), (const, override));
    MOCK_METHOD(float, getMasterMusicGain, (), (const, override));
    MOCK_METHOD(std::optional<RestApi::MusicWrapper>, getMusic , (), (const, override));
    MOCK_METHOD(void, setEnabled, (bool enabled), (override));
    MOCK_METHOD(void, setMasterMusicGain, (float), (override));
    MOCK_METHOD(void, setMusic, (const std::optional<std::string>& id), (override));
};
// ---------------------------------------------------------------------------------------------------------------------
class MockLightWrapper : public RestApi::LightWrapper
{
public:
    MOCK_METHOD(uint32_t, getColor, (), (const, override));
    MOCK_METHOD(float, getRadius, (), (const, override));
    MOCK_METHOD(float, getEnergy, (), (const, override));
    MOCK_METHOD(void, setColor, (uint32_t color), (override));
    MOCK_METHOD(void, setRadius, (float distance), (override));
    MOCK_METHOD(void, setEnergy, (float energy), (override));
};
class MockObjectWrapper : public RestApi::ObjectWrapper
{
public:
    MOCK_METHOD(uint64_t, getId, (), (const, override));
    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(std::string, getType, (), (const, override));
    MOCK_METHOD(bool, isEnabled, (), (const, override));
    MOCK_METHOD(bool, isDrivable, (), (const, override));
    MOCK_METHOD(bool, isAnimated, (), (const, override));
    MOCK_METHOD(RestApi::Position, getPosition, (), (const, override));
    MOCK_METHOD(RestApi::Position, getCenterPosition, (), (const, override));
    MOCK_METHOD(RestApi::Vector, getRotation, (), (const, override));
    MOCK_METHOD(RestApi::Vector, getScale, (), (const, override));
    MOCK_METHOD(std::string, getLodGroup, (), (const, override));
    MOCK_METHOD(std::string, getInteraction, (), (const, override));
    MOCK_METHOD(std::vector<std::unique_ptr<RestApi::ObjectWrapper>>, getChildren, (), (const, override));
    MOCK_METHOD(std::vector<std::unique_ptr<RestApi::ObjectWrapper>>, getMovableChildren, (), (const, override));
    MOCK_METHOD(std::unique_ptr<RestApi::LightWrapper>, getLight, (), (const, override));
    MOCK_METHOD(std::unique_ptr<RestApi::ParticleWrapper>, getParticles, (), (const, override));
    MOCK_METHOD(void, setEnabled, (bool), (override));
    MOCK_METHOD(void, setPosition, (const RestApi::Position& position), (override));
};
class MockRaceObjectExchange : public RestApi::RaceObjectExchange
{
public:
    MOCK_METHOD(std::vector<std::unique_ptr<RestApi::ObjectWrapper>>, getObjects, (), (const, override));
    MOCK_METHOD(void, remove, (size_t), (override));
};
// ---------------------------------------------------------------------------------------------------------------------
class MockRaceQuadExchange : public RestApi::RaceQuadExchange
{
public:
    MOCK_METHOD(std::vector<RestApi::QuadWrapper>, getQuads, (), (const, override));
};
// ---------------------------------------------------------------------------------------------------------------------
class MockSfxSoundWrapper : public RestApi::SfxSoundWrapper
{
public:
    MOCK_METHOD(std::string, getSound, (), (const, override));
    MOCK_METHOD(std::string, getStatus, (), (const, override));
    MOCK_METHOD(bool, inLoop, (), (const, override));
    MOCK_METHOD(float, getVolume, (), (const, override));
    MOCK_METHOD(float, getPitch, (), (const, override));
    MOCK_METHOD(float, getPlayTime, (), (const, override));
    MOCK_METHOD(std::optional<RestApi::Position>, getPosition, (), (const, override));
    MOCK_METHOD(void, setStatus, (const std::string& status), (override));
    MOCK_METHOD(void, setLoop, (bool inLoop), (override));
    MOCK_METHOD(void, setVolume, (float volume), (override));
    MOCK_METHOD(void, setPitch, (float pitch), (override));
    MOCK_METHOD(void, setPosition, (const RestApi::Position& position), (override));
};
class MockRaceSfxExchange : public RestApi::RaceSfxExchange
{
public:
    MOCK_METHOD(RestApi::Position, getListenerPosition, (), (const, override));
    MOCK_METHOD(RestApi::Vector, getListenerDirection, (), (const, override));
    MOCK_METHOD(RestApi::Vector, getListenerUpDirection, (), (const, override));
    MOCK_METHOD(float, getMasterVolume, (), (const, override));
    MOCK_METHOD(void, setMasterVolume, (float), (override));
    MOCK_METHOD(bool, isSfxAllowed, (), (const, override));
    MOCK_METHOD(void, setSfxAllowed, (bool), (override));
    MOCK_METHOD(std::vector<std::unique_ptr<RestApi::SfxSoundWrapper>>, getAllSounds, (), (const, override));
    MOCK_METHOD(std::unique_ptr<RestApi::SfxSoundWrapper>, add, (const std::string&), (override));
    MOCK_METHOD(void, remove, (size_t), (override));
};
// ---------------------------------------------------------------------------------------------------------------------
class MockRaceWeatherExchange : public RestApi::RaceWeatherExchange
{
public:
    MOCK_METHOD(bool, getLightning, (), (const, override));
    MOCK_METHOD(std::optional<std::string>, getSound, (), (const, override));
    MOCK_METHOD(uint32_t, getSkyColor, (), (const, override));
    MOCK_METHOD(std::optional<std::reference_wrapper<const RestApi::ParticleWrapper>>, getParticles, (), (const, override));
};
// ---------------------------------------------------------------------------------------------------------------------
