#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#include "hamming_distance.h"

// `"ruby "*12` size is (5*12)=60
#define SUFFIX_OFFSET 60

#define SUFFIX_SPACE 6956883693LL
#define CASE_SPACE 281474976710656LL

#define WORD_SIZE 4

void
permute_suffix(long long seqnum, unsigned char *suffix)
{
  unsigned char *cp;
  for (cp = suffix; cp < suffix+5; ++cp)
  {
    *cp = (unsigned char)(33 + (seqnum % 93));
    seqnum /= 93;
  }
}

void
permute_case(unsigned long long seqnum, unsigned char *phrase)
{
  int i;
  unsigned char *cp = phrase;
  for (i=0; i < 48; ++i)
  {
    if (!(i % WORD_SIZE) && (i != 0)) cp++; // Skip over spaces.

    if (seqnum & (1ULL << i)) // The i-th bit is set. Make it upper case.
      *cp = (unsigned char) toupper((char) *cp);
    else // make sure it's lower case.
      *cp = (unsigned char) tolower((char) *cp);

    cp++;
  }
}

int
main(int argc, char* argv[])
{
  unsigned char phrase[66] = "ruby ruby ruby ruby ruby ruby ruby ruby ruby ruby ruby ruby XXXXX";
  unsigned char hash[20];
  unsigned char *sfx = (phrase + SUFFIX_OFFSET);
  unsigned long long case_perm;
  unsigned long long suffix_perm;
  int j;
  int len = strlen((char*) phrase);
  int distance;
  int min_distance = 1000000;
  
  /* Parse Command Line for starting point (or not) */
  if (argc == 3) { // We have case and suffix permutation numbers...
    case_perm = atoi(argv[1]);
    suffix_perm = atoi(argv[2]);
  }
  else {
    case_perm = 0;
    suffix_perm = 0;
  }
  
  
  set_goal("6cac827bae250971a8b1fb6e2a96676f7a077b60");
  
  for (/* case_perm */; case_perm < CASE_SPACE; ++case_perm)
  {
    permute_case(case_perm, phrase);
    for (/* suffix_perm */; suffix_perm < SUFFIX_SPACE; ++suffix_perm)
    {
      permute_suffix(suffix_perm, sfx);
      SHA1(phrase, len, hash);

      distance = hamming_distance_from_goal(hash);
      if (distance < min_distance) {
        min_distance = distance;
        for (j = 0; j < 20; j++) {
          printf("%02x", hash[j]);
        }
        printf(",%llu,%llu : %d\n", case_perm, suffix_perm, distance);
      }
      
      
    }
  }

  return EXIT_SUCCESS;
}

