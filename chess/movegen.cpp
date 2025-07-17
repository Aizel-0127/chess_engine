#include "movegen.h"
#include <vector>
#include <future>

Movelist generateLegalMoves(Position& pos)
{
    if (pos.gameover)
        return Movelist{};
    Colour co{ pos.sideToMove };
    Movelist mvlist{};
    // Start generating valid moves.
    int i = 0;
    //std::cout<<pos.pretty_cb();
    addKingMoves(mvlist, pos);
    addKnightMoves(mvlist, pos);
    addBishopMoves(mvlist, pos);
    addRookMoves(mvlist, pos);
    addQueenMoves(mvlist, pos);
    addPawnMoves(mvlist, pos);
    addPawnAttacks(mvlist, pos);
    addCastlingMoves(mvlist, pos);
    // Test for checks.
    for (auto it = mvlist.begin(); it != mvlist.end();) {
        if (isLegal(*it, pos)) {
            ++it;
        }
        else {
            it = mvlist.erase(it);
        }
    }
    if (mvlist.empty())
        pos.gameover=true;
    return mvlist;
}

bool isInCheck(Colour co, const Position& pos)
{
    // Test if a side (colour) is in check.
    Bitboard bb{ pos.bbByType[KING] & pos.bbByColour[co]};
    Square sq{ static_cast<Square>(leastSignificantBit(bb)) }; // assumes exactly one king per side.
    return isAttacked(sq, !co, pos);
}

bool isLegal(Move mv, Position& pos)
{
    // Test if making a move would leave one's own royalty in check.
    // Assumes move is valid.
    // For eventual speedup logic can be improved from naive make-unmake-make.
    pos.makeMove(mv);
    bool isSuicide{ isInCheck(!(pos.sideToMove), pos) };
    pos.unmakeMove(mv);
    return !isSuicide;
}

uint64_t perft(int depth, Position& pos)
{
    // Recursive function to count all legal moves (nodes) at depth n.
    uint64_t nodes = 0;
    // Terminating condition
    if (depth == 0) { return 1; }
    //std::cout << pos.pretty_cb();
    Movelist mvlist = generateLegalMoves(pos);
    int sz = mvlist.size();
    // Recurse.
    for (int i = 0; i < sz; ++i) {
        pos.makeMove(mvlist[i]);
        if (!isInCheck(!pos.sideToMove,pos))
        {
           /* int N= perft(depth - 1, pos);
            nodes += N;*/
            nodes += perft(depth - 1, pos);
           /*if (depth == 1)
           {
                std::cout << toString(mvlist[i]) << "  " << N<<std::endl;
           }*/
        }
        pos.unmakeMove(mvlist[i]);
        
    }
    return nodes;
}

uint64_t perft_parallel(int depth, Position& pos)
{
    // Recursive function to count all legal moves (nodes) at depth n.

    // Terminating condition
    if (depth == 0) { return 1; }
    Movelist mvlist = generateLegalMoves(pos);
    int sz = mvlist.size();

    std::vector<std::future<uint64_t>> futures;  // Для хранения результатов

    for (int i = 0; i < sz; ++i) {
        Position pos_copy(pos);
        pos_copy.makeMove(mvlist[i]);
        futures.emplace_back(std::async(std::launch::async, [depth, pos_copy]() {
            if (!isInCheck(!pos_copy.sideToMove, pos_copy)) {
                return perft(depth - 1, const_cast<Position&>(pos_copy));
            }
            return uint64_t(0);
            }));
    }
    uint64_t total_nodes = 0;
    for (auto& future : futures) {
        total_nodes += future.get();  // Блокирует, пока поток не завершится
    }
    return total_nodes;
}

void addKingMoves(Movelist& mvlist, const Position& pos) {
    Bitboard bbFrom{ pos.bbByType[KING] & pos.bbByColour[pos.sideToMove]};
    Square fromSq{ NO_SQ };
    Bitboard bbTo{ 0 };
    fromSq = static_cast<Square>(leastSignificantBit(bbFrom));
    bbTo = kingAttacks[fromSq] & ~pos.bbByColour[pos.sideToMove];
    Square destination{ NO_SQ };
    while (bbTo) {
        destination = static_cast<Square>(leastSignificantBit(bbTo));
        bbTo &= bbTo - 1;
        mvlist.push_back(buildMove(fromSq, destination));
    }
}

