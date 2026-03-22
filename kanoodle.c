/* kanoodle.c
Copyright (C) 2009 Masta Mappa (John Johnson)
Thu Dec 31 01:40:12 CST 2009
Wed Aug 28 06:21:36 MST 2024 added color output to usage()
*/
#if 0
#!/usr/bin/wish

# 2d board
#   01 02 03 04 05 06 07 08 09 10 11
# A  
# B
# C
# D
# E

# 3d board

#   01
# A  

#   01 02
# A  
# B

#   01 02 03
# A  
# B
# C

#   01 02 03 04
# A  
# B
# C
# D

#   01 02 03 04 05
# A  
# B
# C
# D
# E


# colors
# orange	A
# red	B
# blue	C
# pink	D
# green	E
# white	F
# lt blue	G
# hot pink	H
# yellow	I
# purple	J
# lt green	K
# gray	L

# shapes
#     A         B        C      D      E    F
#     A        BB        C      D      E   FF
#    AA        BB        C     DD     EE
#                       CC      D     E
#
#      G         H     I I      J     KK    L
#      G        HH     III      J     KK   LLL
#    GGG       HH               J           L
#                               J

# posisions
#
#  A    A     AA    AAA   A     AAA   AA      A
#  A    AAA   A       A   A     A      A    AAA
# AA          A           AA           A
#
#  B    BB    BB    BBB   B     BBB   BB     BB
# BB    BBB   BB     BB   BB    BB    BB    BBB
# BB          B           BB           B
#
#  C    C     CC    CCCC  C     CCCC  CC       C
#  C    CCCC  C        C  C     C      C    CCCC
#  C          C           C            C
# CC          C           CC           C
#  
#  D     D    D     DDDD  D     DDDD   D      D
#  D    DDDD  DD      D   D      D    DD    DDDD
# DD          D           DD           D
#  D          D           D            D
#
#  E    EE     E    EEE   E      EEE  E       EE
#  E     EEE  EE      EE  E     EE    EE    EEE
# EE          E           EE           E
# E           E            E           E
#
#  F    F     FF    FF
# FF    FF    F      F
#
#   G   G     GGG   GGG
#   G   G     G       G
# GGG   GGG   G       G
#
#   H   H      HH   HH
#  HH   HH    HH     HH
# HH     HH   H       H
#
# I I   II    III   II
# III   I     I I    I
#       II          II
#
# J     JJJJ
# J
# J
# J
#
# KK
# KK
#
#  L
# LLL
#  L
#

# linear board
# 01234567890
# 12345678901
# 23456789012
# 34567890123
# 45678901234

#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <malloc.h>

#define MAXPIECE 12
#define MAXROT 8
#define MAXBEAD 5
#define MAXSTARTPOS 40
#define BOARDSIZE 55

/* this is the actual board, each cell holds a letter from the piece.
 * it is a linear (single dimention) array. Each row is appended to the previous.
 * this prevents multiply/divides and best of all,
 * allows a single number to describe a bead position rather than an ordered pair
 */
char board[BOARDSIZE];
int allsolutions; // if true keep searching for another solution
int verbose; // set to 1 to show progress

/*
# colors
# orange	A
# red	B
# blue	C
# pink	D
# green	E
# white	F
# lt blue	G
# hot pink	H
# yellow	I
# purple	J
# lt green	K
# gray	L
*/

char *getPrintablePiece(char p)
{
  static char str[32];
  //if (p < 'A') p += 'A';
  switch (p)
  {
  case 'A': snprintf(str, sizeof(str), "\033[38;5;208mA\033[m"); break; // orange
  case 'B': snprintf(str, sizeof(str), "\033[1;31mB\033[m"); break; // red
  case 'C': snprintf(str, sizeof(str), "\033[34mC\033[m"); break; // blue
  case 'D': snprintf(str, sizeof(str), "\033[38;5;210mD\033[m"); break; // pink
  case 'E': snprintf(str, sizeof(str), "\033[32mE\033[m"); break; // green
  case 'F': snprintf(str, sizeof(str), "\033[37mF\033[m"); break; // white
  case 'G': snprintf(str, sizeof(str), "\033[36mG\033[m"); break; // lt blue
  case 'H': snprintf(str, sizeof(str), "\033[38;5;160mH\033[m"); break; // hot pink
  case 'I': snprintf(str, sizeof(str), "\033[33mI\033[m"); break; // yellow
  case 'J': snprintf(str, sizeof(str), "\033[35mJ\033[m"); break; // purple
  case 'K': snprintf(str, sizeof(str), "\033[1;32mK\033[m"); break; // lt green
  case 'L': snprintf(str, sizeof(str), "\033[1;30;107mL\033[m"); break; // gray; white bg
  default:
  case '.': snprintf(str, sizeof(str), "."); break;
  }
  return str;
}

