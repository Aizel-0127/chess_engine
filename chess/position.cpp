#include "position.h"
#include "movegen.h"
#include "bitboard_lookup.h"
#include <chrono>

uint64_t murmur64(uint64_t h) {
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    return h;
}

void Position::setFromFen(const std::string& fenStr) {
    // Reads a FEN string and sets up the Position accordingly.
    std::istringstream fenSs(fenStr);
    fenSs >> std::noskipws;
    unsigned char token{ 0 };
    int isq{ static_cast<int>(A8) };
    size_t idx{ std::string::npos };

    // Clear board.
    reset();
    // Read physical position
    while ((fenSs >> token) && !isspace(token)) {
        if (isdigit(token)) {
            isq += static_cast<int>(token - '0'); // char '0' != int 0
        }
        else if (token == '/') {
            isq -= 8 + 8;
        }
        else {
            std::string white_pieces = "PNBRQK";
            std::string black_pieces = "pnbrqk";
            int index = -1;
            if ((index=white_pieces.find(token)) != std::string::npos)
            {
                addPiece(static_cast<PieceType>(index), WHITE, static_cast<Square>(isq));
            }
            else if ((index = black_pieces.find(token)) != std::string::npos)
            {
                addPiece(static_cast<PieceType>(index), BLACK, static_cast<Square>(isq));
            }
            else {
                throw std::runtime_error("Unknown character in FEN position.");
            }
            isq++;
        }
    }
    do 
    {
        fenSs >> token;
        switch (std::tolower(token)) {
            case 'w': { sideToMove = WHITE; break; }
            case 'b': { sideToMove = BLACK; break; }
            case ' ': break;
            default: {
                throw std::runtime_error("Unknown side to move in FEN.");
                break;
            }
        }
    } while (isspace(token));
    // Read castling rights.
    do {
        fenSs >> token;
    } while (isspace(token));

    while (!isspace(token)) {
        switch (token) {
            case 'K': { castlingRights |= CASTLE_WSHORT; break; }
            case 'Q': { castlingRights |= CASTLE_WLONG; break; }
            case 'k': { castlingRights |= CASTLE_BSHORT; break; }
            case 'q': { castlingRights |= CASTLE_BLONG; break; }
            case '-': { castlingRights = NO_CASTLE; break; }
            default: {
                throw std::runtime_error("Unknown castling rights in FEN.");
            }
        }
        fenSs >> token;
    }

    do {
        fenSs >> token;
    } while (isspace(token));

    // Read en passant rights (one square).
    while (!isspace(token)) {
        if (token == '-') { enPassantRights = NO_SQ; break; }
        else if (('a' <= token) && (token <= 'h')) {
            int x = static_cast<int>(token - 'a');
            if ((fenSs >> token) && ('1' <= token) && (token <= '8')) {
                int y = static_cast<int>(token - '1');
                enPassantRights = static_cast<Square>(y*8+x);
                fenSs >> token;
            }
        }
        else {
            throw std::runtime_error("Unknown en passant rights in FEN.");
        }
    }
    // Read fifty-move and fullmove counters (assumes each is one integer)
    int fullmoveNum{ 1 };
    fenSs >> std::skipws >> fiftyMoveNum >> fullmoveNum;
    // Converting a fullmove number to halfmove number.
    // Halfmove 0 = Fullmove 1 + white to move.
    halfmoveNum = 2 * fullmoveNum - 1 - (sideToMove == WHITE);
    hashes.push_back(calculateHash());
    return;
}

void Position::addPiece(PieceType piece,Colour colour, Square sq) {
    Bitboard bb = (1ULL << sq);
    bbByColour[colour] |= bb;
    bbByType[piece] |= bb;
    occupancy |= bb;
    return;
}

void Position::removePiece(Square sq) {
    Bitboard bb = ~(1ULL << sq);
    bbByColour[WHITE] &= bb;
    bbByColour[BLACK] &= bb;
    occupancy &= bb;
    for (int i = 0; i < 6; i++)
    {
        bbByType[i] &= bb;
    }
    return;
}

