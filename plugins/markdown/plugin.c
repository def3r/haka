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

static void writeH1ToFile(struct hakaContext *ctx) {
  api->writeTextToFile(ctx, "\n# ", "");
}

static void writeH2ToFile(struct hakaContext *ctx) {
  api->writeTextToFile(ctx, "\n## ", "");
}

// clang-format off
BEGIN_BIND
  Bind(writeH1ToFile,       KEY_1);
  Bind(writeH2ToFile,       KEY_2);
  Bind(writePointToFile,    KEY_GRAVE);
  Bind(writeSubPointToFile, KEY_S);
  Bind(sendNewlineToFile,   KEY_N);
END_BIND
// clang-format on
