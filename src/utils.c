#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libevdev/libevdev.h>

#include "base.h"
#include "utils.h"

struct IntSet *initIntSet(int capacity) {
  struct IntSet *set = (struct IntSet *)malloc(sizeof(struct IntSet));
  if (set == NULL) {
    perror("Unable to allocate IntSet.");
    exit(1);
  }

  set->size = 0;
  set->capacity = capacity;
  set->set = (int *)malloc(sizeof(int) * capacity);

  return set;
}

void freeIntSet(struct IntSet **set) {
  if (set == NULL || *set == NULL) {
    return;
  }

  free((*set)->set);
  free(*set);
  *set = NULL;
}

int pushIntSet(struct IntSet *set, int val) {
  if (set == NULL) {
    exit(1);
  }
  int size = set->size;
  while (size-- > 0) {
    if (val == set->set[size]) {
      return 1;
    }
  }

  if (set->size >= set->capacity) {
    dynamicInc(set);
  }
  set->set[set->size++] = val;

  return 0;
}

int dynamicInc(struct IntSet *set) {
  int newCapacity = set->capacity * 2;
  int *newArr = (int *)malloc(sizeof(int) * newCapacity);
  if (newArr == NULL) {
    return 1;
  }

  int size = set->size;
  while (size-- >= 0) {
    newArr[size] = set->set[size];
  }

  set->capacity = newCapacity;
  free(set->set);
  set->set = newArr;
  return 0;
}

void forceSudo() {
  if (!getuid()) {
    return;
  }
  printf("proc no root\nForcing root...\n");

  char buf[100];
  int rl = readlink("/proc/self/exe", buf, 100);
  if (rl < 0) {
    perror("readlink error:");
    exit(1);
  }
  buf[rl] = '\0';
  int ex = execlp("sudo", "sudo", buf, NULL);
  printf("Failed to restart application `%s`\n, error: %d", buf, ex);
  perror("exec failed: ");
  exit(1);
}

int checkPackage(const char *pkgName) {
  char cmd[512];
  sprintf(cmd, "which %s > /dev/null 2>&1", pkgName);

  int retVal = system(cmd);
  if (retVal == -1) {
    Println("system() failed to execute.");
    perror("system err: ");
    exit(1);
  }
  if (WEXITSTATUS(retVal) != 0) {
    Println("Cannot find %s in PATH", pkgName);
    Println("which %s returned %d", pkgName, WEXITSTATUS(retVal));
    return 1;
  }
  return 0;
}

void switchGrp(gid_t *curGID, const char *grpnam) {
  if (grpnam == NULL && curGID != NULL) {
    if (setgid(*curGID) < 0) {
      perror("Unable to set grp id");
      exit(1);
    }
    return;
  }

  struct group *grp = getgrnam(grpnam);
  if (grp == NULL) {
    char erStr[BUFSIZE];
    sprintf(erStr, "Cannot find an `%s` group.", grpnam);
    perror(erStr);
    exit(1);
  }
  *curGID = getgid();
  if (setgid(grp->gr_gid) < 0) {
    perror("Unable to set grp id");
    exit(1);
  }
}

int getKbdEvents(struct IntSet *set) {
  DIR *dir = opendir("/dev/input/by-path/");
  if (dir == NULL) {
    perror("Failed to open directory");
    exit(1);
  }
  ILOG("ref for /dev/input/by-path/ created @ %p", dir);

  if (set == NULL) {
    set = initIntSet(2);
  }

  struct dirent *entry = NULL;

  const size_t bfrsiz = 1024;
  char symlinkTo[bfrsiz], absPath[bfrsiz];

  while ((entry = readdir(dir)) != NULL) {
    size_t dNameLen = strlen(entry->d_name);
    if (dNameLen < 9) {
      continue;
    }

    char compVal[10] = {};
    strncpy(compVal, entry->d_name + dNameLen - 9, 10); // do copy the \0
    if (strcmp("event-kbd", compVal) != 0) {
      continue;
    }

    strcpy(absPath, "/dev/input/by-path/");
    strcat(absPath, entry->d_name);
    ssize_t len = readlink(absPath, symlinkTo, bfrsiz);
    if (len == -1) {
      continue;
    }
    symlinkTo[len] = '\0';

    ILOG("Entry: /dev/input/by-path/%s\t\tis a symlink to -> %s", entry->d_name,
         symlinkTo);
    pushIntSet(set, atoi((symlinkTo + 8)));
  }

  closedir(dir);

  int size = set->size;
  char setStr[BUFSIZE] = {};
  while (size-- > 0) {
    size_t slen = strlen(setStr);
    if (slen < BUFSIZE) {
      snprintf(setStr + slen, BUFSIZE - slen, "%d, ", set->set[size]);
    }
  }
  ILOG("eventX is a keyboard Event | X = %s\b\b;", setStr);

  return 0;
}

int openKbdDevices(struct IntSet *set, int *fds, struct libevdev **devs) {
  int fd;
  char kbd[BUFSIZE];
  struct libevdev *dev = NULL;

  ILOG("------");
  for (int i = 0; i < set->size; i++) {
    snprintf(kbd, BUFSIZE, "/dev/input/event%d", set->set[i]);
    ILOG("Opening: %s", kbd);

    fd = open(kbd, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
      perror("Failed to open device");
      return 1;
    }

    libevdev_new_from_fd(fd, &dev);
    ILOG("Device: %s\nfd: %d", libevdev_get_name(dev), fd);
    ILOG("Listening for key events...\n------");
    fds[i] = fd;
    devs[i] = dev;
  }
  return 0;
}

char *getEnvVar(const char *var) {
  char cmd[BUFSIZE];
  strCpyCat(cmd, "echo ", var);

  FILE *fp = popen(cmd, "r");
  if (fp == NULL) {
    Fprintln(stderr, "Unable to get Env Var %s", var);
    return NULL;
  }

  char *res = (char *)malloc(sizeof(char) * BUFSIZE);
  if (fgets(res, BUFSIZE, fp)) {
    res[strcspn(res, "\n")] = '\0';
  }

  pclose(fp);

  return res;
}

char *ltrim(char *s) {
  for (; s != NULL && (*s == ' ' || *s == '\t'); s++)
    ;
  return s;
}

char *rtrim(char *s) {
  char *t = &s[0] + strlen(s) - 1;
  for (; (t >= s) && (*t == ' ' || *t == '\t'); t--)
    ;
  if (t != NULL)
    *(t + 1) = '\0';
  return s;
}

char *trim(char *s) {
  s = ltrim(s);
  return rtrim(s);
}

void freePlugins(struct PluginVector **plugins) {
  for (int i = 0; i < (*plugins)->size; i++) {
    dlclose((*plugins)->arr[i]);
  }
  FreeVector((*plugins));
}

char *expandValidDir(char *val) {
  if (val[strlen(val) - 1] == '\\' || val[strlen(val) - 1] == '/') {
    val[strlen(val) - 1] = '\0';
  }
  if (val[0] == '~' && (val[1] == '/' || val[1] == '\\')) {
    char *home = getEnvVar("$HOME");
    val = val + 1;
    strcat(home, val);
    val = home;
  }

  return val;
}
