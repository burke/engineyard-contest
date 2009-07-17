#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

//#include "table_generated.h"

#include "hamming_distance.h"

static uchar GOAL[20];

int HAM_TABLE[256][256];


void
set_goal(char* goal)
{
  int i;
  int j;

  for (i=0; i<256; ++i)
    for (j=0; j<256; ++j)
      HAM_TABLE[i][j] = hamming_distance(i, j);
  
  for (i=0; i<20; ++i)
    sscanf(goal+(2*i), "%02x", (unsigned int *) &GOAL[i]);
}


int
hamming_distance(uchar a, uchar b)
{
  uchar c = a ^ b;
  int total = 0;
  while (c > 0)
  {
    if (c % 2)
      ++total;
    c >>= 1;
  }
  return total;
}


int
hamming_distance_from_goal(uchar* hash)
{
  int i;
  int distance = 0;

  for (i = 0; i < 20; ++i)
    distance += HAM_TABLE[GOAL[i]][hash[i]];
    
  return distance;
}
