/*REMEMBER: YOU MUST ADD EXPORTS FOR EACH SYMBOL TO ITS CORRESPONDING MODULE IN THE LIBRARY*/
/*REMEMBER: YOU MUST ADD EXPORTS FOR EACH SYMBOL TO ITS CORRESPONDING MODULE IN THE LIBRARY*/

#pragma once
#include <cstdint>
#include <string_view>

//unscoped because having to add static_cast becomes annoying
enum CompPreset : uint8_t
{
    NO_COMP,
    COMP_1,
    COMP_2,
    COMP_3,
    COMP_4,
    COMP_5,
    COMP_6,
    COMP_7,
    COMP_8,
    COMP_MAX,
};

/**
 * @brief Identify compressor used
 *
 * @note You must not reorder the enums or (especially) remove MAX.
 *		 For every new enum, MAX shall always stay the max enum possible.
 *		 DO NOT CUSTOMIZE THE VALUES. YOU WILL MAKE PREEXISTING .tzf FILES INVALID.
 */
enum class CompType : uint8_t
{
    LZ77,
    MAX,
};

inline constexpr std::string_view COMPRESSOR_STR_OPTIONS[] =
{
    "LZ77",
};
