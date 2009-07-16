#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#include "hamming_distance.h"

// `"ruby "*12` size is (5*12)=60
#define SUFFIX_OFFSET 60

#define SUFFIX_SPACE 6956883693ULL
//#define SUFFIX_SPACE 429496729UL
#define CASE_SPACE 281474976710656ULL

#define WORD_SIZE 4


// This is basically some weird math to print a base 93 number
// into the suffix. remember 1234 = 1*10^4 + 2*10^3 + 3*10^2 + 4*10^1
// Same deal for base 93. It's a bit terse. 
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

// So basically what we're doing here is applying a gigantic bitmask
// to the case of a string.
// 1101 maps to RUbY, and so on. We skip over blanks, using mod, since we
// know they happen every 5 chars. 
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

  // openssl/sha.h uses a data structure something like this:
  // 
  // struct SHA_CTX {
  //   SHA_LONG lots, of, them;
  //   SHA_LONG data[16];
  // };
  //
  // We can calculate the hash incrementally. The case permutation
  // very rarely changes, so we'll calculate the hash of that,
  // then copy the resulting SHA_CTX for each suffix permutation,
  // and feed in the suffix. This is much more efficient than
  // rehashing the phrase portion each time the suffix changes
  // ~42% performance boost.
  
  for (/* case_perm */; case_perm < CASE_SPACE; ++case_perm)
  {
    permute_case(case_perm, phrase);

    // Initialize a SHA_CTX and prime it with the case permutation.
    SHA1_Init(casectx);
    SHA1_Update(casectx, phrase, strlen((char*) phrase));
    
    for (/* suffix_perm */; suffix_perm < SUFFIX_SPACE; ++suffix_perm)
    {
      // Update sfx with the next suffix.
      permute_suffix(suffix_perm, sfx);

      // Copy the SHA_CTX containing the relevant case permutation hash,
      // and append the suffix, then pull out the final hash.
      memcpy(suffctx, casectx, sizeof(SHA_CTX));
      memcpy(suffctx->data, casectx->data, sizeof(SHA_LONG)*SHA_LBLOCK);
      SHA1_Update(suffctx, sfx, 5);
      SHA1_Final(hash,suffctx);

      // If this is a new best, print 'er out.
      distance = hamming_distance_from_goal(hash);
      if (distance < min_distance)
      {
        min_distance = distance;
        // So it's worth noting that openssl/sha.h returns a strange format.
        // You get an array of unsigned chars, each containing the binary representation
        // of two hex digits, so eg. char 35 is 00010011 or "13" in hex.
        for (j = 0; j < 20; j++)
          // Hex representation makes a lot more sense to humans.
          printf("%02x", hash[j]);
        printf(" : %llu : %llu : %d : %s%s\n", case_perm, suffix_perm, distance, phrase, sfx);
      }
    }
  }

  return EXIT_SUCCESS;
}

