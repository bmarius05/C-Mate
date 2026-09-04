#include <iostream>
#include <string>
#include <sstream>
#include "bitboards.h"
#include "moves.h"
#include "evaluation.h"
#include <cassert>

#define NAME "C-Mate"
#define AUTHOR "bmarius05"

#define DEFAULT_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"

//test

char fenNotation[73];
Color sideToMove = WHITE;

#define PLAYER_SIDE WHITE

void init(){
    initBoard();
    initKnightAttacks();
    initKingAttacks();
    initBishopAttacks();
    initRookAttacks();
}

Move getPlayerMove(){
    char column;
    int row;
    int src, dst;
    printf("Insert Move:\nFrom: ");
    std::cin>>column>>row;
    src=toupper(column)-'A'+(row-1)*8;
    printf("To: ");
    std::cin>>column>>row;
    dst=toupper(column)-'A'+(row-1)*8;
    printf("src: %d\tdst: %d\n",src,dst);
    return Move{static_cast<Square>(src), static_cast<Square>(dst)};
}

Move stringToMove(std::string move){
    int from = 8*(move[1]-'0'-1) + toupper(move[0])-'A';
    int to = 8*(move[3]-'0'-1) + toupper(move[2])-'A';
    std::cout<<"From: "<<from<<"\tto:"<<to<<std::endl;
    return Move{static_cast<Square>(from),static_cast<Square>(to)};
}

void UCILoop(){
    init();
    std::string command;
    std::string line;
    std::string move;
    while(std::getline(std::cin, line)){
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if(command == "uci"){
            std::cout<<"id name "<<NAME<<std::endl;
            std::cout<<"id author "<<AUTHOR<<std::endl;
            std::cout<<"uciok\n";
            std::cout << std::flush;
        }else if(command == "isready"){
            std::cout<<"readyok\n";
            std::cout << std::flush;
        }else if(command == "position"){
            iss >> command;
            if(command == "startpos"){
                sideToMove=WHITE;
                
                resetFlags();
                
                iss >> command;
                loadBoard(DEFAULT_POSITION);
                if(command == "moves"){
                    while(iss>>command){
                        makeMove(stringToMove(command));
                        sideToMove = oppositeColor(sideToMove);
                    }
                }
            }else if(command == "fen"){
                iss >> command;
                std::cout<<"Command is: "<<command<<std::endl;
                loadBoard(command.data());
                while(iss>>command){
                    if(command == "b"){

                        std::cout<<"To move black\n";
                        sideToMove = BLACK;
                    }
                    else if(command == "w"){
                        std::cout<<"To move white\n";
                        sideToMove = WHITE;
                    }
                    if(command == "moves"){
                        while(iss>>command){
                            makeMove(stringToMove(command));
                            sideToMove = oppositeColor(sideToMove);
                        }
                    }
                }
            }
            validateBoardState();
            showBoard();
        }else if(command=="go"){
            move.clear();
            std::cout<<sideToMove<<std::endl;
            Move bestMove = findBestMove(sideToMove);
            move.push_back('a'+(bestMove.src%8));
            move.push_back('1'+(bestMove.src/8));
            move.push_back('a'+(bestMove.dst%8));
            move.push_back('1'+(bestMove.dst/8));
            std::cout<<"bestmove "<<move<<std::endl;
            std::cout << std::flush;
        }else if(command=="quit"){
            break;
        }
    }
}

int main(){
    system("clear");
    
    UCILoop();
    
    return 0;
    scanf("%s",fenNotation);
    loadBoard(fenNotation);
    showBoard();

    init();
    /*
    makeMove(Move{D2,D4});
    makeMove(Move{D7,D5});
    makeMove(Move{C1,F4});
    makeMove(Move{C8,F5});
    /*
    */
    int evaluation = 0;
    for(int i=0;i<1000&&evaluation<1000000&&evaluation>-1000000;i++){
        Move currentMove;
        currentMove = findBestMove(i%2?BLACK:WHITE);
        if(currentMove.src==currentMove.dst && currentMove.dst==A1){
            puts("GAME OVER!");
            printf("%s WON!\n",i%2?"White":"Black");
            return 0;
        }
        if(i%2 == PLAYER_SIDE){
            currentMove = getPlayerMove();
        }
        makeMove(currentMove);
        showBoard();
        evaluation = staticEval();
        printf("Eval: %4d\n", evaluation);
        int src = currentMove.src;
        int dst = currentMove.dst;
        printf("%4d: %s moved from:%c%c\tto: %c%c\n\n",i,i%2?"Black":"White", 'A'+(src%8), '1'+(src/8), 'A'+(dst%8), '1'+(dst/8));
    }
    return 0;
}