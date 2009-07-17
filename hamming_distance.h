#ifndef HAMMING_DISTANCE_H
#define HAMMING_DISTANCE_H

#ifndef TYPES
#define TYPES
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned long long ullong;
#endif

void
set_goal(char *hex_goal);

int
hamming_distance_from_goal(unsigned char* hash);

int
hamming_distance(uchar a, uchar b);

#endif
