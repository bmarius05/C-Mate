#include "moves.h"
#include <iostream>
#include <cassert>

#include <immintrin.h> // Pentru intrinsice CPU

static int maxDepth=3;


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

enum bishopDirections:int{
    LEFT_UP=0,
    RIGHT_UP=1,
    RIGHT_DOWN=2,
    LEFT_DOWN=3,
};

enum rookDirections:int{
    UP=0,
    RIGHT=1,
    DOWN=2,
    LEFT=3
};

uint8_t castlingRights = 0x0f;
Square enPassantSq = A1;
bool enPassant = false;

void resetFlags(){
    castlingRights=15;
    enPassantSq=A1;
    enPassant=false;
}

void validateBoardState() {
    for (int sq = 0; sq < 64; sq++) {
        int pieceInArray = board.cells[sq]; // Cum accesezi tu tipul piesei (ex: WHITE_PAWN)
        
            // Verificăm dacă piesa din array chiar are bitul setat în bitboard-ul ei!
        uint64_t mask = 1ULL << sq;
        if ((board.pieces[pieceInArray] & mask) == 0) {
            std::cout << "CRITICAL DESYNC pe patratul " << sq << "\n";
            std::cout << "Array-ul zice ca e piesa " << pieceInArray << ", dar bitboard-ul nu o are!\n";
            assert(false);
        }
    }
}

MoveInfo makeMove(Move move){
    Square src = move.src;
    PieceType piece = board.cells[src];

    uint8_t castlingRightsCopy = castlingRights;
    Square enPassantSqCopy = enPassantSq;
    bool enPassantCopy = enPassant;

    if(piece==EMPTY){
        printf("Was tried from:%d\n to: %d\n", move.src, move.dst);
        assert(piece != EMPTY && "Cant move from an empty square!\n");
    }

    Square dst = move.dst;
    PieceType capturedPiece = board.cells[dst];

    if(piece == WHITE_KING && (dst-src == 2 || dst-src ==-2)){
        uint64_t rookMask=0;
        if (dst-src==2 && castlingRights&1){
            //king side castle
            rookMask = (1ULL<<H1)|(1ULL<<F1);
            board.cells[H1]=EMPTY;
            board.cells[F1]=WHITE_ROOK;
            castlingRights ^=1;
        }else if (dst-src==-2 && castlingRights&2){
            //queen side castle
            board.cells[A1]=EMPTY;
            board.cells[D1]=WHITE_ROOK;
            rookMask = (1ULL<<A1)|(1ULL<<D1);
            castlingRights^=2;
        }
        board.pieces[WHITE_ROOK]^=rookMask;
        board.pieces[WHITE_PIECE]^=rookMask;
        board.pieces[EMPTY]^=rookMask;
    }else if(piece == BLACK_KING && (dst-src == 2 || dst-src ==-2)){
        uint64_t rookMask=0;
        if (dst-src==2 && castlingRights&4){
            //king side castle
            rookMask = (1ULL<<H8)|(1ULL<<F8);
            board.cells[H8]=EMPTY;
            board.cells[F8]=BLACK_ROOK;
            castlingRights ^=4;
        }else if (dst-src==-2 && castlingRights&8){
            //queen side castle
            rookMask = (1ULL<<A8)|(1ULL<<D8);
            board.cells[A8]=EMPTY;
            board.cells[D8]=BLACK_ROOK;
            castlingRights^=8;
        }
        board.pieces[BLACK_ROOK]^=rookMask;
        board.pieces[BLACK_PIECE]^=rookMask;
        board.pieces[EMPTY]^=rookMask;
    }

    if(src == A1 || dst == A1){
        castlingRights &= 13; //~2
    }
    if(src == H1 || dst == H1){
        castlingRights &= 14; //~1
    }
    if(src == E1 || dst == E1){
        castlingRights &= 12; //~3
    }   
    ///TODO: remove ifs and implement using array
    if(src == A8 || dst == A8){
        castlingRights &= 7; //~8
    }
    if(src == H8 || dst == H8){
        castlingRights &= 11; //~4
    }
    if(src == E8 || dst == E8){
        castlingRights &= 3; //~12
    }

    enPassant = false;
    if(piece == WHITE_PAWN){
        if(dst-src == 16){
            enPassantSq = static_cast<Square>(dst - 8);
            enPassant = true;
        }else if(dst == enPassantSq && enPassantCopy == true){
            Square enemySq = static_cast<Square>(dst - 8);
            uint64_t enPassantMask = (1ULL)<<enemySq;
            board.cells[enemySq]=EMPTY;
            board.pieces[BLACK_PAWN]^=enPassantMask;
            board.pieces[BLACK_PIECE]^=enPassantMask;
            board.pieces[EMPTY]^=enPassantMask;
        }
    }else if(piece == BLACK_PAWN){
        if(src-dst == 16){
            enPassantSq = static_cast<Square>(dst + 8);
            enPassant = true;
        }else if(dst == enPassantSq && enPassantCopy == true){
            Square enemySq = static_cast<Square>(dst + 8);
            uint64_t enPassantMask = (1ULL)<<enemySq;
            board.cells[enemySq]=EMPTY;
            board.pieces[WHITE_PAWN]^=enPassantMask;
            board.pieces[WHITE_PIECE]^=enPassantMask;
            board.pieces[EMPTY]^=enPassantMask;
        }
    }

    uint8_t colorIndex = WHITE_PIECE + (piece&0x08);

    board.cells[src]=EMPTY;
    board.cells[dst]=piece;

    uint64_t moveMask = (1ULL<<src) | (1ULL<<dst);
    
    board.pieces[piece] ^= moveMask; 
    board.pieces[colorIndex] ^= moveMask;
    


    if(capturedPiece!=EMPTY){
        uint64_t captureMask = (1ULL<<dst);
        uint8_t enemyColorIndex = colorIndex ^ 0x08;

        board.pieces[capturedPiece] ^= captureMask;
        board.pieces[enemyColorIndex] ^= captureMask;

        board.pieces[EMPTY] ^= (1ULL<<src);
    }
    else
        board.pieces[EMPTY] ^= moveMask;

    ///AUTO QUEEN  TODO:Change
    if(piece==WHITE_PAWN && dst>=56){
        board.pieces[WHITE_PAWN] ^= 1ULL<<dst;
        board.pieces[WHITE_QUEEN] ^= 1ULL<<dst;
        board.cells[dst]=WHITE_QUEEN;
    }else if(piece==BLACK_PAWN && dst<8){
        board.pieces[BLACK_PAWN] ^= 1ULL<<dst;
        board.pieces[BLACK_QUEEN] ^= 1ULL<<dst;
        board.cells[dst]=BLACK_QUEEN;
    }

    validateBoardState();

    return MoveInfo{src,dst,piece,capturedPiece,castlingRightsCopy,enPassantSqCopy,enPassantCopy};
}

