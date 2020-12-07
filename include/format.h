#ifndef FORMAT_H
#define FORMAT_H

#include <string>

namespace Format {
    std::string PaddedString(std::string original_string, int size, char padding);
    std::string ElapsedTime(long times);
};                                    // namespace Format

#endif