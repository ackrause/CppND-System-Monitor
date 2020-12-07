#include <string>

#include "format.h"

using std::string;

string Format::PaddedString(string original_string, int size, char padding) {
    if (original_string.length() >= size) {
        return original_string;
    }

    return string(size - original_string.length(), padding) + original_string;
}

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
    long hours, minutes;
    hours = seconds / 3600;
    minutes = (seconds / 60) % 60;
    seconds = seconds % 60;

    return Format::PaddedString(std::to_string(hours), 2, '0') + ":" +
           Format::PaddedString(std::to_string(minutes), 2, '0') + ":" +
           Format::PaddedString(std::to_string(seconds), 2, '0');
}