void unmakeMove(const MoveInfo move)
{
    Square src = move.src;
    Square dst = move.dst;
    PieceType piece = move.movedPiece;
    PieceType capturedPiece = move.capturedPiece;

    board.cells[src]=piece;
    board.cells[dst]=capturedPiece;

    uint8_t colorIndex = WHITE_PIECE + (piece&0x08);

    uint64_t moveMask = (1ULL<<src) | (1ULL<<dst);
    board.pieces[piece] ^=moveMask;
    board.pieces[colorIndex] ^=moveMask;

    if (piece == WHITE_PAWN && dst >= 56) {
        board.pieces[WHITE_QUEEN] ^= (1ULL << dst);
        board.pieces[WHITE_PAWN] ^= (1ULL << dst);
    } 
    else if (piece == BLACK_PAWN && dst < 8) {
        board.pieces[BLACK_QUEEN] ^= (1ULL << dst);
        board.pieces[BLACK_PAWN] ^= (1ULL << dst);
    }

    if(capturedPiece!=EMPTY){
        uint64_t captureMask = (1ULL<<dst);
        uint8_t enemyColorIndex = colorIndex ^ 0x08;

        board.pieces[capturedPiece] ^= captureMask;
        board.pieces[enemyColorIndex] ^= captureMask;

        board.pieces[EMPTY]^=(1ULL<<src);
    }else{
        board.pieces[EMPTY]^=moveMask;
    }

    if(dst-src == 2 || dst-src ==-2){
        uint64_t rookMask = 0;
        if(piece == WHITE_KING){
            if (dst-src==2){
                //king side castle
                rookMask = (1ULL<<F1)|(1ULL<<H1);
                board.cells[H1] = WHITE_ROOK;
                board.cells[F1] = EMPTY;
            }else if (dst-src==-2){
                rookMask = (1ULL<<A1)|(1ULL<<D1);
                board.cells[A1] = WHITE_ROOK;
                board.cells[D1] = EMPTY;
            }
        }else if(piece == BLACK_KING){
            if (dst-src==2){
                //king side castle
                rookMask = (1ULL<<F8)|(1ULL<<H8);
                board.cells[H8] = BLACK_ROOK;
                board.cells[F8] = EMPTY;
            }else if (dst-src==-2){
                rookMask = (1ULL<<A8)|(1ULL<<D8);
                board.cells[A8] = BLACK_ROOK;
                board.cells[D8] = EMPTY;
            }
        }
        board.pieces[WHITE_ROOK+(piece&0x08)] ^= rookMask;
        board.pieces[colorIndex] ^= rookMask;
        board.pieces[EMPTY]^=rookMask;
    }

    //en Passant
    if((dst-src)%8!=0 && capturedPiece == EMPTY){
        Square epSquare;
        uint64_t epMask;    
        if(piece == WHITE_PAWN){
            epSquare = static_cast<Square>(dst-8);    
            board.cells[epSquare] = BLACK_PAWN;
            epMask = (1ULL)<<epSquare;
            board.pieces[BLACK_PAWN]^=epMask;
            board.pieces[BLACK_PIECE]^=epMask;
            board.pieces[EMPTY]^=epMask;
        }else if(piece == BLACK_PAWN){
            epSquare = static_cast<Square>(dst+8);
            board.cells[epSquare] = WHITE_PAWN;
            epMask = (1ULL)<<epSquare;
            board.pieces[WHITE_PAWN]^=epMask;
            board.pieces[WHITE_PIECE]^=epMask;
            board.pieces[EMPTY]^=epMask;
        }
    }
    
    castlingRights = move.castlingRightsCopy;
    enPassant = move.enPassantCopy;
    enPassantSq = move.enPassantSqCopy;
    validateBoardState();
}

