#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include "mpi.h"

/* Local functions */

static void master(void);
static void slave(void);

#include "hamming_distance.h"

#ifndef TYPES
#define TYPES
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned long long ullong;
#endif

// `"ruby "*12` size is (5*12)=60
#define SUFFIX_OFFSET 60

#define SUFFIX_SPACE 1073741824UL
//#define SUFFIX_SPACE 429496729UL
#define CASE_SPACE 281474976710656ULL

#define WORD_SIZE 4

void
permute_suffix(ulong seqnum, uchar *suffix)
{
  uchar *cp;
  for (cp = suffix; cp < suffix+5; ++cp)
  {
    *cp = (uchar)(33 + (seqnum & 63));
    seqnum >>= 6;
  }
}

// So basically what we're doing here is applying a gigantic bitmask
// to the case of a string.
// 1101 maps to RUbY, and so on. We skip over blanks, using mod, since we
// know they happen every 5 chars. 
void
permute_case(ullong seqnum, uchar *phrase)
{
  int i;
  uchar *cp = phrase;
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

char *goal;

int
main(int argc, char **argv)
{
  int myrank;

  extern char *goal;
  
  if (argc) 
    goal = argv[1];
  
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Barrier(MPI_COMM_WORLD);
  if (myrank == 0)
    master();
  else
    slave();

  MPI_Finalize();
  return 0;
}

static void
master()
{
  char phrase[61] = "ruby ruby ruby ruby ruby ruby ruby ruby ruby ruby ruby ruby ";
  char sfx[6] = "XXXXX";

  extern char *goal;
    
  int ntasks, rank;
  MPI_Status status;
  long long initial = 0;

  long long int result[3];

  long long int distance, case_perm, suffix_perm, best_distance;

  best_distance = 10000000;
  
  /* Find out how many processes there are */
  MPI_Comm_size(MPI_COMM_WORLD, &ntasks);

  if (ntasks == 1)
    printf("WARNING: Run like this: mpirun -np [Number of processes] ./generate\n");
    
  /* Seed the slaves; send one unit of work to each slave. */
  for (rank = 1; rank < ntasks; ++rank) {

    case_perm = initial + (1000000 * rank);

    /* Send it to each rank */
    MPI_Send(&case_perm,        /* message buffer */
             1,                 /* one data item */
             MPI_LONG_LONG_INT, /* data item is an integer */
             rank,              /* destination process rank */
             0,                 /* user chosen message tag */
             MPI_COMM_WORLD);   /* default communicator */
  }

  
  while (1) {
    MPI_Recv(&result,           /* message buffer */
             3,                 /* one data item */
             MPI_LONG_LONG_INT, /* of type double real */
             MPI_ANY_SOURCE,    /* receive from any sender */
             MPI_ANY_TAG,       /* any type of message */
             MPI_COMM_WORLD,    /* default communicator */
             &status);          /* info about the received message */

    case_perm = result[0];
    suffix_perm = result[1];
    distance = result[2];

    if (distance < best_distance)
    {
      best_distance = distance;

      permute_case((ullong) case_perm, (uchar*) phrase);
      permute_suffix((ulong) suffix_perm, (uchar*) sfx);
      
      printf("%llu : %llu : %llu : %s%s\n", case_perm, suffix_perm, distance, phrase, sfx);
      
    }

  }
}


static void 
slave()
{
  MPI_Status status;
  extern char *goal;
  uchar phrase[61] = "ruby ruby ruby ruby ruby ruby ruby ruby ruby ruby ruby ruby ";
  uchar hash[20];
  uchar sfx[6] = "XXXXX";
  ullong case_perm = 0;
  ulong suffix_perm = 0;

  long long int send_buffer[3];

  int distance;
  int min_distance = 1000000;

  SHA_CTX *suffctx = (SHA_CTX *) malloc(sizeof(SHA_CTX));
  SHA_CTX *casectx = (SHA_CTX *) malloc(sizeof(SHA_CTX));



  MPI_Recv(&case_perm, 1, MPI_LONG_LONG_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

  if (!goal)
    goal = "6cac827bae250971a8b1fb6e2a96676f7a077b60";
  set_goal(goal);

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
    
    for (suffix_perm = 0; suffix_perm < SUFFIX_SPACE; ++suffix_perm)
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

        send_buffer[0] = (long long int) case_perm;
        send_buffer[1] = (long long int) suffix_perm;
        send_buffer[2] = (long long int) distance;

        MPI_Send(&send_buffer, 3, MPI_LONG_LONG_INT, 0, 0, MPI_COMM_WORLD);

      }
    }
  }
}


