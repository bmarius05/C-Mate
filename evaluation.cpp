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

int getGamePhase(){
    const int knightPhase = 1;
    const int bishopPhase = 1;
    const int rookPhase   = 2;
    const int queenPhase  = 4;

    const int totalPhase = 24;
    int knights = countBits(board.pieces[WHITE_KNIGHT])+countBits(board.pieces[BLACK_KNIGHT]);
    int bishops = countBits(board.pieces[WHITE_BISHOP])+countBits(board.pieces[BLACK_BISHOP]);
    int rooks = countBits(board.pieces[WHITE_ROOK])+countBits(board.pieces[BLACK_ROOK]);
    int queens = countBits(board.pieces[WHITE_QUEEN])+countBits(board.pieces[BLACK_QUEEN]);

    int currentPhase = knights * knightPhase +
                       bishops * bishopPhase +
                       rooks * rookPhase + 
                       queens * queenPhase;

    return std::min(totalPhase, currentPhase);
}

bool checkThreefold(){
    if(moveCnt<4)
        return false;
    uint8_t found=1;
    uint64_t lastHash = gameHistory[moveCnt-1];
    for(int i=0;i<moveCnt-2;i++)
        if(gameHistory[i]==lastHash)
            found++;

    return found>=2;
}

int isTerminalState(){
    ///return -1(BLACK), 0(DRAW), 1(WHITE)   -2(NONTERMINAL)
    if(checkThreefold())
        return 0; 
    
    return -2;   
    /// TODO: logic
}

bool isPassedPawn(Square sq, Color side){
    int file = sq%8;
    int enemyPawn = WHITE_PAWN|(oppositeColor(side)<<3);
    int start = side==WHITE?(sq+8):(sq-8);
    int step = side==WHITE?(8):(-8);
    for(int pos = start; (side==WHITE)?(pos < 64):(pos>=0);pos+=step){
        if(board.cells[pos] == enemyPawn)
            return false;
        if(file>=1 && board.cells[pos-1] == enemyPawn)
            return false;
        if(file<=6 && board.cells[pos+1] == enemyPawn)
            return false;
    }
    return true;
}
int countPawnsOnFile(int file, Color side){
    int pawns = 0;
    int myPawn = WHITE_PAWN|(side<<3);
    for(int sq = file; sq<64;sq+=8){
        if(board.cells[sq] == myPawn)
            pawns++;
    }
    return pawns;
}
bool hasPawnOnNeighborFile(int file, Color side){
    if(file>=1)
        if(countPawnsOnFile(file-1,side)>0)
            return true;
    if(file<=6)
        if(countPawnsOnFile(file+1,side)>0)
            return true;
    return false;
}

int getPawnsEval(Color side){
    int eval = 0;
    uint64_t pawnsCopy = board.pieces[WHITE_PAWN|side<<3];
    while (pawnsCopy) {
        Square src = static_cast<Square>(getLSBIndex(pawnsCopy));
        int file = src%8;
        int rank = src/8;

        if(isPassedPawn(src, side)){
            int rankBonus = side==WHITE?(rank):(7-rank);
            eval+=rankBonus*rankBonus*10;
        }
        if(countPawnsOnFile(file,side)>1)
            eval-=20;
        if(!hasPawnOnNeighborFile(file,side))
            eval-=15;

        pawnsCopy &= pawnsCopy - 1; 
    }
    return eval;
}

