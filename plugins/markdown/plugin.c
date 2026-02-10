#include <linux/input-event-codes.h>
#include "plug.h"

struct coreApi *api = NULL;

static char pounds[] = "######";

static void writePointToFile(struct hakaContext *ctx) {
  api->writeTextToFile(ctx, "- ", "");
}

static void writeSubPointToFile(struct hakaContext *ctx) {
  api->writeTextToFile(ctx, "  - ", "");
}

static void writeHeadingToFile(struct hakaContext *ctx) {
  int n = 0;
  for (int key = KEY_1; key <= KEY_4 && n == 0; key++) {
    n = (api->keyIsActive(ctx, key)) * (key - KEY_1 + 1);
  }
  char nPounds[9] = {0}; // including whitespace = 9
  snprintf(nPounds, n + 3, "\n%.*s ", n, pounds);
  api->writeTextToFile(ctx, nPounds, "");
}

static void sendNewlineToFile(struct hakaContext *ctx) {
  api->sendTextToFile(ctx, "\n");
}

// clang-format off
BEGIN_BIND
  Bind(writeHeadingToFile,  KEY_1);
  Bind(writeHeadingToFile,  KEY_2);
  Bind(writeHeadingToFile,  KEY_3);
  Bind(writeHeadingToFile,  KEY_4);
  Bind(writePointToFile,    KEY_A);
  Bind(writeSubPointToFile, KEY_S);
  Bind(sendNewlineToFile,   KEY_N);
END_BIND
// clang-format on
