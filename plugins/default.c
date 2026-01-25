#include "plug.h"

struct coreApi* api;

BEGIN_BIND
  Bind(NULL,                      KEY_R); // Reload Bindings
  Bind(api->writeSelectionToFile, KEY_C);
  Bind(api->switchFile,           KEY_M);
  Bind(api->openFile,             KEY_O);
END_BIND
