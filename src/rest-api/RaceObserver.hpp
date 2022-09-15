#pragma once
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace RestApi
{

class RaceResultLoader
{
public:
    static std::unique_ptr<RaceResultLoader> create();

public:
    [[nodiscard]] virtual std::optional<uint64_t> getLatestId() const = 0;
    [[nodiscard]] virtual std::string get(uint64_t id) const = 0;
    virtual void store(uint64_t id, const std::string& result) = 0;
};

class RaceObserver
{
public:
    RaceObserver(RaceResultLoader& loader, std::function<std::string()> getCurrentState);
    void start();
    void update();
    void stop();
    [[nodiscard]] std::optional<uint64_t> getCurrentRaceId() const;

private:
    RaceResultLoader& loader_;
    std::function<std::string()> getCurrentState_;
    bool active_;
    std::optional<uint64_t> id_;
};

}
