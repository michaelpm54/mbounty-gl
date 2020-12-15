#ifndef BTY_DATA_MORALE_HPP_
#define BTY_DATA_MORALE_HPP_

/* clang-format off */
inline constexpr int const kMoraleGroups[25] = {
	0, 2, 0, 3, 4,
	4, 2, 3, 1, 2,
	1, 2, 2, 4, 1,
	3, 2, 3, 1, 2,
	2, 4, 2, 4, 3,
};

/* 0 = normal
   1 = high
   2 = low
*/
inline constexpr int const kMoraleChart[25] = {
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 1, 0, 0,
	2, 0, 2, 1, 0,
	2, 2, 2, 0, 0,
};
/* clang-format on */

namespace bty {

inline constexpr int check_morale(int me, int *army)
{
    bool normal = false;
    for (int i = 0; i < 5; i++) {
        if (army[i] == -1) {
            continue;
        }
        int result = kMoraleChart[kMoraleGroups[army[me]] + kMoraleGroups[army[i]] * 5];
        if (result == 2) {
            return 1;
        }
        if (result == 0) {
            normal = true;
        }
    }
    if (normal) {
        return 0;
    }
    return 2;
}

}    // namespace bty

#endif    // BTY_DATA_MORALE_HPP_
