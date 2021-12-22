#include <httplib.h>
#include "modes/world.hpp"
#include "rest-api/RestApi.hpp"
#include "rest-api/DataExchange.hpp"
#include "utils/log.hpp"

namespace
{
constexpr const char* ANY = "^(.*?)$";

constexpr const char* CURRENT_RACE_REGEX = R"(^\/races(\/(\d+))?$)";
constexpr const char* KART_MODEL_REGEX = R"(^\/karts(\/(.+))?$)";
constexpr const char* MUSIC_REGEX = R"(^\/music(\/(.+))?$)";
constexpr const char* SFX_REGEX = R"(^\/sfx(\/(.+))?$)";
constexpr const char* TRACK_MODEL_REGEX = R"(^\/tracks(\/(.+))?$)";

constexpr const char* RACE_BONUS_ITEM_REGEX = R"(^\/races\/(\d+)\/items(\/(\d+))?$)";
constexpr const char* RACE_CHECKLINE_REGEX = R"(^\/races\/(\d+)\/checklines(\/(\d+))?$)";
constexpr const char* RACE_KART_REGEX = R"(^\/races\/(\d+)\/karts(\/(\d+))?$)";
constexpr const char* RACE_MATERIAL_REGEX = R"(^\/races\/(\d+)\/materials(\/(.+))?$)";
constexpr const char* RACE_MUSIC_REGEX = R"(^\/races\/(\d+)\/music$)";
constexpr const char* RACE_OBJECT_REGEX = R"(^\/races\/(\d+)\/objects(\/(\d+))?$)";
constexpr const char* RACE_QUAD_REGEX = R"(^\/races\/(\d+)\/quads(\/(\d+))?$)";
constexpr const char* RACE_SFX_REGEX = R"(^\/races\/(\d+)\/sfx(\/(\d+))?$)";
constexpr const char* RACE_WEATHER_REGEX = R"(^\/races\/(\d+)\/weather$)";

const RestApi::Path CURRENT_RACE = {std::regex(CURRENT_RACE_REGEX), std::nullopt, 2};
const RestApi::Path KART_MODEL = {std::regex(KART_MODEL_REGEX), std::nullopt, 2};
const RestApi::Path MUSIC = {std::regex(MUSIC_REGEX), std::nullopt, 2};
const RestApi::Path SFX = {std::regex(SFX_REGEX), std::nullopt, 2};
const RestApi::Path TRACK_MODEL = {std::regex(TRACK_MODEL_REGEX), std::nullopt, 2};

const RestApi::Path RACE_BONUS_ITEM = {std::regex(RACE_BONUS_ITEM_REGEX), 1, 3};
const RestApi::Path RACE_CHECKLINE = {std::regex(RACE_CHECKLINE_REGEX), 1, 3};
const RestApi::Path RACE_KART = {std::regex(RACE_KART_REGEX), 1, 3};
const RestApi::Path RACE_MATERIAL = {std::regex(RACE_MATERIAL_REGEX), 1, 3};
const RestApi::Path RACE_MUSIC = {std::regex(RACE_MUSIC_REGEX), 1, std::nullopt};
const RestApi::Path RACE_OBJECT = {std::regex(RACE_OBJECT_REGEX), 1, 3};
const RestApi::Path RACE_QUAD = {std::regex(RACE_QUAD_REGEX), 1, 3};
const RestApi::Path RACE_SFX = {std::regex(RACE_SFX_REGEX), 1, 3};
const RestApi::Path RACE_WEATHER = {std::regex(RACE_WEATHER_REGEX), 1, std::nullopt};
}