void printpiece(char p)
{
  printf("%s", getPrintablePiece(p));
}

// turns the linear board array into a two-dimentional board
void showboard()
{
int i;
  printf("|-----------|\n|");
  for (i = 0; i < BOARDSIZE; i++) {
    printpiece(board[i]);
    if ((i == 10)||(i == 21)||(i == 32)||(i == 43)||(i == 54)) printf("|\n|");
  }
  printf("-----------|\n\n");
}

// stores the solution by piece, rot, and pos
struct sol {
  int rot;
  int pos;
} solution[MAXPIECE];

#if show_piece_as_solving_obsolete
struct sol final[MAXPIECE] = {
{ 3, 0 },  // A
{ 3, 39 }, // B
{ 2, 11 }, // C
{ 3, 7 },  // D
{ 1, 36 }, // E
{ 3, 17 }, // F
{ 1, 23 }, // G
{ 2, 14 }, // H
{ 0, 19 }, // I
{ 1, 3 },  // J
{ 0, 42 }, // K
{ 0, 16 }  // L
};
#endif

// the linked list is used to skip pre-set pieces
struct ll {
  struct ll *next;
  int p;
} *root
#if show_piece_as_solving_obsolete
, *cp
#endif
;

// linear board
// 01234567890
// 12345678901
// 23456789012
// 34567890123
// 45678901234

