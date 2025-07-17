#pragma once
#include <string>
#include <vector>
#include "position.h"
#include "evaluation.h"
#include "movegen.h"
#include "move.h"

class UCIInterface {
public:
    void startUCI();
    //void sendMove(const std::string& move);
    void sendBestMove(const std::string& bestMove, const std::string& ponderMove = "");
    void sendInfo(const std::string& info);

private:
    Position pos;
    void parseCommand(const std::string& command);
    void handlePosition(const std::vector<std::string>& tokens);
    void handleGo(const std::vector<std::string>& tokens);
};