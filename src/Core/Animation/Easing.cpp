#include <cmath>
#include <map>
#include <string>

#include <Animation/Easing.hpp>
#include <Animation/Exceptions.hpp>
#include <Utils/MathUtils.hpp>

namespace obe::Animation::Easing
{
    using Utils::Math::pi;

    double Linear(double t)
    {
        return t;
    }

    double InSine(double t)
    {
        return sin(1.5707963 * t);
    }

    double OutSine(double t)
    {
        return 1 + sin(1.5707963 * (--t));
    }

    double InOutSine(double t)
    {
        return 0.5 * (1 + sin(3.1415926 * (t - 0.5)));
    }

    double InQuad(double t)
    {
        return t * t;
    }

    double OutQuad(double t)
    {
        return t * (2 - t);
    }

    double InOutQuad(double t)
    {
        return t < 0.5 ? 2 * t * t : t * (4 - 2 * t) - 1;
    }

    double InCubic(double t)
    {
        return t * t * t;
    }

    double OutCubic(double t)
    {
        --t;
        return 1 + t * t * t;
    }

    double InOutCubic(double t)
    {
        return t < 0.5 ? 4 * t * t * t : 1 + 4 * (t-2) * (t-2) * (t-2);
    }

    double InQuart(double t)
    {
        t *= t;
        return t * t;
    }

    double OutQuart(double t)
    {
        --t;
        t = t * t;
        return 1 - t * t;
    }

    double InOutQuart(double t)
    {
        if (t < 0.5)
        {
            t *= t;
            return 8 * t * t;
        }
        else
        {
            --t;
            t = t * t;
            return 1 - 8 * t * t;
        }
    }

    double InQuint(double t)
    {
        double t2 = t * t;
        return t * t2 * t2;
    }

    double OutQuint(double t)
    {
        --t;
        double t2 = t * t;
        return 1 + t * t2 * t2;
    }

    double InOutQuint(double t)
    {
        double t2;
        if (t < 0.5)
        {
            t2 = t * t;
            return 16 * t * t2 * t2;
        }
        else
        {
            --t;
            t2 = t * t;
            return 1 + 16 * t * t2 * t2;
        }
    }

    double InExpo(double t)
    {
        return (pow(2, 8 * t) - 1) / 255;
    }

    double OutExpo(double t)
    {
        return 1 - pow(2, -8 * t);
    }

    double InOutExpo(double t)
    {
        if (t < 0.5)
        {
            return (pow(2, 16 * t) - 1) / 510;
        }
        else
        {
            return 1 - 0.5 * pow(2, -16 * (t - 0.5));
        }
    }

    double InCirc(double t)
    {
        return 1 - sqrt(1 - t);
    }

    double OutCirc(double t)
    {
        return sqrt(t);
    }

    double InOutCirc(double t)
    {
        if (t < 0.5)
        {
            return (1 - sqrt(1 - 2 * t)) * 0.5;
        }
        else
        {
            return (1 + sqrt(2 * t - 1)) * 0.5;
        }
    }

    double InBack(double t)
    {
        return t * t * (2.70158 * t - 1.70158);
    }

    double OutBack(double t)
    {
        --t;
        return 1 + t * t * (2.70158 * t + 1.70158);
    }

    double InOutBack(double t)
    {
        if (t < 0.5)
        {
            return t * t * (7 * t - 2.5) * 2;
        }
        else
        {
            --t;
            return 1 + t * t * 2 * (7 * t + 2.5);
        }
    }

    double InElastic(double t)
    {
        double t2 = t * t;
        return t2 * t2 * sin(t * pi * 4.5);
    }

    double OutElastic(double t)
    {
        double t2 = (t - 1) * (t - 1);
        return 1 - t2 * t2 * cos(t * pi * 4.5);
    }

    double InOutElastic(double t)
    {
        double t2;
        if (t < 0.45)
        {
            t2 = t * t;
            return 8 * t2 * t2 * sin(t * pi * 9);
        }
        else if (t < 0.55)
        {
            return 0.5 + 0.75 * sin(t * pi * 4);
        }
        else
        {
            t2 = (t - 1) * (t - 1);
            return 1 - 8 * t2 * t2 * sin(t * pi * 9);
        }
    }

    double InBounce(double t)
    {
        return pow(2, 6 * (t - 1)) * abs(sin(t * pi * 3.5));
    }

    double OutBounce(double t)
    {
        return 1 - pow(2, -6 * t) * abs(cos(t * pi * 3.5));
    }

    double InOutBounce(double t)
    {
        if (t < 0.5)
        {
            return 8 * pow(2, 8 * (t - 1)) * abs(sin(t * pi * 7));
        }
        else
        {
            return 1 - 8 * pow(2, -8 * t) * abs(sin(t * pi * 7));
        }
    }

