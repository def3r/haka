#include "plug.h"

struct coreApi *api = NULL;

static void writePointToFile(struct hakaContext *ctx) {
  api->writeTextToFile(ctx, "- ", "");
}

static void writeSubPointToFile(struct hakaContext *ctx) {
  api->writeTextToFile(ctx, "  - ", "");
}

static void sendNewlineToFile(struct hakaContext *ctx) {
  api->sendTextToFile(ctx, "\n");
}

BEGIN_BIND
  Bind(writePointToFile,    KEY_GRAVE);
  Bind(writeSubPointToFile, KEY_S);
  Bind(sendNewlineToFile,   KEY_N);
END_BIND
