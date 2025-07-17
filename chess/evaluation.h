#pragma once
#include "position.h"
#include "move.h"

const int DRAW_EVALUATION = -2;
const int CHECKMATE_EVALUATION = 10000;

int countOnes(Bitboard n);

int materialEval(Position& pos);

//void play_console(int depth,Colour co, Position& pos);