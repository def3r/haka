#ifndef HAKA_EVENT_HANDLER_H
#define HAKA_EVENT_HANDLER_H

#include <stdio.h>

#include "base.h"
#include "haka.h"
#include "utils.h"

void switchFile(struct hakaContext* haka);
void writeSelectionToFile(struct hakaContext* haka);
void openFile(struct hakaContext* haka);
void sendTextToFile(struct hakaContext* haka, char* text);
void writeTextToFile(struct hakaContext* haka, char* prefix, char* suffix);

void getPrimarySelection(struct hakaContext* haka, FILE** fp);
void getNotesFile(struct hakaContext* haka, char fileName[BUFSIZE * 2]);
int openNotesFile(struct hakaContext* haka);
void spawnChild(struct hakaContext*, char* argv[]);
int closeNotesFile(struct hakaContext* haka);
size_t writeFP2FD(struct hakaContext* haka);
void triggerTofi(struct hakaContext* haka, FILE** fp);

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
