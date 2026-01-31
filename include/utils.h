#ifndef HAKA_UTILS_H
#define HAKA_UTILS_H

#include <grp.h>
#include <stdio.h>
#include <string.h>

#include <libevdev/libevdev.h>

struct DbVoidPtrVector {
  int size;
  int capacity;
  void** arr;
};

typedef struct DbVoidPtrVector PluginVector;
typedef struct DbVoidPtrVector CharVector;

#define MakeVector(vType, v)       \
  do {                             \
    v = malloc(sizeof(vType));     \
    if (v != NULL) {               \
      memset(v, 0, sizeof(vType)); \
    }                              \
  } while (0)

#define FreeVector(v) \
  if (v != NULL) {    \
    free(v->arr);     \
    free(v);          \
    v = NULL;         \
  }

#define DeepFreeVector(v)      \
  if (v != NULL) {             \
    while (v->size) {          \
      free(v->arr[--v->size]); \
    }                          \
    free(v->arr);              \
    free(v);                   \
    v = NULL;                  \
  }

#define VectorPush(v, c)                                        \
  if (v != NULL) {                                              \
    if (v->size == v->capacity) {                               \
      v->capacity = v->capacity == 0 ? 1 : v->capacity;         \
      void* newArr = malloc(sizeof(*v->arr) * 2 * v->capacity); \
      if (v->arr) {                                             \
        memcpy(newArr, v->arr, sizeof(*v->arr) * v->size);      \
        free(v->arr);                                           \
      }                                                         \
      v->arr = newArr;                                          \
      v->capacity *= 2;                                         \
    }                                                           \
    v->arr[v->size++] = c;                                      \
  }

#define VectorFind(v, c, exists)        \
  if (v != NULL) {                      \
    exists = false;                     \
    for (int i = 0; i < v->size; i++) { \
      if (v->arr[i] == c) {             \
        exists = true;                  \
        break;                          \
      }                                 \
    }                                   \
  }

#define VectorCopy(vDest, vSrc) memcpy(vDest, vSrc, sizeof(*vSrc));

#define ForEach(v, c) for (int i = 0; i < v->size && (c = v->arr[i]); i++)

struct IntSet {
  int* set;
  int size;
  int capacity;
};

struct IntSet* initIntSet(int capacity);
void freeIntSet(struct IntSet** set);
int pushIntSet(struct IntSet* set, int val);
int dynamicInc(struct IntSet* set);

int checkPackage(const char* pkgName);
void forceSudo();
char* getEnvVar(const char* var);

void switchGrp(gid_t* curGID, const char* grpnam);

int getKbdEvents(struct IntSet* set);
int openKbdDevices(struct IntSet* set, int* fds, struct libevdev* devs[]);

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
