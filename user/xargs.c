
#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define BUFSIZE 4096

char buf[BUFSIZE];
char *args[MAXARG];

int
run(char *cmd, int count)
{
  args[count] = 0;

  int pid = fork();
  if(pid < 0){
    fprintf(2, "xargs: fork failed\n");
    return 1;
  }

  if(pid == 0){
    exec(cmd, args);
    fprintf(2, "xargs: exec failed\n");
    exit(1);
  }

  int status;
  if(wait(&status) < 0)
    return 1;

  return status != 0;
}

int
main(int argc, char *argv[])
{
  int n = 0;
  int start = 1;
  int limit = MAXARG;
  int mode = 0;
  int failed = 0;

  if(argc > 1 && strcmp(argv[1], "-n") == 0){
    if(argc < 4){
      fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
      exit(1);
    }

    for(int i = 0; argv[2][i]; i++){
      if(argv[2][i] < '0' || argv[2][i] > '9'){
        fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
        exit(1);
      }
    }

    n = atoi(argv[2]);
    if(n < 1){
      fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
      exit(1);
    }

    mode = 1;
    start = 3;
  }

  if(start >= argc){
    fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
    exit(1);
  }

  int base = argc - start;

  if(base > MAXARG){
    fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
    exit(1);
  }

  for(int i = 0; i < base; i++)
    args[i] = argv[start + i];

  limit = MAXARG - base;
  if(mode && n < limit)
    limit = n;

  if(limit <= 0){
    fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
    exit(1);
  }

  int pos = 0;
  int count = 0;
  int inword = 0;
  char c;

  while(read(0, &c, 1) == 1){
    if(c == ' ' || c == '\t' || c == '\n'){
      if(inword){
        buf[pos++] = 0;
        count++;
        inword = 0;
      }

      if((mode && count == limit) ||
         (!mode && c == '\n' && count > 0)){
        int offset = 0;
        for(int i = 0; i < count; i++){
          args[base + i] = buf + offset;
          offset += strlen(buf + offset) + 1;
        }

        if(run(args[0], base + count))
          failed = 1;

        pos = 0;
        count = 0;
      }
    } else {
      if(pos >= BUFSIZE - 2){
        fprintf(2, "xargs: input too long\n");
        exit(1);
      }

      buf[pos++] = c;
      inword = 1;
    }
  }

  if(inword){
    buf[pos++] = 0;
    count++;
  }

  if(count > 0){
    int offset = 0;
    for(int i = 0; i < count; i++){
      args[base + i] = buf + offset;
      offset += strlen(buf + offset) + 1;
    }

    if(run(args[0], base + count))
      failed = 1;
  }

  exit(failed);
}
