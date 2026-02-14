#ifndef HAKA_H
#define HAKA_H

#include <grp.h>
#include <libevdev/libevdev.h>
#include <linux/types.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "base.h"
#include "utils.h"

static volatile sig_atomic_t live = true;
static void handler(int signum) {
  live = false;
}

typedef struct confVars {
  CharVector* editor;
  CharVector* terminal;
  char pluginsDir[BUFSIZE];
  char notesDir[BUFSIZE];
  char tofiCfg[BUFSIZE];
} confVars;

typedef struct keyState {
  int16_t size;
  IntSet* activationCombo;
  bool* keyPress;
} keyState;

typedef struct hakaCtx {
  char execDir[BUFSIZE];
  char notesFileName[BUFSIZE];
  char notesFile[BUFSIZE * 2];
  int fdNotesFile;

  int fdPrevFile;
  char prevFile[BUFSIZE];

  FILE* fp;
  confVars* config;

  bool served;
  int childCount;

  keyState* ks;
} hakaCtx;

coreApi* getCoreApi();

hakaCtx* initHaka();
confVars* initConf(hakaCtx* haka);
void getExeDir(hakaCtx* haka);
void getPrevFile(hakaCtx* haka);

keyState* initKeyState(int16_t size);
void handleKeyEvent(keyState* ks, int evCode, int evVal);
void setActivationCombo(keyState* ks, ...);
bool resetActivationCombo(keyState* ks);
bool activated(keyState* ks);
int parseConf(confVars* conf, char* line);

void reapChild(hakaCtx* haka);

#define SUPPORTED_KEYS 249
#define ActivationCombo(...) setActivationCombo(ks, __VA_ARGS__, -1)

#define ctxCheck(haka)                                           \
  if (haka == NULL) {                                            \
    fprintf(stderr, "The hakaContext object cannot be NULL.\n"); \
    exit(1);                                                     \
  }

#define buildAbsFilePath(haka)                                            \
  snprintf(haka->notesFile, BUFSIZE * 2, "%s/%s", haka->config->notesDir, \
           haka->notesFileName);

#define NextWord(word) \
  for (; *word != '\0' && *word != ' ' && *word != '\t'; word++)

#endif
