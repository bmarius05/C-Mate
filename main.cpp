#include <iostream>
#include <string>
#include <sstream>
#include "bitboards.h"
#include "moves.h"
#include "evaluation.h"
#include <thread>

#define NAME "C-Mate"
#define AUTHOR "bmarius05"
#define BOOK "../../book.txt"

#define DEFAULT_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"


std::thread searchThread;


char fenNotation[73];
Color sideToMove = WHITE;

uint64_t gameHistory[1024];

long long totalExplored = 0;

uint8_t moveCnt;

#define PLAYER_SIDE WHITE

void init(){
    initBoard();
    initKnightAttacks();
    initKingAttacks();
    initBishopAttacks();
    initRookAttacks();;
    initZobrist();
    loadOppeningBook(BOOK);
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

void UCILoop(){
    std::string line;
    int wtime, btime, winc,binc, moveTime, movesToGo;

    while(std::getline(std::cin, line)){
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if(command == "uci"){
            std::cout<<"id name "<<NAME<<std::endl;
            std::cout<<"id author "<<AUTHOR<<std::endl;
            init();
            std::cout<<"uciok\n";
            std::cout << std::flush;
        }else if(command == "isready"){
            std::cout<<"readyok\n";
            std::cout << std::flush;
        }else if(command == "ucinewgame"){
            sideToMove=WHITE;
            resetFlags();
            loadBoard(DEFAULT_POSITION);
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
                        boardHash^=sideKey;
                    }
                    else if(command == "w"){
                        std::cout<<"To move white\n";
                        sideToMove = WHITE;
                    }
                    if(command == "moves"){
                        while(iss>>command){
                            makeMove(stringToMove(command));
                        }
                    }
                }
            }
            validateBoardState();
            showBoard();
        }else if(command=="go"){
            winc=binc=wtime=btime=moveTime=movesToGo=0;
            while(iss>>command){
                if(command == "wtime"){
                    iss>>command;
                    wtime = std::stoi(command);
                }else if(command == "btime"){
                    iss>>command;
                    btime = std::stoi(command);
                }else if(command == "winc"){
                    iss>>command;
                    winc = std::stoi(command);
                }else if(command == "binc"){
                    iss>>command;
                    binc = std::stoi(command);
                }else if(command == "movetime"){
                    iss>>command;
                    moveTime = std::stoi(command);
                }else if(command == "movestogo"){
                    iss>>command;
                    movesToGo = std::stoi(command);   
                }else if(command == "infinite"){
                    winc=binc=wtime=btime=moveTime=0;
                }
            }

            if (searchThread.joinable()) {
                searchThread.join(); 
            }
            Color side = sideToMove;

            searchThread = std::thread([side, wtime, winc,btime,binc, moveTime, movesToGo]() { 
                // Asta rulează complet separat de std::cin!
                Move bestMove;
                if(side == WHITE)
                    bestMove = findBestMoveTime(side, wtime, winc, movesToGo, moveTime);
                else
                    bestMove = findBestMoveTime(side, btime, binc, movesToGo, moveTime);
                // EXTREM DE IMPORTANT: bestmove se trimite de AICI, din interiorul thread-ului,
                // fix în momentul în care funcția s-a oprit (fie natural, fie de la stop)
                std::cout<<"bestmove "<<moveToString(bestMove);
                std::cout <<std::endl<< std::flush;

            });

        }else if (command == "stop") {
            timeIsUp = true; 
            
            if (searchThread.joinable()) {
                searchThread.join(); 
            }
        }
        else if (command == "quit") {
            timeIsUp = true;
            if (searchThread.joinable()) {
                searchThread.join();
            }
            break;
        }
    }
}

int main(){
    std::string line;
    std::getline(std::cin, line);
    std::istringstream iss(line);
    std::string command;
    iss >> command;

    if(command == "uci"){
        std::cout<<"id name "<<NAME<<std::endl;
        std::cout<<"id author "<<AUTHOR<<std::endl;
        init();
        std::cout<<"uciok\n";
        std::cout << std::flush;
        UCILoop();
    }

    return 0;
}