#pragma once
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

namespace RestApi
{

class RaceChecklineExchange;
class RaceBonusItemExchange;
class RaceKartExchange;
class TrackLightExchange;
class RaceMaterialExchange;
class RaceMusicExchange;
class MusicExchange;
class RaceObjectExchange;
class TrackParticleEmitterExchange;
class RaceQuadExchange;
class RaceExchange;
class KartModelExchange;
class TrackModelExchange;
class RaceSfxExchange;
class SfxExchange;
class RaceWeatherExchange;

using GetMutex = std::function<std::mutex*()>;

enum class STATUS_CODE : int
{
    OK = 200,
    CREATED = 201,
    NO_CONTENT = 204,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    INTERNAL_SERVER_ERROR = 500
};

class Handler
{
public:
    static std::unique_ptr<Handler> createRaceHandler(RaceExchange& raceExchange, const GetMutex& getMutex);
    static std::unique_ptr<Handler> createKartModelHandler(KartModelExchange& raceKartExchange, const GetMutex& getMutex);
    static std::unique_ptr<Handler> createMusicHandler(MusicExchange& trackMusicLibraryExchange, const GetMutex& getMutex);
    static std::unique_ptr<Handler> createSfxHandler(SfxExchange& trackSfxTypesExchange, const GetMutex& getMutex);
    static std::unique_ptr<Handler> createTrackModelHandler(TrackModelExchange& raceTrackExchange, const GetMutex& getMutex);

    static std::unique_ptr<Handler> createRaceBonusItemHandler(RaceBonusItemExchange& trackItemExchange, std::mutex& mutex);
    static std::unique_ptr<Handler> createRaceChecklineHandler(const RaceChecklineExchange& checklineExchange, std::mutex& mutex);
    static std::unique_ptr<Handler> createRaceKartHandler(const RaceKartExchange& trackKartExchange, std::mutex& mutex);
    static std::unique_ptr<Handler> createRaceMaterialHandler(const RaceMaterialExchange& trackMaterialExchange, std::mutex& mutex);
    static std::unique_ptr<Handler> createRaceMusicHandler(RaceMusicExchange& trackMusicExchange, std::mutex& mutex);
    static std::unique_ptr<Handler> createRaceObjectHandler(RaceObjectExchange& trackObjectExchange, std::mutex& mutex);
    static std::unique_ptr<Handler> createRaceQuadHandler(const RaceQuadExchange& quadExchange);
    static std::unique_ptr<Handler> createRaceSfxHandler(RaceSfxExchange& trackSoundExchange, std::mutex& mutex);
    static std::unique_ptr<Handler> createRaceWeatherHandler(const RaceWeatherExchange& weatherDataExchange);

public:
    static std::pair<STATUS_CODE, std::string> generateNotFound();

public:
    virtual ~Handler() = default;
    Handler(const Handler&) = delete;
    Handler(Handler&&) = delete;
    virtual std::pair<STATUS_CODE, std::string> handleGet();
    virtual std::pair<STATUS_CODE, std::string> handleGet(const std::string& parameter);
    virtual std::pair<STATUS_CODE, std::string> handlePost(const std::string& body);
    virtual std::pair<STATUS_CODE, std::string> handlePost(const std::string& id, const std::string& body);
    virtual std::pair<STATUS_CODE, std::string> handlePut(const std::string& body);
    virtual std::pair<STATUS_CODE, std::string> handlePutZip(const std::string& zip);
    virtual std::pair<STATUS_CODE, std::string> handleDelete(const std::string& id);

protected:
    Handler() = default;
};

}