void addKnightMoves(Movelist& mvlist, const Position& pos)
{
    Bitboard bbFrom{ pos.bbByType[KNIGHT] & pos.bbByColour[pos.sideToMove] };
    Square fromSq{ NO_SQ };
    Bitboard bbTo{ 0 };
    Square destination{ NO_SQ };
    while (bbFrom) {
        fromSq = static_cast<Square>(leastSignificantBit(bbFrom));
        bbFrom &= bbFrom - 1;
        bbTo = knightAttacks[fromSq] & ~pos.bbByColour[pos.sideToMove];
        while (bbTo) {
            destination = static_cast<Square>(leastSignificantBit(bbTo));
            bbTo &= bbTo - 1;
            mvlist.push_back(buildMove(fromSq, destination));
        }
    }
}

void addBishopMoves(Movelist& mvlist, const Position& pos)
{
    Bitboard bbFrom{ pos.bbByType[BISHOP] & pos.bbByColour[pos.sideToMove] };
    Square fromSq{ NO_SQ };
    Bitboard bbTo{ 0 };
    Square destination{ NO_SQ };
    while (bbFrom) {
        fromSq = static_cast<Square>(leastSignificantBit(bbFrom));
        bbFrom &= bbFrom - 1;
        bbTo = (findDiagAttacks(fromSq,pos.occupancy) | findAntidiagAttacks(fromSq, pos.occupancy)) & ~pos.bbByColour[pos.sideToMove];
        while (bbTo) {
            destination = static_cast<Square>(leastSignificantBit(bbTo));
            bbTo &= bbTo - 1;
            mvlist.push_back(buildMove(fromSq, destination));
        }
    }
}

void addRookMoves(Movelist& mvlist, const Position& pos)
{
    Bitboard bbFrom{ pos.bbByType[ROOK] & pos.bbByColour[pos.sideToMove] };
    Square fromSq{ NO_SQ };
    Bitboard bbTo{ 0 };
    Square destination{ NO_SQ };
    while (bbFrom) {
        fromSq = static_cast<Square>(leastSignificantBit(bbFrom));
        bbFrom &= bbFrom - 1;
        bbTo = (findFileAttacks(fromSq, pos.occupancy) | findRankAttacks(fromSq, pos.occupancy)) & ~pos.bbByColour[pos.sideToMove];
        while (bbTo) {
            destination = static_cast<Square>(leastSignificantBit(bbTo));
            bbTo &= bbTo - 1;
            mvlist.push_back(buildMove(fromSq, destination));
        }
    }
}

void addQueenMoves(Movelist& mvlist, const Position& pos)
{
    Bitboard bbFrom{ pos.bbByType[QUEEN] & pos.bbByColour[pos.sideToMove] };
    Square fromSq{ NO_SQ };
    Bitboard bbTo{ 0 };
    Square destination{ NO_SQ };
    while (bbFrom) {
        fromSq = static_cast<Square>(leastSignificantBit(bbFrom));
        bbFrom &= bbFrom - 1;
        bbTo = (findFileAttacks(fromSq, pos.occupancy) | findRankAttacks(fromSq, pos.occupancy) |
            findDiagAttacks(fromSq, pos.occupancy) | findAntidiagAttacks(fromSq, pos.occupancy)) & ~pos.bbByColour[pos.sideToMove];
        while (bbTo) {
            destination = static_cast<Square>(leastSignificantBit(bbTo));
            bbTo &= bbTo - 1;
            mvlist.push_back(buildMove(fromSq, destination));
        }
    }
}

