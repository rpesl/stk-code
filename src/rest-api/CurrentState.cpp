#include <functional>
#include <mutex>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include "rest-api/CurrentState.hpp"
#include "rest-api/DataExchange.hpp"
#include "rest-api/Handler.hpp"

namespace RestApi
{

static rapidjson::Document toValue(const std::string& input)
{
    rapidjson::Document document;
    document.Parse(input.c_str());
    assert(!document.HasParseError());
    return document;
}

static std::string toString(const rapidjson::Value& value)
{
    rapidjson::StringBuffer stringBuffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(stringBuffer);
    value.Accept(writer);
    return stringBuffer.GetString();
}

template<typename T>
static rapidjson::Document getState(const std::function<std::unique_ptr<Handler>(T&)>& createHandler)
{
    auto data = T::create();
    auto handler = createHandler(*data);
    return toValue(handler->handleGet().second);
}

template<typename T>
static rapidjson::Document getStateMutex(const std::function<std::unique_ptr<Handler>(T&, std::mutex&)>& createHandler, std::mutex& mutex)
{
    std::function<std::unique_ptr<Handler>(T&)> create = [&createHandler, &mutex] (T& data) {
        return createHandler(data, mutex);
    };
    return getState<T>(create);
}

std::string getCurrentState(RaceManager& raceManager)
{
    std::mutex mutex;
    auto raceExchange = RaceExchange::create(raceManager);
    auto raceHandler = Handler::createRaceHandler(*raceExchange, [&mutex] {return &mutex;});
    auto race = toValue(raceHandler->handleGet().second);
    auto checklines = getStateMutex<RaceChecklineExchange>(Handler::createRaceChecklineHandler, mutex);
    auto items = getStateMutex<RaceBonusItemExchange>(Handler::createRaceBonusItemHandler, mutex);
    auto karts = getStateMutex<RaceKartExchange>(Handler::createRaceKartHandler, mutex);
    auto materials = getStateMutex<RaceMaterialExchange>(Handler::createRaceMaterialHandler, mutex);
    auto music = getStateMutex<RaceMusicExchange>(Handler::createRaceMusicHandler, mutex);
    auto objects = getStateMutex<RaceObjectExchange>(Handler::createRaceObjectHandler, mutex);
    auto quads = getState<RaceQuadExchange>(Handler::createRaceQuadHandler);
    auto sfx = getStateMutex<RaceSfxExchange>(Handler::createRaceSfxHandler, mutex);
    auto weather = getState<RaceWeatherExchange>(Handler::createRaceWeatherHandler);

    rapidjson::Document result;
    auto& alloc = result.GetAllocator();
    result.SetObject();
    result.AddMember("status", race, alloc);
    result.AddMember("checklines", checklines, alloc);
    result.AddMember("items", items, alloc);
    result.AddMember("karts", karts, alloc);
    result.AddMember("materials", materials, alloc);
    result.AddMember("music", music, alloc);
    result.AddMember("objects", objects, alloc);
    result.AddMember("quads", quads, alloc);
    result.AddMember("sfx", sfx, alloc);
    result.AddMember("weather", weather, alloc);
    return toString(result);
}

}
