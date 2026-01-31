#ifndef HAKA_BIND_H_
#define HAKA_BIND_H_

#include "haka.h"

// clang-format off
#define OK      0
#define RELOAD  1
#define FAIL   -1
// clang-format on

typedef void (*hakaHook_t)(struct hakaContext*);

struct keyBinding {
  struct IntSet* keys;
  hakaHook_t func;
};

struct keyBindings {
  int size;
  int capacity;
  struct keyBinding* kbind;
};

typedef int (*pluginInit_t)(struct coreApi*, struct keyBindings*);

struct keyBindings* initKeyBindings(int size);
void freeKeyBindings(struct keyBindings** kbinds);
void addKeyBind(struct keyBindings* kbinds,
                void (*func)(struct hakaContext*),
                int keyToBind,
                ...);
void pushKeyBind(struct keyBindings* kbinds, struct keyBinding* kbind);
int executeKeyBind(struct keyBindings* kbinds,
                   struct keyState* ks,
                   struct hakaContext* haka);
void loadBindings(struct hakaContext* haka,
                  struct keyBindings** kbinds,
                  struct keyState* ks,
                  struct coreApi* api,
                  PluginVector** plugins);

void freePlugins(PluginVector** plugins);

#endif