void Position::removePiece(Square sq, PieceType i)
{
    Bitboard bb = ~(1ULL << sq);
    bbByColour[WHITE] &= bb;
    bbByColour[BLACK] &= bb;
    occupancy &= bb;
    bbByType[i] &= bb;
}

PieceType Position::figurePieceFromSq(Square sq)
{
    Bitboard bb = (1ULL << sq);
    for (int i = 0; i < 6; i++)
    {
        if ((bb & bbByType[i]) > 0)
            return static_cast<PieceType>(i);
    }
    return PieceType{ NO_TYPE };
}

void Position::makeCastlingMove(Move mv) {
    //input is e1a1 and not a1c1 (all 4 moves)
    if (!isCastling(mv))
        throw std::runtime_error("trying to castle with wrong move");
    const Colour co{ sideToMove };
    const Square sqKFrom{ static_cast<Square>(mv & 63) };
    const Square sqRFrom{ static_cast<Square>((mv >> 6) & 63) };
    Square sqKTo{ NO_SQ };
    Square sqRTo{ NO_SQ };
    // By square encoding, further east = higher number
    if (sqKFrom > sqRFrom) {
        // King east of rook, i.e. west castling.
        if (co == WHITE) {
            sqKTo = C1;
            sqRTo = D1;
        }
        else {
            sqKTo = C8;
            sqRTo = D8;
        }
    }
    else {
        // King west of rook, i.e. east castling.
        if (co == WHITE) {
            sqKTo = G1;
            sqRTo = F1;
        }
        else {
            sqKTo = G8;
            sqRTo = F8;
        }
    }
    // Remove king and rook, and place them at their final squares.
    bbByColour[co] ^= ((1ULL << sqKFrom) | (1ULL << sqRFrom) | (1ULL << sqKTo) | (1ULL << sqRTo));
    occupancy ^= ((1ULL << sqKFrom) | (1ULL << sqRFrom) | (1ULL << sqKTo) | (1ULL << sqRTo));
    bbByType[KING] ^= ((1ULL << sqKFrom) | (1ULL << sqKTo));
    bbByType[ROOK] ^= ((1ULL << sqRFrom) | (1ULL << sqRTo));

    // Save irreversible information in struct, *before* altering them.
    const StateInfo undoState{ NO_TYPE, castlingRights,
                               enPassantRights, fiftyMoveNum,hashes };
    undoStack.push_back(undoState);
    // Update ep and castling rights.
    enPassantRights = NO_SQ;
    castlingRights &= (co == WHITE) ? ~(CASTLE_WLONG | CASTLE_WSHORT) : ~(CASTLE_BLONG | CASTLE_BSHORT);
    // Change side to move, and update fifty-move and halfmove counts.
    sideToMove = !sideToMove;
    ++fiftyMoveNum;
    hashes.clear();
    ++halfmoveNum;
    return;
}

