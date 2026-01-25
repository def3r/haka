#include <dirent.h>
#include <dlfcn.h>
#include <linux/input-event-codes.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "eventHandler.h"
#include "haka.h"
#include "plug.h"
#include "utils.h"

static void loadPlugins(char *path, struct keyBindings *kbinds,
                        struct coreApi *api, struct PluginVector *plugins) {
  DIR *dir = opendir(path);
  if (dir == NULL) {
    Fprintln(stderr, "Cannot open plugins dir: %s", path);
    return;
  }

  struct dirent *entry = NULL;
  while ((entry = readdir(dir)) != NULL) {
    void *h;
    char fullPath[PATH_MAX];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);

    switch (entry->d_type) {
    case DT_DIR: {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        break;

      loadPlugins(fullPath, kbinds, api, plugins);
    } break;

    case DT_REG: {
      h = dlopen(fullPath, RTLD_NOW | RTLD_LOCAL);
    binding:
      if (h == NULL) {
        Fprintln(stderr, "Error loading plugin %s: %s", fullPath, dlerror());
        break;
      }

      pluginInit_t init = dlsym(h, "hakaPluginInit");
      if (init == NULL) {
        Fprintln(stderr, "Cannot find hakaPluginInit for %s", fullPath);
        dlclose(h);
      } else {
        Println("Plugin Loaded: %s", fullPath);
        if (!init(api, kbinds)) {
          VectorPush(plugins, h);
        }
      }
    } break;

    default:          // Fallback to sys/stat
      struct stat st; // NOLINT
      if (stat(fullPath, &st) == -1) {
        perror("stat");
        continue;
      }

      if (S_ISDIR(st.st_mode)) {
        loadPlugins(fullPath, kbinds, api, plugins);
      }

      if (S_ISREG(st.st_mode)) {
        h = dlopen(fullPath, RTLD_NOW | RTLD_LOCAL);
        goto binding;
      }
      break;
    }
  }

  closedir(dir);
}

void loadBindings(struct keyBindings **kbinds, struct keyState *ks,
                  struct coreApi *api, struct PluginVector **plugins) {
  if (kbinds == 0 || ks == 0) {
    Fprintln(stderr, "cannot bind keys to null");
    exit(EXIT_FAILURE);
  }

  if ((*plugins)->size != 0) {
    freeKeyBindings(kbinds);
    *kbinds = initKeyBindings(2);

    for (int i = 0; i < (*plugins)->size; i++) {
      dlclose((*plugins)->arr[i]);
    }
    FreeVector((*plugins));
    MakeVector(PluginVector, *plugins);
  } else {
    Println("Activating again?");
    // Activation Combo
    ActivationCombo(KEY_LEFTCTRL, KEY_LEFTALT);
  }
  Println("Done FREEing");

  loadPlugins("./plugins", *kbinds, api, *plugins);
}
