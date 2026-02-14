#include "plug.h"

coreApi* api;

static void formatFile(hakaCtx* ctx) {
  char file[BUFSIZE * 2];
  api->getFile(ctx, file);

  char* argv[] = {"/usr/bin/nvim", "--clean", "--headless", "+normal! VGgq",
                  "+wq",           file,      NULL};
  api->spawnChild(ctx, argv);
}

// clang-format off
BEGIN_BIND
  Bind(NULL,                  KEY_R); // Reload Bindings
  Bind(api->appendSelToFile,  KEY_C);
  Bind(api->switchFile,       KEY_M);
  Bind(api->displayFile,      KEY_O);
  Bind(formatFile,            KEY_F);
END_BIND
// clang-format on
