#ifndef HAKA_BIND_H_
#define HAKA_BIND_H_

#include "haka.h"

// clang-format off
#define OK      0
#define RELOAD  1
#define FAIL   -1
// clang-format on

typedef void (*hakaHook_t)(hakaCtx*);

typedef struct keyBinding {
  IntSet* keys;
  hakaHook_t func;
} keyBinding;

typedef struct keyBindings {
  int size;
  int capacity;
  keyBinding* kbind;
} keyBindings;

typedef int (*pluginInit_t)(struct coreApi*, keyBindings*);

keyBindings* initKeyBindings(int size);
void freeKeyBindings(keyBindings** kbinds);
void addKeyBind(keyBindings* kbinds,
                void (*func)(hakaCtx*),
                int keyToBind,
                ...);
void pushKeyBind(keyBindings* kbinds, keyBinding* kbind);
int executeKeyBind(keyBindings* kbinds, keyState* ks, hakaCtx* haka);
void loadBindings(hakaCtx* haka,
                  keyBindings** kbinds,
                  keyState* ks,
                  coreApi* api,
                  PluginVector** plugins);

void freePlugins(PluginVector** plugins);

#endif