void Position::makeMove(Move mv)
{
    // Makes a move by changing the state of Position.
    // Assumes the move is valid (not necessarily legal).
    // Must maintain validity of the Position!

    // Castling is handled in its own method.
    if (isCastling(mv)) {
        makeCastlingMove(mv);
        return;
    }

    const Square fromSq{ static_cast<Square>(mv&63) };
    const Square toSq{ static_cast<Square>((mv>>6) & 63) };
    const PieceType piece= figurePieceFromSq(fromSq);
    if (piece == NO_TYPE)
        throw std::runtime_error("Trying to make move with no piece selected");
    const Colour co{ sideToMove };

    // Remove piece from fromSq
    bbByColour[co] ^= (1ULL<<fromSq);
    bbByType[piece] ^= (1ULL<<fromSq);
    occupancy ^= (1ULL << fromSq);

    // Handle regular captures and en passant separately
    const PieceType pcDest=figurePieceFromSq(toSq);
    const bool isCapture { pcDest != NO_TYPE };
    if (isCapture) 
    {
        // Regular capture is occurring (not ep)
        bbByColour[!co] ^= (1ULL << toSq);
        bbByType[pcDest] ^= (1ULL << toSq);
        occupancy ^= (1ULL << toSq);
    } else if (isEnPassant(mv)) 
    {
        // ep capture is occurring, erase the captured pawn
        if (co == WHITE)
            removePiece(static_cast<Square>(toSq-8), PAWN);
        else 
            removePiece(static_cast<Square>(toSq + 8), PAWN);
    }
    // Place piece on toSq
    if (isPromotion(mv)) {
        PieceType pctyPromo{ getPromotionType(mv) };
        bbByColour[co] ^= (1ULL<<toSq);
        bbByType[pctyPromo] ^= (1ULL<<toSq);
        occupancy ^= (1ULL << toSq);
    }
    else {
        bbByColour[co] ^= (1ULL << toSq);
        bbByType[piece] ^= (1ULL << toSq);
        occupancy ^= (1ULL << toSq);
    }
    // Save irreversible state information in struct, *before* altering them.
    undoStack.push_back(StateInfo{ pcDest, castlingRights, enPassantRights, fiftyMoveNum,hashes });

    // Update ep rights.
    if (piece == PAWN && ((fromSq-toSq==16) || (fromSq - toSq == -16))) {
        enPassantRights = static_cast<Square>((fromSq + toSq) / 2); // average gives middle square
    }
    else {
        enPassantRights = NO_SQ;
    }
    // Update castling rights.
    // Castling rights are lost if the king moves.
    if (piece == KING)
    {
        castlingRights &= (co == WHITE) ? ~(CASTLE_WLONG | CASTLE_WSHORT) : ~(CASTLE_BLONG | CASTLE_BSHORT);
    }
    else if (fromSq==A1 || toSq == A1) {
        castlingRights &= ~CASTLE_WLONG;
    }
    else if (fromSq == H1 || toSq == H1) {
        castlingRights &= ~CASTLE_WSHORT;
    }
    else if (fromSq == A8 || toSq == A8) {
        castlingRights &= ~CASTLE_BLONG;
    }
    else if (fromSq == H8 || toSq == H8) {
        castlingRights &= ~CASTLE_BSHORT;
    }
    // Change side to move, and update fifty-move and halfmove counts.
    sideToMove = !sideToMove;
    if (isCapture || (piece == PAWN)) {
        fiftyMoveNum = 0;
        hashes.clear();
    }
    else {
        hashes.push_back(calculateHash());
        bool second_repetition = false;
        for (int j = hashes.size() - 2; j >= 0; j--)
        {
            if (hashes[j] == hashes.back())
            {
                if (second_repetition)
                    gameover = true;
                else second_repetition = true;
            }
        }
        ++fiftyMoveNum;
    }
    ++halfmoveNum;
    return;
}

void Position::makeMoveFronStr_UCI(std::string usi_str)
{
    if (usi_str.size() < 4)
        return;
    if (usi_str == "e8g8")
    {
        makeCastlingMove(buildCastling(E8, H8));
        return;
    } else if (usi_str == "e8c8")
    {
        makeCastlingMove(buildCastling(E8, A8));
        return;
    }
    else if (usi_str == "e1c1")
    {
        makeCastlingMove(buildCastling(E1, A1));
        return;
    }
    else if (usi_str == "e1g1")
    {
        makeCastlingMove(buildCastling(E1, H1));
        return;
    }
    
    int file_from = usi_str[0] - 'a';
    int rank_from = usi_str[1] - '1';
    int file_to = usi_str[2] - 'a';
    int rank_to = usi_str[3] - '1';
    if (file_from < 0 || file_from>7
        || file_to < 0 || file_to>7
        || rank_from < 0 || rank_from>7
        || rank_to < 0 || rank_to>7)
        return;
    if (static_cast<Square>(rank_to * 8 + file_to) == enPassantRights && figurePieceFromSq(static_cast<Square>(rank_from * 8 + file_from)) == PAWN)
        makeMove(buildEnPassant(static_cast<Square>(rank_from * 8 + file_from), static_cast<Square>(rank_to * 8 + file_to)));
    else
        makeMove(buildMove(static_cast<Square>(rank_from * 8 + file_from), static_cast<Square>(rank_to * 8 + file_to)));
}

