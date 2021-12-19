#include <stdexcept>
#include <unordered_map>
#include <utility>
#include "rest-api/RaceObserver.hpp"

namespace RestApi
{
namespace
{
class InMemoryRaceResultLoader final : public RaceResultLoader
{
public:
    std::optional<size_t> getLatestId() const override
    {
        return latestId_;
    }
    std::string get(size_t id) const override
    {
        auto element = data_.find(id);
        if (element == data_.end())
        {
            throw std::invalid_argument("No results for id " + std::to_string(id));
        }
        return element->second;
    }
    void store(size_t id, const std::string& result) override
    {
        latestId_ = latestId_ ? std::max(latestId_.value(), id) : id;
        data_.insert_or_assign(id, result);
    }
private:
    std::optional<size_t> latestId_;
    std::unordered_map<size_t, std::string> data_;
};
}

std::unique_ptr<RaceResultLoader> RaceResultLoader::create()
{
    return std::make_unique<InMemoryRaceResultLoader>();
}

RaceObserver::RaceObserver(RaceResultLoader& loader, std::function<std::string()>  getCurrentState)
: loader_(loader)
, getCurrentState_(std::move(getCurrentState))
, active_(false)
, id_(loader_.getLatestId())
{
}

void RaceObserver::start()
{
    active_ = true;
    id_ = id_ ? id_.value() + 1 : 0;
    loader_.store(id_.value(), getCurrentState_());
}

void RaceObserver::update()
{
    loader_.store(id_.value(), getCurrentState_());
}

void RaceObserver::stop()
{
    loader_.store(id_.value(), getCurrentState_());
    active_ = false;
}

std::optional<size_t> RaceObserver::getCurrentRaceId() const
{
    return active_ ? id_ : std::nullopt;
}

}
