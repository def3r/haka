#ifndef HAKA_UTILS_H
#define HAKA_UTILS_H

#include <grp.h>
#include <stdio.h>
#include <string.h>

#include <libevdev/libevdev.h>

#include "vector.h"

typedef struct IntSet {
  int* set;
  int size;
  int capacity;
} IntSet;

IntSet* initIntSet(int capacity);
void freeIntSet(IntSet** set);
int pushIntSet(IntSet* set, int val);
int dynamicInc(IntSet* set);

int checkPackage(const char* pkgName);
void forceSudo();
char* getEnvVar(const char* var);

void switchGrp(gid_t* curGID, const char* grpnam);

int getKbdEvents(IntSet* set);
int openKbdDevices(IntSet* set, int* fds, struct libevdev* devs[]);

char* expandValidDir(char* val);

char* ltrim(char* s);
char* rtrim(char* s);
char* trim(char* s);

#define Fprintln(buf, ...)   \
  fprintf(buf, __VA_ARGS__); \
  fprintf(buf, "\n")

#define Println(...)   \
  printf(__VA_ARGS__); \
  printf("\n")

// clang-format off
#if defined(LOG) && LOG == 2
#  define DLOG(...)       \
     printf(__VA_ARGS__); \
     printf("\n")
#else
#  define DLOG(...)
#endif

#if defined(LOG) && LOG >= 1
#  define ILOG(...)       \
     printf(__VA_ARGS__); \
     printf("\n")
#else
#  define ILOG(...)
#endif
// clang-format on

#endif  // !HAKA_UTILS_H
