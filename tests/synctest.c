#include "kernel/types.h"
#include "user/user.h"
int shared_counter = 0;
#define SEM_MUTEX 0
#define SEM_EMPTY 1
#define SEM_FULL 2
#define BUFFER_SIZE 5
int buffer[BUFFER_SIZE]; int in = 0, out = 0;

void thread_func(void *arg) {
  int id = (int)(uint64)arg;
  for(int i=0; i<100000; i++) { sem_wait(SEM_MUTEX); shared_counter++; sem_signal(SEM_MUTEX); }
  printf("Thread %d finished.\n", id); exit(0);
}
void producer(void *arg) {
  for(int i=1; i<=20; i++) {
    sem_wait(SEM_EMPTY); sem_wait(SEM_MUTEX);
    buffer[in] = i; in = (in+1)%BUFFER_SIZE;
    printf("Produced %d\n", i);
    sem_signal(SEM_MUTEX); sem_signal(SEM_FULL);
  } exit(0);
}
void consumer(void *arg) {
  for(int i=0; i<20; i++) {
    sem_wait(SEM_FULL); sem_wait(SEM_MUTEX);
    int item = buffer[out]; out = (out+1)%BUFFER_SIZE;
    printf("Consumed %d\n", item);
    sem_signal(SEM_MUTEX); sem_signal(SEM_EMPTY);
  } exit(0);
}
int main() {
  printf("=== Sync Test (Q2) ===\n");
  sem_init(SEM_MUTEX, 1); sem_init(SEM_EMPTY, BUFFER_SIZE); sem_init(SEM_FULL, 0);
  shared_counter = 0;
  void *s1 = malloc(4096), *s2 = malloc(4096), *s3 = malloc(4096);
  int p1 = clone(thread_func, (void*)1, s1+4096);
  int p2 = clone(thread_func, (void*)2, s2+4096);
  int p3 = clone(thread_func, (void*)3, s3+4096);
  join(p1); join(p2); join(p3);
  printf("Final counter with semaphores: %d (Should be exactly 300000)\n", shared_counter);
  
  printf("\n--- Producer Consumer ---\n");
  void *sp = malloc(4096), *sc = malloc(4096);
  int pp = clone(producer, 0, sp+4096);
  int cp = clone(consumer, 0, sc+4096);
  join(pp); join(cp);
  printf("Producer-Consumer finished.\n");
  exit(0);
}
