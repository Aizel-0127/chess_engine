#pragma once
#include <cstdint>
#include <array>
#include <sstream>
#include "bitboard.h"
#include "bitboard_lookup.h"
#include "move.h"
// === StateInfo ===
// A struct for irreversible info about the position, for unmaking moves.
struct StateInfo {
    StateInfo() = default;
    PieceType capturedPiece{ NO_TYPE };
    int castlingRights{ NO_CASTLE };
    Square enPassantRights{ NO_SQ };
    int fiftyMoveNum{ 0 };
    std::vector<uint64_t> hashes;
};

class Position
{
public:
    Position() = default;
    Position(const Position& pos) :
        bbByColour(pos.bbByColour),
        bbByType(pos.bbByType),
        occupancy(pos.occupancy),
        sideToMove(pos.sideToMove),
        castlingRights(pos.castlingRights),
        enPassantRights(pos.enPassantRights),
        fiftyMoveNum(pos.fiftyMoveNum),
        halfmoveNum(pos.halfmoveNum),
        gameover(pos.gameover),
        undoStack(pos.undoStack)
    {
    };
    std::array<Bitboard, NUM_COLOURS> bbByColour{};
    std::array<Bitboard, NUM_PIECE_TYPES> bbByType{};   //PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
    Bitboard occupancy{ 0 };
    // Game state information
    Colour sideToMove{ WHITE };
    int castlingRights = 0;
    Square enPassantRights{ NO_SQ };
    int fiftyMoveNum{ 0 };
    int halfmoveNum{ 0 };
    bool gameover = false;
    // Vector of hashed positions for 3 move draw
    std::vector<uint64_t> hashes;
    // Vector of unrestorable information for unmaking moves.
    std::vector<StateInfo> undoStack;
public:
    void reset();
    std::string pretty_cb() const;
    // --- Initialise from FEN string ---
    void setFromFen(const std::string& fenStr);
    void addPiece(PieceType piece, Colour colour, Square sq);
    void removePiece(Square sq);
    void removePiece(Square sq, PieceType);
    PieceType figurePieceFromSq(Square sq);
    void makeCastlingMove(Move mv);
    void unmakeCastlingMove(Move mv);
    // --- Move making/unmaking ---
    void makeMove(Move mv);
    void makeMoveFronStr_UCI(std::string usi_str);
    void unmakeMove(Move mv);
    void setStartingPosition();
    uint64_t calculateHash();

    

    //inline Position& operator=(const Position& rhs) {
    //    for (int i = 0; i < NUM_COLOURS; i++)
    //    {
    //        bbByColour[i] = rhs.bbByColour[i];
    //    }
    //    for (int i = 0; i < NUM_COLOURS; i++)
    //    {
    //        bbByColour[i] = rhs.bbByColour[i];
    //    }
    //    occupancy = rhs.occupancy;
    //    sideToMove = rhs.sideToMove;
    //    castlingRights = rhs.castlingRights;
    //    enPassantRights = rhs.enPassantRights;
    //    fiftyMoveNum = rhs.fiftyMoveNum;
    //    halfmoveNum = rhs.halfmoveNum;
    //}

};

uint64_t murmur64(uint64_t h);