uint64_t FILE_A = 0x0101010101010101ULL;
uint64_t FILE_AB = (FILE_A<<1) | FILE_A;
uint64_t FILE_H = FILE_A<<7;
uint64_t FILE_GH = (FILE_H>>1)|FILE_H;
uint64_t RANK_3 = 0x0000000000ff0000ULL;
uint64_t RANK_6 = 0x0000ff0000000000ULL;


uint64_t knightAttacks[64];

void initKnightAttacks(){
    uint64_t attack, binpos;
    
    
    ///TODO: Remove ifs
    
    for(int i=0;i<64;i++){
        attack = 0;
        binpos = (1ULL<<i);
        if(binpos& (~FILE_A)){
            attack |= binpos<<15;
            attack |= binpos>>17;
        }
        if(binpos& (~FILE_H)){
            attack |= binpos<<17;
            attack |= binpos>>15;
        }
        if(binpos& (~FILE_AB)){
            attack |= binpos<<6;
            attack |= binpos>>10;
        }
        if(binpos& (~FILE_GH)){
            attack |= binpos<<10;
            attack |= binpos>>6;
        }
        knightAttacks[i]=attack;
    }

}

uint64_t bishopAttacks[64][4];

void initBishopAttacks(){
    uint64_t currPos;
    for(int i=0;i<64;i++){
        bishopAttacks[i][LEFT_UP]=bishopAttacks[i][LEFT_DOWN]=0;
        bishopAttacks[i][RIGHT_UP]=bishopAttacks[i][RIGHT_DOWN]=0;
        for(currPos = 1ULL << i; currPos & ~FILE_A; ){
            currPos <<= 7;
            bishopAttacks[i][LEFT_UP]|=currPos;
        }
        for(currPos = 1ULL << i; currPos & ~FILE_A; ){
            currPos >>= 9;
            bishopAttacks[i][LEFT_DOWN]|=currPos;
        }
        for(currPos = 1ULL << i; currPos & ~FILE_H; ){
            currPos <<= 9;
            bishopAttacks[i][RIGHT_UP]|=currPos;
        }
        for(currPos = 1ULL << i; currPos & ~FILE_H; ){
            currPos >>= 7;
            bishopAttacks[i][RIGHT_DOWN]|=currPos;
        }
    }
}

