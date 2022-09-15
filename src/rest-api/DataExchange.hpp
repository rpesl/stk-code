#pragma once
#include <array>
#include <filesystem>
#include <memory>
#include <optional>
#include <utility>
#include <vector>
// ---------------------------------------------------------------------------------------------------------------------
class AbstractKart;
class ItemState;
class KartPropertiesManager;
class Material;
class ParticleKind;
class RaceManager;
class SFXBase;
class SFXBuffer;
class TrackManager;
class TrackObject;
class TrackObjectPresentationParticles;
class TrackObjectPresentationLight;
// ---------------------------------------------------------------------------------------------------------------------
namespace RestApi
{
// ---------------------------------------------------------------------------------------------------------------------
template<typename T>
using Interval = std::pair<T, T>;
using Position = std::array<float, 3>;
using Vector = std::array<float, 3>;
// ---------------------------------------------------------------------------------------------------------------------
class DataExchange
{
public:
    virtual ~DataExchange() noexcept = default;
    virtual std::function<std::filesystem::path(const std::string&, const std::filesystem::path&)> getUnzipFunction();
};
// ---------------------------------------------------------------------------------------------------------------------
// Game resources
// ---------------------------------------------------------------------------------------------------------------------
struct NewRace
{
    int numberOfLaps;
    std::string track;
    std::string kart;
    std::vector<std::string> aiKarts;
    std::string difficulty;
    bool reverse;
};
class RaceExchange : public DataExchange
{
public:
    [[nodiscard]] static std::unique_ptr<RaceExchange> create(RaceManager& raceManager);

public:
    ~RaceExchange() noexcept override = default;
    [[nodiscard]] virtual std::optional<uint64_t> getId() const = 0;
    [[nodiscard]] virtual std::string getStatus() const = 0;
    [[nodiscard]] virtual bool isActive() const = 0;
    [[nodiscard]] virtual std::optional<std::string> getTrackName() const = 0;
    [[nodiscard]] virtual std::optional<std::string> getMajorRaceMode() const = 0;
    [[nodiscard]] virtual std::optional<std::string> getMinorRaceMode() const = 0;
    [[nodiscard]] virtual std::optional<std::string> getDifficulty() const = 0;
    [[nodiscard]] virtual std::optional<std::string> getClockType() const = 0;
    [[nodiscard]] virtual std::optional<float> getTime() const = 0;
    [[nodiscard]] virtual std::optional<std::string> getRaceResults(size_t raceId) const = 0;
    virtual void start(const NewRace& newRace) = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
};
// ---------------------------------------------------------------------------------------------------------------------
struct KartModelWrapper
{
    std::string id;
    std::string name;
    float mass;
    float engineMaxSpeed;
    float accelerationEfficiency;
    float nitroConsumption;
};
class KartModelExchange : public DataExchange
{
public:
    [[nodiscard]] static std::unique_ptr<KartModelExchange> create(RaceManager& raceManager);

public:
    [[nodiscard]] virtual std::vector<KartModelWrapper> getAvailableKarts() const = 0;
    [[nodiscard]] virtual std::filesystem::path getDirectory() const = 0;
    [[nodiscard]] virtual std::string loadNewKart(const std::filesystem::path& path) = 0;
    virtual void remove(const std::string& id) = 0;
};
// ---------------------------------------------------------------------------------------------------------------------
struct MusicWrapper
{
    std::string id;
    std::string title;
    std::string composer;
    std::string fileName;
    std::optional<std::string> fastFileName;
};
class MusicExchange : public DataExchange
{
public:
    static std::unique_ptr<MusicExchange> create();

public:
    ~MusicExchange() noexcept override = default;
    [[nodiscard]] virtual std::vector<MusicWrapper> getAllMusic() const = 0;
    [[nodiscard]] virtual std::filesystem::path getDirectory() const = 0;
    [[nodiscard]] virtual std::string loadNewMusic(const std::filesystem::path& path) = 0;
    virtual void remove(const std::string& id) = 0;
};
// ---------------------------------------------------------------------------------------------------------------------
struct SfxWrapper
{
    std::string id;
    std::string file;
    bool loaded;
    bool positional;
    float rollOff;
    float volume;
    float maxDistance;
    std::optional<float> duration;
};
class SfxExchange : public DataExchange
{
public:
    [[nodiscard]] static std::unique_ptr<SfxExchange> create();

public:
    ~SfxExchange() noexcept override = default;
    [[nodiscard]] virtual std::vector<SfxWrapper> getSounds() const = 0;
    [[nodiscard]] virtual std::filesystem::path getDirectory() const = 0;
    [[nodiscard]] virtual std::string loadNewSfx(const std::filesystem::path& path) = 0;
    virtual void remove(const std::string& id) = 0;
};
// ---------------------------------------------------------------------------------------------------------------------
struct TrackModelWrapper
{
    std::string id;
    std::string name;
    std::vector<std::string> groups;
    bool race;
    bool soccer;
    bool arena;
};
class TrackModelExchange : public DataExchange
{
public:
    [[nodiscard]] static std::unique_ptr<TrackModelExchange> create();

public:
    [[nodiscard]] virtual std::vector<TrackModelWrapper> getAvailableTracks() const = 0;
    [[nodiscard]] virtual std::filesystem::path getDirectory() const = 0;
    [[nodiscard]] virtual std::string loadNewTrack(const std::filesystem::path& path) = 0;
    virtual void remove(const std::string& id) = 0;
};
// ---------------------------------------------------------------------------------------------------------------------
// Race resources
// ---------------------------------------------------------------------------------------------------------------------
class BonusItemWrapper
{
public:
    static std::unique_ptr<BonusItemWrapper> create(ItemState& state);

public:
    virtual ~BonusItemWrapper() noexcept = default;
    [[nodiscard]] virtual uint64_t getId() const = 0;
    [[nodiscard]] virtual Position getPosition() const = 0;
    [[nodiscard]] virtual std::string getType() const = 0;
    [[nodiscard]] virtual std::optional<std::string> getOriginalType() const = 0;
    [[nodiscard]] virtual int getTicksUntilReturn() const = 0;
    [[nodiscard]] virtual std::optional<int> getUsedUpCounter() const = 0;
    virtual void setType(const std::string& type) = 0;
    virtual void setOriginalType(const std::optional<std::string>& type) = 0;
    virtual void setTicksUntilReturn(int ticks) = 0;
    virtual void setUsedUpCounter(std::optional<int> usedUpCounter) = 0;
};
struct NewBonusItem
{
    Position position;
    std::string type;
    std::optional<std::string> originalType;
    int ticksUntilReturn;
    std::optional<int> usedUpCounter;
};
class RaceBonusItemExchange : public DataExchange
{
public:
    static std::unique_ptr<RaceBonusItemExchange> create();

public:
    ~RaceBonusItemExchange() noexcept override = default;
    [[nodiscard]] virtual std::vector<std::unique_ptr<BonusItemWrapper>> getItems() const = 0;
    [[nodiscard]] virtual std::unique_ptr<BonusItemWrapper> add(const NewBonusItem& item) = 0;
    virtual void remove(size_t id) = 0;
};
// ---------------------------------------------------------------------------------------------------------------------
struct ChecklineWrapper
{
    int id;
    std::string kind;
    bool activeAtReset;
    std::optional<std::pair<Position, Position>> position;
    std::optional<bool> ignoreHeight;
    std::vector<int> sameGroup;
    std::vector<int> otherIds;
    std::vector<bool> active;
};
class RaceChecklineExchange : public DataExchange
{
public:
    static std::unique_ptr<RaceChecklineExchange> create();

public:
    ~RaceChecklineExchange() noexcept override = default;
    [[nodiscard]] virtual std::vector<ChecklineWrapper> getChecklines() const = 0;
};
// ---------------------------------------------------------------------------------------------------------------------
struct SpeedIncrease
{
    std::string name;
    bool active;
    std::optional<int> timeLeft;
    float speedIncrease;
    float engineForceIncrease;
};
struct SpeedDecrease
{
    std::string name;
    bool active;
    std::optional<int> timeLeft;
    float fraction;
};
struct Attachment
{
    std::string type;
    int ticks;
};
struct PowerUp
{
    std::string name;
    int count;
};
class MaterialWrapper;
class KartWrapper
{
public:
    static std::unique_ptr<KartWrapper> create(AbstractKart& kart);

public:
    virtual ~KartWrapper() noexcept = default;