int evaluateKingSafety(Color side) {
    int phase = getGamePhase();
    if (phase < 10) return 0; 

    Square kingSq = getKingSquare(side);
    int score = castled?70:0;
    if (side == WHITE) {
        if (kingSq == G1 || kingSq == H1 || kingSq == F1) {
            if (board.cells[F2] != WHITE_PAWN && board.cells[F3] != WHITE_PAWN) score -= 25;
            if (board.cells[G2] != WHITE_PAWN && board.cells[G3] != WHITE_PAWN) score -= 35;
            if (board.cells[H2] != WHITE_PAWN && board.cells[H3] != WHITE_PAWN) score -= 20;
        } 
        else if (kingSq == C1 || kingSq == B1 || kingSq == A1) {
            if (board.cells[A2] != WHITE_PAWN && board.cells[A3] != WHITE_PAWN) score -= 20;
            if (board.cells[B2] != WHITE_PAWN && board.cells[B3] != WHITE_PAWN) score -= 35;
            if (board.cells[C2] != WHITE_PAWN && board.cells[C3] != WHITE_PAWN) score -= 25;
        } 
        else {
            score -= 70;
        }
    } 
    else { 
        if (kingSq == G8 || kingSq == H8 || kingSq == F8) {
            if (board.cells[F7] != BLACK_PAWN && board.cells[F6] != BLACK_PAWN) score -= 25;
            if (board.cells[G7] != BLACK_PAWN && board.cells[G6] != BLACK_PAWN) score -= 35;
            if (board.cells[H7] != BLACK_PAWN && board.cells[H6] != BLACK_PAWN) score -= 20;
        } 
        else if (kingSq == C8 || kingSq == B8 || kingSq == A8) {
            if (board.cells[A7] != BLACK_PAWN && board.cells[A6] != BLACK_PAWN) score -= 20;
            if (board.cells[B7] != BLACK_PAWN && board.cells[B6] != BLACK_PAWN) score -= 35;
            if (board.cells[C7] != BLACK_PAWN && board.cells[C6] != BLACK_PAWN) score -= 25;
        } 
        else {
            score -= 70; 
        }
    }

    return (score * phase) / 24;
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

int getPieceValue(PieceType pcs){
    const PieceType type = static_cast<PieceType>(pcs & (~(1<<3)));
    switch (type)
    {
        case WHITE_PAWN:   return 90;
        case WHITE_KNIGHT: return 320;
        case WHITE_BISHOP: return 330;
        case WHITE_ROOK:   return 500;
        case WHITE_QUEEN:  return 980;
        case WHITE_KING:   return 20000;
        default:           return 0;
    }
}
int getPstMg(PieceType p, int pstIndex){
    const PieceType type = static_cast<PieceType>(p & (~(1<<3)));
    switch (type)
    {
        case WHITE_PAWN:
            return mg_pawn_table[pstIndex];
        case WHITE_BISHOP:
            return mg_bishop_table[pstIndex];
        case WHITE_KNIGHT:
            return mg_knight_table[pstIndex];
        case WHITE_ROOK:
            return mg_rook_table[pstIndex];
        case WHITE_QUEEN:
            return mg_queen_table[pstIndex];
        case WHITE_KING:
            return mg_king_table[pstIndex];
        default:
            return 0;
    }
    return 0;
}
int getPstEg(PieceType p, int pstIndex){
    const PieceType type = static_cast<PieceType>(p & (~(1<<3)));
    switch (type)
    {
        case WHITE_PAWN:
            return eg_pawn_table[pstIndex];
        case WHITE_BISHOP:
            return eg_bishop_table[pstIndex];
        case WHITE_KNIGHT:
            return eg_knight_table[pstIndex];
        case WHITE_ROOK:
            return eg_rook_table[pstIndex];
        case WHITE_QUEEN:
            return eg_queen_table[pstIndex];
        case WHITE_KING:
            return eg_king_table[pstIndex];
        default:
            return 0;
    }
    return 0;
}

/*
int staticEval(){
    const int ENDGAME_LIMIT = 400;
    const int MOPUP_DIF=200;
    
    int mgWhite = 0, egWhite = 0;
    int mgBlack = 0, egBlack = 0;
    int whiteMaterial = 0, blackMaterial=0;

    for(int sq = 0; sq<64; sq++){
        PieceType p = board.cells[sq];
        if(p==EMPTY)
            continue;
        
        Color c = (p & (1<<3))?BLACK:WHITE;
        int val = getPieceValue(p);

        int pstIndex = (c==BLACK)?sq:(sq^56);
        int mgPst = getPstMg(p, pstIndex);
        int egPst = getPstEg(p, pstIndex);

        if(c==WHITE){
            mgWhite += val+mgPst;
            egWhite += val+egPst;
            if(p!=WHITE_KING) whiteMaterial += val;
        }else{
            mgBlack += val+mgPst;
            egBlack += val+egPst;
            if(p!=BLACK_KING) blackMaterial += val;
        }
    }

    mgWhite += getPawnsEval(WHITE); egWhite += getPawnsEval(WHITE);
    mgBlack += getPawnsEval(BLACK); egBlack += getPawnsEval(BLACK);

    mgWhite += evaluateKingSafety(WHITE);
    mgBlack += evaluateKingSafety(BLACK);

    int materialDif = whiteMaterial - blackMaterial;

    if(materialDif > MOPUP_DIF && blackMaterial < ENDGAME_LIMIT){
        egWhite+= evalMopUp(WHITE);
    }else if(materialDif < -MOPUP_DIF && whiteMaterial < ENDGAME_LIMIT){
        egBlack+= evalMopUp(BLACK);
    }

    int mgScore = mgWhite - mgBlack;
    int egScore = egWhite - egBlack;

    int phase = getGamePhase();

    int eval = (mgScore * phase + egScore * (24 - phase)) / 24;

    return eval;

}*/

int staticEval(){
    const int ENDGAME_LIMIT = 400;
    
    int whiteMaterial = 0;
    int blackMaterial = 0;

    whiteMaterial+=countBits(board.pieces[WHITE_PAWN])*100;
    whiteMaterial+=countBits(board.pieces[WHITE_BISHOP])*300;
    whiteMaterial+=countBits(board.pieces[WHITE_KNIGHT])*300;
    whiteMaterial+=countBits(board.pieces[WHITE_ROOK])*500;
    whiteMaterial+=countBits(board.pieces[WHITE_QUEEN])*900;
    
    
    blackMaterial+=countBits(board.pieces[BLACK_PAWN])*100;
    blackMaterial+=countBits(board.pieces[BLACK_BISHOP])*300;
    blackMaterial+=countBits(board.pieces[BLACK_KNIGHT])*300;
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

int quiesenceSearch(int qsDepth, Color sideToMove, int alpha, int beta){
    checkTime();
    if (timeIsUp) return 0;
    
    int initialEval = staticEval();

    if(qsDepth<=0)
        return initialEval;

    if (sideToMove == WHITE) {
        if (initialEval >= beta) 
            return beta;
        if (initialEval > alpha) 
            alpha = initialEval;
    } else {
        if (initialEval <= alpha) 
            return alpha;
        if (initialEval < beta) 
            beta = initialEval;
    }

    Move moveList[256];
    int cnt = generateAllCaptures(sideToMove, moveList);
    int score;

    if (sideToMove == WHITE) {
        for(int i = 0; i < cnt; i++){
            MoveInfo info = makeMove(moveList[i]);

            if(isSquareAttacked(getKingSquare(WHITE), BLACK)){
                unmakeMove(info);
                continue;
            }

            score = quiesenceSearch(qsDepth-1 ,BLACK, alpha, beta); 
            unmakeMove(info);

            if(score > alpha) 
                alpha = score;
            if(alpha >= beta) 
                return beta;
        }
        return alpha;
    } 
    else {
        for(int i = 0; i < cnt; i++){
            MoveInfo info = makeMove(moveList[i]);

            if(isSquareAttacked(getKingSquare(BLACK), WHITE)){
                unmakeMove(info);
                continue;
            }

            score = quiesenceSearch(qsDepth-1,WHITE, alpha, beta);
            unmakeMove(info);

            if(score < beta) 
                beta = score;
            if(beta <= alpha) 
                return alpha;
        }
        return beta;
    }

}

int minMax(int depth, Color sideToMove, int alpha, int beta)
{
    checkTime();
    if (timeIsUp) return 0;

    short int terminal = isTerminalState(); 
    if(terminal!=-2){
        return terminal*100000000;
    }
    if(depth==0){
        return quiesenceSearch(qsMaxDepth, sideToMove, alpha,beta);
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