uint64_t rookAttacks[64][4];

void initRookAttacks(){
    uint64_t currPos;
    for(int i=0;i<64;i++){
        rookAttacks[i][UP]=rookAttacks[i][DOWN]=0;
        rookAttacks[i][LEFT]=rookAttacks[i][RIGHT]=0;
        for(currPos = 1ULL << i; currPos & ~FILE_A; ){
            currPos >>= 1;
            rookAttacks[i][LEFT]|=currPos;
        }
        for(currPos = 1ULL << i; currPos & ~FILE_H; ){
            currPos <<= 1;
            rookAttacks[i][RIGHT]|=currPos;
        }
        for(currPos = 1ULL << i; currPos; ){
            currPos <<= 8;
            rookAttacks[i][UP]|=currPos;
        }
        for(currPos = 1ULL << i; currPos; ){
            currPos >>= 8;
            rookAttacks[i][DOWN]|=currPos;
        }
    }
}

uint64_t kingAttacks[64];

void initKingAttacks(){
    uint64_t attack, binpos;
    
    ///TODO: Remove ifs
    
    for(int i=0;i<64;i++){
        attack=0;
        binpos = (1ULL)<<i;
        if(binpos& (~FILE_A)){
            attack|=binpos<<7;
            attack|=binpos>>1;
            attack|=binpos>>9;
        }
        if(binpos& (~FILE_H)){
            attack|=binpos<<9;
            attack|=binpos<<1;
            attack|=binpos>>7;
        }
        attack|=binpos>>8;
        attack|=binpos<<8;
        kingAttacks[i]=attack;
    }
}

void generateKnightMoves(Square sq, uint64_t myPieces, Move* moveList, int& moveCount){
    uint64_t validDst = knightAttacks[sq] & ~myPieces;

    while (validDst) {
        uint8_t dstIndex = getLSBIndex(validDst); 
        moveList[moveCount++] = Move{ sq, static_cast<Square>(dstIndex) };
        validDst &= validDst - 1; 
    }
}

void generateKingMoves(Square sq, uint64_t myPieces, Move* moveList, int& moveCount){
    uint64_t validDst = kingAttacks[sq] & ~myPieces;
    while (validDst) {
        uint8_t dstIndex = getLSBIndex(validDst); 
        moveList[moveCount++] = Move{ sq, static_cast<Square>(dstIndex) };
        validDst &= validDst - 1; 
    }
    if(sq==E1 && board.cells[E1]==WHITE_KING){
        if(castlingRights & 1 && !isSquareAttacked(E1,BLACK)&&!isSquareAttacked(F1,BLACK)&&!isSquareAttacked(G1,BLACK)){
            if(board.cells[F1]==EMPTY && board.cells[G1]==EMPTY)
                moveList[moveCount++] = Move{sq, G1};
        }
        if(castlingRights & 2 && !isSquareAttacked(E1,BLACK)&&!isSquareAttacked(D1,BLACK)&&!isSquareAttacked(C1,BLACK)){
            if(board.cells[D1]==EMPTY && board.cells[C1]==EMPTY && board.cells[B1]==EMPTY)
                moveList[moveCount++] = Move{sq, C1};
        }
    }else if(sq==E8 && board.cells[E8]==BLACK_KING){
        if(castlingRights & 4 && !isSquareAttacked(E8,WHITE)&&!isSquareAttacked(F8,WHITE)&&!isSquareAttacked(G8,WHITE)){
            if(board.cells[F8]==EMPTY && board.cells[G8]==EMPTY)
                moveList[moveCount++] = Move{sq, G8};
        }
        if(castlingRights & 8 && !isSquareAttacked(E8,WHITE)&&!isSquareAttacked(D8,WHITE)&&!isSquareAttacked(C8,WHITE)){
            if(board.cells[D8]==EMPTY && board.cells[C8]==EMPTY && board.cells[B8]==EMPTY)
                moveList[moveCount++] = Move{sq, C8};
        }
    }
}