    // Id
    [[nodiscard]] virtual uint64_t getId() const = 0;

    // Rank
    [[nodiscard]] virtual int getRank() const = 0;

    // Controller
    [[nodiscard]] virtual std::string getController() const = 0;

    // Characteristics
    [[nodiscard]] virtual std::string getIdent() const = 0;
    [[nodiscard]] virtual uint32_t getColor() const = 0;
    [[nodiscard]] virtual std::string getType() const = 0;
    [[nodiscard]] virtual std::vector<std::string> getGroups() const = 0;
    [[nodiscard]] virtual std::optional<std::string> getEngineSfxType() const = 0;
    [[nodiscard]] virtual std::optional<std::string> getSkidSound() const = 0;
    [[nodiscard]] virtual float getFriction() const = 0;
    [[nodiscard]] virtual float getFrictionSlip() const = 0;
    [[nodiscard]] virtual std::string getTerrainImpulseType() const = 0;
    virtual void setKart(const std::string& kart) = 0;

    // Speed
    [[nodiscard]] virtual float getSpeed() const = 0;
    [[nodiscard]] virtual float getMaxSpeed() const = 0;
    [[nodiscard]] virtual std::optional<float> getMinBoostSpeed() const = 0;
    [[nodiscard]] virtual Vector getVelocity() const = 0;
    [[nodiscard]] virtual std::vector<SpeedIncrease> getSpeedIncrease() const = 0;
    [[nodiscard]] virtual std::vector<SpeedDecrease> getSpeedDecrease() const = 0;

