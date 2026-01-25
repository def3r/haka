#ifndef HAKA_EVENT_HANDLER_H
#define HAKA_EVENT_HANDLER_H

#include <stdio.h>

#include "core.h"
#include "haka.h"
#include "plug.h"
#include "utils.h"

// clang-format off
#define OK      0
#define RELOAD  1
#define FAIL   -1

typedef int  (*pluginInit_t)(const struct coreApi*, const struct keyBindings*);
typedef void (*hakaHook_t)(struct hakaContext*);
// clang-format on

struct keyBinding {
  struct IntSet* keys;
  hakaHook_t func;
};

struct keyBindings {
  int size;
  int capacity;
  struct keyBinding* kbind;
};

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
void loadBindings(struct keyBindings** kbinds,
                  struct keyState* ks,
                  struct coreApi* api,
                  struct PluginVector** plugins);

// Event Handler Declarations
void switchFile(struct hakaContext* haka);
void writeSelectionToFile(struct hakaContext* haka);
void writePointToFile(struct hakaContext* haka);
void writeSubPointToFile(struct hakaContext* haka);
void openFile(struct hakaContext* haka);
void sendNewlineToFile(struct hakaContext* haka);
void sendTextToFile(struct hakaContext* haka, char* text);
void writeTextToFile(struct hakaContext* haka, char* prefix, char* suffix);

// Event Handler Helper Functions
void getPrimarySelection(struct hakaContext* haka, FILE** fp);
void getNotesFile(struct hakaContext* haka, char fileName[BUFSIZE * 2]);
int openNotesFile(struct hakaContext* haka);
void spawnChild(struct hakaContext*, char* argv[]);
int closeNotesFile(struct hakaContext* haka);
size_t writeFP2FD(struct hakaContext* haka);
void triggerTofi(struct hakaContext* haka, FILE** fp);

#undef Bind
#define Bind(func, ...) addKeyBind(kbinds, func, __VA_ARGS__, 0)

#define updatePrevFile(haka)                                                   \
  haka->fdPrevFile = open(haka->prevFile, O_TRUNC | O_CREAT | O_WRONLY, 0666); \
  if (haka->fdPrevFile > 0) {                                                  \
    write(haka->fdPrevFile, haka->notesFileName, strlen(haka->notesFileName)); \
  }                                                                            \
  close(haka->fdPrevFile);

#define eventHandlerEpilogue(haka) \
  if (haka != NULL) {              \
    if (haka->fp != NULL) {        \
      pclose(haka->fp);            \
      haka->fp = NULL;             \
    }                              \
    if (haka->fdNotesFile > 0)     \
      close(haka->fdNotesFile);    \
    haka->fdNotesFile = -1;        \
    haka->served = true;           \
  }

#endif  // !HAKA_EVENT_HANDLER_H