void addPawnAttacks(Movelist& mvlist, const Position& pos)
{

    Bitboard bbFrom{ pos.bbByType[PAWN] & pos.bbByColour[pos.sideToMove] };
    Square fromSq{ NO_SQ };
    Square destination{ NO_SQ };
    Bitboard bbTo{ 0 };
    Bitboard EnPassantBB{ 0 };
    if (pos.enPassantRights != NO_SQ)
        EnPassantBB = 1ULL << pos.enPassantRights;
    while (bbFrom) {
        fromSq = static_cast<Square>(leastSignificantBit(bbFrom));
        bbFrom &= bbFrom - 1;
        bbTo = pawnAttacks[pos.sideToMove][fromSq] & pos.bbByColour[!pos.sideToMove] ;
        while (bbTo) {
            destination = static_cast<Square>(leastSignificantBit(bbTo));
            bbTo &= bbTo - 1;
            if (destination >= 0 + 56 * (pos.sideToMove == WHITE) && destination <= 7 + 56 * (pos.sideToMove == WHITE))
            {
                mvlist.push_back(buildPromotion(fromSq, destination, KNIGHT));
                mvlist.push_back(buildPromotion(fromSq, destination, BISHOP));
                mvlist.push_back(buildPromotion(fromSq, destination, ROOK));
                mvlist.push_back(buildPromotion(fromSq, destination, QUEEN));
            } 
            else
                mvlist.push_back(buildMove(fromSq, destination));
        }
        if (bbTo = pawnAttacks[pos.sideToMove][fromSq] & EnPassantBB)
            mvlist.push_back(buildEnPassant(fromSq, static_cast<Square>(leastSignificantBit(bbTo))));
    }
}

void addPawnMoves(Movelist& mvlist, const Position& pos)
{
    
    Bitboard bbFrom{ pos.bbByType[PAWN] & pos.bbByColour[pos.sideToMove]};
    Square toSq{ NO_SQ };

    while (bbFrom) {
        Square fromSq = static_cast<Square>(leastSignificantBit(bbFrom));
        bbFrom &= bbFrom - 1;
        toSq = static_cast<Square>(fromSq + 8 - 16 * (pos.sideToMove == BLACK));
        if (((1ULL << toSq) & pos.occupancy) == 0)
        {
            if (fromSq >= 48 - 40 * (pos.sideToMove == BLACK) && fromSq <= 55 - 40 * (pos.sideToMove == BLACK))
            {
                mvlist.push_back(buildPromotion(fromSq, toSq, KNIGHT));
                mvlist.push_back(buildPromotion(fromSq, toSq, BISHOP));
                mvlist.push_back(buildPromotion(fromSq, toSq, ROOK));
                mvlist.push_back(buildPromotion(fromSq, toSq, QUEEN));
            }
            else
            {
                mvlist.push_back(buildMove(fromSq, toSq));
                if (fromSq >= 8 + 40 * (pos.sideToMove == BLACK) && fromSq <= 15 + 40 * (pos.sideToMove == BLACK))
                {
                    toSq = static_cast<Square>(fromSq + 16 - 32 * (pos.sideToMove == BLACK));
                    if (((1ULL << toSq) & pos.occupancy) == 0)
                    {
                        mvlist.push_back(buildMove(fromSq, toSq));
                    }
                }
            }
        }
    }
}

bool isCastlingValid(CastlingRights cr, const Position& pos)
{
    // Test if king or relevant rook have moved.
    if (!(cr & pos.castlingRights)) {
        return false;
    }
    Bitboard pathMask{ 0 };
    Bitboard bbOthers{ 0 };
    Colour co;
    switch (cr)
    {
    case NO_CASTLE:
        return false;
        break;
    case CASTLE_WSHORT:
        pathMask = 0x0000000000000070;
        bbOthers = pos.occupancy ^ 0x0000000000000010;
        co = WHITE;
        break;
    case CASTLE_WLONG:
        pathMask = 0x000000000000001E;
        bbOthers = pos.occupancy ^ 0x0000000000000010;
        co = WHITE;
        break;
    case CASTLE_BSHORT:
        pathMask = 0x7000000000000000;
        bbOthers = pos.occupancy ^ 0x1000000000000000;
        co = BLACK;
        break;
    case CASTLE_BLONG:
        pathMask = 0x1E00000000000000;
        bbOthers = pos.occupancy ^ 0x1000000000000000;
        co = BLACK;
        break;
    default:
        return false;
        break;
    }
    // Test if king and rook paths are clear of obstruction.
    if (pathMask & bbOthers) {
        return false;
    }
    // Test if there are attacked squares in the king's path.
    Square sq{ NO_SQ };
    if (cr & CASTLE_WLONG)
    {
        pathMask ^= 0x0000000000000002;
    }
    else if (cr & CASTLE_BLONG)
    {
        pathMask ^= 0x0200000000000000;
    }
    while (pathMask) {
        sq = static_cast<Square>(leastSignificantBit(pathMask));
        pathMask &= pathMask - 1;
        if (isAttacked(sq, !co, pos)) {
            return false;
        }
    }
    // Conditions met, castling is valid.
    return true;
}