    EasingFunction get(EasingType easing)
    {
        static std::unordered_map<EasingType, EasingFunction> easingFunctions;
        if (easingFunctions.empty())
        {
            easingFunctions.insert(std::make_pair(EasingType::Linear, Linear));
            easingFunctions.insert(std::make_pair(EasingType::InSine, InSine));
            easingFunctions.insert(std::make_pair(EasingType::OutSine, OutSine));
            easingFunctions.insert(std::make_pair(EasingType::InOutSine, InOutSine));
            easingFunctions.insert(std::make_pair(EasingType::InQuad, InQuad));
            easingFunctions.insert(std::make_pair(EasingType::OutQuad, OutQuad));
            easingFunctions.insert(std::make_pair(EasingType::InOutQuad, InOutQuad));
            easingFunctions.insert(std::make_pair(EasingType::InCubic, InCubic));
            easingFunctions.insert(std::make_pair(EasingType::OutCubic, OutCubic));
            easingFunctions.insert(std::make_pair(EasingType::InOutCubic, InOutCubic));
            easingFunctions.insert(std::make_pair(EasingType::InQuart, InQuart));
            easingFunctions.insert(std::make_pair(EasingType::OutQuart, OutQuart));
            easingFunctions.insert(std::make_pair(EasingType::InOutQuart, InOutQuart));
            easingFunctions.insert(std::make_pair(EasingType::InQuint, InQuint));
            easingFunctions.insert(std::make_pair(EasingType::OutQuint, OutQuint));
            easingFunctions.insert(std::make_pair(EasingType::InOutQuint, InOutQuint));
            easingFunctions.insert(std::make_pair(EasingType::InExpo, InExpo));
            easingFunctions.insert(std::make_pair(EasingType::OutExpo, OutExpo));
            easingFunctions.insert(std::make_pair(EasingType::InOutExpo, InOutExpo));
            easingFunctions.insert(std::make_pair(EasingType::InCirc, InCirc));
            easingFunctions.insert(std::make_pair(EasingType::OutCirc, OutCirc));
            easingFunctions.insert(std::make_pair(EasingType::InOutCirc, InOutCirc));
            easingFunctions.insert(std::make_pair(EasingType::InBack, InBack));
            easingFunctions.insert(std::make_pair(EasingType::OutBack, OutBack));
            easingFunctions.insert(std::make_pair(EasingType::InOutBack, InOutBack));
            easingFunctions.insert(std::make_pair(EasingType::InElastic, InElastic));
            easingFunctions.insert(std::make_pair(EasingType::OutElastic, OutElastic));
            easingFunctions.insert(
                std::make_pair(EasingType::InOutElastic, InOutElastic));
            easingFunctions.insert(std::make_pair(EasingType::InBounce, InBounce));
            easingFunctions.insert(std::make_pair(EasingType::OutBounce, OutBounce));
            easingFunctions.insert(std::make_pair(EasingType::InOutBounce, InOutBounce));
        }

        if (const auto function = easingFunctions.find(easing);
            function != easingFunctions.end())
        {
            return function->second;
        }
        const int enumValue = static_cast<std::underlying_type_t<EasingType>>(easing);
        throw Exceptions::UnknownEasingFromEnum(enumValue, EXC_INFO);
    }

    EasingFunction get(const std::string& easing)
    {
        static std::unordered_map<std::string, EasingFunction> easingFunctions;
        if (easingFunctions.empty())
        {
            easingFunctions.insert(std::make_pair("Linear", Linear));
            easingFunctions.insert(std::make_pair("InSine", InSine));
            easingFunctions.insert(std::make_pair("OutSine", OutSine));
            easingFunctions.insert(std::make_pair("InOutSine", InOutSine));
            easingFunctions.insert(std::make_pair("InQuad", InQuad));
            easingFunctions.insert(std::make_pair("OutQuad", OutQuad));
            easingFunctions.insert(std::make_pair("InOutQuad", InOutQuad));
            easingFunctions.insert(std::make_pair("InCubic", InCubic));
            easingFunctions.insert(std::make_pair("OutCubic", OutCubic));
            easingFunctions.insert(std::make_pair("InOutCubic", InOutCubic));
            easingFunctions.insert(std::make_pair("InQuart", InQuart));
            easingFunctions.insert(std::make_pair("OutQuart", OutQuart));
            easingFunctions.insert(std::make_pair("InOutQuart", InOutQuart));
            easingFunctions.insert(std::make_pair("InQuint", InQuint));
            easingFunctions.insert(std::make_pair("OutQuint", OutQuint));
            easingFunctions.insert(std::make_pair("InOutQuint", InOutQuint));
            easingFunctions.insert(std::make_pair("InExpo", InExpo));
            easingFunctions.insert(std::make_pair("OutExpo", OutExpo));
            easingFunctions.insert(std::make_pair("InOutExpo", InOutExpo));
            easingFunctions.insert(std::make_pair("InCirc", InCirc));
            easingFunctions.insert(std::make_pair("OutCirc", OutCirc));
            easingFunctions.insert(std::make_pair("InOutCirc", InOutCirc));
            easingFunctions.insert(std::make_pair("InBack", InBack));
            easingFunctions.insert(std::make_pair("OutBack", OutBack));
            easingFunctions.insert(std::make_pair("InOutBack", InOutBack));
            easingFunctions.insert(std::make_pair("InElastic", InElastic));
            easingFunctions.insert(std::make_pair("OutElastic", OutElastic));
            easingFunctions.insert(std::make_pair("InOutElastic", InOutElastic));
            easingFunctions.insert(std::make_pair("InBounce", InBounce));
            easingFunctions.insert(std::make_pair("OutBounce", OutBounce));
            easingFunctions.insert(std::make_pair("InOutBounce", InOutBounce));
        }

        if (const auto function = easingFunctions.find(easing);
            function != easingFunctions.end())
        {
            return function->second;
        }
        throw Exceptions::UnknownEasingFromString(easing, EXC_INFO);
    }
}