namespace RestApi
{

template<typename CreateDataExchange, typename CreateHandler>
static Endpoint createEndpoint(const Path& path, CreateDataExchange&& createDataExchange, CreateHandler&& createHandler)
{
    auto dataExchange = createDataExchange();
    auto& dataExchangeReference = *dataExchange;
    return Endpoint {
        path,
        std::move(dataExchange),
        createHandler(dataExchangeReference)
    };
}

template<typename T>
[[nodiscard]]
static Endpoint createGameEndpoint(
    const Path& path,
    const std::function<std::unique_ptr<T>()>& createDataExchange,
    const std::function<std::unique_ptr<Handler>(T&, const GetMutex&)>& createHandler,
    const GetMutex& getMutex)
{
    return createEndpoint(
        path,
        [&createDataExchange] { return createDataExchange(); },
        [&createHandler, &getMutex] (T& dataExchange) { return createHandler(dataExchange, getMutex); }
    );
}

static std::vector<Endpoint> createGameEndpoints(RaceManager& raceManager)
{
    std::vector<Endpoint> gameEndpoints;
    auto getMutex = [] {
        return World::getWorld() ? &World::getWorld()->m_track_mutex : nullptr;
    };
    gameEndpoints.push_back(createGameEndpoint<RaceExchange>(CURRENT_RACE, [&raceManager] { return RaceExchange::create(raceManager); }, Handler::createRaceHandler, getMutex));
    gameEndpoints.push_back(createGameEndpoint<KartModelExchange>(KART_MODEL, [&raceManager] { return KartModelExchange::create(raceManager); }, Handler::createKartModelHandler, getMutex));
    gameEndpoints.push_back(createGameEndpoint<MusicExchange>(MUSIC, MusicExchange::create, Handler::createMusicHandler, getMutex));
    gameEndpoints.push_back(createGameEndpoint<SfxExchange>(SFX, SfxExchange::create, Handler::createSfxHandler, getMutex));
    gameEndpoints.push_back(createGameEndpoint<TrackModelExchange>(TRACK_MODEL, TrackModelExchange::create, Handler::createTrackModelHandler, getMutex));
    return gameEndpoints;
}

Server::Server(RaceManager& raceManager)
: server_(std::make_unique<httplib::Server>())
{
    gameEndpoints_ = createGameEndpoints(raceManager);
    getCurrentRaceId_ = [&exchange = dynamic_cast<RaceExchange&>(*gameEndpoints_.front().dataExchange)] { return exchange.getId(); };
    resetListeners();
    initialize();
    thread_ = std::thread([&] {
        std::string message = std::string("Listening on: http://") + API_ENDPOINT + ":" + std::to_string(API_PORT);
        Log::info(REST_API, message.c_str());
        server_->listen(API_ENDPOINT, API_PORT);
        Log::info(REST_API, "Network listener stopped");
    });
}

Server::~Server() noexcept
{
    server_->stop();
    Log::info(REST_API, "Stop network listener");
    thread_.join();
}

template<typename T>
[[nodiscard]]
static Endpoint createRaceEndpoint(
    const Path& path,
    const std::function<std::unique_ptr<Handler>(const T&)>& createHandler)
{
    return createEndpoint(
        path,
        [] { return T::create(); },
        [&createHandler] (auto& dataExchange) { return createHandler(dataExchange); }
    );
}

template<typename T>
[[nodiscard]]
static Endpoint createRaceEndpoint(
    const Path& path,
    const std::function<std::unique_ptr<Handler>(T&, std::mutex&)>& createHandler,
    std::mutex& mutex)
{
    return createEndpoint(
        path,
        [] { return T::create(); },
        [&createHandler, &mutex] (auto& dataExchange) { return createHandler(dataExchange, mutex); }
    );
}

void Server::startRaceListeners()
{
    Log::info(REST_API, "Start track listeners");
    assert(World::getWorld());
    resetListeners();
    std::mutex& mutex = World::getWorld()->m_track_mutex;
    raceEndpoints_.push_back(createRaceEndpoint<RaceBonusItemExchange>(RACE_BONUS_ITEM, Handler::createRaceBonusItemHandler, mutex));
    raceEndpoints_.push_back(createRaceEndpoint<RaceChecklineExchange>(RACE_CHECKLINE, Handler::createRaceChecklineHandler, mutex));
    raceEndpoints_.push_back(createRaceEndpoint<RaceKartExchange>(RACE_KART, Handler::createRaceKartHandler, mutex));
    raceEndpoints_.push_back(createRaceEndpoint<RaceMaterialExchange>(RACE_MATERIAL, Handler::createRaceMaterialHandler, mutex));
    raceEndpoints_.push_back(createRaceEndpoint<RaceMusicExchange>(RACE_MUSIC, Handler::createRaceMusicHandler, mutex));
    raceEndpoints_.push_back(createRaceEndpoint<RaceObjectExchange>(RACE_OBJECT, Handler::createRaceObjectHandler, mutex));
    raceEndpoints_.push_back(createRaceEndpoint<RaceQuadExchange>(RACE_QUAD, Handler::createRaceQuadHandler));
    raceEndpoints_.push_back(createRaceEndpoint<RaceSfxExchange>(RACE_SFX, Handler::createRaceSfxHandler, mutex));
    raceEndpoints_.push_back(createRaceEndpoint<RaceWeatherExchange>(RACE_WEATHER, Handler::createRaceWeatherHandler));
    registerMatchers(raceEndpoints_);
}

void Server::stopRaceListeners()
{
    Log::info(REST_API, "Stop track listeners");
    resetListeners();
}

void Server::resetListeners()
{
    matchers_.clear();
    raceEndpoints_.clear();
    registerMatchers(gameEndpoints_);
}

static std::smatch::value_type getMatch(const std::smatch& matches, size_t id)
{
    if (matches.size() <= id)
    {
        throw std::invalid_argument("Match id out of range");
    }
    return matches[id];
}

void Server::initialize()
{
    server_->Get(ANY, [&](const httplib::Request& request, httplib::Response& response) {
        std::string message = "Handle GET " + request.path;
        Log::info(REST_API, message.c_str());
        dispatchRequest(
            request,
            response,
            [&] (Handler& handler, const std::string&) {
                return handler.handleGet();
            },
            [&] (Handler& handler, const std::string& resourceId, const std::string&) {
                return handler.handleGet(resourceId);
            }
        );
    });
    server_->Post(ANY, [&](const httplib::Request& request, httplib::Response& response) {
        std::string message = "Handle POST " + request.path;
        Log::info(REST_API, message.c_str());
        dispatchRequest(
            request,
            response,
            [&] (Handler& handler, const std::string& body) {
                return handler.handlePost(body);
            },
            [&] (Handler& handler, const std::string& resourceId, const std::string& body) {
                return handler.handlePost(resourceId, body);
            }
        );
    });
    server_->Put(ANY, [&](const httplib::Request& request, httplib::Response& response) {
        std::string message = "Handle PUT " + request.path;
        Log::info(REST_API, message.c_str());
        dispatchRequest(
            request,
            response,
            [&] (Handler& handler, const std::string& body) {
                if (request.has_header("Content-Type") && request.get_header_value("Content-Type") == "application/zip")
                {
                    return handler.handlePutZip(body);
                }
                return handler.handlePut(body);
            },
            [&] (Handler& handler, const std::string&, const std::string&) {
                return Handler::generateNotFound();
            }
        );
    });
    server_->Delete(ANY, [&](const httplib::Request& request, httplib::Response& response) {
        std::string message = "Handle DELETE " + request.path;
        Log::info(REST_API, message.c_str());
        dispatchRequest(
            request,
            response,
            [&] (Handler& handler, const std::string& body) {
                return Handler::generateNotFound();
            },
            [&] (Handler& handler, const std::string& resourceId, const std::string&) {
                return handler.handleDelete(resourceId);
            }
        );
    });
}

static bool checkInvalidRaceId(const Path& path, std::optional<size_t> currentRaceId, const std::smatch& matches)
{
    return path.raceId && std::stoull(getMatch(matches, path.raceId.value())) != currentRaceId;
}

static std::optional<std::string> extractResourceId(const Path& path, const std::smatch& matches)
{
    if (path.resourceId)
    {
        auto match = getMatch(matches, path.resourceId.value());
        if (match.matched)
        {
            return match;
        }
    }
    return std::nullopt;
}

void Server::dispatchRequest(
    const httplib::Request& request,
    httplib::Response& response,
    const std::function<std::pair<STATUS_CODE, std::string>(Handler&, const std::string&)>& handleGeneral,
    const std::function<std::pair<STATUS_CODE, std::string>(Handler&, const std::string&, const std::string&)>& handleResource)
{
    try
    {
        for (auto i = matchers_.rbegin(); i != matchers_.rend(); i++)
        {
            auto& [path, handler] = *i;
            std::smatch matches;
            if(std::regex_match(request.path, matches, path.get().path))
            {
                std::lock_guard<std::mutex> guard(runMutex_);
                STATUS_CODE status;
                std::string result;
                if (checkInvalidRaceId(path.get(), getCurrentRaceId_(), matches))
                {
                    std::tie(status, result) = Handler::generateNotFound();
                }
                else if (auto resourceId = extractResourceId(path, matches))
                {
                    std::tie(status, result) = handleResource(handler, resourceId.value(), request.body);
                }
                else
                {
                    std::tie(status, result) = handleGeneral(handler, request.body);
                }
                response.set_content(result, "application/json");
                response.status = static_cast<int>(status);
                return;
            }
        }
        response.status = static_cast<int>(STATUS_CODE::NOT_FOUND);
    }
    catch (const std::invalid_argument& exception)
    {
        response.status = static_cast<int>(STATUS_CODE::BAD_REQUEST);
        response.body = exception.what();
    }
    catch (const std::exception& exception)
    {
        response.status = static_cast<int>(STATUS_CODE::INTERNAL_SERVER_ERROR);
        response.body = exception.what();
    }
    catch (...)
    {
        response.status = static_cast<int>(STATUS_CODE::INTERNAL_SERVER_ERROR);
        response.body = "Unknown internal error";
    }
}

void Server::registerMatcher(const Path& path, Handler& handler)
{
    matchers_.emplace_back(path, handler);
}

void Server::registerMatchers(const std::vector<Endpoint>& endpoints)
{
    for (const auto& endpoint : endpoints)
    {
        registerMatcher(endpoint.path, *endpoint.handler);
    }
}

}