// piece data (this took the most time) relization of the puzzle pieces as data stuctures
// number of beads per piece
const int numbeads[MAXPIECE] = { 4, 5, 5, 5, 5, 3, 5, 5, 5, 4, 4, 5 };
// number of rotations per piece
const int numrotations[MAXPIECE] = { 8, 8, 8, 8, 8, 4, 4, 4, 4, 2, 1, 1 };
// indexed by piece and rotation, this places each bead into the linear array and lists all possible starting posistions
// position 0 didn't need to be stored, but for 12 bytes it makes the coding easier
struct piece {
  int beadinc[MAXBEAD]; // relative offsets from first bead for linear storage
  int numpos; // number of start positions
  int startpos[MAXSTARTPOS]; // only valid start positions, none that fall off board or create non-fillable holes
} pieces[MAXPIECE][MAXROT] = {
{
//  A    A     AA    AAA   A     AAA   AA      A
//  A    AAA   A       A   A     A      A    AAA
// AA          A           AA           A
{ { 0, 11, 21, 22 }, 29, { 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 } },
{ { 0, 11, 12, 13 }, 35, { 0, 1, 2, 3, 4, 5, 6, 7, 11, 12, 13, 14, 15, 16, 17, 18, 19, 22, 23, 24, 25, 26, 27, 28, 29, 30, 33, 34, 35, 36, 37, 38, 39, 40, 41 } },
{ { 0, 1, 11, 22 }, 29, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 23, 24, 25, 26, 27, 28, 29, 30 } },
{ { 0, 1, 2, 13 }, 35, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19, 22, 23, 24, 25, 26, 27, 28, 29, 30, 34, 35, 36, 37, 38, 39, 40, 41 } },
{ { 0, 11, 22, 23 }, 30, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 } },
{ { 0, 1, 2, 11 }, 35, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19, 22, 23, 24, 25, 26, 27, 28, 29, 30, 33, 34, 35, 36, 37, 38, 39, 40 } },
{ { 0, 1, 12, 23 }, 29, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 23, 24, 25, 26, 27, 28, 29, 30, 31 } },
{ { 0, 9, 10, 11 }, 35, { 3, 4, 5, 6, 7, 8, 9, 10, 13, 14, 15, 16, 17, 18, 19, 20, 21, 24, 25, 26, 27, 28, 29, 30, 31, 32, 35, 36, 37, 38, 39, 40, 41, 42, 43 } }
},
{
//  B    BB    BB    BBB   B     BBB   BB     BB
// BB    BBB   BB     BB   BB    BB    BB    BBB
// BB          B           BB           B
{ { 0, 10, 11, 21, 22 }, 29, { 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 } },
{ { 0, 1, 11, 12, 13 }, 35, { 0, 1, 2, 3, 4, 5, 6, 7, 11, 12, 13, 14, 15, 16, 17, 18, 19, 22, 23, 24, 25, 26, 27, 28, 29, 30, 33, 34, 35, 36, 37, 38, 39, 40, 41 } },
{ { 0, 1, 11, 12, 22 }, 29, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 23, 24, 25, 26, 27, 28, 29, 30 } },
{ { 0, 1, 2, 12, 13 }, 35, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19, 22, 23, 24, 25, 26, 27, 28, 29, 30, 34, 35, 36, 37, 38, 39, 40, 41 } },
{ { 0, 11, 12, 22, 23 }, 29, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 } },
{ { 0, 1, 2, 11, 12 }, 35, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19, 22, 23, 24, 25, 26, 27, 28, 29, 30, 33, 34, 35, 36, 37, 38, 39, 40 } },
{ { 0, 1, 11, 12, 23 }, 29, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 23, 24, 25, 26, 27, 28, 29, 30, 31 } },
{ { 0, 1, 10, 11, 12 }, 35, { 2, 3, 4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 16, 17, 18, 19, 20, 23, 24, 25, 26, 27, 28, 29, 30, 31, 34, 35, 36, 37, 38, 39, 40, 41, 42 } }
},
{
//  C    C     CC    CCCC  C     CCCC  CC       C
//  C    CCCC  C        C  C     C      C    CCCC
//  C          C           C            C
// CC          C           CC           C
{ { 0, 11, 22, 32, 33 }, 19, { 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 } },
{ { 0, 11, 12, 13, 14 }, 32, { 0, 1, 2, 3, 4, 5, 6, 7, 11, 12, 13, 14, 15, 16, 17, 18, 22, 23, 24, 25, 26, 27, 28, 29, 33, 34, 35, 36, 37, 38, 39, 40 } },
{ { 0, 1, 11, 22, 33 }, 19, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19 } },
{ { 0, 1, 2, 3, 14 }, 31, { 0, 1, 2, 3, 4, 5, 6, 7, 11, 12, 13, 14, 15, 16, 17, 18, 22, 23, 24, 25, 26, 27, 28, 29, 34, 35, 36, 37, 38, 39, 40 } },
{ { 0, 11, 22, 33, 34 }, 19, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 } },
{ { 0, 1, 2, 3, 11 }, 31, { 0, 1, 2, 3, 4, 5, 6, 7, 11, 12, 13, 14, 15, 16, 17, 18, 22, 23, 24, 25, 26, 27, 28, 29, 33, 34, 35, 36, 37, 38, 39 } },
{ { 0, 1, 12, 23, 34 }, 19, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 16, 17, 18, 19, 20 } },
{ { 0, 8, 9, 10, 11 }, 31, { 4, 5, 6, 7, 8, 9, 10, 14, 15, 16, 17, 18, 19, 20, 21, 25, 26, 27, 28, 29, 30, 31, 32, 36, 37, 38, 39, 40, 41, 42, 43 } }
},
{
//  D     D    D     DDDD  D     DDDD   D      D
//  D    DDDD  DD      D   D      D    DD    DDDD
// DD          D           DD           D
//  D          D           D            D
{ { 0, 11, 21, 22, 33 }, 19, { 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 } },
{ { 0, 10, 11, 12, 13 }, 30, { 2, 3, 4, 5, 6, 7, 12, 13, 14, 15, 16, 17, 18, 19, 23, 24, 25, 26, 27, 28, 29, 30, 34, 35, 36, 37, 38, 39, 40, 41 } },
{ { 0, 11, 12, 22, 33 }, 19, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 } },
{ { 0, 1, 2, 3, 13 }, 31, { 0, 1, 2, 3, 4, 5, 6, 7, 11, 12, 13, 14, 15, 16, 17, 18, 22, 23, 24, 25, 26, 27, 28, 29, 34, 35, 36, 37, 38, 39, 40 } },
{ { 0, 11, 22, 23, 33 }, 17, { 0, 1, 2, 3, 4, 5, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19 } },
{ { 0, 1, 2, 3, 12 }, 31, { 0, 1, 2, 3, 4, 5, 6, 7, 11, 12, 13, 14, 15, 16, 17, 18, 22, 23, 24, 25, 26, 27, 28, 29, 34, 35, 36, 37, 38, 39, 49 } },
{ { 0, 10, 11, 22, 33 }, 19, { 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 } },
{ { 0, 9, 10, 11, 12 }, 31, { 3, 4, 5, 6, 7, 8, 9, 13, 14, 15, 16, 17, 18, 19, 20, 24, 25, 26, 27, 28, 29, 30, 31, 35, 36, 37, 38, 39, 40, 41, 42 } }
},
{
//  E    EE     E    EEE   E      EEE  E       EE
//  E     EEE  EE      EE  E     EE    EE    EEE
// EE          E           EE           E
// E           E            E           E
{ { 0, 11, 21, 22, 32 }, 18, { 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20 } },
{ { 0, 1, 12, 13, 14 }, 30, { 0, 1, 2, 3, 4, 5, 6, 11, 12, 13, 14, 15, 16, 17, 18, 22, 23, 24, 25, 26, 27, 28, 29, 34, 35, 36, 37, 38, 39, 40 } },
{ { 0, 10, 11, 21, 32 }, 19, { 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 } },
{ { 0, 1, 2, 13, 14 }, 30, { 0, 1, 2, 3, 4, 5, 6, 11, 12, 13, 14, 15, 16, 17, 18, 22, 23, 24, 25, 26, 27, 28, 29, 34, 35, 36, 37, 38, 39, 40 } },
{ { 0, 11, 22, 23, 34 }, 18, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 13, 14, 15, 16, 17, 18, 19, 20 } },
{ { 0, 1, 2, 10, 11 }, 30, { 2, 3, 4, 5, 6, 7, 8, 12, 13, 14, 15, 16, 17, 18, 19, 23, 24, 25, 26, 27, 28, 29, 30, 34, 35, 36, 37, 38, 39, 40 } },
{ { 0, 11, 12, 23, 34 }, 18, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 13, 14, 15, 16, 17, 18, 19, 20 } },
{ { 0, 1, 9, 10, 11 }, 30, { 3, 4, 5, 6, 7, 8, 9, 13, 14, 15, 16, 17, 18, 19, 20, 24, 25, 26, 27, 28, 29, 30, 31, 35, 36, 37, 38, 39, 40, 41 } }
},
{
//  F    F     FF    FF
// FF    FF    F      F
{ { 0, 10, 11 }, 39, { 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43 } },
{ { 0, 11, 12 }, 39, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42 } },
{ { 0, 1, 11 }, 39, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 33, 34, 35, 36, 37, 38, 39, 40, 41 } },
{ { 0, 1, 12 }, 39, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 34, 35, 36, 37, 38, 39, 40, 41, 42 } }
},
{
//   G   G     GGG   GGG
//   G   G     G       G
// GGG   GGG   G       G
{ { 0, 11, 20, 21, 22 }, 27, { 2, 3, 4, 5, 6, 7, 8, 9, 10, 13, 14, 15, 16, 17, 18, 19, 20, 21, 24, 25, 26, 27, 28, 29, 30, 31, 32 } },
{ { 0, 11, 22, 23, 24 }, 27, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19, 22, 23, 24, 25, 26, 27, 28, 29, 30 } },
{ { 0, 1, 2, 11, 22 }, 27, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19, 22, 23, 24, 25, 26, 27, 28, 29, 30 } },
{ { 0, 1, 2, 13, 24 }, 27, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19, 22, 23, 24, 25, 26, 27, 28, 29, 30 } }
},
{
//   H   H      HH   HH
//  HH   HH    HH     HH
// HH     HH   H       H
{ { 0, 10, 11, 20, 21 }, 22, { 2, 3, 4, 5, 6, 7, 8, 9, 10, 13, 14, 15, 16, 17, 18, 19, 20, 21, 24, 25, 26, 27 } },
{ { 0, 11, 12, 23, 24 }, 26, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19, 23, 24, 25, 26, 27, 28, 29, 30 } },
{ { 0, 1, 10, 11, 21 }, 26, { 2, 3, 4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 16, 17, 18, 19, 20, 23, 24, 25, 26, 27, 28, 29, 30, 31 } },
{ { 0, 1, 12, 13, 24 }, 26, { 0, 1, 2, 3, 4, 5, 6, 7, 11, 12, 13, 14, 15, 16, 17, 18, 19, 22, 23, 24, 25, 26, 27, 28, 29, 30 } }
},
{
// I I   II    III   II
// III   I     I I    I
//       II          II
{ { 0, 2, 11, 12, 13 }, 27, { 11, 12, 13, 14, 15, 16, 17, 18, 19, 22, 23, 24, 25, 26, 27, 28, 29, 30, 33, 34, 35, 36, 37, 38, 39, 40, 41 } },
{ { 0, 1, 11, 22, 23 }, 27, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19, 22, 23, 24, 25, 26, 27, 28, 29, 30 } },
{ { 0, 1, 2, 11, 13 }, 27, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18, 19, 22, 23, 24, 25, 26, 27, 28, 29, 30 } },
{ { 0, 1, 12, 22, 23 }, 27, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 16, 17, 18, 19, 20, 23, 24, 25, 26, 27, 28, 29, 30, 31 } }
},
{
// J     JJJJ
// J
// J
// J
{ { 0, 11, 22, 33 }, 22, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 } },
{ { 0, 1, 2, 3 }, 40, { 0, 1, 2, 3, 4, 5, 6, 7, 11, 12, 13, 14, 15, 16, 17, 18, 22, 23, 24, 25, 26, 27, 28, 29, 33, 34, 35, 36, 37, 38, 39, 40, 44, 45, 46, 47, 48, 49, 50, 51 } }
},
{
// KK
// KK
{ { 0, 1, 11, 12 }, 40, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42 } }
},
{
//  L
// LLL
//  L
{ { 0, 10, 11, 12, 22 }, 23, { 2, 3, 4, 5, 6, 7, 8, 12, 13, 14, 15, 16, 17, 18, 19, 20, 24, 25, 26, 27, 28, 29, 30 } }
}

};