void generateWhitePawnMoves(Move* moveList, int& moveCount){
    uint64_t singlePushes = (board.pieces[WHITE_PAWN]<<8) & board.pieces[EMPTY]; 
    uint64_t doublePushes = ((singlePushes&RANK_3)<<8) & board.pieces[EMPTY];
    
    uint64_t leftCaptures = (board.pieces[WHITE_PAWN]&~FILE_A)<<7&(board.pieces[BLACK_PIECE]);
    uint64_t rightCaptures = (board.pieces[WHITE_PAWN]&~FILE_H)<<9&(board.pieces[BLACK_PIECE]);

    uint8_t dst;

    while (leftCaptures) {
        dst = getLSBIndex(leftCaptures);
        moveList[moveCount++] = Move{ static_cast<Square>(dst-7), static_cast<Square>(dst) };
        leftCaptures &= leftCaptures - 1; 
    }
    while(rightCaptures){
        dst = getLSBIndex(rightCaptures);
        moveList[moveCount++] = Move{ static_cast<Square>(dst-9), static_cast<Square>(dst) };
        rightCaptures &= rightCaptures - 1;
    }
    while (doublePushes) {
        dst = getLSBIndex(doublePushes);
        moveList[moveCount++] = Move{ static_cast<Square>(dst-16), static_cast<Square>(dst) };
        doublePushes &= doublePushes - 1; 
    }
    while (singlePushes) {
        dst = getLSBIndex(singlePushes);
        moveList[moveCount++] = Move{ static_cast<Square>(dst-8), static_cast<Square>(dst) };
        singlePushes &= singlePushes - 1; 
    }
}

void generateBlackPawnMoves(Move* moveList, int& moveCount){
    uint64_t singlePushes = (board.pieces[BLACK_PAWN]>>8) & board.pieces[EMPTY]; 
    uint64_t doublePushes = ((singlePushes&RANK_6)>>8) & board.pieces[EMPTY];
    
    uint64_t rightCaptures = (board.pieces[BLACK_PAWN]&~FILE_A)>>9&board.pieces[WHITE_PIECE];
    uint64_t leftCaptures = (board.pieces[BLACK_PAWN]&~FILE_H)>>7&board.pieces[WHITE_PIECE];

    uint8_t dst;

    while (rightCaptures) {
        dst = getLSBIndex(rightCaptures);
        moveList[moveCount++] = Move{ static_cast<Square>(dst+9), static_cast<Square>(dst) };
        rightCaptures &= rightCaptures - 1; 
    }
    while(leftCaptures){
        dst = getLSBIndex(leftCaptures);
        moveList[moveCount++] = Move{ static_cast<Square>(dst+7), static_cast<Square>(dst) };
        leftCaptures &= leftCaptures - 1;
    }
    while (doublePushes) {
        dst = getLSBIndex(doublePushes);
        moveList[moveCount++] = Move{ static_cast<Square>(dst+16), static_cast<Square>(dst) };
        doublePushes &= doublePushes - 1; 
    }
    while (singlePushes) {
        dst = getLSBIndex(singlePushes);
        moveList[moveCount++] = Move{ static_cast<Square>(dst+8), static_cast<Square>(dst) };
        singlePushes &= singlePushes - 1; 
    }
}

void generateBishopMoves(Square sq, uint64_t myPieces, Move* moveList, int& moveCount){
    uint64_t ray;
    uint64_t allOcupancy = ~board.pieces[EMPTY];
    for(int dir=0;dir<2;dir++){
        ray = bishopAttacks[sq][dir];
        if(ray & allOcupancy){
            uint8_t hit = getLSBIndex(ray&allOcupancy);
            uint64_t dif = bishopAttacks[hit][dir];
            ray ^= dif;
            ray &= ~myPieces;
        }
        while (ray) {
            uint8_t dstIndex = getLSBIndex(ray); 
            moveList[moveCount++] = Move{ sq, static_cast<Square>(dstIndex) };
            ray &= ray - 1; 
        }
    }
    for(int dir=2;dir<4;dir++){
        ray = bishopAttacks[sq][dir];
        if(ray&allOcupancy){
            uint8_t hit = getMSBIndex(ray&allOcupancy);
            uint64_t dif = bishopAttacks[hit][dir];
            ray ^= dif;
            ray &= ~myPieces;
        }
        while (ray) {
            uint8_t dstIndex = getLSBIndex(ray); 
            moveList[moveCount++] = Move{ sq, static_cast<Square>(dstIndex) };
            ray &= ray - 1; 
        }
    }
}

