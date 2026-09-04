#include "evaluation.h"
#include <cstdint>
#include <limits>

#if defined(_MSC_VER)
#include <intrin.h> // Necesar pentru Visual Studio / MSVC
#endif

inline int countBits(uint64_t bb) {
    #if defined(__GNUC__) || defined(__clang__)
        return __builtin_popcountll(bb); // GCC și Clang
    #elif defined(_MSC_VER)
        return static_cast<int>(__popcnt64(bb)); // MSVC (Windows)
    #else
        // Fallback de software ultra-rapid în caz că ești pe o arhitectură ciudată
        int count = 0;
        while (bb) {
            bb &= bb - 1; // Șterge mereu cel mai mic bit de 1 (Brian Kernighan's algorithm)
            count++;
        }
        return count;
    #endif
}

inline uint8_t getLSBIndex(uint64_t bb) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_ctzll(bb); // Count Trailing Zeros (GCC / Clang)
#elif defined(_MSC_VER)
    unsigned long index;
    _BitScanForward64(&index, bb); // MSVC (Windows)
    return static_cast<uint8_t>(index);
#endif
}

inline uint8_t getMSBIndex(uint64_t bb) {
#if defined(__GNUC__) || defined(__clang__)
    return 63 - __builtin_clzll(bb);
#elif defined(_MSC_VER)
    unsigned long index;
    _BitScanReverse64(&index, bb);
    return static_cast<uint8_t>(index);
#endif
}

int getKnightsEval(Color side){
    int eval=0;
    uint64_t knightsCopy = board.pieces[WHITE_KNIGHT|side<<3];
    while (knightsCopy) {
        Square src = static_cast<Square>(getLSBIndex(knightsCopy));
        eval+=300;
        if(src%8==0||src%8==7||src<8||src>=56)
            eval-=10;
        else if(src==D4||src==D5||src==E4||src==E5)
            eval+=10;
        knightsCopy &= knightsCopy - 1; 
    }
    return eval;
}

int staticEval(){
    int eval=0;
    eval+=countBits(board.pieces[WHITE_PAWN])*100;
    eval+=countBits(board.pieces[WHITE_BISHOP])*300;
    eval+=getKnightsEval(WHITE);
    eval+=countBits(board.pieces[WHITE_ROOK])*500;
    eval+=countBits(board.pieces[WHITE_QUEEN])*900;
    eval+=countBits(board.pieces[WHITE_KING])*10000000;
    
    eval-=countBits(board.pieces[BLACK_PAWN])*100;
    eval-=countBits(board.pieces[BLACK_BISHOP])*300;
    eval-=getKnightsEval(BLACK);
    eval-=countBits(board.pieces[BLACK_ROOK])*500;
    eval-=countBits(board.pieces[BLACK_QUEEN])*900;
    eval-=countBits(board.pieces[BLACK_KING])*10000000;

    if (isSquareAttacked(getKingSquare(WHITE), BLACK)) {
        eval -= 50;
    }
    
    if (isSquareAttacked(getKingSquare(BLACK), WHITE)) {
        eval += 50;
    }

    return eval;
}

int minMax(int depth, Color sideToMove)
{
    if(depth==0){
        return staticEval();
    }

    int bestEval=0, eval=0;
    Move moveList[256];
    int moveCount = generateAllMoves(sideToMove,moveList);
    int legalMoves=0;
    if(sideToMove==WHITE){
        bestEval=INT_MIN;
        for(int i=0;i<moveCount;i++){
            MoveInfo moveInfo = makeMove(moveList[i]);

            if(isSquareAttacked(getKingSquare(WHITE),BLACK)){
                unmakeMove(moveInfo);
                continue;
            }
            legalMoves++;

            eval = minMax(depth-1, static_cast<Color>(!sideToMove));
            unmakeMove(moveInfo);
            if(eval>bestEval){
                bestEval=eval;
            }
        }
    }else{
        bestEval=INT_MAX;
        for(int i=0;i<moveCount;i++){
            MoveInfo moveInfo = makeMove(moveList[i]);
            
            if(isSquareAttacked(getKingSquare(BLACK),WHITE)){
                unmakeMove(moveInfo);
                continue;
            }
            legalMoves++;
            
            
            eval = minMax(depth-1, static_cast<Color>(!sideToMove));
            unmakeMove(moveInfo);
            if(eval<bestEval){
                bestEval=eval;
            }
        }
    }

    if(legalMoves==0){
        bool isCheck = isSquareAttacked(getKingSquare(sideToMove),static_cast<Color>(!sideToMove));
        if(isCheck){
            return (sideToMove==WHITE)?-999999-depth:999999+depth; // depth adjusts the rapidity of the mate
        }else{
            return 0; //Stalemate
        }
    }
    return bestEval;
}
