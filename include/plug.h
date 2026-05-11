#ifndef HAKA_PLUG_H_
#define HAKA_PLUG_H_

#include <linux/input-event-codes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// TODO: This dep kills me. How do we eliminate this
#include "vector.h"

#define HAKA_ABI_VERSION 0x1
#define BUFSIZE 1024

// clang-format off
typedef struct hakaCtx     hakaCtx;
typedef struct keyState    keyState;
typedef struct keyBindings keyBindings;

typedef struct coreApi {
  int ver;

  void   (*addKeyBind)(keyBindings* kbinds,
                void (*func)(hakaCtx*),
                int keyToBind,
                ...);

  bool   (*keyIsActive)(hakaCtx*, int keyCode);

  void   (*getFile)(hakaCtx*, char fileName[BUFSIZE * 2]);
  void   (*getTerminal)(hakaCtx*, CharVector* v);
  void   (*getEditor)(hakaCtx*, CharVector* v);

  void   (*spawnChild)(hakaCtx*, char *argv[]);
  void   (*spawnChildVec)(hakaCtx*, CharVector*);
  void   (*switchFileTemp)(hakaCtx* ctx, char* filename);
  void   (*switchFileRestore)(hakaCtx* ctx);
  void   (*switchFile)(hakaCtx*);
  void   (*getPrimarySelection)(hakaCtx*, FILE**);
  void   (*displayFile)(hakaCtx*);
  size_t (*writeFP2FD)(hakaCtx*);
  int    (*closeFile)(hakaCtx*);
  void   (*appendPadSelToFile)(hakaCtx*,
                               char *prefix, char *suffix);
  void   (*appendCboardToFile)(hakaCtx *);
  void   (*appendSelToFile)(hakaCtx *);


  void   (*openFile)(hakaCtx*);

  void   (*appendTextToFile)(hakaCtx*, char *text);
  void   (*triggerTofi)(hakaCtx*, FILE**);
} coreApi;
// clang-format on

extern coreApi* api;

int hakaPluginInit(coreApi* capi, keyBindings* kbinds);

#define Validate(capi, kbinds)                             \
  do {                                                     \
    unsigned e = ((capi == NULL) << 1) | (kbinds == NULL); \
    if (e & 0x2) {                                         \
      fprintf(stderr, "Core API ptr is NULL\n");           \
    }                                                      \
    if (e & 0x1) {                                         \
      fprintf(stderr, "Key binds ptr is NULL\n");          \
    }                                                      \
    if (e) {                                               \
      return -1;                                           \
    }                                                      \
  } while (0)

// clang-format off
#undef Bind
#define Bind(func, ...) api->addKeyBind(kbinds, func, __VA_ARGS__, 0)
// clang-format on

#define BEGIN_BIND                                                           \
  int hakaPluginInit(coreApi* capi, keyBindings* kbinds) {                   \
    Validate(capi, kbinds);                                                  \
    api = capi;                                                              \
    if (api->ver != HAKA_ABI_VERSION) {                                      \
      fprintf(stderr, "ABI MISMATCH, plugin expects %d, got %d\n", api->ver, \
              HAKA_ABI_VERSION);                                             \
      return 1;                                                              \
    }

#define END_BIND \
  return 0;      \
  }

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

#endif  // !HAKA_PLUG_H_
