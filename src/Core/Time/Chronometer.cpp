#include <iostream>

#include <Time/Chronometer.hpp>

namespace obe::Time
{
    void Chronometer::start()
    {
        m_start = epoch();
        m_started = true;
    }

    bool Chronometer::started() const {
        return m_started;
    }
    void Chronometer::stop()
    {
        m_started = false;
    }

    void Chronometer::reset()
    {
        m_start = epoch();
    }

    TimeUnit Chronometer::getTime() const
    {
        if (m_started)
            return epoch() - m_start;
        return 0;
    }

    void Chronometer::setLimit(const TimeUnit limit)
    {
        m_limit = limit;
    }

    TimeUnit Chronometer::getLimit() const
    {
        return m_limit;
    }

    bool Chronometer::over() const
    {
        return m_started && this->getTime() > m_limit;
    }
} // namespace obe::Time