// tell the user how to start
void usage(char *prog, int full)
{
  printf("usage: %s [-a] [P,rot,pos] ...\n or %s -? for full help\n\nwhere:\n\tP is a piece A,B,C,D,E,F,G,H,I,J,K,L\n\trot is the rotation\n\tpos is the starting postion (0 based, lnear top left to bottom right)\n\t-a continue searching for all solutions\n", prog, prog);
  if (full) {
    printf("\tThe following are valid rotations:\n\n");
    printf(" 0     1     2     3     4     5     6     7\n");
    printf(" ===   ===   ===   ===   ===   ===   ===   ===\n");
    printf("\033[38;5;208m");
    printf("  A    A     AA    AAA   A     AAA   AA      A\n");
    printf("  A    AAA   A       A   A     A      A    AAA\n");
    printf(" AA          A           AA           A\n");
    printf("\033[m");
    printf("\n");
    printf("\033[1;31m");
    printf("  B    BB    BB    BBB   B     BBB   BB     BB\n");
    printf(" BB    BBB   BB     BB   BB    BB    BB    BBB\n");
    printf(" BB          B           BB           B\n");
    printf("\033[m");
    printf("\n");
    printf("\033[34m");
    printf("  C    C     CC    CCCC  C     CCCC  CC       C\n");
    printf("  C    CCCC  C        C  C     C      C    CCCC\n");
    printf("  C          C           C            C\n");
    printf(" CC          C           CC           C\n");
    printf("\033[m");
    printf("  \n");
    printf("\033[38;5;210m");
    printf("  D     D    D     DDDD  D     DDDD   D      D\n");
    printf("  D    DDDD  DD      D   D      D    DD    DDDD\n");
    printf(" DD          D           DD           D\n");
    printf("  D          D           D            D\n");
    printf("\033[m");
    printf("\n");
    printf("\033[32m");
    printf("  E    EE     E    EEE   E      EEE  E       EE\n");
    printf("  E     EEE  EE      EE  E     EE    EE    EEE\n");
    printf(" EE          E           EE           E\n");
    printf(" E           E            E           E\n");
    printf("\033[m");
    printf("\n");
    printf(" 0     1     2     3          ");
    printf(" 0     1\n");
    printf(" ===   ===   ===   ===         ===   ===\n");
    printf("\033[37m");
    printf("  F    F     FF    FF         ");
    printf("\033[m\033[35m");
    printf(" J     JJJJ\n");
    printf("\033[m\033[37m");
    printf(" FF    FF    F      F         ");
    printf("\033[m\033[35m");
    printf(" J\n");
    printf("                              ");
    printf(" J\n");
    printf("\033[m\033[36m");
    printf("   G   G     GGG   GGG        ");
    printf("\033[m\033[35m");
    printf(" J\n");
    printf("\033[m\033[36m");
    printf("   G   G     G       G        ");
    printf("\n");
    printf(" GGG   GGG   G       G        ");
    printf("\033[m");
    printf(" 0\n");
    printf("                              ");
    printf(" ===\n");
    printf("\033[38;5;160m");
    printf("   H   H      HH   HH         ");
    printf("\033[m\033[1;32m");
    printf(" KK\n");
    printf("\033[m\033[38;5;160m");
    printf("  HH   HH    HH     HH        ");
    printf("\033[m\033[1;32m");
    printf(" KK\n");
    printf("\033[m\033[38;5;160m");
    printf(" HH     HH   H       H        ");
    printf("\033[m");
    printf("\n");
    printf("                              ");
    printf("\n");
    printf("\033[33m");
    printf(" I I   II    III   II           ");
    printf("\033[m\033[1;30;107m");
    printf("L");
    printf("\033[m\033[33m");
    printf("\n");
    printf(" III   I     I I    I          ");
    printf("\033[m\033[1;30;107m");
    printf("LLL");
    printf("\033[m");
    printf("\n");
    printf("\033[33m");
    printf("       II          II           ");
    printf("\033[m\033[1;30;107m");
    printf("L");
    printf("\033[m");
    printf("\n\n");
  }
  exit(1);
}

