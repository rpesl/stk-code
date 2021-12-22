//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015  Joerg Henrichs, Marianne Gagnon
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "audio/sfx_base.hpp"
#include "audio/sfx_buffer.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_openal.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/weather.hpp"
#include "karts/kart.hpp"
#include "karts/controller/local_player_controller.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "utils/random_generator.hpp"
#include "particle_kind_manager.hpp"


/**  The weather manager stores information about the weather.
 */
Weather::Weather()
: m_lightning(0.0f)
, m_stopped(false)
, m_thunder_sound(nullptr)
, m_weather_sound(nullptr)
{
    if (Track::getCurrentTrack()->getWeatherLightning())
    {
        m_thunder_sound = SFXManager::get()->createSoundSource("thunder");
    }

    const std::string &sound = Track::getCurrentTrack()->getWeatherSound();
    if (!sound.empty())
    {
        m_weather_sound = SFXManager::get()->createSoundSource(sound);
    }

    RandomGenerator g;
    m_next_lightning = static_cast<float>(g.get(35));
}   // Weather

// ----------------------------------------------------------------------------

Weather::~Weather()
{
    if (m_thunder_sound != NULL)
        m_thunder_sound->deleteSFX();

    if (m_weather_sound != NULL)
        m_weather_sound->deleteSFX();
}   // ~Weather

// ----------------------------------------------------------------------------

void Weather::update(float dt)
{
    if (!getLightning())
        return;

    if (World::getWorld()->getRaceGUI() == NULL)
        return;

    if (m_stopped)
        return;

    m_next_lightning -= dt;

    if (m_next_lightning < 0.0f)
    {
        startLightning();

        if (m_thunder_sound &&
            World::getWorld()->getPhase() != WorldStatus::IN_GAME_MENU_PHASE)
        {
            m_thunder_sound->play();
        }

        RandomGenerator g;
        m_next_lightning = 35 + (float)g.get(35);
    }

    if (m_lightning > 0.0f)
    {
        m_lightning -= dt;
    }
}   // update

// ----------------------------------------------------------------------------

void Weather::playSound()
{
    if (m_weather_sound)
    {
        m_weather_sound->setLoop(true);
        m_weather_sound->play();
    }
}

static void setSkyParticles(ParticleKind* sky_particles)
{
    Track* track = Track::getCurrentTrack();
    World* world = World::getWorld();
    if (world && track)
    {
        track->setSkyParticles(sky_particles);
        for (const auto& kart : world->getKarts())
        {
            Controller* controller = kart->getController();
            if (controller && controller->isLocalPlayerController())
            {
                auto* lpc = dynamic_cast<LocalPlayerController*>(controller);
                if (lpc)
                    lpc->initParticleEmitter();
            }
        }
    }
}

void Weather::stop()
{
    m_stopped = true;
    if (m_weather_sound)
    {
        m_weather_sound->stop();
        m_weather_sound->deleteSFX();
        m_weather_sound = nullptr;
    }
    if (m_thunder_sound)
    {
        m_thunder_sound->stop();
        m_thunder_sound->deleteSFX();
        m_thunder_sound = nullptr;
    }
    m_lightning = 0.0f;
    setSkyParticles(nullptr);
}

void Weather::change(const std::string& particles, const std::string& sound, bool lightning)
{
    stop();
    m_stopped = false;
    if (lightning)
        m_thunder_sound = SFXManager::get()->createSoundSource("thunder");
    if (!sound.empty())
    {
        m_weather_sound = SFXManager::get()->createSoundSource(sound);
    }
    m_next_lightning = 0;
    playSound();
    ParticleKind* sky_particles = particles.empty() ? nullptr : ParticleKindManager::get()->getParticles(particles);
    setSkyParticles(sky_particles);
}

irr::core::vector3df Weather::getIntensity() const
{
    irr::core::vector3df value = {0.7f * m_lightning,
                                  0.7f * m_lightning,
                                  0.7f * std::min(1.0f, m_lightning * 1.5f)};

    return value;
}

bool Weather::getLightning() const
{
    return m_thunder_sound;
}

std::string Weather::getSound() const
{
    if (!m_weather_sound)
    {
        return "";
    }
    auto sfx = dynamic_cast<SFXOpenAL*>(m_weather_sound);
    assert(sfx);
    return sfx->getBuffer()->getName();
}

irr::video::SColor Weather::getSkyColor() const
{
    return irr_driver->getClearColor();
}

const ParticleKind* const Weather::getParticles() const
{
    auto track = Track::getCurrentTrack();
    if (!track)
        throw std::runtime_error("Track does not exist");
    return track->getSkyParticles();
}

void Weather::changeCurrentWeather(const WeatherData& weather)
{
    World* world = World::getWorld();
    if (!world)
    {
        throw std::runtime_error("World does not exist");
    }
    world->m_weather_change = std::make_unique<WeatherData>(weather);
    world->m_finished = false;
    std::unique_lock<std::mutex> lk(world->m_mutex);
    world->m_cv.wait(lk, [&] () -> bool {return world->m_finished;});
}
