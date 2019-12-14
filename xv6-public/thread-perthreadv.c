#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"

#define MAX_THREADS 10
#define PAGESIZE 4096

struct balance 
{
    char name[32];
    int amount;
};

struct balance per_thread_balance[MAX_THREADS];

struct thread_mutex
{
  uint locked;
};

struct thread_mutex mutex;

void thread_mutex_init(struct thread_mutex *lk)
{
  lk->locked=0;
}

void thread_mutex_lock(struct thread_mutex *lk)
{
  while(xchg(&lk->locked, 1) != 0)
    sleep(1);
}

void thread_mutex_unlock(struct thread_mutex *lk)
{
  asm volatile("movl $0, %0" : "+m" (lk->locked) : );
}

volatile int total_balance = 0;

volatile unsigned int delay (unsigned int d) 
{
   unsigned int i; 
   for (i = 0; i < d; i++) 
   {
       __asm volatile( "nop" ::: );
   }

   return i;   
}

void do_work(void *arg)
{
    int i; 
    int old;
   
    struct balance *b = (struct balance*) arg; 
    printf(1, "Starting do_work: s:%s\n", b->name);

    for (i = 0; i < b->amount; i++) 
    { 
         thread_mutex_lock(&mutex);
         old = total_balance;
         delay(100000);
         total_balance = old + 1;
         thread_mutex_unlock(&mutex);
    }
  
    printf(1, "Done s:%s\n", b->name);

    thread_exit();
    return;
}

int main(int argc, char *argv[]) {

  char *stacks[MAX_THREADS];
  int threads[MAX_THREADS], collect[MAX_THREADS];
  int correct_balance = 0;

  thread_mutex_init(&mutex);

  int i;

  for(i = 0; i < MAX_THREADS; i++){

	  per_thread_balance[i].name[0] = 65+i;
	  per_thread_balance[i].name[1] = '\0';
	  per_thread_balance[i].amount = (i+1)*1000;
	  correct_balance = correct_balance + per_thread_balance[i].amount;
	  stacks[i] = malloc(PAGESIZE);
	  threads[i] = thread_create(do_work, (void*)&per_thread_balance[i], (void*)stacks[i]);
  }

  for(i = 0; i < MAX_THREADS; i++)
  {
	  collect[i] = thread_join();
  }

  printf(1, "[main] Threads finished: ");

  for(i = 0; i < MAX_THREADS; i++)
  {
	  printf(1, "(%d):%d, ", threads[i], collect[i]);
  }

  printf(1, "[main] Shared balance: %d, correct balance: %d, diff: %d\n", total_balance, correct_balance, total_balance - correct_balance);
  exit();
}