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

#define updatePrevFile(haka)                                                   \
  haka->fdPrevFile = open(haka->prevFile, O_TRUNC | O_CREAT | O_WRONLY, 0666); \
  if (haka->fdPrevFile > 0) {                                                  \
    write(haka->fdPrevFile, haka->notesFileName, strlen(haka->notesFileName)); \
  }                                                                            \
  close(haka->fdPrevFile);

#define eventHandlerEpilogue(haka)                                             \
  if (haka != NULL) {                                                          \
    if (haka->fp != NULL) {                                                    \
      pclose(haka->fp);                                                        \
      haka->fp = NULL;                                                         \
    }                                                                          \
    if (haka->fdNotesFile > 0)                                                 \
      close(haka->fdNotesFile);                                                \
    haka->fdNotesFile = -1;                                                    \
    haka->served = true;                                                       \
  }

// clang-format off
static void switchFile(struct hakaContext *haka);
static void writeSelectionToFile(struct hakaContext *haka);
static void openFile(struct hakaContext *haka);
static void sendTextToFile(struct hakaContext *haka, char *text);
static void writeTextToFile(struct hakaContext *haka, char *prefix,
                            char *suffix);

static void   getPrimarySelection(struct hakaContext *haka, FILE **fp);
static void   getNotesFile(struct hakaContext *haka, char fileName[BUFSIZE * 2]);
static int    openNotesFile(struct hakaContext *haka);
static void   spawnChild(struct hakaContext *, char *argv[]);
static int    closeNotesFile(struct hakaContext *haka);
static size_t writeFP2FD(struct hakaContext *haka);
static void   triggerTofi(struct hakaContext *haka, FILE **fp);
// clang-format off

static struct coreApi hakaCoreAPI = {
    .ver = HAKA_ABI_VERSION,

    .addKeyBind = addKeyBind,

    .spawnChild = spawnChild,
    .getNotesFile = getNotesFile,
    .switchFile = switchFile,
    .getPrimarySelection = getPrimarySelection,
    .openNotesFile = openNotesFile,
    .writeFP2FD = writeFP2FD,
    .closeNotesFile = closeNotesFile,
    .writeTextToFile = writeTextToFile,
    .writeSelectionToFile = writeSelectionToFile,

    .openFile = openFile,

    .sendTextToFile = sendTextToFile,
    .triggerTofi = triggerTofi,
};

struct coreApi* getCoreApi() {
  return &hakaCoreAPI;
}

static void switchFile(struct hakaContext *haka) {
  contextCheck(haka);

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

  eventHandlerEpilogue(haka);
}

static void sendTextToFile(struct hakaContext *haka, char *text) {
  openNotesFile(haka);
  if (text != NULL) {
    write(haka->fdNotesFile, text, strlen(text));
  }
  closeNotesFile(haka);
}

static void writeTextToFile(struct hakaContext *haka, char *prefix,
                            char *suffix) {
  contextCheck(haka);

  openNotesFile(haka);
  if (prefix != NULL) {
    write(haka->fdNotesFile, prefix, strlen(prefix));
  }
  writeSelectionToFile(haka);
  openNotesFile(haka);
  if (suffix != NULL) {
    write(haka->fdNotesFile, suffix, strlen(suffix));
  }

  eventHandlerEpilogue(haka);
}

static void writeSelectionToFile(struct hakaContext *haka) {
  contextCheck(haka);

  ILOG("Dispatching request to get primary selection");
  getPrimarySelection(haka, &haka->fp);
  openNotesFile(haka);

  writeFP2FD(haka);

  eventHandlerEpilogue(haka);
}

static void spawnChild(struct hakaContext *haka, char *argv[]) {
  contextCheck(haka);
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
    execv(argv[0], argv);
    perror("execv failed to child");
    exit(1);
  }
  haka->childCount++;

  eventHandlerEpilogue(haka);
}

static void openFile(struct hakaContext *haka) {
  contextCheck(haka);

  ILOG("Opening current note in editor\n");

  CharVector argv = {.size = 0, .capacity = 0, .arr = NULL};
  CharVector *argvPtr = &argv;
  char *arg;
  ForEach(haka->config->terminal, arg) { VectorPush(argvPtr, arg); }
  ForEach(haka->config->editor, arg) { VectorPush(argvPtr, arg); }
  VectorPush(argvPtr, haka->notesFile);
  VectorPush(argvPtr, NULL);

  DLOG("Executing: ");
  ForEach(argvPtr, arg) { DLOG("%s ", arg); }
  spawnChild(haka, (char **)argv.arr);

  free(argv.arr); // Better be on stack
  eventHandlerEpilogue(haka);
}

static void getPrimarySelection(struct hakaContext *haka, FILE **fp) {
  contextCheck(haka);
  if (fp == NULL)
    return;

  *fp = popen("wl-paste -p", "r");
  if (*fp == NULL) {
    perror("popen error.");
    exit(1);
  }
}

static void getNotesFile(struct hakaContext *haka, char fileName[BUFSIZE * 2]) {
  strcpy(fileName, haka->notesFile);
}

static int openNotesFile(struct hakaContext *haka) {
  contextCheck(haka);

  haka->fdNotesFile = open(haka->notesFile, O_RDWR | O_CREAT | O_APPEND, 0666);
  if (haka->fdNotesFile < 0) {
    char errStr[BUFSIZE];
    sprintf(errStr, "Cannot open %s", haka->notesFile);
    perror(errStr);
    exit(1);
  }
  return haka->fdNotesFile;
}

static int closeNotesFile(struct hakaContext *haka) {
  contextCheck(haka);
  int res = close(haka->fdNotesFile);
  haka->fdNotesFile = (res == 0) ? -1 : haka->fdNotesFile;
  return res;
}

static size_t writeFP2FD(struct hakaContext *haka) {
  contextCheck(haka);

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

static void triggerTofi(struct hakaContext *haka, FILE **fp) {
  contextCheck(haka);
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
