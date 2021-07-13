#include <Transform/Exceptions.hpp>
#include <Transform/Referential.hpp>

#include <array>
#include <cassert>
#include <ostream> // Weird fix on latest versions of MSVC
#include <regex>

#include <fmt/format.h>

namespace obe::Transform
{
    Referential Referential::TopLeft = Referential(0, 0);
    Referential Referential::Top = Referential(0.5, 0);
    Referential Referential::TopRight = Referential(1, 0);
    Referential Referential::Left = Referential(0, 0.5);
    Referential Referential::Center = Referential(0.5, 0.5);
    Referential Referential::Right = Referential(1, 0.5);
    Referential Referential::BottomLeft = Referential(0, 1);
    Referential Referential::Bottom = Referential(0.5, 1);
    Referential Referential::BottomRight = Referential(1, 1);
    std::array<Referential, 9> Referential::Referentials
        = { Referential::TopLeft, Referential::Top, Referential::TopRight,
              Referential::Left, Referential::Center, Referential::Right,
              Referential::BottomLeft, Referential::Bottom, Referential::BottomRight };

    Referential::Referential()
        : m_refX(0)
        , m_refY(0)
    {
    }

    Referential::Referential(const double refX, const double refY)
        : m_refX(refX)
        , m_refY(refY)
    {
        assert(refX >= -1 && refX <= 1);
        assert(refY >= -1 && refY <= 1);
    }


    bool Referential::operator==(const Referential& ref) const
    {
        return (getOffset() == ref.getOffset());
    }
    bool Referential::operator!=(const Referential& ref) const
    {
        return !((*this) == ref);
    }

    Referential Referential::flip(FlipAxis axis) const
    {
        const bool bothOrHorizontal
            = (axis == FlipAxis::Both || axis == FlipAxis::Horizontal);
        const bool bothOrVertical
            = (axis == FlipAxis::Both || axis == FlipAxis::Vertical);
        return Referential(
            bothOrHorizontal ? 1 - m_refX : m_refX, bothOrVertical ? 1 - m_refY : m_refY);
    }

    bool Referential::isOnLeftSide() const
    {
        return m_refX == 0;
    }

    bool Referential::isOnRightSide() const
    {
        return m_refX == 1;
    }

    bool Referential::isOnTopSide() const
    {
        return m_refY == 0;
    }

    bool Referential::isOnBottomSide() const
    {
        return m_refY == 1;
    }

    bool Referential::isOnCorner() const
    {
        return (isOnLeftSide() || isOnRightSide()) && (isOnTopSide() || isOnBottomSide());
    }

    bool Referential::isOnSide() const
    {
        return (isOnLeftSide() || isOnRightSide()) ^ (isOnTopSide() || isOnBottomSide());
    }

    bool Referential::isKnown() const
    {
        return (m_refX == 0 || m_refX == 0.5 || m_refX == 1)
            && (m_refY == 0 || m_refY == 0.5 || m_refY == 1);
    }

    UnitVector Referential::getOffset() const
    {
        return UnitVector(m_refX, m_refY);
    }

    std::string Referential::toString(const std::string& format) const
    {
        if (m_refX == 0 && m_refY == 0)
            return fmt::format(format, "TopLeft");
        if (m_refX == 0.5 && m_refY == 0)
            return fmt::format(format, "Top");
        if (m_refX == 1 && m_refY == 0)
            return fmt::format(format, "TopRight");
        if (m_refX == 0 && m_refY == 0.5)
            return fmt::format(format, "Left");
        if (m_refX == 0.5 && m_refY == 0.5)
            return fmt::format(format, "Center");
        if (m_refX == 1 && m_refY == 0.5)
            return fmt::format(format, "Right");
        if (m_refX == 0 && m_refY == 1)
            return fmt::format(format, "BottomLeft");
        if (m_refX == 0.5 && m_refY == 1)
            return fmt::format(format, "Bottom");
        if (m_refX == 1 && m_refY == 1)
            return fmt::format(format, "BottomRight");
        else
            return fmt::format(
                format, fmt::format("{}, {}", m_refX, m_refY), m_refX, m_refY);
    }

    Referential Referential::FromString(const std::string& ref)
    {
        if (ref == "TopLeft")
            return Referential::TopLeft;
        if (ref == "Top")
            return Referential::Top;
        if (ref == "TopRight")
            return Referential::TopRight;
        if (ref == "Left")
            return Referential::Left;
        if (ref == "Center")
            return Referential::Center;
        if (ref == "Right")
            return Referential::Right;
        if (ref == "BottomLeft")
            return Referential::BottomLeft;
        if (ref == "Bottom")
            return Referential::Bottom;
        if (ref == "BottomRight")
            return Referential::BottomRight;
        std::cmatch regMatch;
        const std::regex refRegex(
            R"(Referential<\s*(-?\d+(\.\d+)?)\s*,\s*(-?\d+(\.\d+)?)\s*>)");
        std::regex_match(ref.c_str(), regMatch, refRegex);
        if (regMatch.size() == 5)
        {
            return Referential(std::stod(regMatch[1]), std::stod(regMatch[3]));
        }
        throw Exceptions::UnknownReferential(ref, EXC_INFO);
    }

    std::ostream& operator<<(std::ostream& os, Referential m)
    {
        os << m.toString();
        return os;
    }
} // namespace obe::Transform
