#include "uci.h"
#include <iostream>
#include <sstream>

struct nextMoveEval
{
    Move move;
    int eval;
};

void go_eval(int depth, Position& pos, int& current_eval)
{
    if (depth == 0)
    {
        int eval = materialEval(pos);
        switch (!pos.sideToMove)    //—торона, котора€ только что сделала ход
        {
        case BLACK:
            if (eval < current_eval)
                current_eval = eval;
            break;
        case WHITE:
            if (eval > current_eval)
                current_eval = eval;
            break;
        default:
            break;
        }
        return;
    }
    Movelist mvlist = generateLegalMoves(pos);
    int sz = mvlist.size();
    int currentMove{-CHECKMATE_EVALUATION + 2 * CHECKMATE_EVALUATION * (pos.sideToMove == BLACK)};
    for (int i = 0; i < sz; ++i) {
        pos.makeMove(mvlist[i]);
        go_eval(depth - 1, pos, currentMove);
        pos.unmakeMove(mvlist[i]);
    }
    switch (!pos.sideToMove)    //—торона, котора€ только что сделала ход
    {
    case BLACK:
        if (currentMove < current_eval)
            current_eval = currentMove;
        break;
    case WHITE:
        if (currentMove > current_eval)
            current_eval = currentMove;
        break;
    default:
        break;
    }
    return;
}

void UCIInterface::startUCI() {
    std::string input;
    while (std::getline(std::cin, input)) {
        parseCommand(input);
    }
}

void UCIInterface::parseCommand(const std::string& command) {
    std::istringstream iss(command);
    std::vector<std::string> tokens;
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.empty()) return;

    if (tokens[0] == "uci") {
        std::cout << "id name Blins\n";
        std::cout << "id author Mikhail D.\n";
        std::cout << "uciok\n";
    }
    else if (tokens[0] == "isready") {
        std::cout << "readyok\n";
    }
    else if (tokens[0] == "ucinewgame") {
        initializeLookupTables();
    }
    else if (tokens[0] == "position") {
        handlePosition(tokens);
    }
    else if (tokens[0] == "go") {
        handleGo(tokens);
    }
    else if (tokens[0] == "quit") {
        exit(0);
    }
}

void UCIInterface::handlePosition(const std::vector<std::string>& tokens) {
    if (tokens.size() > 1 && tokens[1] == "startpos")
    {
        pos.setStartingPosition();
        if (tokens.size()>2 && tokens[2] == "moves")
        {
            for (int i = 3; i < tokens.size(); i++)
            {
                pos.makeMoveFronStr_UCI(tokens[i]);
            }
        }
    }
    else if (tokens.size() > 1 && tokens[1] == "fen")
    {
        if (tokens.size() < 7)
            return;
        pos.setFromFen(tokens[2] + " " + tokens[3] + " " + tokens[4] + " " + tokens[5] + " " + tokens[6] + " " + tokens[7]);
        if (tokens.size() > 7 && tokens[8] == "moves")
        {
            for (int i = 9; i < tokens.size(); i++)
            {
                pos.makeMoveFronStr_UCI(tokens[i]);
            }
        }
    }

}

void UCIInterface::handleGo(const std::vector<std::string>& tokens) {
    if (tokens.size() > 1 && tokens[1] == "perft")
    {
        int i = 1;
        if (tokens.size() > 2 && (i = std::stoi(tokens[2])))
            if (i>0)
                perft_parallel(i, pos);
    }
    else
    {
        Movelist mvlist = generateLegalMoves(pos);
        int sz = mvlist.size();
        nextMoveEval BestMove{ Move(),-2*CHECKMATE_EVALUATION + 4*CHECKMATE_EVALUATION * (pos.sideToMove == BLACK) };
        for (int i = 0; i < sz; ++i) {
            nextMoveEval currentMove{ mvlist[i],-CHECKMATE_EVALUATION + 2* CHECKMATE_EVALUATION * (pos.sideToMove == BLACK) };
            std::cout << toStringUCI(mvlist[i]);
            pos.makeMove(mvlist[i]);
            go_eval(4, pos, currentMove.eval);
            pos.unmakeMove(mvlist[i]);
            switch (pos.sideToMove)    //—торона, котора€ только что сделала ход
            {
            case BLACK:
                if (currentMove.eval < BestMove.eval)
                    BestMove = currentMove;
                break;
            case WHITE:
                if (currentMove.eval > BestMove.eval)
                    BestMove = currentMove;
                break;
            default:
                break;
            }
        }
        sendBestMove(toStringUCI(BestMove.move));
        //std::cout << pos.pretty_cb();
        //std::cout << pretty(pos.occupancy);
        //std::cout << pretty(pos.bbByColour[WHITE]);
        //std::cout << pretty(pos.bbByColour[BLACK]);
        //std::cout << pretty(pos.bbByType[PAWN]);
    }
}
//
//void UCIInterface::sendMove(const std::string& move) {
//    std::cout << "info currmove " << move << "\n";
//}

void UCIInterface::sendBestMove(const std::string& bestMove, const std::string& ponderMove) {
    std::cout << "bestmove " << bestMove;
    if (!ponderMove.empty()) {
        std::cout << " ponder " << ponderMove;
    }
    std::cout << "\n";
}

void UCIInterface::sendInfo(const std::string& info) {
    std::cout << "info " << info << "\n";
}



int main()
{
    UCIInterface uci;
    uci.startUCI();
  /*  uint64_t num1 = 0xab235f76fa0027c1;
    uint64_t res1 = murmur64(num1);
    for (int i = 0; i < 64; i++)
    {
        uint64_t buf = 1ULL << i;
        uint64_t num2 = num1 ^ buf;
        uint64_t res2 = murmur64(num2);
        int fin = countOnes(res1 ^ res2);
        std::cout << "i = " << i << "\t" << fin << std::endl;
    }*/
    
}