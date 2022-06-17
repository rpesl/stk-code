#include "audio/music_information.hpp"
#include "audio/music_manager.hpp"
#include "audio/sfx_base.hpp"
#include "audio/sfx_buffer.hpp"
#include "audio/sfx_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/particle_emitter.hpp"
#include "graphics/particle_kind.hpp"
#include "graphics/weather.hpp"
#include "io/file_manager.hpp"
#include "items/attachment.hpp"
#include "items/item.hpp"
#include "items/item_manager.hpp"
#include "items/powerup.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/abstract_kart_animation.hpp"
#include "karts/kart.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "karts/max_speed.hpp"
#include "karts/skidding.hpp"
#include "karts/controller/controller.hpp"
#include "karts/controller/kart_control.hpp"
#include "modes/world.hpp"
#include "rest-api/DataExchange.hpp"
#include "rest-api/ZipDecompressor.hpp"
#include "tracks/check_line.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/check_structure.hpp"
#include "tracks/drive_node.hpp"
#include "tracks/drive_graph.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/track_object.hpp"
#include "tracks/track_object_manager.hpp"
#include "tracks/track_object_presentation.hpp"
#include "utils/string_utils.hpp"
// ---------------------------------------------------------------------------------------------------------------------
namespace RestApi
{
namespace
{
// ---------------------------------------------------------------------------------------------------------------------
Track& getTrack()
{
    Track* track = Track::getCurrentTrack();
    if (!track)
    {
        throw std::runtime_error("Track does not exist");
    }
    return *track;
}
// ---------------------------------------------------------------------------------------------------------------------
std::array<float, 3> vec3ToArray(const btVector3& vector) noexcept
{
    return {vector.getX(), vector.getZ(), vector.getY()};
}
// ---------------------------------------------------------------------------------------------------------------------
Vec3 arrayToVec(const Vector& vector)
{
    return {vector[0], vector[2], vector[1]};
}
// ---------------------------------------------------------------------------------------------------------------------
std::array<float, 3> vec3ToArray(const vector3d<f32>& vector) noexcept
{
    return {vector.X, vector.Z, vector.Y};
}
// ---------------------------------------------------------------------------------------------------------------------
vector3d<f32> arrayToVec3D(const Vector& vector)
{
    return {vector[0], vector[2], vector[1]};
}
// ---------------------------------------------------------------------------------------------------------------------
ItemState::ItemType stringToItemType(const std::string& type)
{
    if (type == "BONUS_BOX")
        return ItemState::ITEM_BONUS_BOX;
    if (type == "BANANA")
        return ItemState::ITEM_BANANA;
    if (type == "NITRO_BIG")
        return ItemState::ITEM_NITRO_BIG;
    if (type == "NITRO_SMALL")
        return ItemState::ITEM_NITRO_SMALL;
    if (type == "BUBBLEGUM")
        return ItemState::ITEM_BUBBLEGUM;
    if (type == "BUBBLEGUM_NOLOK")
        return ItemState::ITEM_BUBBLEGUM_NOLOK;
    if (type == "EASTER_EGG")
        return ItemState::ITEM_EASTER_EGG;
    throw std::runtime_error("Invalid item type!");
}
// ---------------------------------------------------------------------------------------------------------------------
World& getWorld()
{
    World* world = World::getWorld();
    if (!world)
    {
        throw std::runtime_error("World does not exist");
    }
    return *world;
}
// ---------------------------------------------------------------------------------------------------------------------
std::optional<std::string> stringToOptional(const std::string& value)
{
    return value.empty() ? std::nullopt : std::optional<std::string>(value);
}
// ---------------------------------------------------------------------------------------------------------------------
KartPropertiesManager& getKartManager()
{
    KartPropertiesManager* kartPropertiesManager = kart_properties_manager;
    if (!kartPropertiesManager)
    {
        throw std::runtime_error("KartPropertiesManager does not exist");
    }
    return *kartPropertiesManager;
}
// ---------------------------------------------------------------------------------------------------------------------
constexpr const char* getMaxSpeedIncreaseName(unsigned int id)
{
    switch (id)
    {
        case MaxSpeed::MS_INCREASE_ZIPPER:
            return "ZIPPER";
        case MaxSpeed::MS_INCREASE_SLIPSTREAM:
            return "SLIPSTREAM";
        case MaxSpeed::MS_INCREASE_NITRO:
            return "NITRO";
        case MaxSpeed::MS_INCREASE_RUBBER:
            return "RUBBER";
        case MaxSpeed::MS_INCREASE_SKIDDING:
            return "SKIDDING";
        case MaxSpeed::MS_INCREASE_RED_SKIDDING:
            return "RED_SKIDDING";
    }
    throw std::runtime_error("Unknown speed increase");
}
// ---------------------------------------------------------------------------------------------------------------------
template<int ID>
SpeedIncrease getSpeedIncrease(const MaxSpeed& maxSpeed)
{
    return {
        getMaxSpeedIncreaseName(ID),
        maxSpeed.isIncreaseActive<ID>(),
        maxSpeed.getIncreaseTimeLeft<ID>(),
        maxSpeed.getSpeedIncrease<ID>(),
        maxSpeed.getEngineForceIncrease<ID>()
    };
}
// ---------------------------------------------------------------------------------------------------------------------
template<int ID>
void getSpeedIncreaseFor(const MaxSpeed& maxSpeed, std::vector<SpeedIncrease>& result)
{
    result.emplace_back(getSpeedIncrease<ID>(maxSpeed));
    getSpeedIncreaseFor<ID + 1>(maxSpeed, result);
}
// ---------------------------------------------------------------------------------------------------------------------
template<>
void getSpeedIncreaseFor<MaxSpeed::MS_INCREASE_MAX>(const MaxSpeed&, std::vector<SpeedIncrease>&)
{
}
// ---------------------------------------------------------------------------------------------------------------------
constexpr const char* getMaxSpeedDecreaseName(unsigned int id)
{
    switch (id)
    {
        case MaxSpeed::MS_DECREASE_TERRAIN:
            return "TERRAIN";
        case MaxSpeed::MS_DECREASE_AI:
            return "AI";
        case MaxSpeed::MS_DECREASE_BUBBLE:
            return "BUBBLE";
        case MaxSpeed::MS_DECREASE_SQUASH:
            return "SQUASH";
    }
    throw std::runtime_error("Unknown speed decrease");
}
// ---------------------------------------------------------------------------------------------------------------------
template<int ID>
static SpeedDecrease getSpeedDecrease(const MaxSpeed& maxSpeed)
{
    return {
        getMaxSpeedDecreaseName(ID),
        maxSpeed.isDecreaseActive<ID>(),
        maxSpeed.getDecreaseTimeLeft<ID>(),
        maxSpeed.getSlowdownFraction<ID>()
    };
}
// ---------------------------------------------------------------------------------------------------------------------
template<int ID>
void getSpeedDecreaseFor(const MaxSpeed& maxSpeed, std::vector<SpeedDecrease>& result)
{
    result.emplace_back(getSpeedDecrease<ID>(maxSpeed));
    getSpeedDecreaseFor<ID + 1>(maxSpeed, result);
}
// ---------------------------------------------------------------------------------------------------------------------
template<>
void getSpeedDecreaseFor<MaxSpeed::MS_DECREASE_MAX>(const MaxSpeed&, std::vector<SpeedDecrease>&)
{
}
// ---------------------------------------------------------------------------------------------------------------------
std::optional<std::string> materialToId(const Material* material)
{
    return material ? std::optional<std::string>(material->getTexFname()) : std::nullopt;
}
// ---------------------------------------------------------------------------------------------------------------------
template<typename ExchangeType, typename PresentationType>
std::vector<std::unique_ptr<ExchangeType>> getTrackObjects(const Track& track, const std::string& type)
{
    std::vector<std::unique_ptr<ExchangeType>> result;
    TrackObjectManager* trackObjectManager = track.getTrackObjectManager();
    PtrVector<TrackObject>& trackObjects = trackObjectManager->getObjects();
    for (TrackObject* trackObject : trackObjects)
    {
        if (trackObject && trackObject->getType() == type)
        {
            PresentationType* presentation = trackObject->getPresentation<PresentationType>();
            if (!presentation)
            {
                throw std::runtime_error("Cannot cast to presentation type");
            }
            result.push_back(ExchangeType::create(*presentation));
        }
    }
    return result;
}
// ---------------------------------------------------------------------------------------------------------------------
MusicManager& getMusicManager()
{
    MusicManager* musicManager = music_manager;
    if (!musicManager)
    {
        throw std::runtime_error("MusicManager does not exist");
    }
    return *musicManager;
}
// ---------------------------------------------------------------------------------------------------------------------
MusicWrapper toMusicExchange(const MusicInformation& music)
{
    const auto& fastFilename = music.getFastFilename();
    return {
        music.getBasename(),
        StringUtils::wideToUtf8(music.getTitle()),
        StringUtils::wideToUtf8(music.getComposer()),
        music.getNormalFilename(),
        fastFilename.empty() ? std::nullopt : std::optional<std::string>(fastFilename)
    };
}
// ---------------------------------------------------------------------------------------------------------------------
SFXManager& getSfxManager()
{
    SFXManager* sfxManager = SFXManager::get();
    if (!sfxManager)
    {
        throw std::runtime_error("SFXManager does note exist");
    }
    return *sfxManager;
}
// ---------------------------------------------------------------------------------------------------------------------
void removeDirectoryIfExists(const std::filesystem::path& directory)
{
    if (std::filesystem::exists(directory))
        std::filesystem::remove_all(directory);
}
// ---------------------------------------------------------------------------------------------------------------------
void removeAddonDirectory(const std::filesystem::path& directory, const std::string& id)
{
    auto path = directory / id.substr(6);
    removeDirectoryIfExists(path);
}
// ---------------------------------------------------------------------------------------------------------------------
SfxWrapper toSoundBuffer(const SFXBuffer& sfxBuffer)
{
    const std::string& name = sfxBuffer.getName();
    std::string file = std::filesystem::weakly_canonical(sfxBuffer.getFileName()).string();
    bool loaded = sfxBuffer.isLoaded();
    bool positional = sfxBuffer.isPositional();
    float rollOff = sfxBuffer.getRolloff();
    float volume = sfxBuffer.getGain();
    float maxDistance = sfxBuffer.getMaxDist();
    float duration = sfxBuffer.getDuration();
    return SfxWrapper {
        name,
        file,
        loaded,
        positional,
        rollOff,
        volume,
        maxDistance,
        duration >= 0 ? std::optional<float>(duration) : std::nullopt
    };
}
// ---------------------------------------------------------------------------------------------------------------------
// Game resources
// ---------------------------------------------------------------------------------------------------------------------
class GameRaceExchange final : public RaceExchange
{
public:
    explicit GameRaceExchange(RaceManager& raceManager)
    : raceManager_(raceManager)
    {
    }
    [[nodiscard]]
    std::optional<size_t> getId() const override
    {
        return raceManager_.getCurrentRaceId();
    }
    [[nodiscard]]
    std::string getStatus() const override
    {
        if (isActive())
        {
            return worldPhaseToString(World::getWorld()->getPhase());
        }
        return "NONE";
    }
    [[nodiscard]]
    bool isActive() const override
    {
        return World::getWorld() && Track::getCurrentTrack();
    }
    [[nodiscard]]
    std::optional<std::string> getTrackName() const override
    {
        Track* track = Track::getCurrentTrack();
        if (track)
        {
            return track->getIdent();
        }
        return std::nullopt;
    }
    [[nodiscard]]
    std::optional<std::string> getMajorRaceMode() const override
    {
        if (isActive())
        {
            return majorRaceModeToString(raceManager_.getMajorMode());
        }
        return std::nullopt;
    }
    [[nodiscard]]
    std::optional<std::string> getMinorRaceMode() const override
    {
        if (isActive())
        {
            return minorRaceModeToString(raceManager_.getMinorMode());
        }
        return std::nullopt;
    }
    [[nodiscard]]
    std::optional<std::string> getDifficulty() const override
    {
        if (isActive())
        {
            return difficultyToString(raceManager_.getDifficulty());
        }
        return std::nullopt;
    }
    [[nodiscard]]
    std::optional<std::string> getClockType() const override
    {
        if (isActive())
        {
            switch (World::getWorld()->getClockMode())
            {
                case WorldStatus::CLOCK_NONE:
                    return "NONE";
                case WorldStatus::CLOCK_CHRONO:
                    return "CHRONO";
                case WorldStatus::CLOCK_COUNTDOWN:
                    return "COUNTDOWN";
            }
        }
        return std::nullopt;
    }
    [[nodiscard]]
    std::optional<float> getTime() const override
    {
        if (isActive())
        {
            return World::getWorld()->getTime();
        }
        return std::nullopt;
    }
    [[nodiscard]]
    std::optional<std::string> getRaceResults(size_t raceId) const override
    {
        auto& loader = raceManager_.getRaceResultLoader();
        auto latestId = loader.getLatestId();
        if (latestId && latestId.value() >= raceId)
        {
            return raceManager_.getRaceResultLoader().get(raceId);
        }
        return std::nullopt;
    }
    void start(const NewRace& newRace) override
    {
        if (isActive())
        {
            stop();
        }
        raceManager_.m_start_new = std::make_unique<NewRace>(newRace);
        raceManager_.m_finished = false;
        std::unique_lock<std::mutex> lock(raceManager_.m_mutex);
        raceManager_.m_cv.wait(lock, [&] () -> bool {
            bool resume = World::getWorld();
            if (!resume)
            {
                raceManager_.m_finished = false;
            }
            return resume;
        });
    }
    void stop() override
    {
        if (isActive())
        {
            raceManager_.m_rest_exit = true;
            raceManager_.m_finished = false;
            std::unique_lock<std::mutex> lock(raceManager_.m_mutex);
            raceManager_.m_cv.wait(lock, [&] () -> bool {
                bool resume = !World::getWorld();
                if (!resume)
                {
                    raceManager_.m_finished = false;
                }
                return resume;
            });
        }
    }
    void pause() override
    {
        if (isActive() && getStatus() != "PAUSE")
        {
            getWorld().m_rest_pause = true;
            getWorld().m_finished = false;
            std::unique_lock<std::mutex> lock(getWorld().m_mutex);
            getWorld().m_cv.wait(lock, [&] () -> bool {
                bool resume = getWorld().getPhase() == WorldStatus::IN_GAME_MENU_PHASE;
                if (!resume)
                {
                    getWorld().m_finished = false;
                }
                return resume;
            });
        }
    }
    void resume() override
    {
        if (isActive() && getStatus() == "PAUSE")
        {
            std::unique_lock<std::mutex> lock(getWorld().m_mutex);
            getWorld().m_rest_resume = true;
            getWorld().m_finished = false;
            getWorld().m_cv.wait(lock, [&] () -> bool {
                bool resume = getWorld().getPhase() != WorldStatus::IN_GAME_MENU_PHASE;
                if (!resume)
                {
                    getWorld().m_finished = false;
                }
                return resume;
            });
        }
    }

private:
    static std::string worldPhaseToString(World::Phase phase)
    {
        switch (phase)
        {
            case WorldStatus::TRACK_INTRO_PHASE:
                return "INTRO";
            case WorldStatus::SETUP_PHASE:
                return "SETUP";
            case WorldStatus::WAIT_FOR_SERVER_PHASE:
                return "WAIT_FOR_SERVER";
            case WorldStatus::SERVER_READY_PHASE:
                return "SERVER_READY";
            case WorldStatus::READY_PHASE:
                return "READY";
            case WorldStatus::SET_PHASE:
                return "SET";
            case WorldStatus::GO_PHASE:
                return "GO";
            case WorldStatus::MUSIC_PHASE:
                return "MUSIC";
            case WorldStatus::RACE_PHASE:
                return "RACE";
            case WorldStatus::DELAY_FINISH_PHASE:
                return "DELAY_FINISH";
            case WorldStatus::RESULT_DISPLAY_PHASE:
                return "RESULT_DISPLAY";
            case WorldStatus::FINISH_PHASE:
                return "FINISH";
            case WorldStatus::IN_GAME_MENU_PHASE:
                return "PAUSE";
            case WorldStatus::UNDEFINED_PHASE:
                break;
        }
        throw std::runtime_error("Unknown race phase");
    }
    static std::string majorRaceModeToString(RaceManager::MajorRaceModeType majorRaceMode)
    {
        switch (majorRaceMode)
        {
            case RaceManager::MAJOR_MODE_GRAND_PRIX:
                return "GRAND_PRIX";
            case RaceManager::MAJOR_MODE_SINGLE:
                return "SINGLE";
        }
        throw std::runtime_error("Unknown MajorRaceModeType");
    }
    static std::string minorRaceModeToString(RaceManager::MinorRaceModeType minorRaceMode)
    {
        switch (minorRaceMode)
        {
            case RaceManager::MINOR_MODE_NONE:
                return "NONE";
            case RaceManager::MINOR_MODE_NORMAL_RACE:
                return "NORMAL_RACE";
            case RaceManager::MINOR_MODE_TIME_TRIAL:
                return "TIME_TRIAL";
            case RaceManager::MINOR_MODE_FOLLOW_LEADER:
                return "FOLLOW_LEADER";
            case RaceManager::MINOR_MODE_3_STRIKES:
                return "3_STRIKES";
            case RaceManager::MINOR_MODE_FREE_FOR_ALL:
                return "FREE_FOR_ALL";
            case RaceManager::MINOR_MODE_CAPTURE_THE_FLAG:
                return "CAPTURE_THE_FLAG";
            case RaceManager::MINOR_MODE_SOCCER:
                return "SOCCER";
            case RaceManager::MINOR_MODE_EASTER_EGG:
                return "EASTER_EGG";
            case RaceManager::MINOR_MODE_OVERWORLD:
                return "OVERWORLD";
            case RaceManager::MINOR_MODE_TUTORIAL:
                return "TUTORIAL";
            case RaceManager::MINOR_MODE_CUTSCENE:
                return "CUTSCENE";
            default:
                throw std::runtime_error("Unknown MinorRaceModeType");
        }
    }
    static std::string difficultyToString(RaceManager::Difficulty difficulty)
    {
        switch (difficulty)
        {
            case RaceManager::DIFFICULTY_EASY:
                return "NOVICE";
            case RaceManager::DIFFICULTY_MEDIUM:
                return "INTERMEDIATE";
            case RaceManager::DIFFICULTY_HARD:
                return "EXPERT";
            case RaceManager::DIFFICULTY_BEST:
                return "SUPER_TUX";
            case RaceManager::DIFFICULTY_COUNT:
            case RaceManager::DIFFICULTY_NONE:
                break;
        }
        throw std::runtime_error("Unknown race difficulty");
    }

private:
    RaceManager& raceManager_;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameKartModelExchange final : public KartModelExchange
{
public:
    explicit GameKartModelExchange(RaceManager& raceManager)
    : kartManager_(getKartManager())
    , raceManager_(raceManager)
    {
    }
    [[nodiscard]]
    std::vector<KartModelWrapper> getAvailableKarts() const override
    {
        std::vector<KartModelWrapper> result;
        int size = static_cast<int>(kartManager_.getNumberOfKarts());
        result.reserve(size);
        for (int i = 0; i < size; i++)
        {
            const KartProperties* kart = kartManager_.getKartById(i);
            KartModelWrapper kartCharacteristics{};
            kartCharacteristics.id = kart->getIdent();
            kartCharacteristics.name = StringUtils::wideToUtf8(kart->getName());
            kartCharacteristics.mass = kart->getMass();
            kartCharacteristics.engineMaxSpeed = kart->getEngineMaxSpeed();
            kartCharacteristics.accelerationEfficiency = kart->getAccelerationEfficiency();
            kartCharacteristics.nitroConsumption = kart->getNitroConsumption();
            result.emplace_back(std::move(kartCharacteristics));
        }
        return result;
    }
    [[nodiscard]]
    std::filesystem::path getDirectory() const override
    {
        return file_manager->getAddonsFile("karts");
    }
    [[nodiscard]]
    std::string loadNewKart(const std::filesystem::path& path) override
    {
        raceManager_.m_load_kart = std::make_unique<std::string>(path.string());
        raceManager_.m_finished = false;
        std::optional<std::string> result;
        std::unique_lock<std::mutex> lock(raceManager_.m_mutex);
        raceManager_.m_cv.wait(lock, [&] () -> bool {
            if(raceManager_.m_load_kart_result)
            {
                result = std::move(*raceManager_.m_load_kart_result);
                raceManager_.m_load_kart_result = nullptr;
                return true;
            }
            return false;
        });
        if (!result)
        {
            removeDirectoryIfExists(path);
            throw std::invalid_argument("Cannot load kart");
        }
        return result.value();
    }
    void remove(const std::string& id) override
    {
        std::unique_lock<std::mutex> lock(raceManager_.m_mutex);
        const KartProperties* kart = kartManager_.getKart(id);
        if (kart)
        {
            if (!kart->isAddon())
                throw std::invalid_argument("Cannot delete non-addon kart");
            World* world = World::getWorld();
            if (world)
            {
                const auto& karts = world->getKarts();
                auto kartInWorld = std::find_if(karts.begin(), karts.end(), [&id](const auto& kart) { return kart->getIdent() == id;});
                if (kartInWorld != karts.end())
                    throw std::invalid_argument("Kart is in use");
            }
            raceManager_.m_finished = false;
            raceManager_.m_delete_kart = std::make_unique<std::string>(id);
            raceManager_.m_cv.wait(lock, [&] () -> bool {
                return !raceManager_.m_delete_kart;
            });
            removeAddonDirectory(getDirectory(), id);
        }
    }

private:
    KartPropertiesManager& kartManager_;
    RaceManager& raceManager_;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameMusicExchange final : public MusicExchange
{
public:
    GameMusicExchange()
    : musicManager_(getMusicManager())
    {
    }
    [[nodiscard]]
    std::vector<MusicWrapper> getAllMusic() const override
    {
        std::vector<MusicWrapper> result;
        const auto& music = musicManager_.getAllMusic();
        result.reserve(music.size());
        for (const auto& [id, file] : music)
        {
            assert(file);
            result.push_back(toMusicExchange(*file));
        }
        return result;
    }
    [[nodiscard]]
    std::filesystem::path getDirectory() const override
    {
        return file_manager->getAddonMusicDirectory();
    }
    [[nodiscard]]
    std::string loadNewMusic(const std::filesystem::path& path) override
    {
        try
        {
            MusicInformation& music = musicManager_.loadAddonMusic(path);
            return music.getBasename();
        }
        catch (const std::invalid_argument& exception)
        {
            removeDirectoryIfExists(path);
            throw;
        }
    }
    void remove(const std::string& id) override
    {
        musicManager_.remove(id);
    }

private:
    MusicManager& musicManager_;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameSfxExchange final : public SfxExchange
{
public:
    GameSfxExchange()
    : sfxManager_(getSfxManager())
    {
    }
    [[nodiscard]]
    std::vector<SfxWrapper> getSounds() const override
    {
        std::vector<SfxWrapper> result;
        const std::map<std::string, SFXBuffer*>& sounds = sfxManager_.getAllSfxTypes();
        result.reserve(sounds.size());
        for (const auto& [name, buffer] : sounds)
        {
            assert(buffer);
            result.push_back(toSoundBuffer(*buffer));
        }
        return result;
    }
    [[nodiscard]]
    std::filesystem::path getDirectory() const override
    {
        return file_manager->getAddonSfxDirectory();
    }
    [[nodiscard]]
    std::string loadNewSfx(const std::filesystem::path& path) override
    {
        try
        {
            SFXBuffer* sfx = sfxManager_.loadSingleSfxNow(path);
            if (!sfx)
                throw std::invalid_argument("Cannot load sfx file");
            return sfx->getName();
        }
        catch (...)
        {
            removeDirectoryIfExists(path);
            throw;
        }
    }
    void remove(const std::string& id) override
    {
        if (sfxManager_.soundExist(id))
        {
            SFXBuffer* sfx = sfxManager_.getBuffer(id);
            assert(sfx);
            std::filesystem::path path = sfx->getFileName();
            sfxManager_.deleteSFXMapping(id);
            if (std::filesystem::exists(path) && path.parent_path().parent_path() == file_manager->getAddonSfxDirectory())
                std::filesystem::remove_all(path.parent_path());
        }
    }

private:
    SFXManager& sfxManager_;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameTrackModelExchange final : public TrackModelExchange
{
public:
    GameTrackModelExchange()
    : trackManager_(getTrackManager())
    {
    }
    [[nodiscard]]
    std::vector<TrackModelWrapper> getAvailableTracks() const override
    {
        std::vector<TrackModelWrapper> result;
        size_t size = trackManager_.getNumberOfTracks();
        result.reserve(size);
        for (size_t i = 0; i < size; i++)
        {
            Track* track = trackManager_.getTrack(i);
            assert(track);
            TrackModelWrapper trackCharacteristics{};
            trackCharacteristics.name = StringUtils::wideToUtf8(track->getName());
            trackCharacteristics.id = track->getIdent();
            trackCharacteristics.groups = track->getGroups();
            trackCharacteristics.race = track->isRaceTrack();
            trackCharacteristics.soccer = track->isSoccer();
            trackCharacteristics.arena = track->isArena();
            result.emplace_back(std::move(trackCharacteristics));
        }
        return result;
    }
    [[nodiscard]]
    std::filesystem::path getDirectory() const override
    {
        return file_manager->getAddonsFile("tracks");
    }
    [[nodiscard]]
    std::string loadNewTrack(const std::filesystem::path& path) override
    {
        if (!trackManager_.loadTrack((path / "").string()))
        {
            removeDirectoryIfExists(path);
            throw std::invalid_argument("Cannot load track");
        }
        const auto* newTrack = trackManager_.getTrack(trackManager_.getNumberOfTracks() - 1);
        return newTrack->getIdent();
    }
    void remove(const std::string& id) override
    {
        const Track* track = trackManager_.getTrack(id);
        if (track)
        {
            if (!track->isAddon())
                throw std::invalid_argument("Cannot delete non-addon track");
            const Track* currentTrack = Track::getCurrentTrack();
            if (currentTrack && currentTrack->getIdent() == id)
                throw std::invalid_argument("Track is in use");
            trackManager_.removeTrack(id);
            removeAddonDirectory(getDirectory(), id);
        }
    }

private:
    static TrackManager& getTrackManager()
    {
        TrackManager* trackManager = track_manager;
        if (!trackManager)
        {
            throw std::runtime_error("TrackManager does not exist");
        }
        return *trackManager;
    }

private:
    TrackManager& trackManager_;
};
// ---------------------------------------------------------------------------------------------------------------------
// Race resources
// ---------------------------------------------------------------------------------------------------------------------
class GameBonusItemWrapper final : public BonusItemWrapper
{
public:
    static bool isValidType(ItemState::ItemType type)
    {
        return type == ItemState::ITEM_BONUS_BOX
               || type == ItemState::ITEM_BANANA
               || type == ItemState::ITEM_NITRO_BIG
               || type == ItemState::ITEM_NITRO_SMALL
               || type == ItemState::ITEM_BUBBLEGUM
               || type == ItemState::ITEM_BUBBLEGUM_NOLOK
               || type == ItemState::ITEM_EASTER_EGG;
    }

public:
    explicit GameBonusItemWrapper(ItemState& state)
    : m_state(state)
    {
    }
    [[nodiscard]]
    size_t getId() const override
    {
        return m_state.getItemId();
    }
    [[nodiscard]]
    Position getPosition() const override
    {
        return vec3ToArray(m_state.getXYZ());
    }
    [[nodiscard]]
    std::string getType() const override
    {
        return itemTypeToString(m_state.getType());
    }
    [[nodiscard]]
    std::optional<std::string> getOriginalType() const override
    {
        return m_state.getOriginalType() == ItemState::ITEM_NONE
               ? std::nullopt
               : std::optional<std::string>(itemTypeToString(m_state.getOriginalType()));
    }
    [[nodiscard]]
    int getTicksUntilReturn() const override
    {
        return m_state.getTicksTillReturn();
    }
    [[nodiscard]]
    std::optional<int> getUsedUpCounter() const override
    {
        int usedUpCounter = m_state.getUsedUpCounter();
        return usedUpCounter == -1 ? std::nullopt : std::optional<int>(usedUpCounter);
    }
    void setType(const std::string& type) override
    {
        m_state.setType(stringToItemType(type));
    }
    void setOriginalType(const std::optional<std::string>& type) override
    {
        m_state.setOriginalType(type ? stringToItemType(type.value()) : ItemState::ITEM_NONE);
    }
    void setTicksUntilReturn(int ticks) override
    {
        m_state.setTicksTillReturn(ticks);
    }
    void setUsedUpCounter(std::optional<int> usedUpCounter) override
    {
        m_state.setUsedUpCounter(usedUpCounter ? usedUpCounter.value() : -1);
    }

private:
    static std::string itemTypeToString(ItemState::ItemType type)
    {
        switch (type)
        {
            case ItemState::ITEM_BONUS_BOX:
                return "BONUS_BOX";
            case ItemState::ITEM_BANANA:
                return "BANANA";
            case ItemState::ITEM_NITRO_BIG:
                return "NITRO_BIG";
            case ItemState::ITEM_NITRO_SMALL:
                return "NITRO_SMALL";
            case ItemState::ITEM_BUBBLEGUM:
                return "BUBBLEGUM";
            case ItemState::ITEM_BUBBLEGUM_NOLOK:
                return "BUBBLEGUM_NOLOK";
            case ItemState::ITEM_EASTER_EGG:
                return "EASTER_EGG";
            case ItemState::ITEM_COUNT:
                [[fallthrough]];
            case ItemState::ITEM_NONE:
                break;
        }
        throw std::runtime_error("Invalid item type!");
    }

private:
    ItemState& m_state;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameRaceBonusItemExchange final : public RaceBonusItemExchange
{
public:
    GameRaceBonusItemExchange()
    : itemManager_(getItemManager())
    {
    }
    [[nodiscard]]
    std::vector<std::unique_ptr<BonusItemWrapper>> getItems() const override
    {
        unsigned numberOfItems = itemManager_.getNumberOfItems();
        auto result = std::vector<std::unique_ptr<BonusItemWrapper>>();
        result.reserve(numberOfItems);
        for (unsigned i = 0; i < numberOfItems; ++i)
        {
            ItemState* item = itemManager_.getItem(i);
            std::unique_ptr<BonusItemWrapper> itemExchange;
            if (item && GameBonusItemWrapper::isValidType(item->getType()))
            {
                itemExchange = std::make_unique<GameBonusItemWrapper>(*item);
            }
            result.push_back(std::move(itemExchange));
        }
        return result;
    }
    [[nodiscard]]
    std::unique_ptr<BonusItemWrapper> add(const NewBonusItem& data) override
    {
        Item* item = itemManager_.placeItem(
            stringToItemType(data.type),
            arrayToVec(data.position),
            arrayToVec({0, 0, 1})
        );
        return BonusItemWrapper::create(*item);
    }
    void remove(size_t index) override
    {
        if (index <= itemManager_.getNumberOfItems())
        {
            ItemState* item = itemManager_.getItem(index);
            if (item)
            {
                itemManager_.deleteItem(item);
            }
        }
    }

private:
    static ItemManager& getItemManager()
    {
        const Track& track = getTrack();
        ItemManager* item_manager = track.getItemManager();
        if (!item_manager)
        {
            throw std::runtime_error("Item manager does not exist");
        }
        return *item_manager;
    }

private:
    ItemManager& itemManager_;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameRaceChecklineExchange final : public RaceChecklineExchange
{
public:
    GameRaceChecklineExchange()
    : checkManager_(getCheckManager())
    {
    }
    [[nodiscard]]
    std::vector<ChecklineWrapper> getChecklines() const override
    {
        unsigned int size = checkManager_.getCheckStructureCount();
        std::vector<ChecklineWrapper> result;
        result.reserve(size);
        for (unsigned int i = 0; i < size; i++)
        {
            const CheckStructure* checkStructure = checkManager_.getCheckStructure(i);
            CheckLine::CheckType type = checkStructure->getType();
            if (type == CheckStructure::CT_ACTIVATE || type == CheckStructure::CT_NEW_LAP)
            {
                ChecklineWrapper resultCheckline{};
                resultCheckline.id = checkStructure->getIndex();
                resultCheckline.activeAtReset = checkStructure->isActiveAtReset();
                resultCheckline.active = checkStructure->getIsActive();
                resultCheckline.sameGroup = checkStructure->getSameGroup();
                resultCheckline.otherIds = checkStructure->getOtherIds();
                if (type == CheckStructure::CT_ACTIVATE)
                {
                    auto currentCheckline = dynamic_cast<const CheckLine*>(checkStructure);
                    assert(currentCheckline);
                    resultCheckline.position = {vec3ToArray(currentCheckline->getLeftPoint()), vec3ToArray(currentCheckline->getRightPoint())};
                    resultCheckline.ignoreHeight = currentCheckline->shouldIgnoreHeight();
                    resultCheckline.kind = "ACTIVATE";
                }
                else
                {
                    resultCheckline.kind = "LAP";
                }
                result.push_back(resultCheckline);
            }
        }
        return result;
    }

private:
    static const CheckManager& getCheckManager()
    {
        const Track& track = getTrack();
        const CheckManager* checkManager = track.getCheckManager();
        if (!checkManager)
        {
            throw std::runtime_error("CheckManager does not exist");
        }
        return *checkManager;
    }

private:
    const CheckManager& checkManager_;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameKartWrapper final : public KartWrapper
{
public:
    explicit GameKartWrapper(Kart& kart)
    : kart_(kart)
    , properties_(*kart_.getKartProperties())
    , skidding_(*kart_.getSkidding())
    {
    }

    // Id
    [[nodiscard]]
    size_t getId() const override
    {
        return kart_.getWorldKartId();
    }

    // Rank
    [[nodiscard]]
    int getRank() const override
    {
        return kart_.getPosition();
    }

    // Controller
    [[nodiscard]]
    std::string getController() const override
    {
        const Controller* controller = kart_.getController();
        if (!controller)
        {
            throw std::runtime_error("Controller does not exist");
        }
        return controller->getControllerName();
    }

    // Characteristics
    [[nodiscard]]
    std::string getIdent() const override
    {
        return kart_.getIdent();
    }
    [[nodiscard]]
    uint32_t getColor() const override
    {
        return properties_.getColor().color;
    }
    [[nodiscard]]
    std::string getType() const override
    {
        return properties_.getKartType();
    }
    [[nodiscard]]
    std::vector<std::string> getGroups() const override
    {
        return properties_.getGroups();
    }
    [[nodiscard]]
    std::optional<std::string> getEngineSfxType() const override
    {
        return stringToOptional(properties_.getEngineSfxType());
    }
    [[nodiscard]]
    std::optional<std::string> getSkidSound() const override
    {
        return stringToOptional(properties_.getSkidSound());
    }
    [[nodiscard]]
    float getFriction() const override
    {
        return properties_.getFrictionKartFriction();
    }
    [[nodiscard]]
    float getFrictionSlip() const override
    {
        return properties_.getFrictionSlip();
    }
    [[nodiscard]]
    std::string getTerrainImpulseType() const override
    {
        switch (properties_.getTerrainImpulseType())
        {
            case KartProperties::IMPULSE_NONE:
                return "NONE";
            case KartProperties::IMPULSE_NORMAL:
                return "NORMAL";
            case KartProperties::IMPULSE_TO_DRIVELINE:
                return "TO_DRIVELINE";
        }
        throw std::runtime_error("Unknown TerrainImpulseType");
    }
    void setKart(const std::string& kart) override
    {
        if (!getKartManager().getKart(kart))
        {
            throw std::invalid_argument(std::string("Kart with id \"") + kart + "\" does not exist");
        }
        if (kart_.getIdent() != kart)
        {
            auto& world = getWorld();
            int id = static_cast<int>(kart_.getWorldKartId());
            world.m_new_kart = std::make_unique<std::pair<std::string, int>>(kart, id);
            world.m_finished = false;
            std::unique_lock<std::mutex> lock(world.m_mutex);
            world.m_cv.wait(lock, [&]() -> bool {
                const AbstractKart& newKart = *World::getWorld()->getKart(id);
                bool resume = newKart.getIdent() == kart;
                if (!resume)
                {
                    world.m_finished = false;
                }
                return resume;
            });
        }
    }

    // Speed
    [[nodiscard]]
    float getSpeed() const override
    {
        return kart_.getSpeed();
    }
    [[nodiscard]]
    float getMaxSpeed() const override
    {
        return kart_.getCurrentMaxSpeed();
    }
    [[nodiscard]]
    std::optional<float> getMinBoostSpeed() const override
    {
        float minBoostSpeed = kart_.getMaxSpeed().getMinSpeed();
        return minBoostSpeed < 0.0f ? std::nullopt : std::optional<float>(minBoostSpeed);
    }
    [[nodiscard]]
    Vector getVelocity() const override
    {
        return vec3ToArray(kart_.getVelocity());
    }
    [[nodiscard]]
    std::vector<SpeedIncrease> getSpeedIncrease() const override
    {
        const MaxSpeed& maxSpeed = kart_.getMaxSpeed();
        std::vector<SpeedIncrease> result;
        getSpeedIncreaseFor<MaxSpeed::MS_INCREASE_MIN>(maxSpeed, result);
        return result;
    }
    [[nodiscard]]
    std::vector<SpeedDecrease> getSpeedDecrease() const override
    {
        const MaxSpeed& maxSpeed = kart_.getMaxSpeed();
        std::vector<SpeedDecrease> result;
        getSpeedDecreaseFor<MaxSpeed::MS_DECREASE_MIN>(maxSpeed, result);
        return result;
    }

    // Position
    [[nodiscard]]
    Position getPosition() const override
    {
        return vec3ToArray(kart_.getXYZ());
    }
    [[nodiscard]]
    Position getFrontPosition() const override
    {
        return vec3ToArray(kart_.getFrontXYZ());
    }
    [[nodiscard]]
    bool isJumping() const override
    {
        return kart_.isJumping();
    }
    [[nodiscard]]
    bool isFlying() const override
    {
        return kart_.isFlying();
    }
    [[nodiscard]]
    bool isNearGround() const override
    {
        return kart_.isNearGround();
    }
    [[nodiscard]]
    bool isOnGround() const override
    {
        return kart_.isOnGround();
    }
    [[nodiscard]]
    float getPitch() const override
    {
        return kart_.getPitch();
    }
    [[nodiscard]]
    float getRoll() const override
    {
        return kart_.getRoll();
    }
    [[nodiscard]]
    float getLean() const override
    {
        return kart_.getLean();
    }
    [[nodiscard]]
    float getLeanMax() const override
    {
        return properties_.getLeanMax();
    }

    // Status
    [[nodiscard]]
    std::string getHandicapLevel() const override
    {
        switch (kart_.getHandicap())
        {
            case HANDICAP_NONE:
                return "NONE";
            case HANDICAP_MEDIUM:
                return "MEDIUM";
            case HANDICAP_COUNT:
                break;
        }
        throw std::runtime_error("Unknown HandicapLevel");
    }
    [[nodiscard]]
    bool isBoostedAI() const override
    {
        return kart_.getBoostAI();
    }
    [[nodiscard]]
    bool isBlockedByPlunger() const override
    {
        return kart_.getBlockedByPlungerTicks() > 0;
    }
    [[nodiscard]]
    bool isShielded() const override
    {
        return kart_.isShielded();
    }
    [[nodiscard]]
    bool isSquashed() const override
    {
        return kart_.isSquashed();
    }
    [[nodiscard]]
    bool isEliminated() const override
    {
        return kart_.isEliminated();
    }
    [[nodiscard]]
    bool isGhostKart() const override
    {
        return kart_.isGhostKart();
    }
    [[nodiscard]]
    bool isInRescue() const override
    {
        const AbstractKartAnimation* animation = kart_.getKartAnimation();
        return animation && animation->getAnimationType() == KartAnimationType::KAT_RESCUE;
    }

    // Skidding
    [[nodiscard]]
    std::optional<std::string> getSkidding() const override
    {
        switch (skidding_.getSkidState())
        {
            case Skidding::SKID_NONE:
                return std::nullopt;
            case Skidding::SKID_ACCUMULATE_LEFT:
                return "ACCUMULATE_LEFT";
            case Skidding::SKID_ACCUMULATE_RIGHT:
                return "ACCUMULATE_RIGHT";
            case Skidding::SKID_SHOW_GFX_LEFT:
                return "SHOW_GFX_LEFT";
            case Skidding::SKID_SHOW_GFX_RIGHT:
                return "SHOW_GFX_RIGHT";
            case Skidding::SKID_BREAK:
                return "BREAK";
        }
        throw std::runtime_error("Unknown SkidState");
    }
    [[nodiscard]]
    bool isSkiddingBonusReady() const override
    {
        return skidding_.getSkidBonusReady();
    }
    [[nodiscard]]
    float getSkiddingFactor() const override
    {
        return skidding_.getSkidFactor();
    }
    [[nodiscard]]
    float getMaxSkidding() const override
    {
        return properties_.getSkidMax();
    }

    // Control
    [[nodiscard]]
    float getSteer() const override
    {
        return kart_.getControls().getSteer();
    }
    [[nodiscard]]
    float getMaxSteerAngle() const override
    {
        return kart_.getMaxSteerAngle();
    }
    [[nodiscard]]
    float getAcceleration() const override
    {
        return kart_.getControls().getAccel();
    }
    [[nodiscard]]
    bool isBraking() const override
    {
        return kart_.getControls().getBrake();
    }
    [[nodiscard]]
    bool doesFire() const override
    {
        return kart_.getControls().getFire();
    }
    [[nodiscard]]
    bool doesLookBack() const override
    {
        return kart_.getControls().getLookBack();
    }
    [[nodiscard]]
    std::string getSkidControl() const override
    {
        switch (kart_.getControls().getSkidControl())
        {
            case KartControl::SC_NONE:
                return "NONE";
            case KartControl::SC_NO_DIRECTION:
                return "NO_DIRECTION";
            case KartControl::SC_LEFT:
                return "LEFT";
            case KartControl::SC_RIGHT:
                return "RIGHT";
        }
        throw std::runtime_error("Unknown skid control");
    }

    // Collision
    [[nodiscard]]
    float getCollisionImpulse() const override
    {
        return properties_.getCollisionImpulse();
    }
    [[nodiscard]]
    float getCollisionImpulseTime() const override
    {
        return properties_.getCollisionImpulseTime();
    }
    [[nodiscard]]
    float getRestitution() const override
    {
        return properties_.getRestitution(kart_.getSpeed());
    }

    // Nitro
    [[nodiscard]]
    float getCollectedEnergy() const override
    {
        return kart_.getEnergy();
    }
    [[nodiscard]]
    float getMaxNitro() const override
    {
        return properties_.getNitroMax();
    }
    [[nodiscard]]
    int8_t getMinNitroConsumption() const override
    {
        return properties_.getNitroMinConsumptionTicks();
    }
    [[nodiscard]]
    float getConsumptionPerTick() const override
    {
        return kart_.getNitroConsumptionPerTick();
    }
    [[nodiscard]]
    bool hasNitroActivated() const override
    {
        return kart_.getControls().getNitro();
    }

    // Attachment
    [[nodiscard]]
    std::optional<Attachment> getAttachment() const override
    {
        const auto* attachment = kart_.getAttachment();
        if (attachment && attachment->getType() != ::Attachment::ATTACH_NOTHING)
        {
            auto type = attachmentTypeToString(attachment->getType());
            int ticks = attachment->getTicksLeft();
            return Attachment{type, ticks};
        }
        return std::nullopt;
    }
    void setAttachment(const std::optional<Attachment>& inputAttachment) override
    {
        auto* attachment = kart_.getAttachment();
        attachment->clear();
        attachment->set(toAttachmentType(inputAttachment), inputAttachment ? inputAttachment->ticks : 0);
    }

    // Power-Up
    [[nodiscard]]
    std::optional<PowerUp> getPowerUp() const override
    {
        std::optional<std::string> name = getPowerUpName();
        if (name)
        {
            return PowerUp{name.value(), kart_.getNumPowerup()};
        }
        return std::nullopt;
    }
    void setPowerUp(const std::optional<PowerUp>& inputPowerUp) override
    {
        auto* powerUp = kart_.getPowerup();
        powerUp->reset();
        powerUp->set(toPowerUpType(inputPowerUp), inputPowerUp ? inputPowerUp->count : 0);
    }

    // Icon
    [[nodiscard]]
    std::optional<std::string> getIcon() const override
    {
        return materialToId(properties_.getIconMaterial());
    }

    // Minimap
    [[nodiscard]]
    std::string getMinimapIconPath() const override
    {
        return properties_.getMinimapIcon()->getName().getPath().c_str();
    }

    // Shadow
    [[nodiscard]]
    std::optional<std::string> getShadowMaterial() const override
    {
        return materialToId(properties_.getShadowMaterial());
    }

    // Ground
    [[nodiscard]]
    std::optional<std::string> getGround() const override
    {
        return materialToId(kart_.getMaterial());
    }

    // Result
    [[nodiscard]]
    std::optional<float> getFinishTime() const override
    {
        return kart_.hasFinishedRace() ? std::optional<float>(kart_.getFinishTime()) : std::nullopt;
    }

private:
    [[nodiscard]]
    static std::string attachmentTypeToString(::Attachment::AttachmentType type)
    {
        switch (type)
        {
            case ::Attachment::ATTACH_PARACHUTE:
                return "PARACHUTE";
            case ::Attachment::ATTACH_ANVIL:
                return "ANVIL";
            case ::Attachment::ATTACH_BOMB:
                return "BOMB";
            case ::Attachment::ATTACH_SWATTER:
                return "SWATTER";
            case ::Attachment::ATTACH_NOLOKS_SWATTER:
                return "NOLOKS_SWATTER";
            case ::Attachment::ATTACH_SWATTER_ANIM:
                return "SWATTER_ANIM";
            case ::Attachment::ATTACH_BUBBLEGUM_SHIELD:
                return "BUBBLEGUM_SHIELD";
            case ::Attachment::ATTACH_NOLOK_BUBBLEGUM_SHIELD:
                return "NOLOK_BUBBLEGUM_SHIELD";
            case ::Attachment::ATTACH_NOTHING:
            case ::Attachment::ATTACH_MAX:
                break;
        }
        throw std::runtime_error("Unknown attachment type");
    }
    static ::Attachment::AttachmentType toAttachmentType(const std::optional<Attachment>& attachment)
    {
        if (attachment)
        {
            const auto& type = attachment->type;
            if (type == "PARACHUTE")
                return ::Attachment::ATTACH_PARACHUTE;
            if (type == "ANVIL")
                return ::Attachment::ATTACH_ANVIL;
            if (type == "BOMB")
                return ::Attachment::ATTACH_BOMB;
            if (type == "SWATTER")
                return ::Attachment::ATTACH_SWATTER;
            if (type == "BUBBLEGUM_SHIELD")
                return ::Attachment::ATTACH_BUBBLEGUM_SHIELD;
            if (type == "NOLOK_BUBBLEGUM_SHIELD")
                return ::Attachment::ATTACH_NOLOK_BUBBLEGUM_SHIELD;
            throw std::invalid_argument(std::string("Invalid attachment type \"" + type + "\""));
        }
        return ::Attachment::ATTACH_NOTHING;
    }
    [[nodiscard]]
    std::optional<std::string> getPowerUpName() const
    {
        switch (kart_.getPowerup()->getType())
        {
            case PowerupManager::POWERUP_NOTHING:
                return std::nullopt;
            case PowerupManager::POWERUP_BUBBLEGUM:
                return "BUBBLEGUM";
            case PowerupManager::POWERUP_CAKE:
                return "CAKE";
            case PowerupManager::POWERUP_BOWLING:
                return "BOWLING";
            case PowerupManager::POWERUP_ZIPPER:
                return "ZIPPER";
            case PowerupManager::POWERUP_PLUNGER:
                return "PLUNGER";
            case PowerupManager::POWERUP_SWITCH:
                return "SWITCH";
            case PowerupManager::POWERUP_SWATTER:
                return "SWATTER";
            case PowerupManager::POWERUP_RUBBERBALL:
                return "RUBBERBALL";
            case PowerupManager::POWERUP_PARACHUTE:
                return "PARACHUTE";
            case PowerupManager::POWERUP_ANVIL:
                return "ANVIL";
            case PowerupManager::POWERUP_MAX:
                break;
        }
        throw std::runtime_error("Unknown Power Up type");
    }
    static PowerupManager::PowerupType toPowerUpType(const std::optional<PowerUp>& powerUp)
    {
        if (powerUp)
        {
            const auto& name = powerUp->name;
            if (name == "BUBBLEGUM")
                return PowerupManager::POWERUP_BUBBLEGUM;
            if (name == "CAKE")
                return PowerupManager::POWERUP_CAKE;
            if (name == "BOWLING")
                return PowerupManager::POWERUP_BOWLING;
            if (name == "ZIPPER")
                return PowerupManager::POWERUP_ZIPPER;
            if (name == "PLUNGER")
                return PowerupManager::POWERUP_PLUNGER;
            if (name == "SWITCH")
                return PowerupManager::POWERUP_SWITCH;
            if (name == "SWATTER")
                return PowerupManager::POWERUP_SWATTER;
            if (name == "RUBBERBALL")
                return PowerupManager::POWERUP_RUBBERBALL;
            if (name == "PARACHUTE")
                return PowerupManager::POWERUP_PARACHUTE;
            if (name == "ANVIL")
                return PowerupManager::POWERUP_ANVIL;
            throw std::invalid_argument(std::string("Invalid power up \"" + name + "\""));
        }
        return PowerupManager::POWERUP_NOTHING;
    }

private:
    Kart& kart_;
    const KartProperties& properties_;
    const Skidding& skidding_;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameRaceKartExchange final : public RaceKartExchange
{
public:
    GameRaceKartExchange()
    : world_(getWorld())
    {
    }
    [[nodiscard]]
    std::vector<std::unique_ptr<KartWrapper>> getKarts() const override
    {
        const World::KartList& karts = world_.getKarts();
        std::vector<std::unique_ptr<KartWrapper>> result;
        result.reserve(karts.size());
        std::transform(karts.begin(), karts.end(), std::back_inserter(result), [](const std::shared_ptr<AbstractKart>& abstractKart) {
            Kart* kart = dynamic_cast<Kart*>(abstractKart.get());
            if (!kart)
            {
                throw std::runtime_error("Kart does not exist");
            }
            return std::make_unique<GameKartWrapper>(*kart);
        });
        return result;
    }

private:
    const World& world_;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameParticleWrapper final : public ParticleWrapper
{
public:
    explicit GameParticleWrapper(const ParticleKind* particleKind)
    : m_particle_kind(particleKind)
    {
        if (!particleKind)
        {
            throw std::runtime_error("ParticleKind does not exist");
        }
    }
    std::string getName() const override
    {
        return m_particle_kind->getName();
    }
    Interval<float> getSize() const override
    {
        return {m_particle_kind->getMinSize(), m_particle_kind->getMaxSize()};
    }
    Interval<float> getRate() const override
    {
        return {m_particle_kind->getMinRate(), m_particle_kind->getMaxRate()};
    }
    Interval<float> getLifetime() const override
    {
        return {m_particle_kind->getMinLifetime(), m_particle_kind->getMaxLifetime()};
    }
    int getFadeoutTime() const override
    {
        return m_particle_kind->getFadeoutTime();
    }
    std::string getShape() const override
    {
        switch (m_particle_kind->getShape())
        {
            case EmitterShape::EMITTER_BOX:
                return "BOX";
            case EmitterShape::EMITTER_SPHERE:
                return "SPHERE";
            case EmitterShape::EMITTER_POINT:
                return "POINT";
        }
        throw std::runtime_error("Unknown shape found");
    }
    std::optional<std::string> getMaterial() const override
    {
        return materialToId(m_particle_kind->getMaterial());
    }
    Interval<uint32_t> getColor() const override
    {
        return {m_particle_kind->getMinColor().color, m_particle_kind->getMaxColor().color};
    }
    std::array<float, 3> getBoxSize() const override
    {
        return {
            m_particle_kind->getBoxSizeX(),
            m_particle_kind->getBoxSizeY(),
            m_particle_kind->getBoxSizeZ()
        };
    }
    float getSphereRadius() const override
    {
        return m_particle_kind->getSphereRadius();
    }
    int getAngleSpread() const override
    {
        return m_particle_kind->getAngleSpread();
    }
    std::array<float, 3> getVelocity() const override
    {
        return {
            m_particle_kind->getVelocityX(),
            m_particle_kind->getVelocityY(),
            m_particle_kind->getVelocityZ()
        };
    }
    int getEmissionDecayRate() const override
    {
        return m_particle_kind->getEmissionDecayRate();
    }
    std::array<float, 2> getScaleAffactor() const override
    {
        return {
            m_particle_kind->getScaleAffectorFactorX(),
            m_particle_kind->getScaleAffectorFactorX(),
        };
    }
    bool hasFlips() const override
    {
        return m_particle_kind->getFlips();
    }
    bool isVertical() const override
    {
        return m_particle_kind->isVerticalParticles();
    }
    bool hasRandomInitialY() const override
    {
        return m_particle_kind->randomizeInitialY();
    }

private:
    const ParticleKind* m_particle_kind;
    mutable std::unique_ptr<MaterialWrapper> m_material;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameMaterialWrapper : public MaterialWrapper
{
public:
    explicit GameMaterialWrapper(const Material* material)
    : m_material(material)
    {
        if (!m_material)
        {
            throw std::runtime_error("Material does not exist");
        }
    }
    std::string getName() const override
    {
        return m_material->getTexFname();
    }
    std::string getPath() const override
    {
        return m_material->getTexFullPath();
    }
    std::optional<char> getMirrorAxis() const override
    {
        char mirror_axis = m_material->getMirrorAxisInReverse();
        return mirror_axis == ' ' ? std::nullopt : std::optional<char>(mirror_axis);
    }
    bool isBelowSurface() const override
    {
        return m_material->isBelowSurface();
    }
    bool hasFallingEffect() const override
    {
        return m_material->hasFallingEffect();
    }
    bool isDriveReset() const override
    {
        return m_material->isDriveReset();
    }
    bool isSurface() const override
    {
        return m_material->isSurface();
    }
    bool isJumpTexture() const override
    {
        return m_material->isJumpTexture();
    }
    bool hasGravity() const override
    {
        return m_material->hasGravity();
    }
    bool isIgnore() const override
    {
        return m_material->isIgnore();
    }
    bool hasHighTireAdhesion() const override
    {
        return m_material->highTireAdhesion();
    }
    bool hasTextureCompression() const override
    {
        return m_material->hasTextureCompression();
    }
    std::string getCollisionReaction() const override
    {
        switch(m_material->getCollisionReaction())
        {
            case Material::NORMAL:
                return "NORMAL";
            case Material::RESCUE:
                return "RESCUE";
            case Material::PUSH_BACK:
                return "PUSH_BACK";
            case Material::PUSH_SOCCER_BALL:
                return "PUSH_SOCCER_BALL";
        }
        throw std::runtime_error("Unknown collision reaction found");
    }
    std::optional<std::string> getCollisionParticles() const override
    {
        return stringToOptional(m_material->getCrashResetParticles());
    }
    std::optional<std::reference_wrapper<const ParticleWrapper>> getParticlesOnDrive() const override
    {
        const ParticleKind* kind = m_material->getParticlesWhen(Material::EMIT_ON_DRIVE);
        if (kind)
        {
            m_on_drive_particles = ParticleWrapper::create(kind);
            return *m_on_drive_particles;
        }
        return std::nullopt;
    }
    std::optional<std::reference_wrapper<const ParticleWrapper>> getParticlesOnSkid() const override
    {
        const ParticleKind* kind = m_material->getParticlesWhen(Material::EMIT_ON_SKID);
        if (kind)
        {
            m_on_skid_particles = ParticleWrapper::create(kind);
            return *m_on_skid_particles;
        }
        return std::nullopt;
    }
    bool hasUClamp() const override
    {
        return m_material->hasUClamp();
    }
    bool hasVClamp() const override
    {
        return m_material->hasVClamp();
    }
    std::vector<float> getRandomHueSettings() const override
    {
        return m_material->getHueSettings();
    }
    int getSlowDownTicks() const override
    {
        return m_material->getSlowDownTicks();
    }
    float getSlowDownMaxSpeedFraction() const override
    {
        return m_material->getMaxSpeedFraction();
    }
    bool isZipper() const override
    {
        return m_material->isZipper();
    }
    float getZipperMinSpeed() const override
    {
        return m_material->getZipperMinSpeed();
    }
    float getZipperMaxSpeedIncrease() const override
    {
        float zipper_max_speed_increase;
        float zipper_duration;
        float zipper_speed_gain;
        float zipper_fade_out_time;
        float zipper_engine_force;
        m_material->getZipperParameter(
            &zipper_max_speed_increase,
            &zipper_duration,
            &zipper_speed_gain,
            &zipper_fade_out_time,
            &zipper_engine_force);
        return zipper_max_speed_increase;
    }
    float getZipperDuration() const override
    {
        float zipper_max_speed_increase;
        float zipper_duration;
        float zipper_speed_gain;
        float zipper_fade_out_time;
        float zipper_engine_force;
        m_material->getZipperParameter(
            &zipper_max_speed_increase,
            &zipper_duration,
            &zipper_speed_gain,
            &zipper_fade_out_time,
            &zipper_engine_force);
        return zipper_duration;
    }
    float getZipperSpeedGain() const override
    {
        float zipper_max_speed_increase;
        float zipper_duration;
        float zipper_speed_gain;
        float zipper_fade_out_time;
        float zipper_engine_force;
        m_material->getZipperParameter(
            &zipper_max_speed_increase,
            &zipper_duration,
            &zipper_speed_gain,
            &zipper_fade_out_time,
            &zipper_engine_force);
        return zipper_speed_gain;
    }
    float getZipperFadeOutTime() const override
    {
        float zipper_max_speed_increase;
        float zipper_duration;
        float zipper_speed_gain;
        float zipper_fade_out_time;
        float zipper_engine_force;
        m_material->getZipperParameter(
            &zipper_max_speed_increase,
            &zipper_duration,
            &zipper_speed_gain,
            &zipper_fade_out_time,
            &zipper_engine_force);
        return zipper_fade_out_time;
    }
    float getZipperEngineForce() const override
    {
        float zipper_max_speed_increase;
        float zipper_duration;
        float zipper_speed_gain;
        float zipper_fade_out_time;
        float zipper_engine_force;
        m_material->getZipperParameter(
            &zipper_max_speed_increase,
            &zipper_duration,
            &zipper_speed_gain,
            &zipper_fade_out_time,
            &zipper_engine_force);
        return zipper_engine_force;
    }
    std::optional<std::string> getSfxName() const override
    {
        return stringToOptional(m_material->getSFXName());
    }
    float getSfxMinSpeed() const override
    {
        return m_material->getSfxMinSpeed();
    }
    float getSfxMaxSpeed() const override
    {
        return m_material->getSfxMaxSpeed();
    }
    float getSfxMinPitch() const override
    {
        return m_material->getSfxMinPitch();
    }
    float getSfxMaxPitch() const override
    {
        return m_material->getSfxMaxPitch();
    }
    float getSfxPitchPerSpeed() const override
    {
        return m_material->getSfxPitchPerSpeed();
    }
    std::optional<std::string> getAlphaMask() const override
    {
        return stringToOptional(m_material->getAlphaMask());
    }
    bool isColorizable() const override
    {
        return m_material->isColorizable();
    }
    float getColorizationFactor() const override
    {
        return m_material->getColorizationFactor();
    }
    std::string getColorizationMask() const override
    {
        return m_material->getColorizationMask();
    }
    std::string getShaderName() const override
    {
        return m_material->getShaderName();
    }
    std::optional<std::string> getUVTwoTexture() const override
    {
        return stringToOptional(m_material->getUVTwoTexture());
    }
    std::array<std::string, 6> getSamplerPath() const override
    {
        std::array<std::string, 6> result;
        for (unsigned i = 0; i < 6; i++)
        {
            const auto& currentSamplerPath = m_material->getSamplerPath(i);
            result[i] = currentSamplerPath;
        }
        return result;
    }
private:
    const Material* m_material;
    mutable std::unique_ptr<ParticleWrapper> m_on_drive_particles;
    mutable std::unique_ptr<ParticleWrapper> m_on_skid_particles;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameRaceMaterialExchange final : public RaceMaterialExchange
{
public:
    GameRaceMaterialExchange()
    : m_manager(material_manager)
    {
        if (!m_manager)
        {
            throw std::runtime_error("Material Manager does not exist");
        }
    }
    [[nodiscard]]
    std::vector<std::unique_ptr<MaterialWrapper>> getMaterials() const override
    {
        const std::vector<Material*>& materials = m_manager->getMaterials();
        const std::map<std::string, Material*>& spmMaterials = m_manager->getDefaultMaterials();
        std::vector<std::unique_ptr<MaterialWrapper>> result;
        result.reserve(materials.size() + spmMaterials.size());
        std::transform(materials.begin(), materials.end(), std::back_inserter(result), [](const Material* material) {return MaterialWrapper::create(material);});
        std::transform(spmMaterials.begin(), spmMaterials.end(), std::back_inserter(result), [](const std::pair<std::string, Material*>& element) {return MaterialWrapper::create(element.second);});
        return result;
    }

private:
    const MaterialManager* m_manager;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameRaceMusicExchange : public RaceMusicExchange
{
public:
    GameRaceMusicExchange()
    : musicManager_(getMusicManager())
    {
    }
    [[nodiscard]]
    bool isEnabled() const override
    {
        return UserConfigParams::m_music;
    }
    [[nodiscard]]
    float getMasterMusicGain() const override
    {
        return musicManager_.getMasterMusicVolume();
    }
    [[nodiscard]]
    std::optional<MusicWrapper> getMusic() const override
    {
        const MusicInformation* music = musicManager_.getCurrentMusic();
        return UserConfigParams::m_music && music ? std::optional<MusicWrapper>(toMusicExchange(*music)) : std::nullopt;
    }
    void setEnabled(bool enabled) override
    {
        musicManager_.setMusicEnabled(enabled);
    }
    void setMasterMusicGain(float volume) override
    {
        if (volume < 0.0f || volume > 1.0f)
            throw std::invalid_argument("volume must be between 0.0 and 1.0");
        musicManager_.setMasterMusicVolume(volume);
    }
    void setMusic(const std::optional<std::string>& id) override
    {
        musicManager_.clearCurrentMusic();
        SFXManager& sfxManager = getSfxManager();
        {
            std::unique_lock lock(sfxManager.m_rest_mutex);
            sfxManager.m_rest_condition.wait(lock, [&]() {
                return musicManager_.getCurrentMusic() == nullptr;
            });
        }
        if (id)
        {
            auto& allMusic = musicManager_.getAllMusic();
            auto music = allMusic.find(id.value());
            if (music != allMusic.end())
            {
                musicManager_.startMusic(music->second);
                {
                    std::unique_lock lock(sfxManager.m_rest_mutex);
                    sfxManager.m_rest_condition.wait(lock, [&]() {
                        return musicManager_.getCurrentMusic() == music->second;
                    });
                }
            }
        }
    }

private:
    MusicManager& musicManager_;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameLightWrapper final : public LightWrapper
{
public:
    explicit GameLightWrapper(TrackObjectPresentationLight& light)
    : light_(light)
    {
    }
    [[nodiscard]]
    uint32_t getColor() const override
    {
        video::SColor color = light_.getColor();
        return color.color;
    }
    [[nodiscard]]
    float getRadius() const override
    {
        return light_.getDistance();
    }
    [[nodiscard]]
    float getEnergy() const override
    {
        return light_.getEnergy();
    }
    void setColor(uint32_t color) override
    {
        light_.setColor(color);
    }
    void setRadius(float distance) override
    {
        light_.setDistance(distance);
    }
    void setEnergy(float energy) override
    {
        light_.setEnergy(energy);
    }

private:
    TrackObjectPresentationLight& light_;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameObjectWrapper final : public ObjectWrapper
{
public:
    explicit GameObjectWrapper(TrackObject& object)
    : object_(object)
    {
    }
    [[nodiscard]]
    size_t getId() const override
    {
        return object_.getUniqueIdentifier();
    }
    [[nodiscard]]
    std::string getName() const override
    {
        return object_.getName();
    }
    [[nodiscard]]
    std::string getType() const override
    {
        return object_.getType();
    }
    [[nodiscard]]
    bool isEnabled() const override
    {
        return object_.isEnabled();
    }
    [[nodiscard]]
    bool isDrivable() const override
    {
        return object_.isDriveable();
    }
    [[nodiscard]]
    bool isAnimated() const override
    {
        return object_.hasAnimatorRecursively();
    }
    [[nodiscard]]
    Position getPosition() const override
    {
        return vec3ToArray(object_.getAbsolutePosition());
    }
    [[nodiscard]]
    Position getCenterPosition() const override
    {
        return vec3ToArray(object_.getAbsoluteCenterPosition());
    }
    [[nodiscard]]
    Vector getRotation() const override
    {
        return vec3ToArray(object_.getRotation());
    }
    [[nodiscard]]
    Vector getScale() const override
    {
        return vec3ToArray(object_.getScale());
    }
    [[nodiscard]]
    std::string getLodGroup() const override
    {
        return object_.getLodGroup();
    }
    [[nodiscard]]
    std::string getInteraction() const override
    {
        return object_.getInteraction();
    }
    [[nodiscard]]
    std::vector<std::unique_ptr<ObjectWrapper>> getChildren() const override
    {
        return objectsToWrapper(object_.getChildren());
    }
    [[nodiscard]]
    std::vector<std::unique_ptr<ObjectWrapper>> getMovableChildren() const override
    {
        return objectsToWrapper(object_.getMovableChildren());
    }
    [[nodiscard]]
    std::unique_ptr<LightWrapper> getLight() const override
    {
        if (getType() == "light")
        {
            auto* presentation = object_.getPresentation<TrackObjectPresentationLight>();
            if (!presentation)
            {
                throw std::runtime_error("Cannot access light object");
            }
            return LightWrapper::create(*presentation);
        }
        return nullptr;
    }
    [[nodiscard]]
    std::unique_ptr<ParticleWrapper> getParticles() const override
    {
        if (getType() == "particle-emitter")
        {
            const auto* presentation = object_.getPresentation<TrackObjectPresentationParticles>();
            if (!presentation)
            {
                throw std::runtime_error("Cannot access particle emitter object");
            }
            return ParticleWrapper::create(presentation->getParticleEmitter()->getParticlesInfo());
        }
        return nullptr;
    }
    void setEnabled(bool enabled) override
    {
        object_.setEnabled(enabled);
    }
    void setPosition(const Position& position) override
    {
        object_.move(arrayToVec3D(position), object_.getRotation(), object_.getScale(), true, true);
    }

private:
    static std::vector<std::unique_ptr<ObjectWrapper>> objectsToWrapper(const std::vector<TrackObject*>& trackObjects)
    {

        std::vector<std::unique_ptr<ObjectWrapper>> result;
        result.reserve(trackObjects.size());
        for (TrackObject* trackObject : trackObjects)
        {
            result.push_back(std::make_unique<GameObjectWrapper>(*trackObject));
#ifdef DEBUG
            const TrackObject& parent = *trackObject->getParent();
            int found = 0;
            for (const TrackObject& child : parent.getChildren())
            {
                if (&child == trackObject)
                    found++;
            }
            for (const TrackObject& child : parent.getMovableChildren())
            {
                if (&child == trackObject)
                    found++;
            }
            assert(found == 1);
#endif
        }
        return result;
    }

private:
    TrackObject& object_;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameRaceObjectExchange final : public RaceObjectExchange
{
public:
    GameRaceObjectExchange()
    : track_(getTrack())
    , trackObjectManager_(getTrackObjectManager(track_))
    {
    }
    [[nodiscard]]
    std::vector<std::unique_ptr<ObjectWrapper>> getObjects() const override
    {
        PtrVector<TrackObject>& trackObjects = trackObjectManager_.getObjects();
        std::vector<std::unique_ptr<ObjectWrapper>> result;
        result.reserve(trackObjects.size());
        for (TrackObject* object : trackObjects)
        {
            if (object && !object->getParent())
            {
                result.push_back(std::make_unique<GameObjectWrapper>(*object));
            }
        }
        return result;
    }
    void remove(size_t id) override
    {
        auto& objects = trackObjectManager_.getObjects();
        auto object = std::find_if(objects.begin(), objects.end(), [id](const TrackObject* object) { return object->getUniqueIdentifier() == id; });
        if (object != objects.end())
        {
            trackObjectManager_.removeObject(*object);
        }
    }

private:
    static TrackObjectManager& getTrackObjectManager(Track& track)
    {
        TrackObjectManager* trackObjectManager = track.getTrackObjectManager();
        if (!trackObjectManager)
            throw std::runtime_error("TrackObjectManager does not exist");
        return *trackObjectManager;
    }

private:
    Track& track_;
    TrackObjectManager& trackObjectManager_;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameRaceQuadExchange final : public RaceQuadExchange
{
public:
    GameRaceQuadExchange()
    : driveGraph_(getDriveGraph())
    {
    }
    [[nodiscard]]
    std::vector<QuadWrapper> getQuads() const override
    {
        unsigned int size = driveGraph_.getNumNodes();
        std::vector<QuadWrapper> result;
        result.reserve(size);
        for (unsigned int i = 0; i < size; ++i)
        {
            auto currentQuad = dynamic_cast<const DriveNode*>(driveGraph_.getQuad(i));
            if (!currentQuad)
            {
                continue;
            }
            const DriveNode& driveNode = *currentQuad;
            QuadWrapper quad{};
            quad.id = driveNode.getIndex();
            quad.ignore = driveNode.isIgnored();
            quad.invisible = driveNode.isInvisible();
            quad.aiIgnore = driveNode.letAIIgnore();
            for (int j = 0; j < 4; ++j)
            {
                quad.quad[j] = vec3ToArray(driveNode[0]);
            }
            quad.heightTesting = {driveNode.getMinHeightTesting(), driveNode.getMaxHeightTesting()};
            driveGraph_.getSuccessors(driveNode.getIndex(), quad.successors);
            result.push_back(quad);
        }
        return result;
    }

private:
    static const DriveGraph& getDriveGraph()
    {
        const DriveGraph* driveGraph = DriveGraph::get();
        if (!driveGraph)
        {
            throw std::runtime_error("DriveGraph does not exist");
        }
        return *driveGraph;
    }

private:
    const DriveGraph& driveGraph_;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameSfxSoundWrapper final : public SfxSoundWrapper
{
public:
    explicit GameSfxSoundWrapper(SFXBase& sfxBase)
    : sfxBase_(sfxBase)
    {
    }
    [[nodiscard]]
    std::string getSound() const override
    {
        const SFXBuffer* buffer = sfxBase_.getBuffer();
        assert(buffer);
        return buffer->getName();
    }
    [[nodiscard]]
    std::string getStatus() const override
    {
        return statusToString(sfxBase_.getStatus());
    }
    [[nodiscard]]
    bool inLoop() const override
    {
        return sfxBase_.isLooped();
    }
    [[nodiscard]]
    float getVolume() const override
    {
        return sfxBase_.getVolume();
    }
    [[nodiscard]]
    float getPitch() const override
    {
        return sfxBase_.getPitch();
    }
    [[nodiscard]]
    float getPlayTime() const override
    {
        return sfxBase_.getPlayTime();
    }
    [[nodiscard]]
    std::optional<Position> getPosition() const override
    {
        auto position = sfxBase_.getPosition();
        if (position)
            std::iter_swap(position.value().begin() + 1, position.value().begin() + 2);
        return position;
    }
    void setStatus(const std::string& status) override
    {
        if (status == "PLAY" || status == "STOP" || status == "PAUSE" || status == "RESUME")
            updateSfx(status, &SFXManager::m_rest_status);
        else
            throw std::invalid_argument("Unknown status change \"" + status + "\"");
    }
    void setLoop(bool inLoop) override
    {
        updateSfx(inLoop, &SFXManager::m_rest_loop);
    }
    void setVolume(float volume) override
    {
        if (volume < 0.0f || volume > 1.0)
            throw std::invalid_argument("Volume must be between 0.0 and 1.0");
        updateSfx(volume, &SFXManager::m_rest_volume);
    }
    void setPitch(float pitch) override
    {
        if (pitch < 0.5f || pitch > 2.0)
            throw std::invalid_argument("Pitch must be between 0.5 and 2.0");
        updateSfx(pitch, &SFXManager::m_rest_pitch);
    }
    void setPosition(const Position& position) override
    {
        if (!sfxBase_.getPosition())
            throw std::invalid_argument("Sound is not positional");
        Vec3 positionAsVector = arrayToVec(position);
        updateSfx(positionAsVector, &SFXManager::m_rest_position);
    }

private:
    static std::string statusToString(SFXBase::SFXStatus status)
    {
        switch(status)
        {
            case SFXBase::SFX_UNKNOWN:
                return "UNKNOWN";
            case SFXBase::SFX_STOPPED:
                return "STOPPED";
            case SFXBase::SFX_PAUSED:
                return "PAUSED";
            case SFXBase::SFX_PLAYING:
                return "PLAYING";
            case SFXBase::SFX_NOT_INITIALISED:
                return "NOT_INITIALISED";
        }
        throw std::runtime_error("Unknown SFXStatus");
    }
    template<typename T>
    void updateSfx(const T& value, const T* SFXManager::*member)
    {
        SFXManager& sfxManager = getSfxManager();
        std::unique_lock lock(sfxManager.m_rest_mutex);
        sfxManager.m_rest_change_sfx = &sfxBase_;
        sfxManager.*member = &value;
        sfxManager.m_rest_condition.wait(lock, [&]() {
            return !(sfxManager.*member) && !sfxManager.m_rest_change_sfx;
        });
    }

private:
    SFXBase& sfxBase_;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameRaceSfxExchange final : public RaceSfxExchange
{
public:
    GameRaceSfxExchange()
    : sfxManager_(getSfxManager())
    {
    }
    [[nodiscard]]
    Position getListenerPosition() const override
    {
        return vec3ToArray(sfxManager_.getListenerPos());
    }
    [[nodiscard]]
    Vector getListenerDirection() const override
    {
        return vec3ToArray(sfxManager_.getListenerDirection());
    }
    [[nodiscard]]
    Vector getListenerUpDirection() const override
    {
        return vec3ToArray(sfxManager_.getListenerUpDirection());
    }
    [[nodiscard]]
    float getMasterVolume() const override
    {
        return sfxManager_.getMasterSFXVolume();
    }
    void setMasterVolume(float volume) override
    {
        if (volume < 0.0f || volume > 1.0f)
            throw std::invalid_argument("Volume must be between 0.0 and 1.0");
        UserConfigParams::m_sfx_volume = volume;
        sfxManager_.setMasterSFXVolume(volume);
    }
    [[nodiscard]]
    bool isSfxAllowed() const override
    {
        return sfxManager_.sfxAllowed();
    }
    void setSfxAllowed(bool sfxAllowed) override
    {
        if (UserConfigParams::m_sfx != sfxAllowed)
        {
            if (sfxAllowed)
            {
                UserConfigParams::m_sfx = sfxAllowed;
                sfxManager_.toggleSound(sfxAllowed);
                sfxManager_.resumeAll();
            }
            else
            {
                sfxManager_.pauseAll();
                sfxManager_.toggleSound(sfxAllowed);
                UserConfigParams::m_sfx = sfxAllowed;
            }
        }
    }
    [[nodiscard]]
    std::vector<std::unique_ptr<SfxSoundWrapper>> getAllSounds() const override
    {
        std::vector<std::unique_ptr<SfxSoundWrapper>> result;
        const auto& sounds = sfxManager_.getAllSfx();
        result.reserve(sounds.size());
        for (const auto& sound : sounds)
        {
            assert(sound);
            result.push_back(SfxSoundWrapper::create(*sound));
        }
        return result;
    }
    [[nodiscard]]
    std::unique_ptr<SfxSoundWrapper> add(const std::string& sfxId) override
    {
        if (!sfxManager_.soundExist(sfxId))
            throw std::invalid_argument("Sound with id \"" + sfxId + "\" does not exist");
        SFXBase* soundSource = sfxManager_.createSoundSource(sfxId);
        assert(soundSource);
        soundSource->setCanBeDeleted(true);
        return SfxSoundWrapper::create(*soundSource);
    }
    void remove(size_t id) override
    {
        auto& sounds = sfxManager_.getAllSfx();
        if (id >= sounds.size())
            throw std::invalid_argument("id is out of range");
        Track* track = Track::getCurrentTrack();
        SFXBase* sound = sounds[id];
        if (!sound->canBeDeleted())
            throw std::invalid_argument("This sound is marked as non-deletable");
        if (track)
        {
            for (TrackObject* object: track->getTrackObjectManager()->getObjects())
            {
                if (object->getType() == "sfx-emitter")
                {
                    auto* soundObject = object->getPresentation<TrackObjectPresentationSound>();
                    if (soundObject->getSound() == sound)
                    {
                        track->getTrackObjectManager()->removeObject(object);
                    }
                }
            }
        }
        sfxManager_.m_rest_deleted_sfx = sound;
        std::unique_lock lock(sfxManager_.m_rest_mutex);
        sfxManager_.m_rest_condition.wait(lock, [&]() {
            return !sfxManager_.m_rest_deleted_sfx;
        });
    }

private:
    SFXManager& sfxManager_;
};
// ---------------------------------------------------------------------------------------------------------------------
class GameRaceWeatherExchange final : public RaceWeatherExchange
{
public:
    GameRaceWeatherExchange()
    : weather_(getCurrentWeather())
    {
    }
    bool getLightning() const override
    {
        return weather_.getLightning();
    }
    std::optional<std::string> getSound() const override
    {
        const std::string& sound = weather_.getSound();
        return sound.empty() ? std::nullopt : std::optional<std::string>(sound);
    }
    uint32_t getSkyColor() const override
    {
        return weather_.getSkyColor().color;
    }
    std::optional<std::reference_wrapper<const ParticleWrapper>> getParticles() const override
    {
        const ParticleKind* kind = weather_.getParticles();
        if (kind)
        {
            particles_ = ParticleWrapper::create(kind);
            return *particles_;
        }
        return std::nullopt;
    }

private:
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
    const Weather& weather_;
    mutable std::unique_ptr<ParticleWrapper> particles_;
};
// ---------------------------------------------------------------------------------------------------------------------
}
// ---------------------------------------------------------------------------------------------------------------------
std::function<std::filesystem::path(const std::string&, const std::filesystem::path&)> DataExchange::getUnzipFunction()
{
    return decompressZip;
}
// ---------------------------------------------------------------------------------------------------------------------
// Game resources
// ---------------------------------------------------------------------------------------------------------------------
std::unique_ptr<RaceExchange> RaceExchange::create(RaceManager& raceManager)
{
    return std::make_unique<GameRaceExchange>(raceManager);
}
std::unique_ptr<KartModelExchange> KartModelExchange::create(RaceManager& raceManager)
{
    return std::make_unique<GameKartModelExchange>(raceManager);
}
std::unique_ptr<MusicExchange> MusicExchange::create()
{
    return std::make_unique<GameMusicExchange>();
}
std::unique_ptr<SfxExchange> SfxExchange::create()
{
    return std::make_unique<GameSfxExchange>();
}
std::unique_ptr<TrackModelExchange> TrackModelExchange::create()
{
    return std::make_unique<GameTrackModelExchange>();
}
// ---------------------------------------------------------------------------------------------------------------------
// Race resources
// ---------------------------------------------------------------------------------------------------------------------
std::unique_ptr<BonusItemWrapper> BonusItemWrapper::create(ItemState& state)
{
    return std::make_unique<GameBonusItemWrapper>(state);
}
std::unique_ptr<RaceBonusItemExchange> RaceBonusItemExchange::create()
{
    return std::make_unique<GameRaceBonusItemExchange>();
}
std::unique_ptr<RaceChecklineExchange> RaceChecklineExchange::create()
{
    return std::make_unique<GameRaceChecklineExchange>();
}
std::unique_ptr<KartWrapper> KartWrapper::create(AbstractKart& kart)
{
    return std::make_unique<GameKartWrapper>(dynamic_cast<Kart&>(kart));
}
std::unique_ptr<RaceKartExchange> RaceKartExchange::create()
{
    return std::make_unique<GameRaceKartExchange>();
}
std::unique_ptr<ParticleWrapper> ParticleWrapper::create(const ParticleKind* particleKind)
{
    return std::make_unique<GameParticleWrapper>(particleKind);
}
std::unique_ptr<MaterialWrapper> MaterialWrapper::create(const Material* material)
{
    return std::make_unique<GameMaterialWrapper>(material);
}
std::unique_ptr<RaceMaterialExchange> RaceMaterialExchange::create()
{
    return std::make_unique<GameRaceMaterialExchange>();
}
std::unique_ptr<RaceMusicExchange> RaceMusicExchange::create()
{
    return std::make_unique<GameRaceMusicExchange>();
}
std::unique_ptr<LightWrapper> LightWrapper::create(TrackObjectPresentationLight& object)
{
    return std::make_unique<GameLightWrapper>(object);
}
std::unique_ptr<ObjectWrapper> ObjectWrapper::create(TrackObject& object)
{
    return std::make_unique<GameObjectWrapper>(object);
}
std::unique_ptr<RaceObjectExchange> RaceObjectExchange::create()
{
    return std::make_unique<GameRaceObjectExchange>();
}
std::unique_ptr<RaceQuadExchange> RaceQuadExchange::create()
{
    return std::make_unique<GameRaceQuadExchange>();
}
std::unique_ptr<SfxSoundWrapper> SfxSoundWrapper::create(SFXBase& sfxBase)
{
    return std::make_unique<GameSfxSoundWrapper>(sfxBase);
}
std::unique_ptr<RaceSfxExchange> RaceSfxExchange::create()
{
    return std::make_unique<GameRaceSfxExchange>();
}
std::unique_ptr<RaceWeatherExchange> RestApi::RaceWeatherExchange::create()
{
    return std::make_unique<GameRaceWeatherExchange>();
}
// ---------------------------------------------------------------------------------------------------------------------
}
