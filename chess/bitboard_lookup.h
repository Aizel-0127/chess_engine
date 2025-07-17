#pragma once
#include <array>
#include "bitboard.h"

constexpr int NUM_SQUARES{ 64 };
constexpr int NUM_COLOURS{ 2 };
constexpr int NUM_PIECE_TYPES{ 6 };
enum Colour{BLACK,WHITE};
// Converts white to black and vice versa.
inline Colour operator!(Colour co) {
    return static_cast<Colour>(static_cast<int>(co) ^ 0x1);
}
enum CastlingRights{
  // Four basic types of castling.
  NO_CASTLE = 0,
  CASTLE_WSHORT = 1,
  CASTLE_WLONG = CASTLE_WSHORT << 1,
  CASTLE_BSHORT = CASTLE_WSHORT << 2,
  CASTLE_BLONG = CASTLE_WSHORT << 3
};
enum Square : int {
   A1, B1, C1, D1, E1, F1, G1, H1,
   A2, B2, C2, D2, E2, F2, G2, H2,
   A3, B3, C3, D3, E3, F3, G3, H3,
   A4, B4, C4, D4, E4, F4, G4, H4,
   A5, B5, C5, D5, E5, F5, G5, H5,
   A6, B6, C6, D6, E6, F6, G6, H6,
   A7, B7, C7, D7, E7, F7, G7, H7,
   A8, B8, C8, D8, E8, F8, G8, H8,
  NO_SQ
};

// Indexed by square on the chessboard.
extern std::array<Bitboard, NUM_SQUARES> knightAttacks;
extern std::array<Bitboard, NUM_SQUARES> kingAttacks;
// Pawn attacks depend on colour, so indexed by square then colour.
extern std::array<std::array<Bitboard, NUM_SQUARES>, NUM_COLOURS> pawnAttacks;

// Indexed by square on the chessboard. Contains the Bitboard of the
// corresponding (anti)diagonal passing through that square.
extern std::array<Bitboard, 15> diagMasks;
extern std::array<Bitboard, 15> antidiagMasks;

// Indexed by square on the chessboard.
extern std::array<Bitboard, NUM_SQUARES> knightAttacks;
extern std::array<Bitboard, NUM_SQUARES> kingAttacks;
// Pawn attacks depend on colour, so indexed by square then colour.
extern std::array<std::array<Bitboard, NUM_SQUARES>, NUM_COLOURS> pawnAttacks;

void initialiseFirstRankAttacks();
void initialiseFirstFileAttacks();
void initialiseAllDiagMasks();
void initialiseKnightAttacks();
void initialiseKingAttacks();
void initialisePawnAttacks();
void initializeLookupTables();

Bitboard findDiagAttacks(int sq, const Bitboard& occupancy);
Bitboard findAntidiagAttacks(int sq, const Bitboard& occupancy);
Bitboard findRankAttacks(int sq, const Bitboard& occupancy);
Bitboard findFileAttacks(int sq, const Bitboard& occupancy);
int leastSignificantBit(uint64_t x);
int mostSignificantBit(uint64_t x);