void generateRookMoves(Square sq, uint64_t myPieces, Move* moveList, int& moveCount){
    uint64_t ray;
    uint64_t allOcupancy = ~board.pieces[EMPTY];
    for(int dir=0;dir<2;dir++){
        ray = rookAttacks[sq][dir];
        if(ray & allOcupancy){
            uint8_t hit = getLSBIndex(ray&allOcupancy);
            uint64_t dif = rookAttacks[hit][dir];
            ray ^= dif;
            ray &= ~myPieces;
        }
        while (ray) {
            uint8_t dstIndex = getLSBIndex(ray); 
            moveList[moveCount++] = Move{ sq, static_cast<Square>(dstIndex) };
            ray &= ray - 1; 
        }
    }
    for(int dir=2;dir<4;dir++){
        ray = rookAttacks[sq][dir];
        if(ray&allOcupancy){
            uint8_t hit = getMSBIndex(ray&allOcupancy);
            uint64_t dif = rookAttacks[hit][dir];
            ray ^= dif;
            ray &= ~myPieces;
        }
        while (ray) {
            uint8_t dstIndex = getLSBIndex(ray); 
            moveList[moveCount++] = Move{ sq, static_cast<Square>(dstIndex) };
            ray &= ray - 1; 
        }
    }
}

void generateQueenMoves(Square sq, uint64_t myPieces, Move* moveList, int& moveCount){
    ///TODO: Rewrite in a nice maner:)
    uint64_t ray;
    uint64_t allOcupancy = ~board.pieces[EMPTY];
    for(int dir=0;dir<2;dir++){
        ray = bishopAttacks[sq][dir];
        if(ray & allOcupancy){
            uint8_t hit = getLSBIndex(ray&allOcupancy);
            uint64_t dif = bishopAttacks[hit][dir];
            ray ^= dif;
            ray &= ~myPieces;
        }
        while (ray) {
            uint8_t dstIndex = getLSBIndex(ray); 
            moveList[moveCount++] = Move{ sq, static_cast<Square>(dstIndex) };
            ray &= ray - 1; 
        }
    }
    for(int dir=2;dir<4;dir++){
        ray = bishopAttacks[sq][dir];
        if(ray&allOcupancy){
            uint8_t hit = getMSBIndex(ray&allOcupancy);
            uint64_t dif = bishopAttacks[hit][dir];
            ray ^= dif;
            ray &= ~myPieces;
        }
        while (ray) {
            uint8_t dstIndex = getLSBIndex(ray); 
            moveList[moveCount++] = Move{ sq, static_cast<Square>(dstIndex) };
            ray &= ray - 1; 
        }
    }
    for(int dir=0;dir<2;dir++){
        ray = rookAttacks[sq][dir];
        if(ray & allOcupancy){
            uint8_t hit = getLSBIndex(ray&allOcupancy);
            uint64_t dif = rookAttacks[hit][dir];
            ray ^= dif;
            ray &= ~myPieces;
        }
        while (ray) {
            uint8_t dstIndex = getLSBIndex(ray); 
            moveList[moveCount++] = Move{ sq, static_cast<Square>(dstIndex) };
            ray &= ray - 1; 
        }
    }
    for(int dir=2;dir<4;dir++){
        ray = rookAttacks[sq][dir];
        if(ray&allOcupancy){
            uint8_t hit = getMSBIndex(ray&allOcupancy);
            uint64_t dif = rookAttacks[hit][dir];
            ray ^= dif;
            ray &= ~myPieces;
        }
        while (ray) {
            uint8_t dstIndex = getLSBIndex(ray); 
            moveList[moveCount++] = Move{ sq, static_cast<Square>(dstIndex) };
            ray &= ray - 1; 
        }
    }
}

void generateAllKnightMoves(Color sideToMove, Move* moveList, int& moveCount){
    uint8_t myKnights = WHITE_KNIGHT | sideToMove<<3;
    uint8_t myPieces = WHITE_PIECE | sideToMove<<3;
    uint64_t knightsCopy = board.pieces[myKnights];
    while (knightsCopy) {
        Square src = static_cast<Square>(getLSBIndex(knightsCopy)); 
        generateKnightMoves(src,board.pieces[myPieces],moveList,moveCount);
        knightsCopy &= knightsCopy - 1; 
    }
}

