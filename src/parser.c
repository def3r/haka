#include <stdio.h>
#include <string.h>

#include "haka.h"
#include "utils.h"

static int parseConfVal(char *var, char *val, char *arg,
                        CharVector *argv) {
  while (strlen(val)) {
    char *word = val;
    NextWord(word);
    if (*word == '\0') {
      // We have reached the end of line
      arg = (char *)calloc(strlen(val) + 1, sizeof(char));
      strcat(arg, val);
      VectorPush(argv, arg);
      break;
    }
    char *begin = NULL;
    char charAt = *word;
    *word = '\0';

    if (strlen(val) == 0) {
      val = word + 1;
      continue;
    }

    // TODO: escape chars \

    if ((begin = strstr(val, "$(")) != NULL) {
      // We have $( in the val, find the ')'
      *word = charAt;
      word = begin + 2;
      int bop = 1;
      for (word++; *word != '\0' && !(*word == ')' && bop == 1); word++) {
        bop += (*word == '(') - (*word == ')');
      }
      if (*word == '\0') {
        Fprintln(stderr, "%s: Invalid syntax for $(): Missing ')'", var);
        return 1;
      }
      *word = '\0';
      *begin = '\0';

      begin += 2;
      FILE *sh = popen(begin, "r");
      if (sh == NULL) {
        perror("Unable to popen: ");
        exit(1);
      }

      arg = (char *)calloc(BUFSIZE, sizeof(char));
      strcat(arg, val);

      char res[BUFSIZE];
      if (fgets(res, BUFSIZE, sh)) {
        res[strcspn(res, "\n")] = '\0';
        strcat(arg, res);
      }

      fclose(sh);

      val = ++word;
      if (*word != '"') {
        NextWord(word);
        if (*word == '\0') {
          // We have reached the end of line
          word--;
        } else {
          *word = '\0';
        }
        strcat(arg, val);
      } else {
        word--;
      }
      VectorPush(argv, arg);

    } else if ((begin = strstr(val, "\"")) != NULL) {
      *begin = '\0';
      *word = charAt;
      if (strlen(val)) {
        arg = (char *)calloc(BUFSIZE, sizeof(char));
        strcat(arg, val);
        VectorPush(argv, arg);
      }

      begin++;
      word = begin;
      for (word++; *word != '\0' && !(*word == '"'); word++)
        ;
      if (*word == '\0') {
        Fprintln(stderr, "%s: Missing '\"'", var);
        return 1;
      }
      *word = '\0';

      arg = (char *)calloc(BUFSIZE, sizeof(char));
      strcat(arg, begin);
      VectorPush(argv, arg);

    } else {
      arg = (char *)calloc(strlen(val) + 1, sizeof(char));
      strcat(arg, val);
      VectorPush(argv, arg);
    }

    val = word + 1;
  }

  return 0;
}

int parseConf(struct confVars *conf, char *line) {
  if (line == NULL || conf == NULL) {
    return -1;
  }

  line = trim(line);
  if (line[0] == '#')
    return 0;

  char *c = line;
  for (; *c != '\0' && *c != '='; c++)
    ;
  if (*c != '=') {
    return 1;
  }

  *c = '\0';
  char *var = line;
  char *val = ++c;
  var = trim(var);
  val = trim(val);

  char *arg;
  CharVector *argv;
  MakeVector(CharVector, argv);

  if (parseConfVal(var, val, arg, argv)) {
    FreeVector(argv);
    return 1;
  }

  VectorPush(argv, NULL);
  if (strcmp(var, "editor") == 0) {
    if (conf->editor != NULL) {
      FreeVector(conf->editor);
    }
    conf->editor = argv;
  } else if (strcmp(var, "notes-dir") == 0) {
    strcpy(conf->notesDir, expandValidDir(val));
  } else if (strcmp(var, "tofi-cfg") == 0) {
    strcpy(conf->tofiCfg, expandValidDir(val));
  } else if (strcmp(var, "terminal") == 0) {
    if (conf->terminal != NULL) {
      FreeVector(conf->terminal);
    }
    conf->terminal = argv;
  } else if (strcmp(var, "plugins") == 0) {
    strcpy(conf->pluginsDir, expandValidDir(val));
  }

  DLOG("%s [%d]:", var, argv->size);
  ForEach(argv, arg) { DLOG("Arg: %s", arg); }

  return 0;
}