    // Position
    [[nodiscard]] virtual Position getPosition() const = 0;
    [[nodiscard]] virtual Position getFrontPosition() const = 0;
    [[nodiscard]] virtual bool isJumping() const = 0;
    [[nodiscard]] virtual bool isFlying() const = 0;
    [[nodiscard]] virtual bool isNearGround() const = 0;
    [[nodiscard]] virtual bool isOnGround() const = 0;
    [[nodiscard]] virtual float getPitch() const = 0;
    [[nodiscard]] virtual float getRoll() const = 0;
    [[nodiscard]] virtual float getLean() const = 0;
    [[nodiscard]] virtual float getLeanMax() const = 0;

    // Status
    [[nodiscard]] virtual std::string getHandicapLevel() const = 0;
    [[nodiscard]] virtual bool isBoostedAI() const = 0;
    [[nodiscard]] virtual bool isBlockedByPlunger() const = 0;
    [[nodiscard]] virtual bool isShielded() const = 0;
    [[nodiscard]] virtual bool isSquashed() const = 0;
    [[nodiscard]] virtual bool isEliminated() const = 0;
    [[nodiscard]] virtual bool isGhostKart() const = 0;
    [[nodiscard]] virtual bool isInRescue() const = 0;

    // Skidding
    [[nodiscard]] virtual std::optional<std::string> getSkidding() const = 0;
    [[nodiscard]] virtual bool isSkiddingBonusReady() const = 0;
    [[nodiscard]] virtual float getSkiddingFactor() const = 0;
    [[nodiscard]] virtual float getMaxSkidding() const = 0;

    // Control
    [[nodiscard]] virtual float getSteer() const = 0;
    [[nodiscard]] virtual float getMaxSteerAngle() const = 0;
    [[nodiscard]] virtual float getAcceleration() const = 0;
    [[nodiscard]] virtual bool isBraking() const = 0;
    [[nodiscard]] virtual bool doesFire() const = 0;
    [[nodiscard]] virtual bool doesLookBack() const = 0;
    [[nodiscard]] virtual std::string getSkidControl() const = 0;

    // Collision
    [[nodiscard]] virtual float getCollisionImpulse() const = 0;
    [[nodiscard]] virtual float getCollisionImpulseTime() const = 0;
    [[nodiscard]] virtual float getRestitution() const = 0;

    // Nitro
    [[nodiscard]] virtual float getCollectedEnergy() const = 0;
    [[nodiscard]] virtual float getMaxNitro() const = 0;
    [[nodiscard]] virtual int8_t getMinNitroConsumption() const = 0;
    [[nodiscard]] virtual float getConsumptionPerTick() const = 0;
    [[nodiscard]] virtual bool hasNitroActivated() const = 0;