void Position::unmakeCastlingMove(Move mv) {
    const Colour co{ !sideToMove }; // retraction is by nonmoving side.
    const Square sqKFrom{ static_cast<Square>(mv & 63) };
    const Square sqRFrom{ static_cast<Square>((mv >> 6) & 63) };
    Square sqKTo{ NO_SQ };
    Square sqRTo{ NO_SQ };
    if (sqKFrom > sqRFrom) {    //king to the right of the rook?
        // King east of rook, i.e. west castling.
        if (co == WHITE) {
            sqKTo = C1;
            sqRTo = D1;
        }
        else {
            sqKTo = C8;
            sqRTo = D8;
        }
    }
    else {
        // King west of rook, i.e. east castling.
        if (co == WHITE) {
            sqKTo = G1;
            sqRTo = F1;
        }
        else {
            sqKTo = G8;
            sqRTo = F8;
        }
    }
    // Grab undo information off the stack. Assumes it matches the move called.
    StateInfo undoState{ undoStack.back() };
    undoStack.pop_back();

    // Revert side to move, castling and ep rights, fifty- and half-move counts.
    sideToMove = !sideToMove;
    castlingRights = undoState.castlingRights;
    enPassantRights = undoState.enPassantRights;
    fiftyMoveNum = undoState.fiftyMoveNum;
    hashes = undoState.hashes;
    halfmoveNum--;

    // Put king and rook back on their original squares.
    bbByColour[co] ^= ((1ULL << sqKFrom) | (1ULL << sqRFrom) | (1ULL << sqKTo) | (1ULL << sqRTo));
    occupancy ^= ((1ULL << sqKFrom) | (1ULL << sqRFrom) | (1ULL << sqKTo) | (1ULL << sqRTo));
    bbByType[KING] ^= ((1ULL << sqKFrom) | (1ULL << sqKTo));
    bbByType[ROOK] ^= ((1ULL << sqRFrom) | (1ULL << sqRTo));
    return;
}

void Position::unmakeMove(Move mv)
{
    // Castling is handled separately.
    if (isCastling(mv)) {
        unmakeCastlingMove(mv);
        return;
    }
    const Square fromSq{ static_cast<Square>(mv & 63) };
    const Square toSq{ static_cast<Square>((mv >> 6) & 63) };
    const PieceType piece = figurePieceFromSq(toSq);
    if (piece == NO_TYPE)
        throw std::runtime_error("Trying to unmake move with no piece selected");
    const Colour co{ !sideToMove };

    // Grab undo information off the stack. Assumes it matches the move called.
    StateInfo undoState{ undoStack.back() };
    undoStack.pop_back();

    // Revert side to move, castling and ep rights, fifty- and half-move counts.
    sideToMove = !sideToMove;
    castlingRights = undoState.castlingRights;
    enPassantRights = undoState.enPassantRights;
    fiftyMoveNum = undoState.fiftyMoveNum;
    hashes = undoState.hashes;
    --halfmoveNum;

    // Put unit back on original square.
    bbByColour[co] ^= (1ULL << toSq) | (1ULL << fromSq);
    occupancy ^= (1ULL << toSq) | (1ULL << fromSq);
    bbByType[piece] ^= (1ULL << toSq);
    if (isPromotion(mv)) {
        bbByType[PAWN] ^= (1ULL << fromSq);
    }
    else {
        bbByType[piece] ^= (1ULL << fromSq);
    }
    // Put back captured piece, if any (en passant handled separately.)
    const PieceType pcCap = undoState.capturedPiece;
    if (pcCap != NO_TYPE) {
        bbByColour[!co] ^= (1ULL<<toSq);
        bbByType[pcCap] ^= (1ULL << toSq);
        occupancy ^= (1ULL << toSq);

    }

    // replace en passant captured pawn.
    if (isEnPassant(mv)) {
        Square sqEpCap{ static_cast<Square>(toSq + 8 - 16 * (co == WHITE)) };
        bbByColour[!co] ^= (1ULL << sqEpCap);
        bbByType[PAWN] ^= (1ULL << sqEpCap);
        occupancy ^= (1ULL << sqEpCap);
    }
    gameover = false;
    return;
}