// used during piece data entry ... (debug only)
void dumpdata()
{
int p, rot, i;

  for (p = 0; p < MAXPIECE; p++)
  {
    printf("piece %d numbeads = %d, numrot %d\n", p, numbeads[p], numrotations[p]);
    for (rot = 0; rot < numrotations[p]; rot++)
    {
      printf("rot %d, beadinc = { 0", rot);
      for (i = 1; i < numbeads[p]; i++)
        printf(", %d", pieces[p][rot].beadinc[i]);
      printf(" }, numpos = %d, startpos = { %d", pieces[p][rot].numpos, pieces[p][rot].startpos[0]);
      for (i = 1; i < MAXSTARTPOS && pieces[p][rot].startpos[i]; i++)
        printf(", %d", pieces[p][rot].startpos[i]);
      printf(" }, %saudit %d=?%d\033[m\n", (i==pieces[p][rot].numpos?"":"\033[7m"), i, pieces[p][rot].numpos);
    }
    printf("================\n");
  }
}

// add a piece to the board, also stores piece into solution array
int addpiece(int p, int rot, int pos)
{
  char pl;
  int i, b;

  solution[p].rot = rot; // we never need to delete from this
  solution[p].pos = pos;

  pl = 'A' + p;
  // 2 pass, don't add any if there is a collision
  for (i = 0; i < numbeads[p]; i++) {
    b = pos + pieces[p][rot].beadinc[i];
    if (board[b] > 0) {
      //printf("overlap @ %d for piece %d inc=%d\n", b, p, pieces[p][rot].beadinc[i]);
      return 1;
    }
  }
  for (i = 0; i < numbeads[p]; i++) {
    b = pos + pieces[p][rot].beadinc[i];
    board[b] = pl;
  }
#if show_piece_as_solving_obsolete
  if (cp && cp->p == p) {
    if ((rot == final[p].rot) && (pos == final[p].pos)) {
      printf("piece %c %d @ %d\n", 'A'+p, rot, pos);
      showboard();
      cp = cp->next;
    }
  }
#endif
  return 0;
}

