
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

int match(char *p, char *s)
{
  if(*p == '\0')
    return *s == '\0';

  if(*p == '*'){
    if(match(p + 1, s))
      return 1;
    if(*s && match(p, s + 1))
      return 1;
    return 0;
  }

  if(*p == *s && *s)
    return match(p + 1, s + 1);

  return 0;
}

void find(char *path, char *pattern, int type)
{
  int fd;
  struct stat st;
  struct dirent de;
  char buf[512];

  fd = open(path, 0);
  if(fd < 0){
    fprintf(2, "find: cannot open %s\n", path);
    exit(1);
  }

  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    if(de.inum == 0)
      continue;

    if(strcmp(de.name, ".") == 0 ||
       strcmp(de.name, "..") == 0)
      continue;

    if(strlen(path) + strlen(de.name) + 2 > sizeof(buf))
      continue;

    strcpy(buf, path);
    int len = strlen(buf);

    if(len > 0 && buf[len - 1] != '/')
      buf[len++] = '/';

    strcpy(buf + len, de.name);

    if(stat(buf, &st) < 0)
      continue;

    if(match(pattern, de.name)){
      if(type == 0 ||
         (type == 1 && (st.type == T_FILE || st.type == T_DEVICE)) ||
         (type == 2 && st.type == T_DIR)){
        printf("%s\n", buf);
      }
    }

    if(st.type == T_DIR)
      find(buf, pattern, type);
  }

  close(fd);
}

int main(int argc, char *argv[])
{
  int type = 0;

  if(argc != 3 && argc != 5){
    fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
    exit(1);
  }

  if(argc == 5){
    if(strcmp(argv[3], "-type") != 0){
      fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
      exit(1);
    }

    if(strcmp(argv[4], "f") == 0)
      type = 1;
    else if(strcmp(argv[4], "d") == 0)
      type = 2;
    else{
      fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
      exit(1);
    }
  }

  find(argv[1], argv[2], type);
  exit(0);
}
