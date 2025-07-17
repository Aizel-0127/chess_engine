#include "evaluation.h"

int countOnes(Bitboard n)
{
    n -= (n >> 1) & 0x5555555555555555llu;
    n = ((n >> 2) & 0x3333333333333333llu) + (n & 0x3333333333333333llu);
    n = ((((n >> 4) + n) & 0x0F0F0F0F0F0F0F0Fllu)
        * 0x0101010101010101) >> 56;
    return n;
}

int materialEval(Position& pos)
{
    if (pos.fiftyMoveNum >= 50)
        return DRAW_EVALUATION * (!pos.sideToMove==WHITE) - 2 * DRAW_EVALUATION * (!pos.sideToMove == BLACK);
    bool second_repetition=false;
    for (int i = pos.hashes.size() - 1; i >= 0; i--)
    {
        second_repetition = false;
        for (int j = i - 1; j >= 0; j--)
        {
            //std::cout << "\ni : " << pos.hashes[i] << "\nj : " << pos.hashes[j] << std::endl;
            if (pos.hashes[i] == pos.hashes[j])
            {
                if (second_repetition)
                    return DRAW_EVALUATION * ((!pos.sideToMove == WHITE) - 2 * (!pos.sideToMove == BLACK));
                else second_repetition = true;
            }
        }
        
    }
    int res = countOnes(pos.bbByType[PAWN] & pos.bbByColour[WHITE]) - countOnes(pos.bbByType[PAWN] & pos.bbByColour[BLACK]) +
        3 * (countOnes(pos.bbByType[KNIGHT] & pos.bbByColour[WHITE]) - countOnes(pos.bbByType[KNIGHT] & pos.bbByColour[BLACK])) +
        3 * (countOnes(pos.bbByType[BISHOP] & pos.bbByColour[WHITE]) - countOnes(pos.bbByType[BISHOP] & pos.bbByColour[BLACK])) +
        5 * (countOnes(pos.bbByType[ROOK] & pos.bbByColour[WHITE]) - countOnes(pos.bbByType[ROOK] & pos.bbByColour[BLACK])) +
        9 * (countOnes(pos.bbByType[QUEEN] & pos.bbByColour[WHITE]) - countOnes(pos.bbByType[QUEEN] & pos.bbByColour[BLACK]));
        
    return res;
}

//void play_console(int depth,Colour co, Position& pos)
//{
//    // Recursive function to count all legal moves (nodes) at depth n.
//    uint64_t nodes = 0;
//    // Terminating condition
//    if (depth == 0) { return; }
//    //std::cout << pos.pretty_cb();
//    Movelist mvlist = generateLegalMoves(pos);
//    int sz = mvlist.size();
//    // Recurse.
//    for (int i = 0; i < sz; ++i) {
//        pos.makeMove(mvlist[i]);
//        if (!isInCheck(!pos.sideToMove, pos))
//        {
//            /* int N= perft(depth - 1, pos);
//             nodes += N;*/
//            nodes += perft(depth - 1, pos);
//            /*if (depth == 1)
//            {
//                 std::cout << toString(mvlist[i]) << "  " << N<<std::endl;
//            }*/
//        }
//        pos.unmakeMove(mvlist[i]);
//
//    }
//}