// removes a previosly placed piece from the board
int delpiece(int p, int rot, int pos)
{
  char pl;
  int i, b;

  pl = 'A' + p;
  for (i = 0; i < numbeads[p]; i++) {
    b = pos + pieces[p][rot].beadinc[i];
    if (board[b] != pl) { // call me paranoid!!
      printf("missing @ %d for piece %s inc=%d\n", b, getPrintablePiece(pl), pieces[p][rot].beadinc[i]);
      //return 1;
    }
    board[b] = 0;
  }
  return(0);
}

// another debug routine, show each piece (and rotation and starting position) individually on the board
// also for unit testing addpiece/delpiece and showboard
void demo()
{
int p, rot, pos;

  for (p = 0; p < MAXPIECE; p++) {
    for (rot = 0; rot < numrotations[p]; rot++) {
      for (pos = 0; pos < pieces[p][rot].numpos; pos++) {
        addpiece(p, rot, pieces[p][rot].startpos[pos]);
        showboard();
usleep(100000);
        delpiece(p, rot, pieces[p][rot].startpos[pos]);
      }
    }
  }
}

// puts each piece into the linked list
void initlist()
{
  int p;
  struct ll *llp;
  root = (struct ll *) malloc(sizeof(struct ll));
  root->p = -1;
  llp = root;
  for (p = 0; p < MAXPIECE; p++) {
    llp->next = (struct ll *) malloc(sizeof(struct ll));
    llp = llp->next;
    llp->p = p;
  }
  llp->next = 0;
}

