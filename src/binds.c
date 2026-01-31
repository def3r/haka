#include <dirent.h>
#include <dlfcn.h>
#include <linux/input-event-codes.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "base.h"
#include "binds.h"
#include "haka.h"
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
        DLOG("Error loading plugin %s: %s", fullPath, dlerror());
        break;
      }

      pluginInit_t init = dlsym(h, "hakaPluginInit");
      if (init == NULL) {
        Fprintln(stderr, "Cannot find hakaPluginInit for %s", fullPath);
        dlclose(h);
      } else {
        ILOG("Plugin Loaded: %s", fullPath);
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

void loadBindings(struct hakaContext *haka, struct keyBindings **kbinds,
                  struct keyState *ks, struct coreApi *api,
                  struct PluginVector **plugins) {
  if (kbinds == 0 || ks == 0) {
    Fprintln(stderr, "cannot bind keys to null");
    exit(EXIT_FAILURE);
  }

  if ((*plugins)->size != 0) {
    DLOG("Reloading Plugins");
    freeKeyBindings(kbinds);
    *kbinds = initKeyBindings(2);

    freePlugins(plugins);
    MakeVector(PluginVector, *plugins);
    DLOG("Done FREEing Keybinds");
  } else {
    // Activation Combo
    ActivationCombo(KEY_LEFTCTRL, KEY_LEFTALT);
  }

  loadPlugins(haka->config->pluginsDir, *kbinds, api, *plugins);
}

struct keyBindings *initKeyBindings(int size) {
  if (size < 0) {
    Fprintln(stderr, "keybinds size < 0?");
    exit(EXIT_FAILURE);
  }

  struct keyBindings *kbinds =
      (struct keyBindings *)malloc(sizeof(struct keyBindings));
  if (kbinds == 0) {
    Fprintln(stderr, "malloc failed for key bindings");
    exit(EXIT_FAILURE);
  }

  kbinds->size = 0;
  kbinds->capacity = size;
  kbinds->kbind = (struct keyBinding *)malloc(sizeof(struct keyBinding) * size);
  if (kbinds->kbind == 0) {
    Fprintln(stderr, "malloc failed for key binding");
    exit(EXIT_FAILURE);
  }

  return kbinds;
}

void freeKeyBindings(struct keyBindings **kbinds) {
  if (kbinds == NULL || *kbinds == NULL) {
    return;
  }

  for (int i = 0; i < (*kbinds)->size; i++) {
    freeIntSet(&(*kbinds)->kbind[i].keys);
  }
  free((*kbinds)->kbind);
  free(*kbinds);
  *kbinds = NULL;
}

void addKeyBind(struct keyBindings *kbinds, void (*func)(struct hakaContext *),
                int keyToBind, ...) {
  if (kbinds == 0) {
    Fprintln(stderr, "keybinds are null; abort adding a keybind");
    return;
  }

  struct IntSet *keys = initIntSet(5);
  pushIntSet(keys, keyToBind);

  va_list args;
  va_start(args, keyToBind);
  int key = va_arg(args, int);
  while (key != 0) {
    pushIntSet(keys, key);
    key = va_arg(args, int);
  }
  va_end(args);

  struct keyBinding kbind = (struct keyBinding){.keys = keys, .func = func};
  pushKeyBind(kbinds, &kbind);
}

void pushKeyBind(struct keyBindings *kbinds, struct keyBinding *kbind) {
  if (kbinds == 0) {
    Fprintln(stderr, "keybinds are null; abort pushing a keybind");
    return;
  }
  if (kbind == 0) {
    Fprintln(stderr, "keybind is null; abort pushing to keybinds");
    return;
  }

  if (kbinds->size >= kbinds->capacity) {
    struct keyBinding *newKBindArr = (struct keyBinding *)malloc(
        sizeof(struct keyBinding) * kbinds->capacity * 2);
    if (newKBindArr == 0) {
      Fprintln(stderr, "malloc failed for dynamic array kbinds");
      exit(EXIT_FAILURE);
    }
    memcpy(newKBindArr, kbinds->kbind,
           sizeof(struct keyBinding) * kbinds->capacity);
    free(kbinds->kbind);
    kbinds->kbind = newKBindArr;
    kbinds->capacity *= 2;
  }

  kbinds->kbind[kbinds->size++] =
      (struct keyBinding){.keys = kbind->keys, .func = kbind->func};
}

int executeKeyBind(struct keyBindings *kbinds, struct keyState *ks,
                   struct hakaContext *haka) {
  int retval = FAIL;
  if (kbinds == 0) {
    Fprintln(stderr, "keybinds are null; abort executing a keybind");
    return retval;
  }
  if (ks == 0) {
    Fprintln(stderr, "keystatus is null; abort executing a keybind");
    return retval;
  }

  retval++;
  haka->served = false;
  for (int i = 0; i < kbinds->size; i++) {
    struct keyBinding kbind = kbinds->kbind[i];
    struct IntSet *keys = kbind.keys;
    int j;
    for (j = 0; j < keys->size && ks->keyPress[keys->set[j]]; j++)
      ;
    if (j != keys->size)
      continue;

    if (kbind.func == NULL) {
      retval = RELOAD;
    } else {
      kbind.func(haka);
    }

    // Issue: After Tofi launches, haka waits for it to return
    // sometimes the KeyUp events are missed by haka resulting
    // in undefined behaviour.
    //
    // Clean up after serving can fix it. Need a better way to
    // do this
    haka->served = true;
    for (j = 0; j < keys->size; j++)
      ks->keyPress[keys->set[j]] = false;
    for (int i = 0; i < ks->activationCombo->size; i++)
      ks->keyPress[ks->activationCombo->set[i]] = false;
    return retval;
  }
  return retval;
}
