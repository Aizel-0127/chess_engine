#pragma once
#include <cstdint>
#include <string>


typedef uint64_t Bitboard;

// Returns a string visualisation of a bitboard; useful to print and debug
const std::string pretty(Bitboard bb);

void occupancy_from_FEN(std::string FEN);