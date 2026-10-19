#pragma once
#include "bitboards.h"
#include "evaluation.h"
#include <chrono>
#include <atomic>

struct MoveInfo{
    Square src;
    Square dst;
    PieceType movedPiece;
    PieceType capturedPiece;
    bool castledCopy;
    uint8_t castlingRightsCopy;
    Square enPassantSqCopy;
    bool enPassantCopy;
};
struct Move{
    Square src;
    Square dst;
};

inline std::unordered_map<uint64_t, std::vector<Move>> openingBook;

inline long long lastInfoPrintTime= 0; 
inline std::atomic<bool> timeIsUp{false};
inline std::chrono::time_point<std::chrono::steady_clock> startTime;
inline int allocatedTimeMs;
inline int nodesTicked = 0;
inline bool castled=false;

static int qsMaxDepth=7;

extern uint8_t moveCnt;
extern uint64_t gameHistory[1024];

extern uint8_t castlingRights;
extern Square enPassantSq;
extern bool enPassant;

extern long long totalExplored;

void resetFlags();

MoveInfo makeMove(Move move);
void unmakeMove(const MoveInfo move);
void initKnightAttacks();
void initKingAttacks();
void initBishopAttacks();
void initRookAttacks();

int generateAllMoves(Color sideToMove, Move* moveList);
int generateAllCaptures(Color sideToMove, Move* moveList);
Move findBestMoveDepth(Color sideToMove, int depth);
Move findBestMoveTime(Color sideToMove, int timeLeft, int increment, int movesToGo, int moveTime);
bool isSquareAttacked(Square sq, Color attacker);
Square getKingSquare(Color color);
std::string moveToString(Move move);

void validateBoardState();
void checkTime();

Move stringToMove(std::string move);
void loadOppeningBook(const std::string filename);