void generateAllKingMoves(Color sideToMove, Move* moveList, int& moveCount){
    uint8_t myKing = WHITE_KING | sideToMove<<3;
    uint8_t myPieces = WHITE_PIECE | sideToMove<<3;
    uint64_t kingCopy = board.pieces[myKing];
    while (kingCopy) {
        Square src = static_cast<Square>(getLSBIndex(kingCopy)); 
        generateKingMoves(src,board.pieces[myPieces],moveList,moveCount);
        kingCopy &= kingCopy - 1; 
    }
}

void generateAllBishopMoves(Color sideToMove, Move* moveList, int& moveCount){
uint8_t myBishops = WHITE_BISHOP | sideToMove<<3;
    uint8_t myPieces = WHITE_PIECE | sideToMove<<3;
    uint64_t bishopsCopy = board.pieces[myBishops];
    while (bishopsCopy) {
        Square src = static_cast<Square>(getLSBIndex(bishopsCopy)); 
        generateBishopMoves(src,board.pieces[myPieces],moveList,moveCount);
        bishopsCopy &= bishopsCopy - 1; 
    }
}

void generateAllRookMoves(Color sideToMove, Move* moveList, int& moveCount){
    uint8_t myRooks = WHITE_ROOK | sideToMove<<3;
    uint8_t myPieces = WHITE_PIECE | sideToMove<<3;
    uint64_t rooksCopy = board.pieces[myRooks];
    while (rooksCopy) {
        Square src = static_cast<Square>(getLSBIndex(rooksCopy)); 
        generateRookMoves(src,board.pieces[myPieces],moveList,moveCount);
        rooksCopy &= rooksCopy - 1; 
    }
}

void generateAllQueenMoves(Color sideToMove, Move* moveList, int& moveCount){
    uint8_t myQueens = WHITE_QUEEN | sideToMove<<3;
    uint8_t myPieces = WHITE_PIECE | sideToMove<<3;
    uint64_t queensCopy = board.pieces[myQueens];
    while (queensCopy) {
        Square src = static_cast<Square>(getLSBIndex(queensCopy)); 
        generateQueenMoves(src,board.pieces[myPieces],moveList,moveCount);
        queensCopy &= queensCopy - 1; 
    }
}

int generateAllMoves(Color sideToMove, Move *moveList){
    int cnt = 0;
    if(sideToMove==WHITE){
        generateWhitePawnMoves(moveList,cnt);
    }else{
        generateBlackPawnMoves(moveList,cnt);
    }
    generateAllKnightMoves(sideToMove,moveList,cnt);
    generateAllBishopMoves(sideToMove,moveList,cnt);
    generateAllRookMoves(sideToMove,moveList,cnt);
    generateAllQueenMoves(sideToMove,moveList,cnt);
    generateAllKingMoves(sideToMove,moveList,cnt);
    return cnt;
}

