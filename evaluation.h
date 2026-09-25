#include "bitboards.h"
#include "moves.h"

int staticEval();
int minMax(int depth,Color sideToMove,int alpha, int beta);