void addCastlingMoves(Movelist& mvlist, const Position& pos)
{
    if (pos.sideToMove == WHITE) {
        if (isCastlingValid(CASTLE_WSHORT, pos)) {
            Move mv{ buildCastling(E1, H1) };
            mvlist.push_back(mv);
        }
        if (isCastlingValid(CASTLE_WLONG, pos)) {
            Move mv{ buildCastling(E1, A1) };
            mvlist.push_back(mv);
        }
    }
    else if (pos.sideToMove == BLACK) {
        if (isCastlingValid(CASTLE_BSHORT, pos)) {
            Move mv{ buildCastling(E8, H8) };
            mvlist.push_back(mv);
        }
        if (isCastlingValid(CASTLE_BLONG, pos)) {
            Move mv{ buildCastling(E8, A8) };
            mvlist.push_back(mv);
        }
    }
}

Bitboard attacksFrom(Square sq, Colour co, PieceType pcty, const Position& pos)
{
    // Returns bitboard of squares attacked by a given piece type placed on a
    // given square.
    Bitboard bbAttacked{ 0 };
    switch (pcty) {
    case PAWN:
        // Note: Does not check that an enemy piece is on the target square!
        bbAttacked = pawnAttacks[co][sq];
        break;
    case KNIGHT:
        bbAttacked = knightAttacks[sq];
        break;
    case BISHOP:
        bbAttacked = findDiagAttacks(sq, pos.occupancy) | findAntidiagAttacks(sq, pos.occupancy);
        break;
    case ROOK:
        bbAttacked = findRankAttacks(sq, pos.occupancy) | findFileAttacks(sq, pos.occupancy);
        break;
    case QUEEN:
        bbAttacked = findRankAttacks(sq, pos.occupancy) | findFileAttacks(sq, pos.occupancy) |
            findDiagAttacks(sq, pos.occupancy) | findAntidiagAttacks(sq, pos.occupancy);
        break;
    case KING:
        bbAttacked = kingAttacks[sq];
        break;
    }
    return bbAttacked;
}

Bitboard attacksTo(Square sq, Colour co, const Position& pos)
{
    // Returns bitboard of units of a given colour that attack a given square.
    // In chess, most piece types have the following property: if piece PC is on
    // square SQ_A attacking SQ_B, then from SQ_B it would attack SQ_A.
    Bitboard bbAttackers{ 0 };
    bbAttackers = kingAttacks[sq] & pos.bbByType[KING] & pos.bbByColour[co];
    bbAttackers |= knightAttacks[sq] & pos.bbByType[KNIGHT] & pos.bbByColour[co];
    bbAttackers |= (findDiagAttacks(sq, pos.occupancy) | findAntidiagAttacks(sq, pos.occupancy))
        & (pos.bbByType[BISHOP] | pos.bbByType[QUEEN]) & pos.bbByColour[co];
    bbAttackers |= (findRankAttacks(sq, pos.occupancy) | findFileAttacks(sq, pos.occupancy))
        & (pos.bbByType[ROOK] | pos.bbByType[QUEEN]) & pos.bbByColour[co];
    // But for pawns, a square SQ_A is attacked by a [Colour] pawn on SQ_B,
    // if a [!Colour] pawn on SQ_A would attack SQ_B.
    bbAttackers |= pawnAttacks[!co][sq] & pos.bbByType[PAWN] & pos.bbByColour[co];
    return bbAttackers;
}

bool isAttacked(Square sq, Colour co, const Position& pos)
{
    // Returns if a square is attacked by pieces of a particular colour.
    return attacksTo(sq, co, pos);
}
