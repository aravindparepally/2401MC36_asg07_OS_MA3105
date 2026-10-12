#include "kernel/types.h"
#include "user/user.h"
int shared_counter = 0;
void thread_func(void *arg) {
  int id = (int)(uint64)arg;
  for(int i=0; i<100000; i++) shared_counter++;
  printf("Thread %d finished.\n", id);
  exit(0);
}
int main() {
  printf("=== Thread Test (Q1) ===\n");
  shared_counter = 0;
  void *s1 = malloc(4096), *s2 = malloc(4096), *s3 = malloc(4096);
  int p1 = clone(thread_func, (void*)1, s1 + 4096);
  int p2 = clone(thread_func, (void*)2, s2 + 4096);
  int p3 = clone(thread_func, (void*)3, s3 + 4096);
  join(p1); join(p2); join(p3);
  printf("Final counter: %d (Expected 300000, likely less due to race condition)\n", shared_counter);
  exit(0);
}