// remove a piece from the linked list (because it's being placed on the board as an initial condition)
void llremovep(int p)
{
  struct ll *llp, *last;
  last = root;
  llp = root->next;
  while (llp) {
    if (llp->p == p) {
      last->next = llp->next;
      free(llp);
      return;
    }
    last = llp;
    llp = llp->next;
  }
  printf("%d not found!\n", p);
}

// lists the pieces in the linked list
void printll()
{
  struct ll *llp;

  llp = root->next;
  while (llp) {
    printf(" ");
    printpiece('A' + llp->p);
    llp = llp->next;
  }
  printf("\n");
}

// validates user supplied data then
// places pieces onto the board as a starting point
// and removes the piece from the linked list so we dont try to place it twice
void parse(int argc, char **argv)
{
#if 1
int i, p, pos, rot;
char l;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == 'a') {
        allsolutions = 1;
        continue;
      }
      if (argv[i][1] == 'v') {
        verbose = 1;
        continue;
      } else {
        usage(argv[0], 1);
      }
    }
    if (sscanf(argv[i], "%c,%d,%d", &l, &rot, &pos) != 3) {
      printf("unable to parse %s\n", argv[i]);
      usage(argv[0], 0);
    }
    p = (l | 0x20) - 'a';
    if (p < 0 || p >= MAXPIECE) {
      printf("piece must be A-L\n");
      usage(argv[0], 0);
    }
    if (rot < 0 || rot >= numrotations[p]) {
      printf("piece %c rotation out of range, must be 0-%d\n", 'A'+p, numrotations[p]-1);
      usage(argv[0], 0);
    }
    if (pos < 0 || pos >= BOARDSIZE) {
      printf("piece %c position off board, must be 0-%d\n", 'A'+p, BOARDSIZE-1);
      usage(argv[0], 0);
    }
    if (addpiece(p, rot, pos) != 0) {
      printf("failed to add piece %c %d @ %d\n", 'A'+p, rot, pos);
      usage(argv[0], 0);
    }
    llremovep(p);
  }
#else // a,3,0 j,1,3 d,3,7 (starting point during debug)
  addpiece(0, 3, 0);
  llremovep(0);
  addpiece(9, 1, 3);
  llremovep(9);
  addpiece(3, 3, 7);
  llremovep(3);
#endif
#if show_piece_as_solving_obsolete
  cp = root->next;
#endif
}

// as described
int findfirsthole()
{
  int p;
  for (p = 0; p < BOARDSIZE; p++) {
    if (board[p] == 0) {
      return p;
    }
  }
  return -1;
}

int issingle(int b) {
    int up = (b >= 11) ? (board[b-11] != 0) : 1;
    int down = (b <= 43) ? (board[b+11] != 0) : 1;
    int left = (b % 11 != 0) ? (board[b-1] != 0) : 1;
    int right = (b % 11 != 10) ? (board[b+1] != 0) : 1;

    // If all 4 adjacent sides are blocked/filled, this hole is a "Single"
    return (up && down && left && right);
}

