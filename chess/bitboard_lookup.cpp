#include <iostream>
#include <cstdint>
#include "bitboard_lookup.h"
#include "bitboard.h"

// Arrays of first-rank/file attacks, for slider move generation.
// Indexed by the 8 possible slider locations, and 2^(8 - 2) = 64 non-edge
// occupancy states.
std::array<std::array<Bitboard, 64>, 8> firstRankAttacks{};
std::array<std::array<Bitboard, 64>, 8> firstFileAttacks{};

// Defining lookup arrays exposed in .h
std::array<Bitboard, NUM_SQUARES> knightAttacks{};
std::array<Bitboard, NUM_SQUARES> kingAttacks{};
std::array<std::array<Bitboard, NUM_SQUARES>, NUM_COLOURS> pawnAttacks{};
std::array<Bitboard, 15> diagMasks{};
std::array<Bitboard, 15> antidiagMasks{};

const int index64[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

/**
 * bitScanForward
 * @author Kim Walisch (2012)
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */
int leastSignificantBit(uint64_t bb) {
    if (bb == 0)
        return -1;
    const Bitboard debruijn64 = 0x03f79d71b4cb0a89;
    return index64[((bb ^ (bb - 1)) * debruijn64) >> 58];
}

int mostSignificantBit(uint64_t x) {
    if (x == 0) return -1;  // если все биты нулевые
    uint64_t x_buf = static_cast<uint64_t>(1) << 63;
    int index = 63;
    while (index>=0)
    {
        if (x & x_buf) return index;
        else
        {
            x_buf = x_buf >> 1;
            index--;
        }
    }
    return index;
}

// --- 1st-rank and 1st-file attacks ---
void initialiseFirstRankAttacks() {
    for (int ioc = 0; ioc < 64; ++ioc) {
        // +129 sets the end bits of rank to 1 (cannot attack past board edge)
        Bitboard oc = (ioc << 1) + 129;
        //std::cout << pretty(oc);
        // If slider at end of rank, that end must be handled differently.
        // Therefore use two separate if conditions to isolate them.
        for (int SliderSquareIndex = 0; SliderSquareIndex <= 7; SliderSquareIndex++) {
            int mostLeftAttackedSquare{ 0 };
            int mostRightAttackedSquare{ 7 };
            Bitboard r{ Bitboard(1)<< SliderSquareIndex };
            //std::cout << pretty(r);
            if (SliderSquareIndex != 0) {
                mostLeftAttackedSquare = mostSignificantBit(oc & (r - 1));
            }
            if (SliderSquareIndex != 7) {
                mostRightAttackedSquare = leastSignificantBit(oc & ~((r << 1) - 1));
            }
            // Get bitboard of all bits between limits, inclusive.
            Bitboard bb = (static_cast<uint64_t>(1) << (mostRightAttackedSquare + 1)) - (static_cast<uint64_t>(1) << mostLeftAttackedSquare);
            //std::cout << pretty(bb);
            bb ^= r; // slider does not attack itself
            //std::cout << pretty(bb);
            bb *= 0x0101010101010101ULL; // north-fill multiplication
            //std::cout << pretty(bb);
            firstRankAttacks[SliderSquareIndex][ioc] = bb;
        }
    }
    return;
}

// --- 1st-rank and 1st-file attacks ---
void initialiseFirstFileAttacks() {
    for (int ioc = 0; ioc < 64; ++ioc) {
        // +129 sets the end bits of rank to 1 (cannot attack past board edge)
        Bitboard oc = (ioc << 1) + 129;
       // std::cout << pretty(oc);
        // If slider at end of rank, that end must be handled differently.
        // Therefore use two separate if conditions to isolate them.
        for (int SliderSquareIndex = 0; SliderSquareIndex <= 7; SliderSquareIndex++) {
            int mostLeftAttackedSquare{ 0 };
            int mostRightAttackedSquare{ 7 };
            Bitboard r{ Bitboard(1) << SliderSquareIndex };
            //std::cout << pretty(r);
            if (SliderSquareIndex != 0) {
                mostLeftAttackedSquare = mostSignificantBit(oc & (r - 1));
            }
            if (SliderSquareIndex != 7) {
                mostRightAttackedSquare = leastSignificantBit(oc & ~((r << 1) - 1));
            }
            // Get bitboard of all bits between limits, inclusive.
            Bitboard bb = (static_cast<uint64_t>(1) << (mostRightAttackedSquare + 1)) - (static_cast<uint64_t>(1) << mostLeftAttackedSquare);
            //std::cout << pretty(bb);
            bb ^= r; // slider does not attack itself
           // std::cout << pretty(bb);
            bb = (bb * 0x8040201008040201ULL) & (0x0101010101010101ULL<<7); // rotate attacks to the h-file.
            //std::cout << pretty(bb);
            // Now fill left.
            bb |= bb >> 1;
           // std::cout << pretty(bb);
            bb |= bb >> 2;
            //std::cout << pretty(bb);
            bb |= bb >> 4;
            //std::cout << pretty(bb);
            firstFileAttacks[7-SliderSquareIndex][ioc] = bb;
        }
    }
    return;
}

void initialiseAllDiagMasks() {
    diagMasks[0] = 0x0000000000000080ULL;
    antidiagMasks[0] = 0x0000000000000001ULL;

    for (int i = 1; i < 8; i++) {
        diagMasks[i] = (diagMasks[i - 1]>>1) ^ (1ULL<<(8*(i&7)+7));
        antidiagMasks[i] = (antidiagMasks[i-1]<<1) ^ (1ULL << 8*(i&7));
    }
    diagMasks[8] = (diagMasks[7] >> 1);
    antidiagMasks[8] = (antidiagMasks[7] << 1) ^ (1ULL<<8);
    for (int i = 9; i < 15; i++) {
        diagMasks[i] = (diagMasks[i - 1] >> 1) ^ (1ULL << (8 * ((i & 7) -1) + 7));
        antidiagMasks[i] = (antidiagMasks[i - 1] << 1) ^ (1ULL << 8 * ((i & 7)+1));
    }
    return;
}

// --- Simple piece attacks ---
void initialiseKnightAttacks() {
    for (int sq = 0; sq < NUM_SQUARES; sq++) {
        Bitboard bb{ 1ULL<<sq };
        Bitboard NotA_file = 0xFEFEFEFEFEFEFEFE;
        Bitboard NotH_file = 0x7F7F7F7F7F7F7F7F;
        Bitboard NotAB_file = 0xFCFCFCFCFCFCFCFC;
        Bitboard NotGH_file = 0x3F3F3F3F3F3F3F3F;
        bb = ((((bb<<16)<<1) & NotA_file) | (((bb << 16) >> 1) & NotH_file) |
            (((bb >> 16) << 1) & NotA_file) | (((bb >> 16) >> 1) & NotH_file) |
            (((bb << 8) << 2) & NotAB_file) | (((bb << 8) >> 2) & NotGH_file) |
            (((bb >> 8) << 2) & NotAB_file) | (((bb >> 8) >> 2) & NotGH_file));
        knightAttacks[sq] = bb;
    }
    return;
}


void initialiseKingAttacks() {
    Bitboard NotA_file = 0xFEFEFEFEFEFEFEFE;
    Bitboard NotH_file = 0x7F7F7F7F7F7F7F7F;
    for (int sq = 0; sq < NUM_SQUARES; sq++) {
        Bitboard bb{ 1ULL<<sq };
        bb = (((bb<<9) & NotA_file) | (bb<<8) | ((bb << 7) & NotH_file) | ((bb << 1) & NotA_file) |
            ((bb >> 1) & NotH_file) | ((bb >> 7) & NotA_file) | (bb>>8) | ((bb >> 9) & NotH_file));
        kingAttacks[sq] = bb;
    }
    return;
}


void initialisePawnAttacks() {
    // Will generate legal moves for illegal pawn positions too (1st/8th rank)
    Bitboard NotA_file = 0xFEFEFEFEFEFEFEFE;
    Bitboard NotH_file = 0x7F7F7F7F7F7F7F7F;
    for (int sq = 0; sq < NUM_SQUARES; sq++) {
        Bitboard bb{ 1ULL<<sq };
        pawnAttacks[WHITE][sq] = ((bb << 9) & NotA_file) | ((bb << 7) & NotH_file);
        pawnAttacks[BLACK][sq] = ((bb >> 7) & NotA_file) | ((bb >> 9) & NotH_file);
    }
    return;
}

void initializeLookupTables()
{
    initialiseFirstRankAttacks();
    initialiseFirstFileAttacks();
    initialiseAllDiagMasks();
    initialiseKnightAttacks();
    initialiseKingAttacks();
    initialisePawnAttacks();
}

Bitboard diagonalFromSq(int sq)
{
    return diagMasks[7 - (sq & 7) + (sq >> 3)];
}

Bitboard antidiagonalFromSq(int sq)
{
    return antidiagMasks[(sq & 7) + (sq >> 3)];
}

Bitboard findFileAttacks(int sq,const Bitboard& occupancy) {
    int irank{ sq / 8 };
    int ifile{ sq % 8 };
    Bitboard oc{ (occupancy >> ifile) & 0x0101010101010101ULL }; // send desired file bits to a-file
    //std::cout << pretty(oc);
    // multiply by b2-h7 diagonal; flip multiplication extracts lookup index
    int ioc{ static_cast<int>(oc * 0x0080402010080400ULL >> 58) }; //0004081020408000
    //std::cout << pretty(ioc);
    return firstFileAttacks[irank][ioc] & (0x0101010101010101ULL << ifile);
}

Bitboard findRankAttacks(int sq, const Bitboard& occupancy) {
    int irank{ sq / 8 };
    int ifile{ sq & 7 };
    uint64_t BB_1 = 255;
    uint64_t BB_B{ 0x0101010101010101ULL << 1};
    Bitboard oc{ occupancy & (BB_1 << (8 * irank)) }; // extract just desired rank
    // b-file multiplication puts desired bits on 8th rank to extract.
    int ioc{ static_cast<int>((oc * BB_B) >> (64 - 6)) };
    return (BB_1 << (8 * irank)) & firstRankAttacks[ifile][ioc];
}

Bitboard findDiagAttacks(int sq, const Bitboard& occupancy) {
    int ifile{ sq & 7 };
    Bitboard oc{ occupancy & (diagonalFromSq(sq)) }; // extract just desired diagonal
    // b-file multiplication puts desired bits on 8th rank.
    int ioc = { static_cast<int>((oc * 0x0202020202020202) >> 58) };
    return diagonalFromSq(sq) & firstRankAttacks[ifile][ioc];
}

Bitboard findAntidiagAttacks(int sq, const Bitboard& occupancy) {
    int ifile{ sq & 7 };
    Bitboard oc{ occupancy & (antidiagonalFromSq(sq)) };
    // b-file multiplication puts desired bits on 8th rank.
    int ioc = { static_cast<int>((oc * 0x0202020202020202) >> 58) };
    return antidiagonalFromSq(sq) & firstRankAttacks[ifile][ioc];
}
//
//int main()
//{
//    std::cout.setf(std::ios::unitbuf);
//    occupancy_from_FEN("r1bk3r/p2pBpNp/n4n2/1p1NP2P/6P1/3P4/P1P1K3/q5b1 1");
//    //std::cout << pretty(occupancy);
//   
//    initialiseFirstRankAttacks();
//    initialiseAllDiagMasks();
//    initialiseFirstFileAttacks();
//    initialiseKnightAttacks();
//    initialiseKingAttacks();
//    initialisePawnAttacks();
//   // std::cout << pretty(findFileAttacks(29));
//    for (int i=0;i<64;i++)
//        std::cout << pretty(kingAttacks[i]);
//}