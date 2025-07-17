#ifndef MOVEGEN_INCLUDED
#define MOVEGEN_INCLUDED

#include "bitboard.h"
#include "bitboard_lookup.h"
#include "move.h"
#include "position.h"
#include <cstdint>

Movelist generateLegalMoves(Position& pos);
bool isInCheck(Colour co, const Position& pos);
bool isLegal(Move mv, Position& pos);

uint64_t perft(int depth, Position& pos);
uint64_t perft_parallel(int depth, Position& pos);
// === Functions to generate particular types of valid moves ===
void addKingMoves(Movelist& mvlist, const Position& pos);
void addKnightMoves(Movelist& mvlist, const Position& pos);
void addBishopMoves(Movelist& mvlist, const Position& pos);
void addRookMoves(Movelist& mvlist, const Position& pos);
void addQueenMoves(Movelist& mvlist, const Position& pos);

void addPawnAttacks(Movelist& mvlist, const Position& pos);	//includes EnPassant
void addPawnMoves(Movelist& mvlist, const Position& pos);

bool isCastlingValid(CastlingRights cr, const Position& pos);
void addCastlingMoves(Movelist& mvlist, const Position& pos);

// === Useful auxiliary functions ===
Bitboard attacksFrom(Square sq, Colour co, PieceType pcty, const Position& pos);
Bitboard attacksTo(Square sq, Colour co, const Position& pos);
bool isAttacked(Square sq, Colour co, const Position& pos);

#endif //#ifndef MOVEGEN_INCLUDED
