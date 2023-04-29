#include "start.h"

int main() {
  // printf("Fork Test, pid %d\n", getpid());
  print_s("Fork Test, pid ");
  print_i(getpid());
  print_s("\n");
  int cnt = 1;
  int ret = 0;
  if ((ret = fork()) == 0) {  // child
    // printf("pid: %d, cnt: %d, ptr: %p\n", getpid(), cnt, &cnt);
    print_s("pid: ");
    print_i(getpid());
    print_s(", cnt: ");
    print_i(cnt);
    print_s(", ptr: ");
    print_h((unsigned long long)&cnt);
    print_s("\n");
    ++cnt;
    fork();
    while (cnt < 5) {
      // printf("pid: %d, cnt: %d, ptr: %p\n", getpid(), cnt, &cnt);
      print_s("pid: ");
      print_i(getpid());
      print_s(", cnt: ");
      print_i(cnt);
      print_s(", ptr: ");
      print_h((unsigned long long)&cnt);
      print_s("\n");
      delay(1000000000);
      ++cnt;
    }
  } else {
    // printf("parent here, pid %d, child %d\n", getpid(), ret);
    for(int i = 0 ; i < 5;i++){
      print_s("parent here, pid: ");
      print_i(getpid());
      print_s(", child: ");
      print_i(ret);
      print_s("\n");
      delay(1000000000);
    }
  }
  // while(1);
  return 0;
}