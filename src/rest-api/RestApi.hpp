#pragma once
#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <thread>
#include <utility>
#include "rest-api/Handler.hpp"

class RaceManager;

namespace httplib
{
    class Server;
    class Request;
    class Response;
}

namespace RestApi
{
class DataExchange;
class RaceExchange;
class KartModelExchange;
class TrackModelExchange;
class SfxExchange;

static constexpr const char* REST_API = "REST API";

struct Path
{
    std::regex path;
    std::optional<size_t> raceId;
    std::optional<size_t> resourceId;
};

struct Endpoint
{
    std::reference_wrapper<const Path> path;
    std::unique_ptr<DataExchange> dataExchange;
    std::unique_ptr<Handler> handler;
};

class Server
{
public:
    static constexpr const char* API_ENDPOINT = "0.0.0.0";
    static constexpr int API_PORT = 8000;

public:
    explicit Server(RaceManager& raceManager);
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    ~Server() noexcept;
    void startRaceListeners();
    void stopRaceListeners();

private:
    void resetListeners();
    void initialize();
    void dispatchRequest(
        const httplib::Request& request,
        httplib::Response& response,
        const std::function<std::pair<STATUS_CODE, std::string>(Handler&, const std::string&)>& handleGeneral,
        const std::function<std::pair<STATUS_CODE, std::string>(Handler&, const std::string&, const std::string&)>& handleResource);
    void registerMatcher(const Path& path, Handler& handler);
    void registerMatchers(const std::vector<Endpoint>& endpoints);

private:
    std::unique_ptr<httplib::Server> server_;
    std::mutex runMutex_;
    std::thread thread_;
    std::function<std::optional<size_t>()> getCurrentRaceId_;
    std::vector<Endpoint> gameEndpoints_;
    std::vector<Endpoint> raceEndpoints_;
    std::vector<std::pair<std::reference_wrapper<const Path>, std::reference_wrapper<Handler>>> matchers_;
};

}
