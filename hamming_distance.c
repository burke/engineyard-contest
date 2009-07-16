#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#include "table_generated.h"

static unsigned char GOAL[20];

void
set_goal(char* goal)
{
  int i;
  
  build_hamming_table();

  
  for (i=0; i<20; ++i)
  {
    
  }
  
}

int
hamming_distance_from_goal(unsigned char* hash)
{
  int i;
  int distance = 0;
  
  for (i = 0; i < 20; ++i)
    distance += HAM_TABLE[(int) GOAL[i]][(int) hash[i]];
  
  return distance;
}
