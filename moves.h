#pragma once
#include "bitboards.h"
#include "evaluation.h"

struct MoveInfo{
    Square src;
    Square dst;
    PieceType movedPiece;
    PieceType capturedPiece;

    uint8_t castlingRightsCopy;
    Square enPassantSqCopy;
    bool enPassantCopy;
};
struct Move{
    Square src;
    Square dst;
};

void resetFlags();

MoveInfo makeMove(Move move);
void unmakeMove(const MoveInfo move);
void initKnightAttacks();
void initKingAttacks();
void initBishopAttacks();
void initRookAttacks();

int generateAllMoves(Color sideToMove, Move* moveList);
Move findBestMove(Color sideToMove);
bool isSquareAttacked(Square sq, Color attacker);
Square getKingSquare(Color color);

void validateBoardState();