int cantfill_global() {
    for (int i = 0; i < BOARDSIZE; i++) {
        if (board[i] == 0) {
            if (issingle(i)) return 1; // Found an unfillable hole
        }
    }
    return 0;
}

// Helper for the flood fill
int count_region(char *temp_board, int b) {
    if (b < 0 || b >= BOARDSIZE || temp_board[b] != 0) return 0;
    
    temp_board[b] = -1; // Mark as visited
    int count = 1;

    // Check adjacent cells (with boundary awareness)
    if (b >= 11) count += count_region(temp_board, b - 11); // Up
    if (b <= 43) count += count_region(temp_board, b + 11); // Down
    if (b % 11 != 0) count += count_region(temp_board, b - 1); // Left
    if (b % 11 != 10) count += count_region(temp_board, b + 1); // Right

    return count;
}

// Optimized pruning check
int cantfill_advanced() {
    char temp_board[BOARDSIZE];
    for (int i = 0; i < BOARDSIZE; i++) temp_board[i] = board[i];

    for (int i = 0; i < BOARDSIZE; i++) {
        if (temp_board[i] == 0) {
            // Find the size of this specific empty pocket
            int size = count_region(temp_board, i);
            if (size < 3) return 1; // Any pocket < 3 beads is a fail
        }
    }
    return 0;
}

// print solution as piece, rotation, startpos
void showsol()
{
  int p;
  for (p = 0; p < MAXPIECE; p++) {
    if (p > 0) {
      printf(" ");
    }
    printpiece('A'+p);
    printf(",%d,%d", solution[p].rot, solution[p].pos);
  }
  printf("\n");
}

// recursive function, calls itself for each peice in the linked list
int solve(struct ll *llp_ignored) // We no longer strictly follow the LL order
{
    int hole, p, rot, sp;
    struct ll *prev, *curr;

    // 1. Find the VERY FIRST empty spot on the board
    hole = findfirsthole();

    // If no holes are left, we have a complete solution!
    if (hole == -1) {
        if (verbose) printf("\n\n\n\n\n\n\n\n");
        showboard();
        showsol();
        return 0; 
    }

    // 2. Try every piece currently in our "available" linked list
    prev = root;
    curr = root->next;

    while (curr) {
        p = curr->p;

        // 3. For this piece, try every rotation
        for (rot = 0; rot < numrotations[p]; rot++) {
            
            // 4. Try every startpos, but ONLY those that actually cover our 'hole'
            // Optimization: A piece only needs to check startpos 
            // where (pos + beadinc[i] == hole) for any bead i.
            for (sp = 0; sp < pieces[p][rot].numpos; sp++) {
                int pos = pieces[p][rot].startpos[sp];
                int covers_hole = 0;

                // Check if any bead in this placement lands on our target hole
                for (int b_idx = 0; b_idx < numbeads[p]; b_idx++) {
                    if (pos + pieces[p][rot].beadinc[b_idx] == hole) {
                        covers_hole = 1;
                        break;
                    }
                }

                if (!covers_hole) continue;

                // 5. Attempt to add the piece
                if (addpiece(p, rot, pos) == 0) {
                    if (verbose) {
                        showboard();
                        showsol();
                    }

                    // Pruning: Check if we just created an unreachable 1-bead island
                    if (cantfill_advanced() == 0) {
                        
                        // Pull piece from available list and recurse
                        prev->next = curr->next; 
                        if (solve(NULL) == 0) { // NULL because we find 'hole' inside
                            if (allsolutions == 0) return 0;
                        }
                        // Backtrack: put piece back in list
                        prev->next = curr; 
                    }
                    
                    delpiece(p, rot, pos);
                }
            }
        }
        prev = curr;
        curr = curr->next;
    }

    return 1; // No piece could fill this hole; backtrack
}

int main(int argc, char **argv)
{
  //dumpdata();
  //printf("\n\n\n\n\n\n\n\n\n\n\n");
  //demo();
  initlist();
  parse(argc, argv);
  printll();
  showboard();
  if (solve(root->next) == 1) {
    if (verbose)
      printf("\n\n\n\n\n\n\n\n");
    printf("no solution\n");
  }
}
