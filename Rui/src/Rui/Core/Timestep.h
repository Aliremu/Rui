#pragma once

namespace Rui {
    class Timestep {
    public:
        Timestep(double time, double dt, double interpolation) : m_Time(time), m_dt(dt), m_Interpolation(interpolation) {};
        ~Timestep() = default;

        double m_Time;
        double m_dt;
        double m_Interpolation;
    };
}