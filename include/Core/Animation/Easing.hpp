#pragma once

#include <functional>
#include <string>

namespace obe::Animation::Easing
{
    enum class EasingType
    {
        Linear,
        InSine,
        OutSine,
        InOutSine,
        InQuad,
        OutQuad,
        InOutQuad,
        InCubic,
        OutCubic,
        InOutCubic,
        InQuart,
        OutQuart,
        InOutQuart,
        InQuint,
        OutQuint,
        InOutQuint,
        InExpo,
        OutExpo,
        InOutExpo,
        InCirc,
        OutCirc,
        InOutCirc,
        InBack,
        OutBack,
        InOutBack,
        InElastic,
        OutElastic,
        InOutElastic,
        InBounce,
        OutBounce,
        InOutBounce
    };

    double Linear(double t);
    double InSine(double t);
    double OutSine(double t);
    double InOutSine(double t);
    double InQuad(double t);
    double OutQuad(double t);
    double InOutQuad(double t);
    double InCubic(double t);
    double OutCubic(double t);
    double InOutCubic(double t);
    double InQuart(double t);
    double OutQuart(double t);
    double InOutQuart(double t);
    double InQuint(double t);
    double OutQuint(double t);
    double InOutQuint(double t);
    double InExpo(double t);
    double OutExpo(double t);
    double InOutExpo(double t);
    double InCirc(double t);
    double OutCirc(double t);
    double InOutCirc(double t);
    double InBack(double t);
    double OutBack(double t);
    double InOutBack(double t);
    double InElastic(double t);
    double OutElastic(double t);
    double InOutElastic(double t);
    double InBounce(double t);
    double OutBounce(double t);
    double InOutBounce(double t);

    using EasingFunction = std::function<double(double)>;

    EasingFunction get(const std::string& easing);
    EasingFunction get(EasingType easing);
}
