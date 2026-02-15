#ifndef HAKA_VECTOR_H_
#define HAKA_VECTOR_H_

#include <stdlib.h>
#include <string.h>

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
  do {                \
    if (v != NULL) {  \
      free(v->arr);   \
      free(v);        \
      v = NULL;       \
    }                 \
  } while (0)

#define DeepFreeVector(v)            \
  do {                               \
    if (v != NULL) {                 \
      while (v->size) {              \
        if (v->arr[v->size] != NULL) \
          free(v->arr[v->size]);     \
        v->size--;                   \
      }                              \
      free(v->arr);                  \
      free(v);                       \
      v = NULL;                      \
    }                                \
  } while (0)

#define VectorPush(v, c)                                          \
  do {                                                            \
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
    }                                                             \
  } while (0)

#define VectorPushStr(v, c)     \
  do {                          \
    if (v != NULL) {            \
      char* c_copy = strdup(c); \
      VectorPush(v, c_copy);    \
    }                           \
  } while (0)

#define VectorPushStrArr(v, arr)                                   \
  do {                                                             \
    for (int i = 0; i < (int)(sizeof(arr) / sizeof(char*)); i++) { \
      VectorPushStr(v, arr[i]);                                    \
    }                                                              \
  } while (0)

#define VectorFind(v, c, exists)          \
  do {                                    \
    if (v != NULL) {                      \
      exists = false;                     \
      for (int i = 0; i < v->size; i++) { \
        if (v->arr[i] == c) {             \
          exists = true;                  \
          break;                          \
        }                                 \
      }                                   \
    }                                     \
  } while (0)

#define VectorCopy(vDest, vSrc) memcpy(vDest, vSrc, sizeof(*vSrc));

#define ForEach(v, c) for (int i = 0; i < v->size && (c = v->arr[i]); i++)

#endif
