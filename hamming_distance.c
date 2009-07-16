#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#include "table_generated.h"

static unsigned char GOAL[20];
extern int HAM_TABLE[256][256];

void
set_goal(char* goal)
{
  int i;
  build_hamming_table();
  
  for (i=0; i<20; ++i)
    sscanf(goal+(2*i), "%02x", (unsigned int *) &GOAL[i]);
}

int
hamming_distance_from_goal(unsigned char* hash)
{
  int i;
  int distance = 0;

  for (i = 0; i < 20; ++i) {
    printf("%d,",HAM_TABLE[GOAL[i]][hash[i]]);
    distance += HAM_TABLE[GOAL[i]][hash[i]];

  }
    
  return distance;
}
