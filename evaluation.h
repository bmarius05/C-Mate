#pragma once
#include "bitboards.h"
#include "moves.h"
#include "pst.h"

int staticEval();
int minMax(int depth,Color sideToMove,int alpha, int beta);