void Position::setStartingPosition()
{
    setFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

uint64_t Position::calculateHash()
{
    uint64_t hash = bbByType[PAWN] | static_cast<uint64_t>(castlingRights) | (static_cast<uint64_t>(enPassantRights) << 56);
    for (int i = NUM_PIECE_TYPES-1; i >= 0; i--)
    {
        if (i == PAWN)
            continue;
        hash ^= murmur64(bbByType[i]);
    }
    return hash;
}

void Position::reset()
{
    bbByColour={};
    bbByType={};
    // Game state information
    occupancy = 0;
    sideToMove={ WHITE };
    castlingRights= 0;
    enPassantRights={ NO_SQ };
    fiftyMoveNum={ 0 };
    halfmoveNum={ 0 };
    hashes.clear();
}

std::string Position::pretty_cb() const {
    // Makes a human-readable string of the board represented by Position.
    char board[64];
    std::fill(std::begin(board), std::end(board), '.');
    std::string strOut{ "+--------+\n" };

    for (int Color = 0; Color < NUM_COLOURS; Color++) //BLACK=0
    {
        for (int piece = 0; piece < NUM_PIECE_TYPES; piece++)
        {
            Bitboard current_piece{ bbByColour[Color] & bbByType[piece] };
            int piece_index = piece + Color * NUM_PIECE_TYPES;
            for (int i = 0; i < NUM_SQUARES; i++)
            {
                if ((current_piece & (1ULL<<i))>0)
                    switch (piece_index)
                    {
                    case 0+BLACK* NUM_PIECE_TYPES:board[i] = 'p';
                        break;
                    case 1 + BLACK * NUM_PIECE_TYPES:board[i] = 'n';
                        break;
                    case 2 + BLACK * NUM_PIECE_TYPES:board[i] = 'b';
                        break;
                    case 3 + BLACK * NUM_PIECE_TYPES:board[i] = 'r';
                        break;
                    case 4 + BLACK * NUM_PIECE_TYPES:board[i] = 'q';
                        break;
                    case 5 + BLACK * NUM_PIECE_TYPES:board[i] = 'k';
                        break;
                    case 0 + WHITE * NUM_PIECE_TYPES:board[i] = 'P';
                        break;
                    case 1 + WHITE * NUM_PIECE_TYPES:board[i] = 'N';
                        break;
                    case 2 + WHITE * NUM_PIECE_TYPES:board[i] = 'B';
                        break;
                    case 3 + WHITE * NUM_PIECE_TYPES:board[i] = 'R';
                        break;
                    case 4 + WHITE * NUM_PIECE_TYPES:board[i] = 'Q';
                        break;
                    case 5 + WHITE * NUM_PIECE_TYPES:board[i] = 'K';
                        break;
                    default:
                        break;
                    }
            }
        }
    }
    // loops over FEN-ordered array to print
    for (int i = 7; i >= 0; i--) {
        for (int j = 0; j < 8; j++)
            strOut += board[i * 8 + j];
        strOut += "\n";
    }
    strOut += "+--------+\n";
    // Output state info (useful for debugging)
    strOut += "sideToMove: " + std::to_string(sideToMove) + "\n";
    strOut += "castlingRights: " + std::to_string(castlingRights) + "\n";
    strOut += "epRights: " + std::to_string(enPassantRights) + "\n";
    strOut += "fiftyMoveNum: " + std::to_string(fiftyMoveNum) + "\n";
    strOut += "halfmoveNum: " + std::to_string(halfmoveNum) + "\n";
    return strOut;
}

//int main()
//{
//    Position cg;
//    cg.setFromFen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
//    //cg.setStartingPosition();
//    initialiseFirstRankAttacks();
//    initialiseAllDiagMasks();
//    initialiseFirstFileAttacks();
//    initialiseKnightAttacks();
//    initialiseKingAttacks();
//    initialisePawnAttacks();
//    auto start = std::chrono::high_resolution_clock::now();
//    std::cout<< perft_parallel(6,cg);
//    auto end = std::chrono::high_resolution_clock::now();
//    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//
//    std::cout << "Execution time: " << duration.count() << " ms\n";
//    start = std::chrono::high_resolution_clock::now();
//    std::cout << perft(6, cg);
//    end = std::chrono::high_resolution_clock::now();
//    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//
//    std::cout << "Execution time: " << duration.count() << " ms\n";
//    
//}