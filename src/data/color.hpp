#ifndef BTY_DATA_COLOR_HPP_
#define BTY_DATA_COLOR_HPP_

#include <glm/vec4.hpp>

inline constexpr unsigned char const kBoxAccents[5][2][3] =
    {
        // Easy, green
        {
            {0, 32, 0},
            {66, 130, 66},
        },
        // Normal, blue
        {
            {0, 0, 33},
            {65, 65, 132},
        },
        // Hard, red
        {
            {33, 0, 0},
            {132, 65, 65},
        },
        // Impossible, grey
        {
            {33, 32, 33},
            {132, 130, 132},
        },
        // Intro, light blue
        {
            {0, 32, 99},
            {33, 163, 232},
        },
};

inline constexpr glm::vec4 const kBoxAccentVecs[5][2] = {
    {
        {kBoxAccents[0][0][0] / 255.0f, kBoxAccents[0][0][1] / 255.0f, kBoxAccents[0][0][2] / 255.0f, 1.0f},
        {kBoxAccents[0][1][0] / 255.0f, kBoxAccents[0][1][1] / 255.0f, kBoxAccents[0][1][2] / 255.0f, 1.0f},
    },
    {
        {kBoxAccents[1][0][0] / 255.0f, kBoxAccents[1][0][1] / 255.0f, kBoxAccents[1][0][2] / 255.0f, 1.0f},
        {kBoxAccents[1][1][0] / 255.0f, kBoxAccents[1][1][1] / 255.0f, kBoxAccents[1][1][2] / 255.0f, 1.0f},
    },
    {
        {kBoxAccents[2][0][0] / 255.0f, kBoxAccents[2][0][1] / 255.0f, kBoxAccents[2][0][2] / 255.0f, 1.0f},
        {kBoxAccents[2][1][0] / 255.0f, kBoxAccents[2][1][1] / 255.0f, kBoxAccents[2][1][2] / 255.0f, 1.0f},
    },
    {
        {kBoxAccents[3][0][0] / 255.0f, kBoxAccents[3][0][1] / 255.0f, kBoxAccents[3][0][2] / 255.0f, 1.0f},
        {kBoxAccents[3][1][0] / 255.0f, kBoxAccents[3][1][1] / 255.0f, kBoxAccents[3][1][2] / 255.0f, 1.0f},
    },
    {
        {kBoxAccents[4][0][0] / 255.0f, kBoxAccents[4][0][1] / 255.0f, kBoxAccents[4][0][2] / 255.0f, 1.0f},
        {kBoxAccents[4][1][0] / 255.0f, kBoxAccents[4][1][1] / 255.0f, kBoxAccents[4][1][2] / 255.0f, 1.0f},
    },
};

namespace bty {

enum class BoxColor {
    Easy,
    Normal,
    Hard,
    Impossible,
    Intro,
    None,
};

inline constexpr glm::vec4 getColor(BoxColor color, bool outline = false)
{
    int o = static_cast<int>(!outline);
    switch (color) {
        case BoxColor::Easy:
            return kBoxAccentVecs[0][o];
        case BoxColor::Normal:
            return kBoxAccentVecs[1][o];
        case BoxColor::Hard:
            return kBoxAccentVecs[2][o];
        case BoxColor::Impossible:
            return kBoxAccentVecs[3][o];
        case BoxColor::Intro:
            return kBoxAccentVecs[4][o];
        default:
            break;
    }
    return {1, 1, 1, 1};
}

inline constexpr BoxColor getBoxColor(int difficulty)
{
    switch (difficulty) {
        case 0:
            return BoxColor::Easy;
        case 1:
            return BoxColor::Normal;
        case 2:
            return BoxColor::Hard;
        case 3:
            return BoxColor::Impossible;
        default:
            break;
    }
    return BoxColor::None;
}

}    // namespace bty

#endif    // BTY_DATA_COLOR_HPP_
