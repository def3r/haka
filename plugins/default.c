#include "plug.h"

struct coreApi *api;

static void formatFile(struct hakaContext *ctx) {
  char file[BUFSIZE * 2];
  api->getNotesFile(ctx, file);

  char *argv[] = {"/usr/bin/nvim", "--clean", "--headless", "+normal! VGgq",
                  "+wq",           file,      NULL};
  api->spawnChild(ctx, argv);
}

// clang-format off
BEGIN_BIND
  Bind(NULL,                      KEY_R); // Reload Bindings
  Bind(api->writeSelectionToFile, KEY_C);
  Bind(api->switchFile,           KEY_M);
  Bind(api->openFile,             KEY_O);
  Bind(formatFile,                KEY_F);
END_BIND
// clang-format on
