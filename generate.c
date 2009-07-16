#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#include "hamming_distance.h"

// `"ruby "*12` size is (5*12)=60
#define SUFFIX_OFFSET 60

#define SUFFIX_SPACE 6956883693ULL
#define CASE_SPACE 281474976710656ULL

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
      *cp = toupper(*cp);
    else // make sure it's lower case.
      *cp = tolower(*cp);

    cp++;
  }
}

int
main(int argc, char* argv[])
{
  unsigned char phrase[61] = "ruby ruby ruby ruby ruby ruby ruby ruby ruby ruby ruby ruby ";
  unsigned char hash[20];
  unsigned char sfx[6] = "XXXXX";
  unsigned long long case_perm = 0;
  unsigned long long suffix_perm = 0;
  int j;
  int distance;
  int min_distance = 1000000;

  SHA_CTX *suffctx = malloc(sizeof(SHA_CTX));
  SHA_CTX *casectx = malloc(sizeof(SHA_CTX));
  
  /* Parse Command Line for starting point (or not) */
  switch (argc)
  {
    case 4:
      suffix_perm = atoi(argv[3]);
    case 3:
      case_perm = atoi(argv[2]);
    case 2:
      set_goal(argv[1]);
      break;
    default:
      case_perm = 0;
      suffix_perm = 0;
      set_goal("6cac827bae250971a8b1fb6e2a96676f7a077b60");
  }  
  
  for (/* case_perm */; case_perm < CASE_SPACE; ++case_perm)
  {
    permute_case(case_perm, phrase);

    SHA1_Init(casectx);
    SHA1_Update(casectx, phrase, strlen((char*) phrase));
    
    for (/* suffix_perm */; suffix_perm < SUFFIX_SPACE; ++suffix_perm)
    {
      permute_suffix(suffix_perm, sfx);

      memcpy(suffctx, casectx, sizeof(SHA_CTX));
      memcpy(suffctx->data, casectx->data, sizeof(SHA_LONG)*SHA_LBLOCK);
      SHA1_Update(suffctx, sfx, 5);
      SHA1_Final(hash,suffctx);

      distance = hamming_distance_from_goal(hash);
      if (distance < min_distance)
      {
        min_distance = distance;
        for (j = 0; j < 20; j++)
          printf("%02x", hash[j]);
        printf(" : %llu : %llu : %d : %s%s\n", case_perm, suffix_perm, distance, phrase, sfx);
      }
    }
  }

  return EXIT_SUCCESS;
}

