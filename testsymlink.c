#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

static void die(const char *m){ printf(1, "FAIL: %s\n", m); exit(); }
static void ok(const char *m){ printf(1, "PASS: %s\n", m); }

int
main(void)
{
  int fd, n;
  char buf[64];

  printf(1, "A: create tgt\n");
  fd = open("tgt", O_CREATE | O_WRONLY);
  if(fd < 0) die("open tgt create");
  if(write(fd, "hello\n", 6) != 6) die("write tgt");
  close(fd);

  printf(1, "B: create symlink lnk -> tgt\n");
  if(symlink("tgt", "lnk") != 0) die("symlink tgt lnk");
  ok("symlink created");

  printf(1, "C: open lnk\n");
  fd = open("lnk", O_RDONLY);
  if(fd < 0) die("open lnk");

  printf(1, "D: read lnk\n");
  n = read(fd, buf, sizeof(buf) - 1);
  if(n < 0) die("read lnk");
  buf[n] = 0;
  close(fd);

  printf(1, "E: compare contents\n");
  if(strcmp(buf, "hello\n") != 0) die("content mismatch through symlink");
  ok("read through symlink");

  printf(1, "F: create a -> b\n");
  if(symlink("b", "a") != 0) die("symlink b a");

  printf(1, "G: create b -> a\n");
  if(symlink("a", "b") != 0) die("symlink a b");

  printf(1, "H: open a (should fail)\n");
  fd = open("a", O_RDONLY);
  if(fd >= 0){
    close(fd);
    die("expected open(a) to fail due to loop");
  }
  ok("loop detected (open failed)");

  printf(1, "testsymlink done\n");
  exit();
}