#include "bitboard.h"

const std::string pretty(Bitboard bb) {
    std::string outStr{ "+--------+\n" };
    for (int y = 0; y < 8; y++) {
        outStr.push_back('|');
        uint8_t byte = bb >> (8 * (7 - y));
        for (int x = 7; x >= 0; x--) {
            outStr += (byte & (1 << x)) ? 'X' : '0';
        }
        outStr += "|\n";
    }
    outStr += "+--------+\n";
    return outStr;
}

