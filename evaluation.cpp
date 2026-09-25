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

bool checkThreefold(){
    uint8_t found=1;
    uint64_t lastHash = gameHistory[moveCnt-1];
    for(int i=0;i<moveCnt-2;i++)
        if(gameHistory[i]==lastHash)
            found++;

    if(found==3)
        return true; //threefold
    return false;
}

int isTerminalState(){
    ///return -1(BLACK), 0(DRAW), 1(WHITE)   -2(NONTERMINAL)
    if(checkThreefold())
        return 0; 
    
    return -2;   
    /// TODO: logic
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

int getPawnsEval(Color side){
    int eval = 0;
    int bonus = 0;
    uint64_t pawnsCopy = board.pieces[WHITE_PAWN|side<<3];
    while (pawnsCopy) {
        Square src = static_cast<Square>(getLSBIndex(pawnsCopy));
        eval+=100;
        bonus = (src/16)*10;
        if(side == WHITE)
            eval+=bonus;
        else
            eval+= (30-bonus);
        if(src == D4 || src == E4 || src == D5 || src == E5)
            eval+=10;
        pawnsCopy &= pawnsCopy - 1; 
    }
    return eval;
}

const int mopUpTable[64] = {
    100, 90, 80, 70, 70, 80, 90, 100,
     90, 70, 60, 50, 50, 60, 70,  90,
     80, 60, 40, 30, 30, 40, 60,  80,
     70, 50, 30, 20, 20, 30, 50,  70,
     70, 50, 30, 20, 20, 30, 50,  70,
     80, 60, 40, 30, 30, 40, 60,  80,
     90, 70, 60, 50, 50, 60, 70,  90,
    100, 90, 80, 70, 70, 80, 90, 100
};

int evalMopUp(Color side){
    Square myKing = getKingSquare(side);
    Square enemyKing = getKingSquare(oppositeColor(side));

    int eval = 0;
    eval += mopUpTable[enemyKing];

    int myRank = myKing / 8;
    int myFile = myKing % 8;
    int enemyRank = enemyKing / 8;
    int enemyFile = enemyKing % 8;

    int distance = abs(myRank - enemyRank) + abs(myFile - enemyFile);

    eval += (14-distance)*10;

    return eval;
}

int staticEval(){
    const int ENDGAME_LIMIT = 400;
    
    int whiteMaterial = 0;
    int blackMaterial = 0;

    whiteMaterial+=getPawnsEval(WHITE);
    whiteMaterial+=countBits(board.pieces[WHITE_BISHOP])*300;
    whiteMaterial+=getKnightsEval(WHITE);
    whiteMaterial+=countBits(board.pieces[WHITE_ROOK])*500;
    whiteMaterial+=countBits(board.pieces[WHITE_QUEEN])*900;
    
    
    blackMaterial+=getPawnsEval(BLACK);
    blackMaterial+=countBits(board.pieces[BLACK_BISHOP])*300;
    blackMaterial+=getKnightsEval(BLACK);
    blackMaterial+=countBits(board.pieces[BLACK_ROOK])*500;
    blackMaterial+=countBits(board.pieces[BLACK_QUEEN])*900;
    
    int eval=whiteMaterial-blackMaterial;

    if (isSquareAttacked(getKingSquare(WHITE), BLACK)) {
        eval -= 50;
    }
    
    if (isSquareAttacked(getKingSquare(BLACK), WHITE)) {
        eval += 50;
    }

    if(eval > 300 && blackMaterial < ENDGAME_LIMIT){
        eval+= evalMopUp(WHITE);
    }else if(eval < -300 && whiteMaterial < ENDGAME_LIMIT){
        eval-= evalMopUp(BLACK);
    }



    return eval;
}

int minMax(int depth, Color sideToMove, int alpha, int beta)
{
    short int terminal = isTerminalState(); 
    if(terminal!=-2){
        return terminal*100000000;
    }
    if(depth==0){
        return staticEval();
    }

    int bestEval=0, eval=0;
    Move moveList[256];
    int moveCount = generateAllMoves(sideToMove,moveList);
    int legalMoves=0;
    if(sideToMove==WHITE){
        bestEval=-100000000;
        for(int i=0;i<moveCount;i++){
            MoveInfo moveInfo = makeMove(moveList[i]);

            if(isSquareAttacked(getKingSquare(WHITE),BLACK)){
                unmakeMove(moveInfo);
                continue;
            }
            legalMoves++;

            eval = minMax(depth-1, BLACK, alpha, beta);
            unmakeMove(moveInfo);
            if(eval>bestEval){
                bestEval=eval;
                if(bestEval>alpha)
                    alpha = bestEval;
            }

            if(beta<=alpha)
                break;
        }
    }else{
        bestEval=100000000;
        for(int i=0;i<moveCount;i++){
            MoveInfo moveInfo = makeMove(moveList[i]);
            
            if(isSquareAttacked(getKingSquare(BLACK),WHITE)){
                unmakeMove(moveInfo);
                continue;
            }
            legalMoves++;
            
            
            eval = minMax(depth-1, WHITE, alpha,beta);
            unmakeMove(moveInfo);
            if(eval<bestEval){
                bestEval=eval;
                if(bestEval<beta)
                    beta = bestEval;
            }

            if(beta<=alpha)
                break;
        }
    }

    if(legalMoves==0){
        bool isCheck = isSquareAttacked(getKingSquare(sideToMove),static_cast<Color>(!sideToMove));
        if(isCheck){
            return (sideToMove==WHITE)?-999999-depth:999999+depth;
        }else{
            return 0; //Stalemate
        }
    }
    return bestEval;
}
