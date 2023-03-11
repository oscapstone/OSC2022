#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static char pwd[256];
static char pathbuf[256];

#define VFS_DELIM "/"

int my_realpath(const char *relpath, char **dst) {
  *dst = NULL;
  char *path = (char*)malloc(256);
  if (path == NULL) return -ENOMEM;
  if (relpath[0] == '/') {
    strncpy(path, relpath, 256);
    *dst = path;
    return 0;
  }
  
  memset(path, 0, 256);
  memset(pathbuf, 0, sizeof(pathbuf));

  strncpy(pathbuf, pwd, sizeof(pathbuf));
  int len = strlen(pathbuf);
  if (len >= sizeof(pathbuf)) return -1;
  strncpy(pathbuf+len, relpath, sizeof(pathbuf)-len);

  char *cur = strtok(pathbuf, VFS_DELIM);
  char *pos = path;
  *pos++ = '/';
  
  //  *pos++ = '/';
  while (cur != NULL) {
    if (strncmp(cur, "..", sizeof(pathbuf)) == 0) {
      int dcnt = 0;
      while (1) {
        if (*(pos-1) == '/') dcnt++;
        *pos = '\0';
        if (dcnt == 2) break;
        if (pos == path+1) break;
        else pos--;
      }
    } else if (strncmp(cur, ".", sizeof(pathbuf)) != 0) {
      while(*cur != '\0') {
        *pos = *cur;
        pos++; cur++;
      }
      *pos++ = '/';
    }
    cur = strtok(NULL, VFS_DELIM);
  }
  // strncpy(path, pathbuf, 256);
  *dst = path;
  return 0;
}

int main() {
  static char inp2[256];
  while (1) {
    printf("pwd relpath> ");
    scanf("%s%s", pwd, inp2);
    char *fullpath;
    int ret = my_realpath(inp2, &fullpath);
    if (ret < 0) break;
    printf("realpath: %s\n", fullpath);
    free(fullpath);
  }
  return 0;
}
