#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "binds.h"
#include "haka.h"
#include "plug.h"
#include "utils.h"

// Declarations & Core Api Def {{{

#define updatePrevFile(haka)                                                   \
  haka->fdPrevFile = open(haka->prevFile, O_TRUNC | O_CREAT | O_WRONLY, 0666); \
  if (haka->fdPrevFile > 0) {                                                  \
    write(haka->fdPrevFile, haka->notesFileName, strlen(haka->notesFileName)); \
  }                                                                            \
  close(haka->fdPrevFile);

#define ctxReset(haka)          \
  if (haka != NULL) {           \
    if (haka->fp != NULL) {     \
      pclose(haka->fp);         \
      haka->fp = NULL;          \
    }                           \
    if (haka->fdNotesFile > 0)  \
      close(haka->fdNotesFile); \
    haka->fdNotesFile = -1;     \
    haka->served = true;        \
  }

// clang-format off
static bool keyIsActive(hakaCtx *haka, int keyCode);

static void switchFile(hakaCtx *haka);
static void appendSelToFile(hakaCtx *haka);
static void displayFile(hakaCtx *haka);
static void appendTextToFile(hakaCtx *haka, char *text);
static void appendPadSelToFile(hakaCtx *haka, char *prefix,
                            char *suffix);

static void   getPrimarySelection(hakaCtx *haka, FILE **fp);
static void   getFile(hakaCtx *haka, char fileName[BUFSIZE * 2]);
static void   openFile(hakaCtx *haka);
static void   spawnChild(hakaCtx *, char *argv[]);
static int    closeFile(hakaCtx *haka);
static size_t writeFP2FD(hakaCtx *haka);
static void   triggerTofi(hakaCtx *haka, FILE **fp);
// clang-format on

static coreApi hakaCoreAPI = {
    .ver = HAKA_ABI_VERSION,

    .addKeyBind = addKeyBind,

    .keyIsActive = keyIsActive,

    .spawnChild = spawnChild,
    .getFile = getFile,
    .switchFile = switchFile,
    .getPrimarySelection = getPrimarySelection,
    .displayFile = displayFile,
    .writeFP2FD = writeFP2FD,
    .closeFile = closeFile,
    .appendPadSelToFile = appendPadSelToFile,
    .appendSelToFile = appendSelToFile,

    .openFile = openFile,

    .appendTextToFile = appendTextToFile,
    .triggerTofi = triggerTofi,
};

// }}}

coreApi* getCoreApi() {
  return &hakaCoreAPI;
}

static bool keyIsActive(hakaCtx* haka, int keyCode) {
  ctxCheck(haka);
  if (!haka->ks || !haka->ks->keyPress)
    return false;
  return haka->ks->keyPress[keyCode];
}

static void switchFile(hakaCtx* haka) {
  ctxCheck(haka);

  ILOG("Launching tofi\n");
  DLOG("tofi.cfg path: %s\n", haka->config->tofiCfg);

  triggerTofi(haka, &haka->fp);

  char buf[BUFSIZE];
  bool selection = false;
  while (fgets(buf, BUFSIZE, haka->fp)) {
    selection = true;
    buf[strcspn(buf, "\n")] = 0;
    ILOG("Selected: %ld %s\n", strlen(buf), buf);
    fflush(stdout);
  }

  if (selection) {
    strcpy(haka->notesFileName, buf);
    buildAbsFilePath(haka);
    updatePrevFile(haka);
  }

  ctxReset(haka);
}

static void appendTextToFile(hakaCtx* haka, char* text) {
  openFile(haka);
  if (text != NULL) {
    write(haka->fdNotesFile, text, strlen(text));
  }
  closeFile(haka);
}

static void appendPadSelToFile(hakaCtx* haka, char* prefix, char* suffix) {
  ctxCheck(haka);

  openFile(haka);
  if (prefix != NULL) {
    write(haka->fdNotesFile, prefix, strlen(prefix));
  }
  appendSelToFile(haka);
  openFile(haka);
  if (suffix != NULL) {
    write(haka->fdNotesFile, suffix, strlen(suffix));
  }

  ctxReset(haka);
}