Move findBestMove(Color sideToMove){
    Move moveList[256];
    MoveInfo moveInfo;
    int moveCount = generateAllMoves(sideToMove, moveList);
    int bestIndex=-1;
    int currEval;
    int legalMoves=0;
    if(sideToMove==WHITE){
        int maxEval=-10000000;
        for(int i=0;i<moveCount;i++){
            moveInfo = makeMove(moveList[i]);

            if (isSquareAttacked(getKingSquare(WHITE), BLACK)) {
                //printf("info string DEBUG: Mutarea %d -> %d a primit scorul %d\n",moveList[i].src,moveList[i].dst,currEval);
                unmakeMove(moveInfo);
                continue;
            }
            legalMoves++;
            currEval = minMax(maxDepth, BLACK);
            //printf("Mutarea %d -> %d a primit scorul %d\n",moveList[i].src,moveList[i].dst,currEval);
            if(currEval > maxEval || bestIndex == -1){
                maxEval=currEval;
                bestIndex=i;
            }
            unmakeMove(moveInfo);
        }
    }else{
        int minEval=10000000;
        for(int i=0;i<moveCount;i++){
            moveInfo = makeMove(moveList[i]);
            Square blackKingSq = getKingSquare(BLACK);
            bool isAttacked = isSquareAttacked(blackKingSq, WHITE);
            
            // Printăm adevărul gol-goluț!
            std::cout << "info string DEBUG: Regele Negru este pe patratul " << static_cast<int>(blackKingSq) << "\n";
            std::cout << "DEBUG: isSquareAttacked zice ca e atacat? " 
                    << (isAttacked ? "DA" : "NU") << "\n";
            std::cout<<"info string DEBUG: index is: "<<i<<std::endl;
            std::cout<<std::flush;

            if (isSquareAttacked(getKingSquare(BLACK), WHITE)) {
                unmakeMove(moveInfo);
                continue;
            }
            legalMoves++;
            currEval = minMax(maxDepth, WHITE);
            //printf("info string DEBUG: Mutarea %d -> %d a primit scorul %d\n",moveList[i].src,moveList[i].dst,currEval);
            if(currEval < minEval || bestIndex == -1){
                printf("DEBUG: Mutarea %d -> %d a primit scorul %d\n",moveList[i].src,moveList[i].dst,currEval);
                minEval=currEval;
                bestIndex=i;
            }
            unmakeMove(moveInfo);
        }
    }
    printf("info string DEBUG: legalmoves: %d -\n",legalMoves);
    if (legalMoves == 0) {
        //semnal de Game Over
        return Move{A1, A1}; 
    }

    return moveList[bestIndex];
}

bool isSquareAttacked(Square sq, Color attacker)
{
    uint64_t square = 1ULL<<sq;

    if(attacker == WHITE) {
        uint64_t possibleAttackers = ((square & ~FILE_H) >> 7) | ((square & ~FILE_A) >> 9);
        if(possibleAttackers & board.pieces[WHITE_PAWN])
            return true;
    } else {
        uint64_t possibleAttackers = ((square & ~FILE_A) << 7) | ((square & ~FILE_H) << 9);
        if(possibleAttackers & board.pieces[BLACK_PAWN])
            return true;
    }

    if(knightAttacks[sq] & board.pieces[attacker==WHITE?WHITE_KNIGHT:BLACK_KNIGHT])
        return true;
    
    if(kingAttacks[sq] & board.pieces[attacker==WHITE?WHITE_KING:BLACK_KING])
        return true;

    uint64_t bishopsQueenCopy = board.pieces[attacker==WHITE?WHITE_BISHOP:BLACK_BISHOP]|board.pieces[attacker==WHITE?WHITE_QUEEN:BLACK_QUEEN];
    uint64_t rooksQueenCopy = board.pieces[attacker==WHITE?WHITE_ROOK:BLACK_ROOK]|board.pieces[attacker==WHITE?WHITE_QUEEN:BLACK_QUEEN];
    uint64_t allOcupancy = ~board.pieces[EMPTY];
    uint64_t myPieces = board.pieces[attacker==WHITE?BLACK_PIECE:WHITE_PIECE];

    uint64_t bishopRays = 0;
    for(int dir=0;dir<4;dir++){
        uint64_t ray = bishopAttacks[sq][dir];
        if(ray & allOcupancy){
            uint8_t hit;
            if(ray > square)
                hit = getLSBIndex(ray&allOcupancy);
            else
                hit = getMSBIndex(ray&allOcupancy);
            uint64_t dif = bishopAttacks[hit][dir];
            ray ^= dif;
            //ray &= ~myPieces;
        }
        bishopRays |=ray;
    }
    if(bishopRays & bishopsQueenCopy)
        return true;
    
    uint64_t rookRays = 0;
    for(int dir=0;dir<4;dir++){
        uint64_t ray = rookAttacks[sq][dir];
        if(ray & allOcupancy){
            uint8_t hit;
            if(ray > square)
                hit = getLSBIndex(ray&allOcupancy);
            else
                hit = getMSBIndex(ray&allOcupancy);
            uint64_t dif = rookAttacks[hit][dir];
            ray ^= dif;
            //ray &= ~myPieces;
        }
        rookRays |=ray;
    }
    if(rookRays&rooksQueenCopy)
        return true;

    return false;
}

Square getKingSquare(Color color){
    int king = WHITE_KING | color<<3;
    return static_cast<Square>(getMSBIndex(board.pieces[king]));
}