    // Attachment
    [[nodiscard]] virtual std::optional<Attachment> getAttachment() const = 0;
    virtual void setAttachment(const std::optional<Attachment>& powerUp) = 0;

    // Power-Up
    [[nodiscard]] virtual std::optional<PowerUp> getPowerUp() const = 0;
    virtual void setPowerUp(const std::optional<PowerUp>& powerUp) = 0;

    // Icon
    [[nodiscard]] virtual std::optional<std::string> getIcon() const = 0;

    // Minimap
    [[nodiscard]] virtual std::string getMinimapIconPath() const = 0;

    // Shadow
    [[nodiscard]] virtual std::optional<std::string> getShadowMaterial() const = 0;

    // Ground
    [[nodiscard]] virtual std::optional<std::string> getGround() const = 0;

    // Result
    [[nodiscard]] virtual std::optional<float> getFinishTime() const = 0;
};
class RaceKartExchange : public DataExchange
{
public:
    static std::unique_ptr<RaceKartExchange> create();

public:
    ~RaceKartExchange() noexcept override = default;
    [[nodiscard]]
    virtual std::vector<std::unique_ptr<KartWrapper>> getKarts() const = 0;
};
// ---------------------------------------------------------------------------------------------------------------------
class ParticleWrapper
{
public:
    static std::unique_ptr<ParticleWrapper> create(const ParticleKind* particleKind);

public:
    virtual ~ParticleWrapper() noexcept = default;
    [[nodiscard]] virtual std::string getName() const = 0;
    [[nodiscard]] virtual Interval<float> getSize() const = 0;
    [[nodiscard]] virtual Interval<float> getRate() const = 0;
    [[nodiscard]] virtual Interval<float> getLifetime() const = 0;
    [[nodiscard]] virtual int getFadeoutTime() const = 0;
    [[nodiscard]] virtual std::string getShape() const = 0;
    [[nodiscard]] virtual std::optional<std::string> getMaterial() const = 0;
    [[nodiscard]] virtual Interval<uint32_t> getColor() const = 0;
    [[nodiscard]] virtual std::array<float, 3> getBoxSize() const = 0;
    [[nodiscard]] virtual float getSphereRadius() const = 0;
    [[nodiscard]] virtual int getAngleSpread() const = 0;
    [[nodiscard]] virtual std::array<float, 3> getVelocity() const = 0;
    [[nodiscard]] virtual int getEmissionDecayRate() const = 0;
    [[nodiscard]] virtual std::array<float, 2> getScaleAffactor() const = 0;
    [[nodiscard]] virtual bool hasFlips() const = 0;
    [[nodiscard]] virtual bool isVertical() const = 0;
    [[nodiscard]] virtual bool hasRandomInitialY() const = 0;
};
class MaterialWrapper
{
public:
    static std::unique_ptr<MaterialWrapper> create(const Material* material);

public:
    virtual ~MaterialWrapper() noexcept = default;
    [[nodiscard]] virtual std::string getName() const = 0;
    [[nodiscard]] virtual std::string getPath() const = 0;
    [[nodiscard]] virtual std::optional<char> getMirrorAxis() const = 0;
    [[nodiscard]] virtual bool isBelowSurface() const = 0;
    [[nodiscard]] virtual bool hasFallingEffect() const = 0;
    [[nodiscard]] virtual bool isDriveReset() const = 0;
    [[nodiscard]] virtual bool isSurface() const = 0;
    [[nodiscard]] virtual bool isJumpTexture() const = 0;
    [[nodiscard]] virtual bool hasGravity() const = 0;
    [[nodiscard]] virtual bool isIgnore() const = 0;
    [[nodiscard]] virtual bool hasHighTireAdhesion() const = 0;
    [[nodiscard]] virtual bool hasTextureCompression() const = 0;
    [[nodiscard]] virtual std::string getCollisionReaction() const = 0;
    [[nodiscard]] virtual std::optional<std::string> getCollisionParticles() const = 0;
    [[nodiscard]] virtual std::optional<std::reference_wrapper<const ParticleWrapper>> getParticlesOnDrive() const = 0;
    [[nodiscard]] virtual std::optional<std::reference_wrapper<const ParticleWrapper>> getParticlesOnSkid() const = 0;
    [[nodiscard]] virtual bool hasUClamp() const = 0;
    [[nodiscard]] virtual bool hasVClamp() const = 0;
    [[nodiscard]] virtual std::vector<float> getRandomHueSettings() const = 0;
    [[nodiscard]] virtual int getSlowDownTicks() const = 0;
    [[nodiscard]] virtual float getSlowDownMaxSpeedFraction() const = 0;
    [[nodiscard]] virtual bool isZipper() const = 0;
    [[nodiscard]] virtual float getZipperMinSpeed() const = 0;
    [[nodiscard]] virtual float getZipperMaxSpeedIncrease() const = 0;
    [[nodiscard]] virtual float getZipperDuration() const = 0;
    [[nodiscard]] virtual float getZipperSpeedGain() const = 0;
    [[nodiscard]] virtual float getZipperFadeOutTime() const = 0;
    [[nodiscard]] virtual float getZipperEngineForce() const = 0;
    [[nodiscard]] virtual std::optional<std::string> getSfxName() const = 0;
    [[nodiscard]] virtual float getSfxMinSpeed() const = 0;
    [[nodiscard]] virtual float getSfxMaxSpeed() const = 0;
    [[nodiscard]] virtual float getSfxMinPitch() const = 0;
    [[nodiscard]] virtual float getSfxMaxPitch() const = 0;
    [[nodiscard]] virtual float getSfxPitchPerSpeed() const = 0;
    [[nodiscard]] virtual std::optional<std::string> getAlphaMask() const = 0;
    [[nodiscard]] virtual bool isColorizable() const = 0;
    [[nodiscard]] virtual float getColorizationFactor() const = 0;
    [[nodiscard]] virtual std::string getColorizationMask() const = 0;
    [[nodiscard]] virtual std::string getShaderName() const = 0;
    [[nodiscard]] virtual std::optional<std::string> getUVTwoTexture() const = 0;
    [[nodiscard]] virtual std::array<std::string, 6> getSamplerPath() const = 0;
};
class RaceMaterialExchange : public DataExchange
{
public:
    static std::unique_ptr<RaceMaterialExchange> create();

public:
    ~RaceMaterialExchange() noexcept override = default;
    [[nodiscard]]
    virtual std::vector<std::unique_ptr<MaterialWrapper>> getMaterials() const = 0;
};
// ---------------------------------------------------------------------------------------------------------------------
class RaceMusicExchange : public DataExchange
{
public:
    static std::unique_ptr<RaceMusicExchange> create();

public:
    ~RaceMusicExchange() noexcept override = default;
    [[nodiscard]] virtual bool isEnabled() const = 0;
    [[nodiscard]] virtual float getMasterMusicGain() const = 0;
    [[nodiscard]] virtual std::optional<MusicWrapper> getMusic() const = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual void setMasterMusicGain(float volume) = 0;
    virtual void setMusic(const std::optional<std::string>& id) = 0;
};
// ---------------------------------------------------------------------------------------------------------------------
class LightWrapper
{
public:
    static std::unique_ptr<LightWrapper> create(TrackObjectPresentationLight& state);

public:
    virtual ~LightWrapper() noexcept = default;
    [[nodiscard]] virtual uint32_t getColor() const = 0;
    [[nodiscard]] virtual float getRadius() const = 0;
    [[nodiscard]] virtual float getEnergy() const = 0;
    virtual void setColor(uint32_t color) = 0;
    virtual void setRadius(float distance) = 0;
    virtual void setEnergy(float energy) = 0;
};
class ObjectWrapper
{
public:
    static std::unique_ptr<ObjectWrapper> create(TrackObject& object);

public:
    virtual ~ObjectWrapper() noexcept = default;
    [[nodiscard]] virtual uint64_t getId() const = 0;
    [[nodiscard]] virtual std::string getName() const = 0;
    [[nodiscard]] virtual std::string getType() const = 0;
    [[nodiscard]] virtual bool isEnabled() const = 0;
    [[nodiscard]] virtual bool isDrivable() const = 0;
    [[nodiscard]] virtual bool isAnimated() const = 0;
    [[nodiscard]] virtual Position getPosition() const = 0;
    [[nodiscard]] virtual Position getCenterPosition() const = 0;
    [[nodiscard]] virtual Vector getRotation() const = 0;
    [[nodiscard]] virtual Vector getScale() const = 0;
    [[nodiscard]] virtual std::string getLodGroup() const = 0;
    [[nodiscard]] virtual std::string getInteraction() const = 0;
    [[nodiscard]] virtual std::vector<std::unique_ptr<ObjectWrapper>> getChildren() const = 0;
    [[nodiscard]] virtual std::vector<std::unique_ptr<ObjectWrapper>> getMovableChildren() const = 0;
    [[nodiscard]] virtual std::unique_ptr<LightWrapper> getLight() const = 0;
    [[nodiscard]] virtual std::unique_ptr<ParticleWrapper> getParticles() const = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual void setPosition(const Position& position) = 0;
};
class RaceObjectExchange : public DataExchange
{
public:
    static std::unique_ptr<RaceObjectExchange> create();

public:
    ~RaceObjectExchange() noexcept override = default;
    [[nodiscard]] virtual std::vector<std::unique_ptr<ObjectWrapper>> getObjects() const = 0;
    virtual void remove(size_t id) = 0;
};
// ---------------------------------------------------------------------------------------------------------------------
struct QuadWrapper
{
    int id;
    bool ignore;
    bool invisible;
    bool aiIgnore;
    std::array<Position, 4> quad;
    Interval<float> heightTesting;
    std::vector<unsigned int> successors;
};
class RaceQuadExchange : public DataExchange
{
public:
    static std::unique_ptr<RaceQuadExchange> create();

public:
    ~RaceQuadExchange() noexcept override = default;
    [[nodiscard]] virtual std::vector<QuadWrapper> getQuads() const = 0;
};
// ---------------------------------------------------------------------------------------------------------------------
class SfxSoundWrapper
{
public:
    [[nodiscard]]
    static std::unique_ptr<SfxSoundWrapper> create(SFXBase& sfxBase);

public:
    virtual ~SfxSoundWrapper() noexcept = default;
    [[nodiscard]] virtual std::string getSound() const = 0;
    [[nodiscard]] virtual std::string getStatus() const = 0;
    [[nodiscard]] virtual bool inLoop() const = 0;
    [[nodiscard]] virtual float getVolume() const = 0;
    [[nodiscard]] virtual float getPitch() const = 0;
    [[nodiscard]] virtual float getPlayTime() const = 0;
    [[nodiscard]] virtual std::optional<Position> getPosition() const = 0;
    virtual void setStatus(const std::string& status) = 0;
    virtual void setLoop(bool inLoop) = 0;
    virtual void setVolume(float volume) = 0;
    virtual void setPitch(float pitch) = 0;
    virtual void setPosition(const Position& position) = 0;
};
class RaceSfxExchange : public DataExchange
{
public:
    [[nodiscard]]
    static std::unique_ptr<RaceSfxExchange> create();

public:
    ~RaceSfxExchange() noexcept override = default;
    [[nodiscard]] virtual Position getListenerPosition() const = 0;
    [[nodiscard]] virtual Vector getListenerDirection() const = 0;
    [[nodiscard]] virtual Vector getListenerUpDirection() const = 0;
    [[nodiscard]] virtual float getMasterVolume() const = 0;
    virtual void setMasterVolume(float volume) = 0;
    [[nodiscard]] virtual bool isSfxAllowed() const = 0;
    virtual void setSfxAllowed(bool sfxAllowed) = 0;
    [[nodiscard]] virtual std::vector<std::unique_ptr<SfxSoundWrapper>> getAllSounds() const = 0;
    [[nodiscard]] virtual std::unique_ptr<SfxSoundWrapper> add(const std::string& sfxId) = 0;
    virtual void remove(size_t id) = 0;
};
// ---------------------------------------------------------------------------------------------------------------------
class RaceWeatherExchange : public DataExchange
{
public:
    static std::unique_ptr<RaceWeatherExchange> create();

public:
    ~RaceWeatherExchange() noexcept override = default;
    [[nodiscard]] virtual bool getLightning() const = 0;
    [[nodiscard]] virtual std::optional<std::string> getSound() const = 0;
    [[nodiscard]] virtual uint32_t getSkyColor() const = 0;
    [[nodiscard]] virtual std::optional<std::reference_wrapper<const ParticleWrapper>> getParticles() const = 0;
};
// ---------------------------------------------------------------------------------------------------------------------
}
