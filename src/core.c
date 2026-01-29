#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "core.h"
#include "haka.h"
#include "plug.h"
#include "utils.h"

void switchFile(struct hakaContext *haka) {
  contextCheck(haka);

  printf("CTRL + ALT + M detected!\n");
  printf("Launching tofi\n");
  printf("tofi.cfg path: %s\n", haka->config->tofiCfg);

  triggerTofi(haka, &haka->fp);

  char buf[BUFSIZE];
  bool selection = false;
  while (fgets(buf, BUFSIZE, haka->fp)) {
    selection = true;
    buf[strcspn(buf, "\n")] = 0;
    printf("Selected: %ld %s\n", strlen(buf), buf);
    fflush(stdout);
  }

  if (selection) {
    strcpy(haka->notesFileName, buf);
    buildAbsFilePath(haka);
    updatePrevFile(haka);
  }

  eventHandlerEpilogue(haka);
}

void sendTextToFile(struct hakaContext *haka, char *text) {
  openNotesFile(haka);
  if (text != NULL) {
    write(haka->fdNotesFile, text, strlen(text));
  }
  closeNotesFile(haka);
}

void writeTextToFile(struct hakaContext *haka, char *prefix, char *suffix) {
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

void writeSelectionToFile(struct hakaContext *haka) {
  contextCheck(haka);

  Println("Dispatching request to get primary selection");
  getPrimarySelection(haka, &haka->fp);
  openNotesFile(haka);

  writeFP2FD(haka);

  eventHandlerEpilogue(haka);
}

void spawnChild(struct hakaContext *haka, char *argv[]) {
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
    Println("Executing %s", *argv);
    execv(argv[0], argv);
    perror("execv failed to child");
    exit(1);
  }
  haka->childCount++;

  eventHandlerEpilogue(haka);
}

void openFile(struct hakaContext *haka) {
  contextCheck(haka);

  printf("Opening current note in editor\n");

  CharVector argv = {.size = 0, .capacity = 0, .arr = NULL};
  CharVector *argvPtr = &argv;
  char *arg;
  ForEach(haka->config->terminal, arg) { VectorPush(argvPtr, arg); }
  ForEach(haka->config->editor, arg) { VectorPush(argvPtr, arg); }
  VectorPush(argvPtr, haka->notesFile);
  VectorPush(argvPtr, NULL);

  printf("Executing: ");
  ForEach(argvPtr, arg) { printf("%s ", arg); }
  printf("\n");
  spawnChild(haka, (char **)argv.arr);

  free(argv.arr); // Better be on stack
  eventHandlerEpilogue(haka);
}

void getPrimarySelection(struct hakaContext *haka, FILE **fp) {
  contextCheck(haka);
  if (fp == NULL)
    return;

  *fp = popen("wl-paste -p", "r");
  if (*fp == NULL) {
    perror("popen error.");
    exit(1);
  }
}

void getNotesFile(struct hakaContext *haka, char fileName[BUFSIZE * 2]) {
  strcpy(fileName, haka->notesFile);
}

int openNotesFile(struct hakaContext *haka) {
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

int closeNotesFile(struct hakaContext *haka) {
  contextCheck(haka);
  int res = close(haka->fdNotesFile);
  haka->fdNotesFile = (res == 0) ? -1 : haka->fdNotesFile;
  return res;
}

size_t writeFP2FD(struct hakaContext *haka) {
  contextCheck(haka);

  size_t bytes = 0;
  char buf[BUFSIZE];
  while (fgets(buf, BUFSIZE, haka->fp)) {
    buf[strcspn(buf, "\n")] = 0;
    bytes += strlen(buf);
    printf("%ld %s", strlen(buf), buf);
    write(haka->fdNotesFile, buf, strlen(buf));
  }
  write(haka->fdNotesFile, "\n", 1);
  return bytes;
}

void triggerTofi(struct hakaContext *haka, FILE **fp) {
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
  printf("Executing: %s\n", cmd);

  *fp = popen(cmd, "r");
  if (*fp == NULL) {
    perror("popen error.");
    exit(1);
  }
}