static void appendSelToFile(hakaCtx* haka) {
  ctxCheck(haka);

  ILOG("Dispatching request to get primary selection");
  getPrimarySelection(haka, &haka->fp);
  openFile(haka);

  writeFP2FD(haka);

  ctxReset(haka);
}

static void spawnChild(hakaCtx* haka, char* argv[]) {
  ctxCheck(haka);
  if (argv == NULL || *argv == NULL) {
    Fprintln(stderr, "no args provided to spawn a child");
    return;
  }

  Println("Spawn Child");

  pid_t pid = fork();
  if (pid < 0) {
    Fprintln(stderr, "unable to create a fork");
    return;
  }
  if (pid == 0) {
    ILOG("Executing %s", *argv);
    execvp(argv[0], argv);
    perror("execvp failed to child");
    exit(1);
  }
  haka->childCount++;
}

static void displayFile(hakaCtx* haka) {
  ctxCheck(haka);

  ILOG("Opening current note in editor\n");

  CharVector argv = {.size = 0, .capacity = 0, .arr = NULL};
  CharVector* argvPtr = &argv;
  char* arg;
  ForEach(haka->config->terminal, arg) {
    VectorPush(argvPtr, arg);
  }
  ForEach(haka->config->editor, arg) {
    VectorPush(argvPtr, arg);
  }
  VectorPush(argvPtr, haka->notesFile);
  VectorPush(argvPtr, NULL);

  DLOG("Executing: ");
  ForEach(argvPtr, arg) {
    DLOG("%s ", arg);
  }
  spawnChild(haka, (char**)argv.arr);

  free(argv.arr);  // Better be on stack
  ctxReset(haka);
}

static void getPrimarySelection(hakaCtx* haka, FILE** fp) {
  ctxCheck(haka);
  if (fp == NULL)
    return;

  *fp = popen("wl-paste -p", "r");
  if (*fp == NULL) {
    perror("popen error.");
    exit(1);
  }
}

static void getFile(hakaCtx* haka, char fileName[BUFSIZE * 2]) {
  strcpy(fileName, haka->notesFile);
}

static void openFile(hakaCtx* haka) {
  ctxCheck(haka);

  haka->fdNotesFile = open(haka->notesFile, O_RDWR | O_CREAT | O_APPEND, 0666);
  if (haka->fdNotesFile < 0) {
    char errStr[BUFSIZE];
    sprintf(errStr, "Cannot open %s", haka->notesFile);
    perror(errStr);
    exit(1);
  }
}

static int closeFile(hakaCtx* haka) {
  ctxCheck(haka);
  int res = close(haka->fdNotesFile);
  haka->fdNotesFile = (res == 0) ? -1 : haka->fdNotesFile;
  return res;
}

static size_t writeFP2FD(hakaCtx* haka) {
  ctxCheck(haka);

  size_t bytes = 0;
  char buf[BUFSIZE];
  while (fgets(buf, BUFSIZE, haka->fp)) {
    buf[strcspn(buf, "\n")] = 0;
    bytes += strlen(buf);
    DLOG("%ld %s", strlen(buf), buf);
    write(haka->fdNotesFile, buf, strlen(buf));
  }
  write(haka->fdNotesFile, "\n", 1);
  return bytes;
}

static void triggerTofi(hakaCtx* haka, FILE** fp) {
  ctxCheck(haka);
  if (fp == NULL) {
    return;
  }

  char cmd[BUFSIZE * 2], basecmd[BUFSIZE * 2];
  snprintf(basecmd, BUFSIZE * 2, "ls %s -Ap1 | grep -v / | tofi -c %s",
           haka->config->notesDir, haka->config->tofiCfg);
  snprintf(cmd, BUFSIZE * 2,
           "%s  --prompt-text=\"  select:  \" "
           "--placeholder-text=\"%s\" --require-match=false",
           basecmd, haka->notesFileName);
  DLOG("Executing: %s\n", cmd);

  *fp = popen(cmd, "r");
  if (*fp == NULL) {
    perror("popen error.");
    exit(1);
  }
}

// vim: foldmethod=marker
