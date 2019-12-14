#include "types.h"
#include "user.h"
#include "x86.h"


struct thread_mutex
{
  uint locked;
};

struct thread_cond
{
  // my conditional variable is the channel 
  // in which I am sleeping the thread
  int channel;
};

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

void thread_cond_init(struct thread_cond *cond)
{
  cond->channel = 0;
}

void thread_cond_wait(struct thread_cond *cond, struct thread_mutex *mutex)
{
  struct thread_cond *mycond = cond;
  thread_mutex_unlock(mutex);

  cv_sleep(cond->channel);

  thread_mutex_lock(mutex);
  return;
}

void thread_cond_signal(struct thread_cond *cond)
{
  cv_wakeup(cond->channel);
  return;
}


void *pld;

struct q 
{
   struct thread_cond cv;
   struct thread_mutex m;
 
   void *ptr;
};


// Thread 1 (sender)
void*
send(struct q *q, void *p)
{
  thread_mutex_lock(&q->m);

  // wait for the payload pipe to be empty
  while(q->ptr != 0);

  // send the payload
  q->ptr = p;

  // wake up the receiver to take the payload
  thread_cond_signal(&q->cv);

  thread_mutex_unlock(&q->m);
}


// Thread 2 (receiver)
void*
recv(struct q *q)
{
  void *p;

  thread_mutex_lock(&q->m);

  // wait for the sender to send the payload
  while((p = q->ptr) == 0)
    thread_cond_wait(&q->cv, &q->m);
  
  // reset the payload pipe
  q->ptr = 0;

  thread_mutex_unlock(&q->m);

  // recevie the payload
  pld = p;
  return;
}

struct payload 
{
  struct q *q;
  void *p;
};

int main(int argc, char *argv[]) 
{
  void* p = "payload";
  struct q *q;

  q->ptr=0;
  void *s1, *s2;
  
  thread_cond_init(&q->cv);
  thread_mutex_init(&q->m);

  struct payload payload = {&q, p};

  s1 = malloc(4096);
  s2 = malloc(4096);

  thread_create(recv, (void*)&payload, s1);
  thread_create(send, (void*)&payload, s2); 

  thread_join();
  thread_join();
  
  printf(1, "transmitted message is:%s",pld);

  exit();
}
