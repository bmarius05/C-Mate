#include <iostream>
#include <string>
#include "bitboards.h"
#include <cassert>

char printablePieces[]="-PBNRQK--pbnrqk---";

void showPositions(){
    printf("\n\n  ");
    for(int j=0;j<8;j++){
        printf("%2c ",'A'+j);
    }
    puts("");
    for(int i=7;i>=0;i--){
        printf("%d ",i+1);
        for(int j=0;j<8;j++){
            printf("%02d ",i*8+j);
        }
        printf(" %d\n",i+1);
    }
    printf("  ");
    for(int j=0;j<8;j++){
        printf("%2c ",'A'+j);
    }
    puts("");
    puts("");
}

void showBoard(){
    printf("\n  ABCDEFGH  \n");
    for(int i=7;i>=0;i--){
        printf("%d ",i+1);
        for(int j=0;j<8;j++){
            std::cout<<printablePieces[board.cells[i*8+j]];
        }
        printf(" %d\n",i+1);
    }
    printf("  ABCDEFGH  \n");
    //showPositions();
}

void initBoard(){
    board.cells[E1]=WHITE_KING;
    board.cells[D1]=WHITE_QUEEN;
    board.cells[A1]=board.cells[H1]=WHITE_ROOK;
    board.cells[B1]=board.cells[G1]=WHITE_KNIGHT;
    board.cells[C1]=board.cells[F1]=WHITE_BISHOP;
    for(int i=A2;i<=H2;i++){
        board.cells[i]=WHITE_PAWN;
    }

    board.cells[E8]=BLACK_KING;
    board.cells[D8]=BLACK_QUEEN;
    board.cells[A8]=board.cells[H8]=BLACK_ROOK;
    board.cells[B8]=board.cells[G8]=BLACK_KNIGHT;
    board.cells[C8]=board.cells[F8]=BLACK_BISHOP;
    for(int i=A7;i<=H7;i++){
        board.cells[i]=BLACK_PAWN;
    }

    PieceType pcs;
    for(int i=A1;i<=H8;i++){
        pcs = board.cells[i];
        board.pieces[pcs]|=bSq(i);
        if(WHITE(pcs))
            board.pieces[WHITE_PIECE]|=bSq(i);
        else if(BLACK(pcs))
            board.pieces[BLACK_PIECE]|=bSq(i);
    }
}
void setPiece(int pos, PieceType piece){
    board.cells[pos]=piece;
    board.pieces[piece]|=bSq(pos);
    if(WHITE(piece))
        board.pieces[WHITE_PIECE]|=bSq(pos);
    else if(BLACK(piece))
        board.pieces[BLACK_PIECE]|=bSq(pos);
    board.pieces[EMPTY] = ~(board.pieces[WHITE_PIECE] | board.pieces[BLACK_PIECE]);
}

void clearBoard(){
    for(int i=A1;i<=H8;i++){
        board.cells[i]=EMPTY;
    }
    for(int i=1;i<=15;i++)
        board.pieces[i]=0;
    board.pieces[EMPTY] = ~0ULL;
}

void validateBoardState33() {
    for (int sq = 0; sq < 64; sq++) {
        int pieceInArray = board.cells[sq]; 
        uint64_t mask = 1ULL << sq;
        if ((board.pieces[pieceInArray] & mask) == 0) {
            std::cout << "CRITICAL DESYNC pe patratul " << sq << "\n";
            std::cout << "Array-ul zice ca e piesa " << pieceInArray << ", dar bitboard-ul nu o are!\n";
            assert(false);
        }
    }
}

void loadBoard(char* fen){
    //BPNKQR

    clearBoard();
    int lookupTable['R'-'B'+1];
    lookupTable['B'-'B'] = WHITE_BISHOP;
    lookupTable['P'-'B'] = WHITE_PAWN;
    lookupTable['N'-'B'] = WHITE_KNIGHT;
    lookupTable['K'-'B'] = WHITE_KING;
    lookupTable['Q'-'B'] = WHITE_QUEEN;
    lookupTable['R'-'B'] = WHITE_ROOK;

    size_t len = strlen(fen);
    int pos = A8;
    char* p = fen;
    while((*p)!=NULL&& (*p) != ' '){
        char pieceChar = *p;
        if(pieceChar>='0'&&pieceChar<='9'){
            pos+=pieceChar-'0';
            if(pos%8==0)
                pos-=8;
        }
        else if(pieceChar=='/'){
            pos-=8;
        }
        else if(isalpha(pieceChar)){
            int piece = lookupTable[toupper(pieceChar)-'B'];
            if(pieceChar >= 'a'){
                piece |= 1ULL<<3;
            }
            setPiece(pos, static_cast<PieceType>(piece));
            pos++;
            if(pos%8==0)
                pos-=8;
        }
        p++;
    }
    validateBoardState33();
}

Color oppositeColor(Color c)
{
    if(c == WHITE)
        return BLACK;
    else
        return WHITE;
    return WHITE;
}
