#ifndef _PLUG_H
#define _PLUG_H

#include <linux/input-event-codes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define HAKA_ABI_VERSION 0x1

extern struct coreApi* api;

struct hakaContext;
struct keyBindings;
struct keyState;

// clang-format off
struct coreApi {
  int ver;

  void   (*addKeyBind)(struct keyBindings* kbinds,
                void (*func)(struct hakaContext*),
                int keyToBind,
                ...);

  void   (*switchFile)(struct hakaContext *);
  void   (*getPrimarySelection)(struct hakaContext*, FILE**);
  int    (*openNotesFile)(struct hakaContext*);
  size_t (*writeFP2FD)(struct hakaContext*);
  int    (*closeNotesFile)(struct hakaContext*);
  void   (*writeTextToFile)(struct hakaContext *,
                            char *prefix, char *suffix);
  void   (*writeSelectionToFile)(struct hakaContext *);


  void   (*openFile)(struct hakaContext *);

  void   (*sendTextToFile)(struct hakaContext *,
                           char *text);
  void   (*triggerTofi)(struct hakaContext *, FILE**);
};
// clang-format on

int hakaPluginInit(struct coreApi* capi, struct keyBindings* kbinds);

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

#define Bind(func, ...) api->addKeyBind(kbinds, func, __VA_ARGS__, 0)

#define BEGIN_BIND                                                           \
  int hakaPluginInit(struct coreApi* capi, struct keyBindings* kbinds) {     \
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

#endif  // !_PLUG_H
