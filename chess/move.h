#ifndef MOVE
#define MOVE
#pragma once
#include <iostream>
#include <cstdint>
#include <vector>
#include "bitboard_lookup.h"

enum PieceType : int {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
    NO_TYPE
};

typedef uint16_t Move;
typedef std::vector<Move> Movelist;
enum MoveSpecial {
    MV_NORMAL, MV_PROMOTION, MV_CASTLING, MV_ENPASSANT
};
//enum PromotionFlag {
//    KNIGHT, BISHOP, ROOK, QUEEN
//};
// === move.h ===
// Contains the internal representation of a chess move and associated methods.
// Uses a plain 16-bit integer type (uint16_t).
// Uses Stockfish move encoding: 16 bits to store a move. From least to most
// significant bits:
//
// -from- --to-- sp pr
// 100000 100000 10 10
//
// 6 bits each for the from/to squares (since 2^6 = 64), 2 bits for special move
// flags, 2 bits for promotion piece type flag.
// Special flag: promotion = 1 (01), castling = 2 (10), en passant = 3 (11)
// Promotion type flag: KNIGHT = 00, BISHOP = 01, ROOK = 10, QUEEN = 11.
//
// If extending for variants, promotion type flag can hold extra information (if
// the special flag is not set as promotion, then the bits can be repurposed.)

inline bool isPromotion(Move mv) { return ((mv >> 12) & 0x3) == MV_PROMOTION; }
inline bool isCastling(Move mv) { return ((mv >> 12) & 0x3) == MV_CASTLING; }
inline bool isEnPassant(Move mv) { return ((mv >> 12) & 0x3) == MV_ENPASSANT; }
inline int getSpecial(Move mv) { return (mv >> 12) & 0x3; }
inline PieceType getPromotionType(Move mv) {
    return PieceType(((mv >> 14) & 0x3) + 1); // here N=0 but PieceType N=1.
}
// === "constructors" for Move ===
inline Move buildMove(Square fromSq, Square toSq) {
    Move mv{ 0 };
    mv |= fromSq;
    mv |= (toSq << 6);
    return mv;
};

inline Move buildPromotion(Square fromSq, Square toSq, PieceType p_type) {
    Move mv{ 0 };
    mv |= fromSq;
    mv |= (toSq << 6);
    mv |= (MV_PROMOTION << 12);
    mv |= ((p_type - 1) << 14);
    return mv;
}

inline Move buildCastling(Square fromSq, Square toSq) {
    // For move encoding, fromSq/toSq are the king's/rook's initial squares.
    Move mv{ 0 };
    mv |= fromSq;
    mv |= (toSq << 6);
    mv |= (MV_CASTLING << 12);
    return mv;
}

inline Move buildEnPassant(Square fromSq, Square toSq) {
    Move mv{ 0 };
    mv |= fromSq;
    mv |= (toSq << 6);
    mv |= (MV_ENPASSANT << 12);
    return mv;
}


// Conversion to string for debugging
inline std::string toString(Move mv) {
    if (mv == 0) { return "-----  "; }
    std::string outStr;
    outStr.push_back('a' + (mv & 7));
    outStr.push_back('1' + ((mv >> 3) & 7));
    outStr += "-";
    outStr.push_back('a' + ((mv >> 6) & 7));
    outStr.push_back('1' + ((mv >> 9) & 7));
    int sp = getSpecial(mv);
    std::string PieceTypeStr = "PNBRQK";
    switch (sp) {
    case MV_PROMOTION: {
        int ipt = getPromotionType(mv);
        outStr += "=";
        outStr.push_back(PieceTypeStr[ipt]);
        break;
    }
    case MV_CASTLING: outStr += "csl"; break;
    case MV_ENPASSANT: outStr += "en_pas"; break;
    default: outStr += "  "; break;
    }
    outStr += " ";
    return outStr;
}

inline std::string toStringUCI(Move mv) {
    if (mv == 0) { return ""; }
    std::string outStr;
    outStr.push_back('a' + (mv & 7));
    outStr.push_back('1' + ((mv >> 3) & 7));
    outStr.push_back('a' + ((mv >> 6) & 7));
    outStr.push_back('1' + ((mv >> 9) & 7));
    if (outStr == "e1h1")
        outStr = "e1g1";
    else if (outStr == "e1a1")
        outStr = "e1c1";
    else if (outStr == "e8h8")
        outStr = "e8g8";
    else if (outStr == "e8a8")
        outStr = "e8c8";
    return outStr;
}

#endif //#ifndef MOVE

