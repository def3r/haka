#include "plug.h"

coreApi* api;

static void formatFile(hakaCtx* ctx) {
  char file[BUFSIZE * 2];
  api->getFile(ctx, file);

  char* argv[] = {"nvim", "--clean", "--headless", "+normal! VGgq",
                  "+wq",  file,      NULL};
  api->spawnChild(ctx, argv);
}

static void hakaJournal(hakaCtx* ctx) {
  static char* journal[] = {"journalctl", "--user", "-u", "haka", "-n", "100"};

  CharVector* argv;
  MakeVector(CharVector, argv);
  api->getTerminal(ctx, argv);
  VectorPushStrArr(argv, journal);

  api->spawnChildVec(ctx, argv);

  DeepFreeVector(argv);
}

// clang-format off
BEGIN_BIND
  Bind(NULL,                  KEY_R); // Reload Bindings
  Bind(api->appendSelToFile,  KEY_C);
  Bind(api->switchFile,       KEY_M);
  Bind(api->displayFile,      KEY_O);
  Bind(formatFile,            KEY_F);
  Bind(hakaJournal,           KEY_J);
END_BIND
// clang-format on
