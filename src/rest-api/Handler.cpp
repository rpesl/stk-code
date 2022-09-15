#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <functional>
#include <mutex>
#include <optional>
#include <utility>
#include "graphics/particle_kind.hpp"
#include "graphics/weather.hpp"
#include "modes/world.hpp"
#include "rest-api/Handler.hpp"
#include "rest-api/DataExchange.hpp"
#include <iostream>
// ---------------------------------------------------------------------------------------------------------------------
namespace RestApi
{
namespace
{
// ---------------------------------------------------------------------------------------------------------------------
rapidjson::Value vectorToJson(const std::array<float, 3>& position, rapidjson::Document::AllocatorType& alloc)
{
    rapidjson::Value result;
    result.SetArray();
    for (int i = 0; i < 3; i++)
    {
        result.PushBack(position[i], alloc);
    }
    return result;
}
// ---------------------------------------------------------------------------------------------------------------------
rapidjson::Value optionalStringToJson(const std::optional<std::string>& value, rapidjson::Document::AllocatorType& alloc)
{
    rapidjson::Value result;
    if (value)
    {
        result.SetString(value->c_str(), alloc);
    }
    return result;
}
// ---------------------------------------------------------------------------------------------------------------------
rapidjson::Value shaderToJson(const MaterialWrapper& material, rapidjson::Document::AllocatorType& alloc)
{
    rapidjson::Value shader;
    shader.SetObject();
    rapidjson::Value shaderName;
    shaderName.SetString(material.getShaderName().c_str(), alloc);
    shader.AddMember("name", shaderName, alloc);
    shader.AddMember("uv-two-texture", optionalStringToJson(material.getUVTwoTexture(), alloc), alloc);
    rapidjson::Value samplerPath;
    samplerPath.SetArray();
    for (const auto& path: material.getSamplerPath())
    {
        rapidjson::Value layer;
        layer.SetString(path.c_str(), alloc);
        samplerPath.PushBack(layer, alloc);
    }
    shader.AddMember("sampler-path", samplerPath, alloc);
    return shader;
}
// ---------------------------------------------------------------------------------------------------------------------
rapidjson::Value collisionToJson(const MaterialWrapper& material, rapidjson::Document::AllocatorType& alloc)
{
    rapidjson::Value collision;
    collision.SetObject();
    auto collisionReactionValue = material.getCollisionReaction();
    rapidjson::Value collisionReaction;
    collisionReaction.SetString(collisionReactionValue.c_str(), alloc);
    collision.AddMember("collision-reaction-value", collisionReaction, alloc);
    collision.AddMember("particles", optionalStringToJson(material.getCollisionParticles(), alloc), alloc);
    return collision;
}
// ---------------------------------------------------------------------------------------------------------------------
rapidjson::Value particleKindToJson(std::optional<std::reference_wrapper<const ParticleWrapper>> kind, rapidjson::Document::AllocatorType& alloc);
// ---------------------------------------------------------------------------------------------------------------------
rapidjson::Value materialParticlesToJson(const MaterialWrapper& material, rapidjson::Document::AllocatorType& alloc)
{
    rapidjson::Value particles;
    particles.SetObject();
    particles.AddMember("on-drive", particleKindToJson(material.getParticlesOnDrive(), alloc), alloc);
    particles.AddMember("on-skid", particleKindToJson(material.getParticlesOnSkid(), alloc), alloc);
    return particles;
}
// ---------------------------------------------------------------------------------------------------------------------
rapidjson::Value materialZipperToJson(const MaterialWrapper& material, rapidjson::Document::AllocatorType& alloc)
{
    rapidjson::Value zipper;
    if (material.isZipper())
    {
        zipper.SetObject();
        zipper.AddMember("min-speed", material.getZipperMinSpeed(), alloc);
        zipper.AddMember("max-speed-increase", material.getZipperMaxSpeedIncrease(), alloc);
        zipper.AddMember("duration", material.getZipperDuration(), alloc);
        zipper.AddMember("speed-gain", material.getZipperSpeedGain(), alloc);
        zipper.AddMember("fade-out-time", material.getZipperFadeOutTime(), alloc);
        zipper.AddMember("engine-force", material.getZipperEngineForce(), alloc);
    }
    return zipper;
}
// ---------------------------------------------------------------------------------------------------------------------
rapidjson::Value materialSfxToJson(const MaterialWrapper& material, rapidjson::Document::AllocatorType& alloc)
{
    rapidjson::Value sfx;
    if (auto nameValue = material.getSfxName())
    {
        sfx.SetObject();
        rapidjson::Value name;
        name.SetString(nameValue->c_str(), alloc);
        sfx.AddMember("name", name, alloc);
        rapidjson::Value speed;
        speed.SetObject();
        speed.AddMember("min", material.getSfxMinSpeed(), alloc);
        speed.AddMember("max", material.getSfxMaxSpeed(), alloc);
        sfx.AddMember("speed", speed, alloc);
        rapidjson::Value pitch;
        pitch.SetObject();
        pitch.AddMember("min", material.getSfxMinPitch(), alloc);
        pitch.AddMember("max", material.getSfxMaxPitch(), alloc);
        pitch.AddMember("per-speed", material.getSfxPitchPerSpeed(), alloc);
        sfx.AddMember("pitch", pitch, alloc);
    }
    return sfx;
}
// ---------------------------------------------------------------------------------------------------------------------
rapidjson::Value materialColorizationToJson(const MaterialWrapper& material, rapidjson::Document::AllocatorType& alloc)
{
    rapidjson::Value colorization;
    if (material.isColorizable())
    {
        colorization.SetObject();
        colorization.AddMember("factor", material.getColorizationFactor(), alloc);
        rapidjson::Value colorizationMask;
        colorizationMask.SetString(material.getColorizationMask().c_str(), alloc);
        colorization.AddMember("mask", colorizationMask, alloc);
    }
    return colorization;
}
// ---------------------------------------------------------------------------------------------------------------------
rapidjson::Value materialToJson(const MaterialWrapper& material, rapidjson::Document::AllocatorType& alloc)
{
    rapidjson::Value result;
    result.SetObject();
    rapidjson::Value texture;
    texture.SetObject();
    rapidjson::Value name;
    name.SetString(material.getName().c_str(), alloc);
    texture.AddMember("name", name, alloc);
    rapidjson::Value path;
    path.SetString(material.getPath().c_str(), alloc);
    texture.AddMember("path", path, alloc);
    result.AddMember("texture", texture, alloc);
    rapidjson::Value mirrorAxis;
    if (auto mirrorAxisValue = material.getMirrorAxis())
    {
        mirrorAxis.SetString(&mirrorAxisValue.value(), 1, alloc);
    }
    result.AddMember("mirror-axis-when-reverse", mirrorAxis, alloc);
    result.AddMember("below-surface", material.isBelowSurface(), alloc);
    result.AddMember("falling-effect", material.hasFallingEffect(), alloc);
    result.AddMember("surface", material.isSurface(), alloc);
    result.AddMember("drive-reset", material.isDriveReset(), alloc);
    result.AddMember("jump-texture", material.isJumpTexture(), alloc);
    result.AddMember("gravity", material.hasGravity(), alloc);
    result.AddMember("ignore", material.isIgnore(), alloc);
    result.AddMember("high-tire-adhesion", material.hasHighTireAdhesion(), alloc);
    result.AddMember("texture-compression", material.hasTextureCompression(), alloc);
    result.AddMember("collision", collisionToJson(material, alloc), alloc);
    result.AddMember("particles", materialParticlesToJson(material, alloc), alloc);
    rapidjson::Value clamp;
    clamp.SetObject();
    clamp.AddMember("u", material.hasUClamp(), alloc);
    clamp.AddMember("v", material.hasVClamp(), alloc);
    result.AddMember("clamp", clamp, alloc);
    rapidjson::Value hue;
    if (auto hueValue = material.getRandomHueSettings(); !hueValue.empty())
    {
        hue.SetArray();
        for (const auto& currentHue: hueValue)
        {
            hue.PushBack(currentHue, alloc);
        }
    }
    result.AddMember("hue", hue, alloc);
    rapidjson::Value slowdown;
    if (material.getSlowDownMaxSpeedFraction() != 1.0f)
    {
        slowdown.SetObject();
        slowdown.AddMember("ticks", material.getSlowDownTicks(), alloc);
        slowdown.AddMember("max-speed-fraction", material.getSlowDownMaxSpeedFraction(), alloc);
    }
    result.AddMember("slowdown", slowdown, alloc);
    result.AddMember("zipper", materialZipperToJson(material, alloc), alloc);
    result.AddMember("sfx", materialSfxToJson(material, alloc), alloc);
    result.AddMember("alpha-mask", optionalStringToJson(material.getAlphaMask(), alloc), alloc);
    result.AddMember("colorization", materialColorizationToJson(material, alloc), alloc);
    result.AddMember("shader", shaderToJson(material, alloc), alloc);
    return result;
}
// ---------------------------------------------------------------------------------------------------------------------
rapidjson::Value particleKindToJson(const ParticleWrapper& kind, rapidjson::Document::AllocatorType& alloc)
{
    rapidjson::Value result;
    result.SetObject();
    rapidjson::Value name;
    name.SetString(kind.getName().c_str(), alloc);
    result.AddMember("name", name, alloc);
    rapidjson::Value size;
    size.SetObject();
    auto[minSize, maxSize] = kind.getSize();
    size.AddMember("min", minSize, alloc);
    size.AddMember("max", maxSize, alloc);
    result.AddMember("size", size, alloc);
    rapidjson::Value rate;
    rate.SetObject();
    auto[minRate, maxRate] = kind.getRate();
    rate.AddMember("min", minRate, alloc);
    rate.AddMember("max", maxRate, alloc);
    result.AddMember("rate", rate, alloc);
    rapidjson::Value lifetime;
    lifetime.SetObject();
    auto[minLifetime, maxLifetime] = kind.getLifetime();
    lifetime.AddMember("min", minLifetime, alloc);
    lifetime.AddMember("max", maxLifetime, alloc);
    result.AddMember("lifetime", lifetime, alloc);
    result.AddMember("fadeout-time", kind.getFadeoutTime(), alloc);
    rapidjson::Value shape;
    shape.SetString(kind.getShape().c_str(), alloc);
    result.AddMember("shape", shape, alloc);
    result.AddMember("material", optionalStringToJson(kind.getMaterial(), alloc), alloc);
    rapidjson::Value color;
    color.SetObject();
    auto[minColor, maxColor] = kind.getColor();
    color.AddMember("min", minColor, alloc);
    color.AddMember("max", maxColor, alloc);
    result.AddMember("color", color, alloc);
    rapidjson::Value boxSize;
    boxSize.SetArray();
    auto boxSizeValues = kind.getBoxSize();
    boxSize.PushBack(boxSizeValues[0], alloc);
    boxSize.PushBack(boxSizeValues[1], alloc);
    boxSize.PushBack(boxSizeValues[2], alloc);
    result.AddMember("box-size", boxSize, alloc);
    result.AddMember("sphere-radius", kind.getSphereRadius(), alloc);
    result.AddMember("angle-spread", kind.getAngleSpread(), alloc);
    rapidjson::Value velocity;
    velocity.SetArray();
    auto velocityValues = kind.getVelocity();
    velocity.PushBack(velocityValues[0], alloc);
    velocity.PushBack(velocityValues[1], alloc);
    velocity.PushBack(velocityValues[2], alloc);
    result.AddMember("velocity", velocity, alloc);
    result.AddMember("emission-decay-rate", kind.getEmissionDecayRate(), alloc);
    rapidjson::Value scaleAffector;
    scaleAffector.SetArray();
    auto scaleAffectorValues = kind.getScaleAffactor();
    scaleAffector.PushBack(scaleAffectorValues[0], alloc);
    scaleAffector.PushBack(scaleAffectorValues[1], alloc);
    result.AddMember("scale-affector", scaleAffector, alloc);
    result.AddMember("flips", kind.hasFlips(), alloc);
    result.AddMember("vertical-particles", kind.isVertical(), alloc);
    result.AddMember("randomize-initial-y", kind.hasRandomInitialY(), alloc);
    return result;
}
// ---------------------------------------------------------------------------------------------------------------------
rapidjson::Value particleKindToJson(std::optional<std::reference_wrapper<const ParticleWrapper>> kind, rapidjson::Document::AllocatorType& alloc)
{
    return kind ? particleKindToJson(kind->get(), alloc) : rapidjson::Value();
}
// ---------------------------------------------------------------------------------------------------------------------
std::string toString(const rapidjson::Value& value)
{
    rapidjson::StringBuffer stringBuffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(stringBuffer);
    value.Accept(writer);
    return stringBuffer.GetString();
}
// ---------------------------------------------------------------------------------------------------------------------
std::optional<uint64_t> parseString(const std::string& value)
{
    try
    {
        return std::stoull(value);
    }
    catch (...)
    {
        return std::nullopt;
    }
}
// ---------------------------------------------------------------------------------------------------------------------
rapidjson::Document parseBody(const std::string& rawBody)
{
    rapidjson::Document body;
    body.Parse(rawBody.c_str());
    if (body.HasParseError())
    {
        throw std::invalid_argument("Cannot parse body!");
    }
    return body;
}
// ---------------------------------------------------------------------------------------------------------------------
const rapidjson::Value& getMember(const rapidjson::Value& value, const char* name)
{
    const auto& member = value.FindMember(name);
    if (member == value.MemberEnd())
    {
        throw std::invalid_argument("Missing member \"" + std::string(name) + "\"");
    }
    return member->value;
}
// ---------------------------------------------------------------------------------------------------------------------
bool getBool(const rapidjson::Value& value, const char* name)
{
    const auto& member = getMember(value, name);
    if (member.IsBool())
    {
        return member.GetBool();
    }
    throw std::invalid_argument(std::string("Member \"") + name + "\" is not of type boolean");
}
// ---------------------------------------------------------------------------------------------------------------------
std::string getString(const rapidjson::Value& value, const char* name)
{
    const auto& member = getMember(value, name);
    if (member.IsString())
    {
        return member.GetString();
    }
    throw std::invalid_argument(std::string("Member \"") + name + "\" is not of type string");
}
// ---------------------------------------------------------------------------------------------------------------------
std::optional<std::string> getOptionalString(const rapidjson::Value& value, const char* name)
{
    const auto& member = getMember(value, name);
    if (member.IsNull())
    {
        return std::nullopt;
    }
    return getString(value, name);
}
// ---------------------------------------------------------------------------------------------------------------------
int getInt(const rapidjson::Value& value, const char* name)
{
    const auto& member = getMember(value, name);
    if (member.IsInt())
    {
        return member.GetInt();
    }
    throw std::invalid_argument(std::string("Member \"") + name + "\" is not of type int");
}
// ---------------------------------------------------------------------------------------------------------------------
std::optional<int> getOptionalInt(const rapidjson::Value& value, const char* name)
{
    const auto& member = getMember(value, name);
    if (member.IsNull())
    {
        return std::nullopt;
    }
    return getInt(value, name);
}
// ---------------------------------------------------------------------------------------------------------------------
uint32_t getUInt32(const rapidjson::Value& value, const char* name)
{
    const auto& member = getMember(value, name);
    if (member.IsUint64())
    {
        return member.GetUint();
    }
    throw std::invalid_argument(std::string("Member \"") + name + "\" is not of type UInt");
}
// ---------------------------------------------------------------------------------------------------------------------
float getFloat(const rapidjson::Value& value, const char* name)
{
    const auto& member = getMember(value, name);
    if (member.IsFloat())
    {
        return member.GetFloat();
    }
    throw std::invalid_argument(std::string("Member \"") + name + "\" is not of type float");
}
// ---------------------------------------------------------------------------------------------------------------------
std::optional<Position> getOptionalPosition(const rapidjson::Value& value, const char* name)
{
    const auto& member = getMember(value, name);
    if (member.IsNull())
        return std::nullopt;
    if (member.IsArray())
    {
        const auto& array = member.GetArray();
        if (array.Size() == 3)
        {
            Position result;
            for (size_t i = 0; i < result.size(); i++)
            {
                const auto& element = array[i];
                if (!element.IsFloat())
                {
                    throw std::invalid_argument("Position elements must be float");
                }
                result[i] = element.GetFloat();
            }
            return result;
        }
    }
    throw std::invalid_argument("Position must be array or null");
}
// ---------------------------------------------------------------------------------------------------------------------
Position getPosition(const rapidjson::Value& value, const char* name)
{
    auto position = getOptionalPosition(value, name);
    if (!position)
        throw std::invalid_argument(std::string("Member \"") + name + "\" is no valid position");
    return position.value();
}
// ---------------------------------------------------------------------------------------------------------------------
std::unique_ptr<std::unique_lock<std::mutex>> createLock(std::mutex* mutex)
{
    return mutex ? std::make_unique<std::unique_lock<std::mutex>>(*mutex) : nullptr;
}
// ---------------------------------------------------------------------------------------------------------------------
template<typename Exchange, typename T>
static std::pair<STATUS_CODE, std::string> removeElement(Exchange& exchange, const T& id, std::mutex* mutex)
{
    auto lock = createLock(mutex);
    exchange.remove(id);
    lock.reset();
    return std::make_pair(STATUS_CODE::NO_CONTENT, toString(rapidjson::Document()));
}
// ---------------------------------------------------------------------------------------------------------------------
template<typename Exchange, typename Id>
std::pair<STATUS_CODE, std::string> findById(const std::vector<std::unique_ptr<Exchange>>& elements, const Id& id, Id(Exchange::*getId)() const, const std::function<rapidjson::Value(const Exchange&, rapidjson::Document::AllocatorType&)>& toJson)
{
    for (const auto& element : elements)
    {
        if ((element.get()->*getId)() == id)
        {
            rapidjson::Document result;
            result.CopyFrom(toJson(*element, result.GetAllocator()), result.GetAllocator());
            return {STATUS_CODE::OK, toString(result)};
        }
    }
    return Handler::generateNotFound();
}
// ---------------------------------------------------------------------------------------------------------------------
rapidjson::Value musicToJson(const MusicWrapper& musicExchange, rapidjson::Document::AllocatorType& alloc)
{
    rapidjson::Value music;
    music.SetObject();
    music.AddMember("id", rapidjson::Value().SetString(musicExchange.id.c_str(), alloc), alloc);
    music.AddMember("title", rapidjson::Value().SetString(musicExchange.title.c_str(), alloc), alloc);
    music.AddMember("composer", rapidjson::Value().SetString(musicExchange.composer.c_str(), alloc), alloc);
    music.AddMember("filename", rapidjson::Value().SetString(musicExchange.fileName.c_str(), alloc), alloc);
    rapidjson::Value fastFilenameValue;
    if (const auto& fastFilename = musicExchange.fastFileName)
    {
        fastFilenameValue.SetString(fastFilename->c_str(), alloc);
    }
    music.AddMember("fast-filename", fastFilenameValue, alloc);
    return music;
}
// ---------------------------------------------------------------------------------------------------------------------
template<typename Exchange>
static std::string putNew(Exchange& exchange, const std::string& zip, std::string (Exchange::*loadNew)(const std::filesystem::path&), std::mutex* mutex)
{
    auto path = exchange.getUnzipFunction()(zip, exchange.getDirectory());
    auto lock = createLock(mutex);
    return (exchange.*loadNew)(path);
}
// ---------------------------------------------------------------------------------------------------------------------
rapidjson::Value soundToJson(const SfxWrapper& soundBuffer, rapidjson::Document::AllocatorType& alloc)
{
    rapidjson::Value sound;
    sound.SetObject();
    sound.AddMember("id", rapidjson::Value().SetString(soundBuffer.id.c_str(), alloc), alloc);
    sound.AddMember("file", rapidjson::Value().SetString(soundBuffer.file.c_str(), alloc), alloc);
    sound.AddMember("loaded", soundBuffer.loaded, alloc);
    sound.AddMember("positional", soundBuffer.positional, alloc);
    sound.AddMember("roll-off", soundBuffer.rollOff, alloc);
    sound.AddMember("volume", soundBuffer.volume, alloc);
    sound.AddMember("max-distance", soundBuffer.maxDistance, alloc);
    rapidjson::Value durationValue;
    if (auto duration = soundBuffer.duration)
    {
        durationValue.Set(duration.value());
    }
    sound.AddMember("duration", durationValue, alloc);
    return sound;
}
// ---------------------------------------------------------------------------------------------------------------------
// Game resources
// ---------------------------------------------------------------------------------------------------------------------
class RaceHandler final : public Handler
{
public:
    explicit RaceHandler(RaceExchange& raceExchange, GetMutex getMutex)
    : raceExchange_(raceExchange)
    , getMutex_(std::move(getMutex))
    {
    }
    std::pair<STATUS_CODE, std::string> handleGet() override
    {
        rapidjson::Document result;
        result.SetObject();
        auto& alloc = result.GetAllocator();
        result.CopyFrom(raceToJson(alloc, getMutex_()), alloc);
        return {STATUS_CODE::OK, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handleGet(const std::string& raceId) override
    {
        auto currentRaceId = raceExchange_.getId();
        size_t id = std::stoull(raceId);
        if (currentRaceId == id)
        {
            return handleGet();
        }
        if (auto result = raceExchange_.getRaceResults(id))
        {
            return std::make_pair(STATUS_CODE::OK, result.value());
        }
        return generateNotFound();
    }
    std::pair<STATUS_CODE, std::string> handlePost(const std::string& request) override
    {
        rapidjson::Document body = parseBody(request);
        const auto& status = getMember(body, "status");
        if (!status.IsString())
        {
            throw std::invalid_argument("Member \"status\" must be of type string");
        }
        std::string statusValue = status.GetString();
        constexpr const char* TOO_MANY_MEMBERS = "Request contains too many members";
        if (statusValue == "EXIT")
        {
            if (body.MemberCount() > 1)
            {
                throw std::invalid_argument(TOO_MANY_MEMBERS);
            }
            raceExchange_.stop();
        }
        else if (statusValue == "PAUSE")
        {
            if (body.MemberCount() > 1)
            {
                throw std::invalid_argument(TOO_MANY_MEMBERS);
            }
            raceExchange_.pause();
        }
        else if (statusValue == "RESUME")
        {
            if (body.MemberCount() > 1)
            {
                throw std::invalid_argument(TOO_MANY_MEMBERS);
            }
            raceExchange_.resume();
        }
        else if (statusValue == "NEW")
        {
            if (body.MemberCount() > 2)
            {
                throw std::invalid_argument(TOO_MANY_MEMBERS);
            }
            const auto& setup = getMember(body, "race");
            if (setup.MemberCount() > 6)
            {
                throw std::invalid_argument(TOO_MANY_MEMBERS);
            }
            NewRace newRace{};
            newRace.numberOfLaps = getMember(setup, "laps").GetInt();
            newRace.track = getMember(setup, "track").GetString();
            newRace.kart = getMember(setup, "kart").GetString();
            const auto& aiKarts = getMember(setup, "ai-karts").GetArray();
            newRace.aiKarts.reserve(aiKarts.Size());
            for (const auto& kart : aiKarts)
            {
                newRace.aiKarts.emplace_back(kart.GetString());
            }
            newRace.difficulty = getMember(setup, "difficulty").GetString();
            newRace.reverse = getMember(setup, "reverse").GetBool();
            raceExchange_.start(newRace);
        }
        else
        {
            throw std::invalid_argument("Invalid status value \"" + statusValue + "\"");
        }
        return handleGet();
    }
    std::pair<STATUS_CODE, std::string> handlePost(const std::string& id, const std::string& request) override
    {
        auto currentRaceId = raceExchange_.getId();
        if (currentRaceId == std::stoull(id))
        {
            return handlePost(request);
        }
        return generateNotFound();
    }

private:
    rapidjson::Value raceToJson(rapidjson::Document::AllocatorType& alloc, std::mutex* mutex)
    {
        rapidjson::Value race;
        race.SetObject();
        auto lock = createLock(mutex);
        race.AddMember("status", rapidjson::Value().SetString(raceExchange_.getStatus().c_str(), alloc), alloc);
        rapidjson::Value raceValue;
        if (raceExchange_.isActive())
        {
            raceValue.SetObject();
            rapidjson::Value idValue;
            if (const auto& id = raceExchange_.getId())
            {
                idValue.Set(id.value());
            }
            raceValue.AddMember("id", idValue, alloc);
            rapidjson::Value trackNameValue;
            if (const auto& trackName = raceExchange_.getTrackName())
            {
                trackNameValue.SetString(trackName->c_str(), alloc);
            }
            raceValue.AddMember("track", trackNameValue, alloc);
            rapidjson::Value majorRaceModeValue;
            if (const auto& majorRaceMode = raceExchange_.getMajorRaceMode())
            {
                majorRaceModeValue.SetString(majorRaceMode->c_str(), alloc);
            }
            raceValue.AddMember("major-race-mode", majorRaceModeValue, alloc);
            rapidjson::Value minorRaceModeValue;
            if (const auto& minorRaceMode = raceExchange_.getMinorRaceMode())
            {
                minorRaceModeValue.SetString(minorRaceMode->c_str(), alloc);
            }
            raceValue.AddMember("minor-race-mode", minorRaceModeValue, alloc);
            rapidjson::Value difficultyValue;
            if (const auto& difficulty = raceExchange_.getDifficulty())
            {
                difficultyValue.SetString(difficulty->c_str(), alloc);
            }
            raceValue.AddMember("difficulty", difficultyValue, alloc);
            rapidjson::Value clockTypeValue;
            if (const auto& clockType = raceExchange_.getClockType())
            {
                clockTypeValue.SetString(clockType->c_str(), alloc);
            }
            raceValue.AddMember("clock-type", clockTypeValue, alloc);
            rapidjson::Value timeValue;
            if (const auto& time = raceExchange_.getTime())
            {
                timeValue.Set(time.value());
            }
            raceValue.AddMember("time", timeValue, alloc);
        }
        if (lock)
            lock->unlock();
        race.AddMember("race", raceValue, alloc);
        return race;
    }

private:
    RaceExchange& raceExchange_;
    GetMutex getMutex_;
};
// ---------------------------------------------------------------------------------------------------------------------
class KartModelHandler final : public Handler
{
public:
    KartModelHandler(KartModelExchange& raceKartExchange, GetMutex)
    : raceKartExchange_(raceKartExchange)
    {
    }
    std::pair<STATUS_CODE, std::string> handleGet() override
    {
        rapidjson::Document result;
        result.SetObject();
        auto& alloc = result.GetAllocator();
        result.SetArray();
        for (const auto& kart: raceKartExchange_.getAvailableKarts())
        {
            result.PushBack(kartToJson(kart, alloc), alloc);
        }
        return {STATUS_CODE::OK, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handleGet(const std::string& kartId) override
    {
        rapidjson::Document result;
        auto& alloc = result.GetAllocator();
        auto status = STATUS_CODE::NOT_FOUND;
        for (const auto& kart: raceKartExchange_.getAvailableKarts())
        {
            if (kart.id == kartId)
            {
                result.CopyFrom(kartToJson(kart, alloc), alloc);
                status = STATUS_CODE::OK;
                break;
            }
        }
        return {status, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handlePutZip(const std::string& zip) override
    {
        auto id = putNew(raceKartExchange_, zip, &KartModelExchange::loadNewKart, nullptr);
        return {STATUS_CODE::CREATED, handleGet(id).second};
    }
    std::pair<STATUS_CODE, std::string> handleDelete(const std::string& id) override
    {
        return removeElement(raceKartExchange_, id, nullptr);
    }

private:
    static rapidjson::Value kartToJson(const KartModelWrapper& kartCharacteristics, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value kart;
        kart.SetObject();
        kart.AddMember("id", rapidjson::Value().SetString(kartCharacteristics.id.c_str(), alloc), alloc);
        kart.AddMember("name", rapidjson::Value().SetString(kartCharacteristics.name.c_str(), alloc), alloc);
        kart.AddMember("mass", kartCharacteristics.mass, alloc);
        kart.AddMember("engine-max-speed", kartCharacteristics.engineMaxSpeed, alloc);
        kart.AddMember("acceleration-efficiency", kartCharacteristics.accelerationEfficiency, alloc);
        kart.AddMember("nitro-consumption", kartCharacteristics.nitroConsumption, alloc);
        return kart;
    }

private:
    KartModelExchange& raceKartExchange_;
};
// ---------------------------------------------------------------------------------------------------------------------
class MusicHandler final : public Handler
{
public:
    MusicHandler(MusicExchange& trackMusicLibraryExchange, GetMutex getMutex)
    : trackMusicLibraryExchange_(trackMusicLibraryExchange)
    , getMutex_(std::move(getMutex))
    {
    }
    std::pair<STATUS_CODE, std::string> handleGet() override
    {
        rapidjson::Document result;
        result.SetArray();
        auto& alloc = result.GetAllocator();
        {
            auto lock = createLock(getMutex_());
            for (const auto& music : trackMusicLibraryExchange_.getAllMusic())
            {
                result.PushBack(musicToJson(music, alloc), alloc);
            }
        }
        return {STATUS_CODE::OK, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handleGet(const std::string& parameter) override
    {
        rapidjson::Document result;
        auto& alloc = result.GetAllocator();
        {
            auto lock = createLock(getMutex_());
            for (const auto& music : trackMusicLibraryExchange_.getAllMusic())
            {
                if (music.id == parameter)
                {
                    auto musicValue = musicToJson(music, alloc);
                    result.CopyFrom(musicValue, alloc);
                    break;
                }
            }
        }
        auto status = result.IsNull() ? STATUS_CODE::NOT_FOUND : STATUS_CODE::OK;
        return {status, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handlePutZip(const std::string& zip) override
    {
        auto id = putNew(trackMusicLibraryExchange_, zip, &MusicExchange::loadNewMusic, getMutex_());
        return {STATUS_CODE::CREATED, handleGet(id).second};
    }
    std::pair<STATUS_CODE, std::string> handleDelete(const std::string& id) override
    {
        return removeElement(trackMusicLibraryExchange_, id, getMutex_());
    }

private:
    MusicExchange& trackMusicLibraryExchange_;
    GetMutex getMutex_;
};
// ---------------------------------------------------------------------------------------------------------------------
class SfxHandler final : public Handler
{
public:
    SfxHandler(SfxExchange& library, GetMutex getMutex)
    : library_(library)
    , getMutex_(std::move(getMutex))
    {
    }
    std::pair<STATUS_CODE, std::string> handleGet() override
    {
        rapidjson::Document result;
        auto& alloc = result.GetAllocator();
        result.SetArray();
        {
            auto lock = createLock(getMutex_());
            for (const auto& sound: library_.getSounds())
            {
                result.PushBack(soundToJson(sound, alloc), alloc);
            }
        }
        return {STATUS_CODE::OK, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handleGet(const std::string& parameter) override
    {
        rapidjson::Document result;
        auto& alloc = result.GetAllocator();
        {
            auto lock = createLock(getMutex_());
            for (const auto& sound: library_.getSounds())
            {
                if (sound.id == parameter)
                {
                    auto soundValue = soundToJson(sound, alloc);
                    result.CopyFrom(soundValue, alloc);
                    break;
                }
            }
        }
        STATUS_CODE status = result.IsNull() ? STATUS_CODE::NOT_FOUND : STATUS_CODE::OK;
        return {status, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handlePutZip(const std::string& zip) override
    {
        auto id = putNew(library_, zip, &SfxExchange::loadNewSfx, getMutex_());
        return {STATUS_CODE::CREATED, handleGet(id).second};
    }
    std::pair<STATUS_CODE, std::string> handleDelete(const std::string& id) override
    {
        return removeElement(library_, id, getMutex_());
    }

private:
    SfxExchange& library_;
    GetMutex getMutex_;
};
// ---------------------------------------------------------------------------------------------------------------------
class TrackModelHandler final : public Handler
{
public:
    TrackModelHandler(TrackModelExchange& raceTrackExchange, GetMutex getMutex)
    : raceTrackExchange_(raceTrackExchange)
    , getMutex_(std::move(getMutex))
    {
    }
    std::pair<STATUS_CODE, std::string> handleGet() override
    {
        rapidjson::Document result;
        result.SetObject();
        auto& alloc = result.GetAllocator();
        result.SetArray();
        for (const auto& track: raceTrackExchange_.getAvailableTracks())
        {
            result.PushBack(trackToJson(track, alloc), alloc);
        }
        return {STATUS_CODE::OK, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handleGet(const std::string& trackId) override
    {
        rapidjson::Document result;
        auto& alloc = result.GetAllocator();
        auto status = STATUS_CODE::NOT_FOUND;
        for (const auto& track: raceTrackExchange_.getAvailableTracks())
        {
            if (track.id == trackId)
            {
                result.CopyFrom(trackToJson(track, alloc), alloc);
                status = STATUS_CODE::OK;
                break;
            }
        }
        return {status, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handlePutZip(const std::string& zip) override
    {
        auto id = putNew(raceTrackExchange_, zip, &TrackModelExchange::loadNewTrack, getMutex_());
        return {STATUS_CODE::CREATED, handleGet(id).second};
    }
    std::pair<STATUS_CODE, std::string> handleDelete(const std::string& id) override
    {
        return removeElement(raceTrackExchange_, id, getMutex_());
    }

private:
    static rapidjson::Value trackToJson(const TrackModelWrapper& trackCharacteristics, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value track;
        track.SetObject();
        track.AddMember("id", rapidjson::Value().SetString(trackCharacteristics.id.c_str(), alloc), alloc);
        track.AddMember("name", rapidjson::Value().SetString(trackCharacteristics.name.c_str(), alloc), alloc);
        rapidjson::Value modes;
        modes.SetObject();
        modes.AddMember("race", trackCharacteristics.race, alloc);
        modes.AddMember("soccer", trackCharacteristics.soccer, alloc);
        modes.AddMember("arena", trackCharacteristics.arena, alloc);
        track.AddMember("modes", modes, alloc);
        rapidjson::Value groups;
        groups.SetArray();
        for (const auto& group : trackCharacteristics.groups)
        {
            groups.PushBack(rapidjson::Value().SetString(group.c_str(), alloc), alloc);
        }
        track.AddMember("groups", groups, alloc);
        return track;
    }

private:
    TrackModelExchange& raceTrackExchange_;
    GetMutex getMutex_;
};
// ---------------------------------------------------------------------------------------------------------------------
// Game resources
// ---------------------------------------------------------------------------------------------------------------------
class RaceBonusItemHandler final : public Handler
{
public:
    explicit RaceBonusItemHandler(RaceBonusItemExchange& trackItemExchange, std::mutex& mutex)
    : trackItemExchange_(trackItemExchange)
    , mutex_(mutex)
    {
    }
    std::pair<STATUS_CODE, std::string> handleGet() override
    {
        rapidjson::Document result;
        auto& alloc = result.GetAllocator();
        result.SetArray();
        {
            std::unique_lock lock(mutex_);
            for (const auto& item: trackItemExchange_.getItems())
            {
                result.PushBack(itemToJson(item.get(), alloc), alloc);
            }
        }
        return std::make_pair(STATUS_CODE::OK, toString(result));
    }
    std::pair<STATUS_CODE, std::string> handleGet(const std::string& parameter) override
    {
        auto index = parseString(parameter);
        if (!index)
        {
            return generateNotFound();
        }
        rapidjson::Document result;
        auto& alloc = result.GetAllocator();
        STATUS_CODE status = STATUS_CODE::NOT_FOUND;
        {
            std::unique_lock lock(mutex_);
            auto items = trackItemExchange_.getItems();
            if (index < items.size())
            {
                auto item = std::move(items[index.value()]);
                result.SetObject();
                result.CopyFrom(itemToJson(item.get(), alloc), alloc);
                status = STATUS_CODE::OK;
            }
        }
        return std::make_pair(status, toString(result));
    }
    std::pair<STATUS_CODE, std::string> handlePost(const std::string& id, const std::string& body) override
    {
        auto index = parseString(id);
        if (!index)
        {
            return generateNotFound();
        }
        auto input = parseBody(body);
        rapidjson::Document result;
        auto& alloc = result.GetAllocator();
        STATUS_CODE status = STATUS_CODE::NOT_FOUND;
        {
            std::unique_lock lock(mutex_);
            auto items = trackItemExchange_.getItems();
            if (index < items.size())
            {
                auto item = std::move(items[index.value()]);
                if (item)
                {
                    if (input.HasMember("type"))
                    {
                        item->setType(getString(input, "type"));
                    }
                    if (input.HasMember("original-type"))
                    {
                        const auto& originalType = getMember(input, "original-type");
                        item->setOriginalType(originalType.IsNull() ? std::nullopt : std::optional<std::string>(
                            getString(input, "original-type")));
                    }
                    if (input.HasMember("ticks-until-return"))
                    {
                        item->setTicksUntilReturn(getInt(input, "ticks-until-return"));
                    }
                    if (input.HasMember("used-up-counter"))
                    {
                        const auto& usedUpCounter = getMember(input, "used-up-counter");
                        item->setUsedUpCounter(usedUpCounter.IsNull() ? std::nullopt : std::optional<int>(
                            getInt(input, "used-up-counter")));
                    }
                    result.CopyFrom(itemToJson(item.get(), alloc), alloc);
                    status = STATUS_CODE::OK;
                }
            }
        }
        return std::make_pair(status, toString(result));
    }
    std::pair<STATUS_CODE, std::string> handlePut(const std::string& body) override
    {
        auto input = parseBody(body);
        auto position = getPosition(input, "position");
        auto type = getString(input, "type");
        auto originalType = getOptionalString(input, "original-type");
        auto ticksUntilReturn = getInt(input, "ticks-until-return");
        auto usedUpCounter = getOptionalInt(input, "used-up-counter");
        {
            std::unique_lock lock(mutex_);
            auto item = trackItemExchange_.add({position, type, originalType, ticksUntilReturn, usedUpCounter});
            lock.unlock();
            rapidjson::Document result;
            auto& alloc = result.GetAllocator();
            result.CopyFrom(itemToJson(item.get(), alloc), alloc);
            return std::make_pair(STATUS_CODE::CREATED, toString(result));
        }
    }
    std::pair<STATUS_CODE, std::string> handleDelete(const std::string& id) override
    {
        return removeElement(trackItemExchange_, std::stoull(id), &mutex_);
    }

private:
    static rapidjson::Value itemToJson(const BonusItemWrapper* itemExchange, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value item;
        if (itemExchange)
        {
            item.SetObject();
            item.AddMember("id", itemExchange->getId(), alloc);
            item.AddMember("position", vectorToJson(itemExchange->getPosition(), alloc), alloc);
            rapidjson::Value type;
            type.SetString(itemExchange->getType().c_str(), alloc);
            item.AddMember("type", type, alloc);
            item.AddMember("original-type", optionalStringToJson(itemExchange->getOriginalType(), alloc), alloc);
            item.AddMember("ticks-until-return", itemExchange->getTicksUntilReturn(), alloc);
            rapidjson::Value usedUpCounterValue;
            if (auto usedUpCounter = itemExchange->getUsedUpCounter())
            {
                usedUpCounterValue.Set(usedUpCounter.value(), alloc);
            }
            item.AddMember("used-up-counter", usedUpCounterValue, alloc);
        }
        return item;
    }

private:
    RaceBonusItemExchange& trackItemExchange_;
    std::mutex& mutex_;
};
// ---------------------------------------------------------------------------------------------------------------------
class RaceChecklineHandler final : public Handler
{
public:
    explicit RaceChecklineHandler(const RaceChecklineExchange& checklineExchange, std::mutex& mutex)
    : checklineExchange_(checklineExchange)
    , mutex_(mutex)
    {
    }
    std::pair<STATUS_CODE, std::string> handleGet() override
    {
        rapidjson::Document result;
        result.SetArray();
        auto& alloc = result.GetAllocator();
        {
            std::unique_lock lock(mutex_);
            for (const ChecklineWrapper& checkline : checklineExchange_.getChecklines())
            {
                result.PushBack(checklineToJson(checkline, alloc), alloc);
            }
        }
        return {STATUS_CODE::OK, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handleGet(const std::string& parameter) override
    {
        auto id = parseString(parameter);
        if (!id)
        {
            return generateNotFound();
        }
        auto status = STATUS_CODE::NOT_FOUND;
        rapidjson::Document result;
        auto& alloc = result.GetAllocator();
        {
            std::unique_lock lock(mutex_);
            for (const ChecklineWrapper& checkline : checklineExchange_.getChecklines())
            {
                if (checkline.id == id)
                {
                    result.CopyFrom(checklineToJson(checkline, alloc), alloc);
                    status = STATUS_CODE::OK;
                    break;
                }
            }
        }
        return {status, toString(result)};
    }

private:
    static rapidjson::Value checklineToJson(const ChecklineWrapper& checkline, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value checklineValue;
        checklineValue.SetObject();
        checklineValue.AddMember("id", checkline.id, alloc);
        checklineValue.AddMember("kind", rapidjson::Value().SetString(checkline.kind.c_str(), alloc), alloc);
        checklineValue.AddMember("active-at-reset", checkline.activeAtReset, alloc);
        std::optional<bool> ignoreHeight = checkline.ignoreHeight;
        if (ignoreHeight)
        {
            checklineValue.AddMember("ignore-height", ignoreHeight.value(), alloc);
        }
        std::optional<std::pair<Position, Position>> position = checkline.position;
        if (position)
        {
            rapidjson::Value positionValue;
            positionValue.SetArray();
            positionValue.PushBack(vectorToJson(position->first, alloc), alloc);
            positionValue.PushBack(vectorToJson(position->second, alloc), alloc);
            checklineValue.AddMember("position", positionValue, alloc);
        }
        rapidjson::Value otherIdsValue;
        otherIdsValue.SetArray();
        for (const auto& id : checkline.otherIds)
        {
            otherIdsValue.PushBack(id, alloc);
        }
        checklineValue.AddMember("other-ids", otherIdsValue, alloc);
        rapidjson::Value sameGroupValue;
        sameGroupValue.SetArray();
        for (const auto& id : checkline.sameGroup)
        {
            sameGroupValue.PushBack(id, alloc);
        }
        checklineValue.AddMember("same-group", sameGroupValue, alloc);
        rapidjson::Value active;
        active.SetArray();
        for (uint64_t i = 0; i < checkline.active.size(); i++)
        {
            if (checkline.active[i])
            {
                active.PushBack(i, alloc);
            }
        }
        checklineValue.AddMember("karts", active, alloc);
        return checklineValue;
    }

private:
    const RaceChecklineExchange& checklineExchange_;
    std::mutex& mutex_;
};
// ---------------------------------------------------------------------------------------------------------------------
class RaceKartHandler final : public Handler
{
public:
    RaceKartHandler(const RaceKartExchange& trackKartExchange, std::mutex& mutex)
    : trackKartExchange_(trackKartExchange)
    , mutex_(mutex)
    {
    }
    std::pair<STATUS_CODE, std::string> handleGet() override
    {
        rapidjson::Document result;
        auto& alloc = result.GetAllocator();
        result.SetArray();
        {
            std::unique_lock lock(mutex_);
            for (const auto& kart: trackKartExchange_.getKarts())
            {
                result.PushBack(kartToJson(*kart, alloc), alloc);
            }
        }
        return std::make_pair(STATUS_CODE::OK, toString(result));
    }
    std::pair<STATUS_CODE, std::string> handleGet(const std::string& parameter) override
    {
        auto id = parseString(parameter);
        if (!id)
            return generateNotFound();
        std::unique_lock lock(mutex_);
        return findById<KartWrapper>(trackKartExchange_.getKarts(), id.value(), &KartWrapper::getId, kartToJson);
    }
    std::pair<STATUS_CODE, std::string> handlePost(const std::string& parameter, const std::string& body) override
    {
        auto id = parseString(parameter);
        if (!id)
        {
            return generateNotFound();
        }
        auto input = parseBody(body);
        rapidjson::Document result;
        auto& alloc = result.GetAllocator();
        auto status = STATUS_CODE::NOT_FOUND;
        {
            std::unique_lock lock(mutex_);
            for (const auto& kart : trackKartExchange_.getKarts())
            {
                if (kart->getId() == id.value())
                {
                    lock.unlock();
                    setKart(input, *kart);
                    break;
                }
            }
            if (!lock.owns_lock())
            {
                lock.lock();
            }
            for (const auto& kart : trackKartExchange_.getKarts())
            {
                if (kart->getId() == id.value())
                {
                    setAttachment(input, *kart);
                    setPowerUp(input, *kart);
                    result.CopyFrom(kartToJson(*kart, alloc), alloc);
                    status = STATUS_CODE::OK;
                    break;
                }
            }
        }
        return std::make_pair(status, toString(result));
    }

private:
    static rapidjson::Value kartToJson(const KartWrapper& kartExchange, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value kart;
        kart.SetObject();
        kart.AddMember("id", kartExchange.getId(), alloc);
        kart.AddMember("rank", kartExchange.getRank(), alloc);
        rapidjson::Value controller;
        controller.SetString(kartExchange.getController().c_str(), alloc);
        kart.AddMember("controller", controller, alloc);
        kart.AddMember("characteristics", getCharacteristics(kartExchange, alloc), alloc);
        kart.AddMember("speed", getSpeed(kartExchange, alloc), alloc);
        kart.AddMember("position", getPosition(kartExchange, alloc), alloc);
        kart.AddMember("status", getStatus(kartExchange, alloc), alloc);
        kart.AddMember("skidding", getSkidding(kartExchange, alloc), alloc);
        kart.AddMember("control", getControl(kartExchange, alloc), alloc);
        kart.AddMember("collision", getCollision(kartExchange, alloc), alloc);
        kart.AddMember("nitro", getNitro(kartExchange, alloc), alloc);
        rapidjson::Value attachmentValue;
        if (auto attachment = kartExchange.getAttachment())
        {
            attachmentValue.SetObject();
            attachmentValue.AddMember("type", rapidjson::Value().SetString(attachment->type.c_str(), alloc), alloc);
            attachmentValue.AddMember("ticks", attachment->ticks, alloc);
        }
        kart.AddMember("attachment", attachmentValue, alloc);
        kart.AddMember("power-up", getPowerUp(kartExchange, alloc), alloc);
        kart.AddMember("icon", optionalStringToJson(kartExchange.getIcon(), alloc), alloc);
        rapidjson::Value minimapIcon;
        minimapIcon.SetString(kartExchange.getMinimapIconPath().c_str(), alloc);
        kart.AddMember("minimap-icon", minimapIcon, alloc);
        kart.AddMember("shadow", optionalStringToJson(kartExchange.getShadowMaterial(), alloc), alloc);
        kart.AddMember("ground", optionalStringToJson(kartExchange.getGround(), alloc), alloc);
        kart.AddMember("result", getResults(kartExchange, alloc), alloc);
        return kart;
    }
    static rapidjson::Value getCharacteristics(const KartWrapper& kartExchange, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value characteristics;
        characteristics.SetObject();
        rapidjson::Value name;
        rapidjson::Value ident;
        ident.SetString(kartExchange.getIdent().c_str(), alloc);
        characteristics.AddMember("ident", ident, alloc);
        characteristics.AddMember("color", kartExchange.getColor(), alloc);
        rapidjson::Value type;
        type.SetString(kartExchange.getType().c_str(), alloc);
        characteristics.AddMember("type", type, alloc);
        characteristics.AddMember("groups", getGroups(kartExchange, alloc), alloc);
        characteristics.AddMember("engine-sfx", optionalStringToJson(kartExchange.getEngineSfxType(), alloc), alloc);
        characteristics.AddMember("skid-sound", optionalStringToJson(kartExchange.getSkidSound(), alloc), alloc);
        characteristics.AddMember("friction", kartExchange.getFriction(), alloc);
        characteristics.AddMember("friction-slip", kartExchange.getFrictionSlip(), alloc);
        rapidjson::Value terrainImpulseType;
        terrainImpulseType.SetString(kartExchange.getTerrainImpulseType().c_str(), alloc);
        characteristics.AddMember("terrain-impulse-type", terrainImpulseType, alloc);
        return characteristics;
    }
    static rapidjson::Value getGroups(const KartWrapper& kartExchange, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value groups;
        groups.SetArray();
        for (const auto& groupValue : kartExchange.getGroups())
        {
            rapidjson::Value group;
            group.SetString(groupValue.c_str(), alloc);
            groups.PushBack(group, alloc);
        }
        return groups;
    }
    static rapidjson::Value getSpeed(const KartWrapper& kartExchange, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value speed;
        speed.SetObject();
        speed.AddMember("current", kartExchange.getSpeed(), alloc);
        speed.AddMember("max", kartExchange.getMaxSpeed(), alloc);
        rapidjson::Value minBoostSpeedValue;
        if (auto minBoostSpeed = kartExchange.getMinBoostSpeed())
        {
            minBoostSpeedValue.Set(minBoostSpeed.value());
        }
        speed.AddMember("min-boost-speed", minBoostSpeedValue, alloc);
        speed.AddMember("velocity", vectorToJson(kartExchange.getVelocity(), alloc), alloc);
        speed.AddMember("increase", getSpeedIncrease(kartExchange, alloc), alloc);
        speed.AddMember("decrease", getSpeedDecrease(kartExchange, alloc), alloc);
        return speed;
    }
    static rapidjson::Value getSpeedIncrease(const KartWrapper& kartExchange, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value speedIncreaseValue;
        speedIncreaseValue.SetArray();
        for (const auto& speedIncrease : kartExchange.getSpeedIncrease())
        {
            if (speedIncrease.active)
            {
                rapidjson::Value nameValue;
                nameValue.SetString(speedIncrease.name.c_str(), alloc);
                rapidjson::Value increase;
                increase.SetObject();
                increase.AddMember("kind", nameValue, alloc);
                if (speedIncrease.timeLeft)
                {
                    increase.AddMember("time-left", speedIncrease.timeLeft.value(), alloc);
                }
                increase.AddMember("value", speedIncrease.speedIncrease, alloc);
                increase.AddMember("engine-force", speedIncrease.engineForceIncrease, alloc);
                speedIncreaseValue.PushBack(increase, alloc);
            }
        }
        return speedIncreaseValue;
    }
    static rapidjson::Value getSpeedDecrease(const KartWrapper& kartExchange, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value speedDecreaseValue;
        speedDecreaseValue.SetArray();
        for (const auto& speedDecrease : kartExchange.getSpeedDecrease())
        {
            if (speedDecrease.active)
            {
                rapidjson::Value nameValue;
                nameValue.SetString(speedDecrease.name.c_str(), alloc);
                rapidjson::Value decrease;
                decrease.SetObject();
                decrease.AddMember("kind", nameValue, alloc);
                if (speedDecrease.timeLeft)
                {
                    decrease.AddMember("time-left", speedDecrease.timeLeft.value(), alloc);
                }
                decrease.AddMember("fraction", speedDecrease.fraction, alloc);
                speedDecreaseValue.PushBack(decrease, alloc);
            }
        }
        return speedDecreaseValue;
    }
    static rapidjson::Value getPosition(const KartWrapper& kartExchange, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value position;
        position.SetObject();
        position.AddMember("current", vectorToJson(kartExchange.getPosition(), alloc), alloc);
        position.AddMember("front", vectorToJson(kartExchange.getFrontPosition(), alloc), alloc);
        position.AddMember("jumping", kartExchange.isJumping(), alloc);
        position.AddMember("flying", kartExchange.isFlying(), alloc);
        position.AddMember("near-ground", kartExchange.isNearGround(), alloc);
        position.AddMember("on-ground", kartExchange.isOnGround(), alloc);
        position.AddMember("pitch", kartExchange.getPitch(), alloc);
        position.AddMember("roll", kartExchange.getRoll(), alloc);
        position.AddMember("lean", kartExchange.getLean(), alloc);
        position.AddMember("lean-max", kartExchange.getLeanMax(), alloc);
        return position;
    }
    static rapidjson::Value getStatus(const KartWrapper& kartExchange, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value status;
        status.SetObject();
        rapidjson::Value handicap;
        handicap.SetString(kartExchange.getHandicapLevel().c_str(), alloc);
        status.AddMember("handicap", handicap, alloc);
        status.AddMember("boosted-ai", kartExchange.isBoostedAI(), alloc);
        status.AddMember("blocked-by-plunger", kartExchange.isBlockedByPlunger(), alloc);
        status.AddMember("shielded", kartExchange.isShielded(), alloc);
        status.AddMember("squashed", kartExchange.isSquashed(), alloc);
        status.AddMember("eliminated", kartExchange.isEliminated(), alloc);
        status.AddMember("ghost", kartExchange.isGhostKart(), alloc);
        status.AddMember("rescue", kartExchange.isInRescue(), alloc);
        return status;
    }
    static rapidjson::Value getSkidding(const KartWrapper& kartExchange, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value skidding;
        skidding.SetObject();
        rapidjson::Value statusValue;
        skidding.AddMember("status", optionalStringToJson(kartExchange.getSkidding(), alloc), alloc);
        skidding.AddMember("ready", kartExchange.isSkiddingBonusReady(), alloc);
        skidding.AddMember("factor", kartExchange.getSkiddingFactor(), alloc);
        skidding.AddMember("max", kartExchange.getMaxSkidding(), alloc);
        return skidding;
    }
    static rapidjson::Value getControl(const KartWrapper& kartExchange, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value control;
        control.SetObject();
        control.AddMember("steer", kartExchange.getSteer(), alloc);
        control.AddMember("max-steer", kartExchange.getMaxSteerAngle(), alloc);
        control.AddMember("acceleration", kartExchange.getAcceleration(), alloc);
        control.AddMember("braking", kartExchange.isBraking(), alloc);
        rapidjson::Value skidControl;
        skidControl.SetString(kartExchange.getSkidControl().c_str(), alloc);
        control.AddMember("fire", kartExchange.doesFire(), alloc);
        control.AddMember("look-back", kartExchange.doesLookBack(), alloc);
        control.AddMember("skid-control", skidControl, alloc);
        return control;
    }
    static rapidjson::Value getCollision(const KartWrapper& kartExchange, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value collision;
        collision.SetObject();
        collision.AddMember("impulse", kartExchange.getCollisionImpulse(), alloc);
        collision.AddMember("time", kartExchange.getCollisionImpulseTime(), alloc);
        collision.AddMember("restitution", kartExchange.getRestitution(), alloc);
        return collision;
    }
    static rapidjson::Value getPowerUp(const KartWrapper& kartExchange, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value powerUpValue;
        if (auto powerUp = kartExchange.getPowerUp())
        {
            const auto& [name, count] = powerUp.value();
            powerUpValue.SetObject();
            rapidjson::Value type;
            type.SetString(name.c_str(), alloc);
            powerUpValue.AddMember("type", type, alloc);
            powerUpValue.AddMember("count", count, alloc);
        }
        return powerUpValue;
    }
    static rapidjson::Value getNitro(const KartWrapper& kartExchange, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value nitro;
        nitro.SetObject();
        nitro.AddMember("collected", kartExchange.getCollectedEnergy(), alloc);
        nitro.AddMember("max", kartExchange.getMaxNitro(), alloc);
        nitro.AddMember("min-ticks", kartExchange.getMinNitroConsumption(), alloc);
        nitro.AddMember("consumption-per-tick", kartExchange.getConsumptionPerTick(), alloc);
        nitro.AddMember("activated", kartExchange.hasNitroActivated(), alloc);
        return nitro;
    }
    static rapidjson::Value getResults(const KartWrapper& kartExchange, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value result;
        result.SetObject();
        auto finishTime = kartExchange.getFinishTime();
        result.AddMember("finished", finishTime.has_value(), alloc);
        rapidjson::Value timeValue;
        if (auto time = kartExchange.getFinishTime())
        {
            timeValue.Set(time.value());
        }
        result.AddMember("time", timeValue, alloc);
        return result;
    }
    static void setKart(const rapidjson::Value& input, KartWrapper& kart)
    {
        constexpr const char* KART = "characteristics";
        if (input.HasMember(KART))
        {
            const auto& kartInput = getMember(input, KART);
            auto ident = getString(kartInput, "ident");
            kart.setKart(ident);
        }
    }
    static void setAttachment(const rapidjson::Value& input, KartWrapper& kart)
    {
        constexpr const char* ATTACHMENT = "attachment";
        if (input.HasMember(ATTACHMENT))
        {
            const auto& attachmentInput = getMember(input, ATTACHMENT);
            std::optional<RestApi::Attachment> attachment =
                attachmentInput.IsNull()
                ? std::optional<RestApi::Attachment>()
                : RestApi::Attachment{
                    getString(attachmentInput, "type"),
                    getInt(attachmentInput, "ticks")
                };
            kart.setAttachment(attachment);
        }
    }
    static void setPowerUp(const rapidjson::Value& input, KartWrapper& kart)
    {
        constexpr const char* POWER_UP = "power-up";
        if (input.HasMember(POWER_UP))
        {
            const auto& powerUpInput = getMember(input, POWER_UP);
            std::optional<RestApi::PowerUp> powerUp =
                powerUpInput.IsNull()
                ? std::optional<RestApi::PowerUp>()
                : RestApi::PowerUp{
                    getString(powerUpInput, "name"),
                    getInt(powerUpInput, "count")
                };
            kart.setPowerUp(powerUp);
        }
    }

private:
    const RaceKartExchange& trackKartExchange_;
    std::mutex& mutex_;
};
// ---------------------------------------------------------------------------------------------------------------------
class RaceMaterialHandler final : public Handler
{
public:
    explicit RaceMaterialHandler(const RaceMaterialExchange& trackMaterialExchange, std::mutex& mutex)
    : materialExchange_(trackMaterialExchange)
    , mutex_(mutex)
    {
    }
    std::pair<STATUS_CODE, std::string> handleGet() override
    {
        rapidjson::Document result;
        result.SetArray();
        {
            std::unique_lock lock(mutex_);
            for (const auto& material: materialExchange_.getMaterials())
            {
                result.PushBack(materialToJson(*material, result.GetAllocator()), result.GetAllocator());
            }
        }
        return {STATUS_CODE::OK, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handleGet(const std::string& parameter) override
    {
        std::unique_lock lock(mutex_);
        return findById<MaterialWrapper>(materialExchange_.getMaterials(), parameter, &MaterialWrapper::getName, materialToJson);
    }

private:
    const RaceMaterialExchange& materialExchange_;
    std::mutex& mutex_;
};
// ---------------------------------------------------------------------------------------------------------------------
class RaceMusicHandler final : public Handler
{
public:
    RaceMusicHandler(RaceMusicExchange& trackMusicExchange, std::mutex& mutex)
    : trackMusicExchange_(trackMusicExchange)
    , mutex_(mutex)
    {
    }
    std::pair<STATUS_CODE, std::string> handleGet() override
    {
        rapidjson::Document result;
        result.SetObject();
        auto& alloc = result.GetAllocator();
        {
            std::unique_lock lock(mutex_);
            result.AddMember("enabled", trackMusicExchange_.isEnabled(), alloc);
            result.AddMember("volume", trackMusicExchange_.getMasterMusicGain(), alloc);
            rapidjson::Value musicValue;
            if (const auto& music = trackMusicExchange_.getMusic())
            {
                musicValue = musicToJson(music.value(), alloc);
            }
            result.AddMember("music", musicValue, alloc);
        }
        return {STATUS_CODE::OK, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handlePost(const std::string& body) override
    {
        auto input = parseBody(body);
        if (input.HasMember("enabled"))
        {
            trackMusicExchange_.setEnabled(getBool(input, "enabled"));
        }
        if (input.HasMember("volume"))
        {
            trackMusicExchange_.setMasterMusicGain(getFloat(input, "volume"));
        }
        if (input.HasMember("music"))
        {
            trackMusicExchange_.setMusic(getOptionalString(input, "music"));
        }
        return handleGet();
    }

private:
    RaceMusicExchange& trackMusicExchange_;
    std::mutex& mutex_;
};
// ---------------------------------------------------------------------------------------------------------------------
class RaceObjectHandler final : public Handler
{
public:
    explicit RaceObjectHandler(RaceObjectExchange& trackObjectExchange, std::mutex& mutex)
    : trackObjectExchange_(trackObjectExchange)
    , mutex_(mutex)
    {
    }
    std::pair<STATUS_CODE, std::string> handleGet() override
    {
        rapidjson::Document result;
        result.SetArray();
        auto& alloc = result.GetAllocator();
        {
            std::unique_lock lock(mutex_);
            for (const auto& object: trackObjectExchange_.getObjects())
            {
                result.PushBack(objectToJson(*object, alloc), alloc);
            }
        }
        return {STATUS_CODE::OK, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handleGet(const std::string& parameter) override
    {
        auto id = parseString(parameter);
        if (!id)
        {
            return generateNotFound();
        }
        rapidjson::Document result;
        auto status = STATUS_CODE::NOT_FOUND;
        auto& alloc = result.GetAllocator();
        {
            std::unique_lock lock(mutex_);
            const auto& object = findObject(id.value());
            if (object)
            {
                status = STATUS_CODE::OK;
                result.CopyFrom(objectToJson(*object, alloc), alloc);
            }
        }
        return {status, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handlePost(const std::string& id, const std::string& body) override
    {
        auto input = parseBody(body);
        rapidjson::Document result;
        auto status = STATUS_CODE::NOT_FOUND;
        auto& alloc = result.GetAllocator();
        {
            std::unique_lock lock(mutex_);
            auto object = findObject(id);
            if (object)
            {
                if (input.HasMember("position"))
                {
                    object->setPosition(getPosition(input, "position"));
                }
                if (input.HasMember("enabled"))
                {
                    object->setEnabled(getBool(input, "enabled"));
                }
                auto light = object->getLight();
                if (light)
                {
                    if (input.HasMember("color"))
                    {
                        light->setColor(getUInt32(input, "color"));
                    }
                    if (input.HasMember("radius"))
                    {
                        light->setRadius(getFloat(input, "radius"));
                    }
                    if (input.HasMember("energy"))
                    {
                        light->setEnergy(getFloat(input, "energy"));
                    }
                }
                status = STATUS_CODE::OK;
                result.CopyFrom(objectToJson(*object, light.get(), alloc), alloc);
            }
        }
        return {status, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handleDelete(const std::string& id) override
    {
        return removeElement(trackObjectExchange_, std::stoull(id), &mutex_);
    }

private:
    static rapidjson::Value objectToJson(const ObjectWrapper& objectExchange, rapidjson::Document::AllocatorType& alloc)
    {
        return objectToJson(objectExchange, objectExchange.getLight().get(), alloc);
    }
    static rapidjson::Value objectToJson(const ObjectWrapper& objectExchange, const LightWrapper* lightExchange, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value object;
        object.SetObject();
        object.AddMember("id", objectExchange.getId(), alloc);
        object.AddMember("name", rapidjson::Value().SetString(objectExchange.getName().c_str(), alloc), alloc);
        object.AddMember("type", rapidjson::Value().SetString(objectExchange.getType().c_str(), alloc), alloc);
        object.AddMember("enabled", objectExchange.isEnabled(), alloc);
        object.AddMember("drivable", objectExchange.isDrivable(), alloc);
        object.AddMember("animated", objectExchange.isAnimated(), alloc);
        object.AddMember("position", vectorToJson(objectExchange.getPosition(), alloc), alloc);
        object.AddMember("center", vectorToJson(objectExchange.getCenterPosition(), alloc), alloc);
        object.AddMember("rotation", vectorToJson(objectExchange.getRotation(), alloc), alloc);
        object.AddMember("scale", vectorToJson(objectExchange.getScale(), alloc), alloc);
        rapidjson::Value lodGroup;
        lodGroup.SetString(objectExchange.getLodGroup().c_str(), alloc);
        object.AddMember("lod-group", lodGroup, alloc);
        rapidjson::Value interaction;
        interaction.SetString(objectExchange.getInteraction().c_str(), alloc);
        object.AddMember("interaction", interaction, alloc);
        auto type = objectExchange.getType();
        if (lightExchange)
        {
            rapidjson::Value light;
            light.SetObject();
            light.AddMember("color", lightExchange->getColor(), alloc);
            light.AddMember("energy", lightExchange->getEnergy(), alloc);
            light.AddMember("radius", lightExchange->getRadius(), alloc);
            object.AddMember("light", light, alloc);
        }
        if (auto particleExchange = objectExchange.getParticles())
        {
            auto particles = particleKindToJson(*particleExchange, alloc);
            object.AddMember("particle-emitter", particles, alloc);
        }

        rapidjson::Value children;
        children.SetArray();
        for (const auto& child : objectExchange.getChildren())
        {
            children.PushBack(objectToJson(*child, alloc), alloc);
        }
        object.AddMember("children", children, alloc);
        rapidjson::Value movableChildren;
        movableChildren.SetArray();
        for (const auto& child : objectExchange.getMovableChildren())
        {
            movableChildren.PushBack(objectToJson(*child, alloc), alloc);
        }
        object.AddMember("movable-children", movableChildren, alloc);

        return object;
    }
    std::unique_ptr<ObjectWrapper> findObject(const std::string& idInput)
    {
        auto id = parseString(idInput);
        return id ? findObject(id.value()) : nullptr;
    }
    std::unique_ptr<ObjectWrapper> findObject(size_t id)
    {
        auto check = [&id](const std::unique_ptr<ObjectWrapper>& object) {
            return object->getId() == id;
        };
        for (auto& object : trackObjectExchange_.getObjects())
        {
            if (check(object))
                return std::move(object);
            auto children = object->getChildren();
            auto childFound = std::find_if(children.begin(), children.end(), check);
            if (childFound != children.end())
                return std::move(*childFound);
            auto movableChildren = object->getMovableChildren();
            auto movableChildFound = std::find_if(movableChildren.begin(), movableChildren.end(), check);
            if (movableChildFound != movableChildren.end())
                return std::move(*movableChildFound);
        }
        return nullptr;
    }

private:
    RaceObjectExchange& trackObjectExchange_;
    std::mutex& mutex_;
};
// ---------------------------------------------------------------------------------------------------------------------
class RaceQuadHandler final : public Handler
{
public:
    explicit RaceQuadHandler(const RaceQuadExchange& quadExchange)
    : quadExchange_(quadExchange)
    {
    }
    std::pair<STATUS_CODE, std::string> handleGet() override
    {
        rapidjson::Document result;
        result.SetArray();
        auto& alloc = result.GetAllocator();
        for (const QuadWrapper& quad : quadExchange_.getQuads())
        {
            result.PushBack(quadToJson(quad, alloc), alloc);
        }
        return {STATUS_CODE::OK, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handleGet(const std::string& parameter) override
    {
        auto id = parseString(parameter);
        if (!id)
        {
            return generateNotFound();
        }
        auto status = STATUS_CODE::NOT_FOUND;
        rapidjson::Document result;
        auto& alloc = result.GetAllocator();
        for (const QuadWrapper& quad : quadExchange_.getQuads())
        {
            if (quad.id == id)
            {
                result.CopyFrom(quadToJson(quad, alloc), alloc);
                status = STATUS_CODE::OK;
                break;
            }
        }
        return {status, toString(result)};
    }

private:
    static rapidjson::Value quadToJson(const QuadWrapper& quad, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value quadValue;
        quadValue.SetObject();
        quadValue.AddMember("id", quad.id, alloc);
        quadValue.AddMember("ignore", quad.ignore, alloc);
        quadValue.AddMember("invisible", quad.invisible, alloc);
        quadValue.AddMember("ai-ignore", quad.aiIgnore, alloc);
        rapidjson::Value position;
        position.SetArray();
        for (size_t i = 0; i < 4; ++i)
        {
            position.PushBack(vectorToJson(quad.quad[i], alloc), alloc);
        }
        quadValue.AddMember("position", position, alloc);
        rapidjson::Value heightTesting;
        heightTesting.SetObject();
        const auto& [min, max] = quad.heightTesting;
        heightTesting.AddMember("min", min, alloc);
        heightTesting.AddMember("max", max, alloc);
        quadValue.AddMember("height-testing", heightTesting, alloc);
        rapidjson::Value successors;
        successors.SetArray();
        for (const auto& id : quad.successors)
        {
            successors.PushBack(id, alloc);
        }
        quadValue.AddMember("successors", successors, alloc);
        return quadValue;
    }

private:
    const RaceQuadExchange& quadExchange_;
};
// ---------------------------------------------------------------------------------------------------------------------
class RaceSfxHandler final : public Handler
{
public:
    RaceSfxHandler(RaceSfxExchange& trackSfxExchange, std::mutex& mutex)
    : trackSfxExchange_(trackSfxExchange)
    , mutex_(mutex)
    {
    }
    std::pair<STATUS_CODE, std::string> handleGet() override
    {
        rapidjson::Document result;
        auto& alloc = result.GetAllocator();
        result.SetObject();
        {
            std::unique_lock lock(mutex_);
            result.AddMember("sfx-allowed", trackSfxExchange_.isSfxAllowed(), alloc);
            result.AddMember("master-volume", trackSfxExchange_.getMasterVolume(), alloc);
            rapidjson::Value listener;
            listener.SetObject();
            listener.AddMember("position", vectorToJson(trackSfxExchange_.getListenerPosition(), alloc), alloc);
            listener.AddMember("direction", vectorToJson(trackSfxExchange_.getListenerDirection(), alloc), alloc);
            listener.AddMember("up", vectorToJson(trackSfxExchange_.getListenerUpDirection(), alloc), alloc);
            result.AddMember("listener", listener, alloc);
            rapidjson::Value allSounds;
            allSounds.SetArray();
            for (const auto& sound: trackSfxExchange_.getAllSounds())
            {
                allSounds.PushBack(sfxToJson(*sound, alloc), alloc);
            }
            result.AddMember("sounds", allSounds, alloc);
        }
        return {STATUS_CODE::OK, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handleGet(const std::string& id) override
    {
        std::unique_lock lock(mutex_);
        auto sfx = getSfxById(id);
        if (!sfx)
            return generateNotFound();
        rapidjson::Document result;
        result.CopyFrom(sfxToJson(*sfx, result.GetAllocator()), result.GetAllocator());
        return {STATUS_CODE::OK, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handlePost(const std::string& body) override
    {
        auto input = parseBody(body);
        {
            std::unique_lock lock(mutex_);
            if (input.HasMember("sfx-allowed"))
                trackSfxExchange_.setSfxAllowed(getBool(input, "sfx-allowed"));
            if (input.HasMember("master-volume"))
                trackSfxExchange_.setMasterVolume(getFloat(input, "master-volume"));
        }
        return handleGet();
    }
    std::pair<STATUS_CODE, std::string> handlePost(const std::string& id, const std::string& body) override
    {
        auto input = parseBody(body);
        rapidjson::Document result;
        {
            std::unique_lock lock(mutex_);
            auto sfx = getSfxById(id);
            if (!sfx)
                return generateNotFound();
            if (input.HasMember("status"))
                sfx->setStatus(getString(input, "status"));
            if (input.HasMember("loop"))
                sfx->setLoop(getBool(input, "loop"));
            if (input.HasMember("volume"))
                sfx->setVolume(getFloat(input, "volume"));
            if (input.HasMember("pitch"))
                sfx->setPitch(getFloat(input, "pitch"));
            if (input.HasMember("position"))
                sfx->setPosition(getPosition(input, "position"));
            result.CopyFrom(sfxToJson(*sfx, result.GetAllocator()), result.GetAllocator());
        }
        return {STATUS_CODE::OK, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handlePut(const std::string& body) override
    {
        auto input = parseBody(body);
        auto sfxId = getString(input, "sound");
        auto createdSfx = trackSfxExchange_.add(sfxId);
        rapidjson::Document result;
        result.CopyFrom(sfxToJson(*createdSfx, result.GetAllocator()), result.GetAllocator());
        return {STATUS_CODE::CREATED, toString(result)};
    }
    std::pair<STATUS_CODE, std::string> handleDelete(const std::string& id) override
    {
        return removeElement(trackSfxExchange_, std::stoull(id), &mutex_);
    }

private:
    static rapidjson::Value sfxToJson(const SfxSoundWrapper& soundBase, rapidjson::Document::AllocatorType& alloc)
    {
        rapidjson::Value sound;
        sound.SetObject();
        sound.AddMember("sound", rapidjson::Value().SetString(soundBase.getSound().c_str(), alloc), alloc);
        sound.AddMember("status", rapidjson::Value().SetString(soundBase.getStatus().c_str(), alloc), alloc);
        sound.AddMember("loop", soundBase.inLoop(), alloc);
        sound.AddMember("volume", soundBase.getVolume(), alloc);
        sound.AddMember("pitch", soundBase.getPitch(), alloc);
        sound.AddMember("play-time", soundBase.getPlayTime(), alloc);
        rapidjson::Value positionValue;
        if (const auto& position = soundBase.getPosition())
        {
            positionValue = vectorToJson(position.value(), alloc);
        }
        sound.AddMember("position", positionValue, alloc);
        return sound;
    }
    std::unique_ptr<SfxSoundWrapper> getSfxById(const std::string& parameter)
    {
        auto id = parseString(parameter);
        if (!id)
            return nullptr;
        auto sounds = trackSfxExchange_.getAllSounds();
        if (id.value() >= sounds.size())
            return nullptr;
        return std::move(sounds[id.value()]);
    }

private:
    RaceSfxExchange& trackSfxExchange_;
    std::mutex& mutex_;
};
// ---------------------------------------------------------------------------------------------------------------------
class RaceWeatherHandler final : public Handler
{
public:
    explicit RaceWeatherHandler(const RaceWeatherExchange& weatherDataExchange)
    : weatherExchange_(weatherDataExchange)
    {
    }

    std::pair<STATUS_CODE, std::string> handleGet() override
    {
        return std::make_pair(STATUS_CODE::OK, currentWeatherToJson());
    }

    std::pair<STATUS_CODE, std::string> handlePost(const std::string& rawBody) override
    {
        rapidjson::Document body = parseBody(rawBody);
        if (!body.IsObject())
        {
            throw std::invalid_argument("Body must be Object");
        }
        const Weather& weather = getCurrentWeather();
        WeatherData weatherData = {
            weather.getSkyColor().color,
            weather.getSound(),
            weather.getParticles() ? weather.getParticles()->getName() : "",
            weather.getLightning()
        };
        auto sky_color = body.FindMember("sky-color");
        if (sky_color != body.MemberEnd())
        {
            if (!sky_color->value.IsUint())
            {
                throw std::invalid_argument("sky-color must be Uint");
            }
            weatherData.skyColorAsARGB = sky_color->value.GetUint();
        }
        auto sound = body.FindMember("sound");
        if (sound != body.MemberEnd())
        {
            if (!sound->value.IsString())
            {
                throw std::invalid_argument("sound must be string");
            }
            weatherData.sound = sound->value.GetString();
        }
        auto particles = body.FindMember("particles");
        if (particles != body.MemberEnd())
        {
            if (!particles->value.IsString())
            {
                throw std::invalid_argument("particles must be string");
            }
            weatherData.particles = particles->value.GetString();
        }
        auto lightning = body.FindMember("lightning");
        if (lightning != body.MemberEnd())
        {
            if (!lightning->value.IsBool())
            {
                throw std::invalid_argument("lightning must be boolean");
            }
            weatherData.lightning = lightning->value.GetBool();
        }
        Weather::changeCurrentWeather(weatherData);
        return std::make_pair(STATUS_CODE::OK, currentWeatherToJson());
    }

private:
    std::string currentWeatherToJson()
    {
        rapidjson::Document result;
        auto& alloc = result.GetAllocator();
        result.SetObject();
        result.AddMember("sky-color", weatherExchange_.getSkyColor(), alloc);
        rapidjson::Value soundValue;
        if (const auto& sound = weatherExchange_.getSound())
        {
            soundValue.SetString(weatherExchange_.getSound()->c_str(), alloc);
        }
        result.AddMember("sound", soundValue, alloc);
        result.AddMember("particles", particleKindToJson(weatherExchange_.getParticles(), alloc), alloc);
        result.AddMember("lightning", weatherExchange_.getLightning(), alloc);
        return toString(result);
    }

    static const Weather& getCurrentWeather()
    {
        Weather* weather = Weather::getInstance();
        if (!weather)
        {
            throw std::runtime_error("Weather does not exist!");
        }
        return *weather;
    }

private:
    const RaceWeatherExchange& weatherExchange_;
};
// ---------------------------------------------------------------------------------------------------------------------
}
// ---------------------------------------------------------------------------------------------------------------------
// Game resources
// ---------------------------------------------------------------------------------------------------------------------
std::unique_ptr<Handler> Handler::createRaceHandler(RaceExchange& raceExchange, const GetMutex& getMutex)
{
    return std::make_unique<RaceHandler>(raceExchange, getMutex);
}
std::unique_ptr<Handler> Handler::createKartModelHandler(KartModelExchange& raceKartExchange, const GetMutex& getMutex)
{
    return std::make_unique<KartModelHandler>(raceKartExchange, getMutex);
}
std::unique_ptr<Handler> Handler::createMusicHandler(MusicExchange& trackMusicLibraryExchange, const GetMutex& getMutex)
{
    return std::make_unique<MusicHandler>(trackMusicLibraryExchange, getMutex);
}
std::unique_ptr<Handler> Handler::createSfxHandler(SfxExchange& trackSfxTypesExchange, const GetMutex& getMutex)
{
    return std::make_unique<SfxHandler>(trackSfxTypesExchange, getMutex);
}
std::unique_ptr<Handler> Handler::createTrackModelHandler(TrackModelExchange& raceTrackExchange, const GetMutex& getMutex)
{
    return std::make_unique<TrackModelHandler>(raceTrackExchange, getMutex);
}
// ---------------------------------------------------------------------------------------------------------------------
// Race resources
// ---------------------------------------------------------------------------------------------------------------------
std::unique_ptr<Handler> Handler::createRaceBonusItemHandler(RaceBonusItemExchange& trackItemExchange, std::mutex& mutex)
{
    return std::make_unique<RaceBonusItemHandler>(trackItemExchange, mutex);
}
std::unique_ptr<Handler> Handler::createRaceChecklineHandler(const RaceChecklineExchange& checklineExchange, std::mutex& mutex)
{
    return std::make_unique<RaceChecklineHandler>(checklineExchange, mutex);
}
std::unique_ptr<Handler> Handler::createRaceKartHandler(const RaceKartExchange& trackKartExchange, std::mutex& mutex)
{
    return std::make_unique<RaceKartHandler>(trackKartExchange, mutex);
}
std::unique_ptr<Handler> Handler::createRaceMaterialHandler(const RaceMaterialExchange& trackMaterialExchange, std::mutex& mutex)
{
    return std::make_unique<RaceMaterialHandler>(trackMaterialExchange, mutex);
}
std::unique_ptr<Handler> Handler::createRaceMusicHandler(RaceMusicExchange& trackMusicExchange, std::mutex& mutex)
{
    return std::make_unique<RaceMusicHandler>(trackMusicExchange, mutex);
}
std::unique_ptr<Handler> Handler::createRaceObjectHandler(RaceObjectExchange& trackObjectExchange, std::mutex& mutex)
{
    return std::make_unique<RaceObjectHandler>(trackObjectExchange, mutex);
}
std::unique_ptr<Handler> Handler::createRaceQuadHandler(const RaceQuadExchange& quadExchange)
{
    return std::make_unique<RaceQuadHandler>(quadExchange);
}
std::unique_ptr<Handler> Handler::createRaceSfxHandler(RaceSfxExchange& trackSoundExchange, std::mutex& mutex)
{
    return std::make_unique<RaceSfxHandler>(trackSoundExchange, mutex);
}
std::unique_ptr<Handler> Handler::createRaceWeatherHandler(const RaceWeatherExchange& weatherDataExchange)
{
    return std::make_unique<RaceWeatherHandler>(weatherDataExchange);
}
// ---------------------------------------------------------------------------------------------------------------------
std::pair<STATUS_CODE, std::string> Handler::generateNotFound()
{
    rapidjson::Document result;
    rapidjson::StringBuffer stringBuffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(stringBuffer);
    result.Accept(writer);
    return std::make_pair(STATUS_CODE::NOT_FOUND, stringBuffer.GetString());
}
// ---------------------------------------------------------------------------------------------------------------------
std::pair<STATUS_CODE, std::string> Handler::handleGet()
{
    return generateNotFound();
}
std::pair<STATUS_CODE, std::string> Handler::handleGet(const std::string&)
{
    return generateNotFound();
}
std::pair<STATUS_CODE, std::string> Handler::handlePost(const std::string&)
{
    return generateNotFound();
}
std::pair<STATUS_CODE, std::string> Handler::handlePost(const std::string&, const std::string&)
{
    return generateNotFound();
}
std::pair<STATUS_CODE, std::string> Handler::handlePut(const std::string&)
{
    return generateNotFound();
}
std::pair<STATUS_CODE, std::string> Handler::handlePutZip(const std::string&)
{
    return generateNotFound();
}
std::pair<STATUS_CODE, std::string> Handler::handleDelete(const std::string&)
{
    return generateNotFound();
}
// ---------------------------------------------------------------------------------------------------------------------
}
