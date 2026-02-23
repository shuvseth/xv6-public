#include "types.h"
#include "stat.h"
#include "user.h"

static volatile int sink = 0;
static void burn(void){
  for(int i = 0; i < 5000; i++)
    sink += i;
}

int
main(int argc, char *argv[])
{
  if(argc < 4){
    printf(1, "usage: workratio <ticks_to_run> <t1> <t2> [t3 ...]\n");
    exit();
  }

  int run_ticks = atoi(argv[1]);
  int n = argc - 2;
  if(n > 16){
    printf(1, "too many children (max 16)\n");
    exit();
  }

  int start = uptime();
  int end = start + run_ticks;

  int rep[16][2];   // child -> parent pipe
  int pids[16];

  for(int i = 0; i < n; i++){
    if(pipe(rep[i]) < 0){
      printf(1, "pipe failed\n");
      exit();
    }

    int t = atoi(argv[i+2]);
    int pid = fork();
    if(pid < 0){
      printf(1, "fork failed\n");
      exit();
    }

    if(pid == 0){
      // child
      close(rep[i][0]); // close read end
      if(settickets(t) != 0){
        int bad = -1;
        uint iters = 0;
        write(rep[i][1], (char*)&bad, sizeof(bad));
        write(rep[i][1], (char*)&iters, sizeof(iters));
        exit();
      }

      uint iters = 0;
      while(uptime() < end){
        burn();
        iters++;
      }

      // Send (tickets, iters) to parent
      write(rep[i][1], (char*)&t, sizeof(t));
      write(rep[i][1], (char*)&iters, sizeof(iters));
      exit();
    } else {
      // parent
      pids[i] = pid;
      close(rep[i][1]); // close write end
    }
  }

  // Parent reads results and prints neatly
  for(int i = 0; i < n; i++){
    int t;
    uint iters;

    read(rep[i][0], (char*)&t, sizeof(t));
    read(rep[i][0], (char*)&iters, sizeof(iters));
    close(rep[i][0]);

    if(t < 0)
      printf(1, "child %d (pid %d): settickets failed\n", i, pids[i]);
    else
      printf(1, "child %d (pid %d): tickets %d iters %d\n", i, pids[i], t, iters);
  }

  for(int i = 0; i < n; i++)
    wait();

  printf(1, "workratio: done (ran %d ticks)\n", run_ticks);
  exit();
}