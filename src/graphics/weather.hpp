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

#ifndef HEADER_WEATHER_HPP
#define HEADER_WEATHER_HPP

#include "utils/singleton.hpp"
#include <vector3d.h>

class SFXBase;
class ParticleKind;

namespace irr::video
{
class SColor;
}

struct WeatherData
{
    uint32_t skyColorAsARGB;
    std::string sound;
    std::string particles;
    bool lightning;
};

class Weather final : public AbstractSingleton<Weather>
{
public:
    static void changeCurrentWeather(const WeatherData& weather);

private:
    float m_next_lightning;
    float m_lightning;
    bool m_stopped;

    SFXBase* m_thunder_sound;
    SFXBase* m_weather_sound;

public:
    Weather();
    ~Weather() override;

    void update(float dt);
    void playSound();
    void stop();
    void change(const std::string& particles, const std::string& sound, bool lightning);
    bool getLightning() const;
    std::string getSound() const;
    irr::video::SColor getSkyColor() const;
    const ParticleKind* const getParticles() const;

    /** Set the flag that a lightning should be shown. */
    void startLightning() { m_lightning = 1.0f; }
    bool shouldLightning() const { return m_lightning > 0.0f; }

    irr::core::vector3df getIntensity() const;

};

#endif
