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

struct sem
{
    int count; 
    struct thread_mutex lock;
    struct thread_cond cond;
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

int sem_init(struct sem *sem, int value) 
{
    sem->count = value;
    thread_mutex_init(&sem->lock);
    thread_cond_init(&sem->cond);
    return 0;
}

void sem_post(struct sem *sem) 
{
    thread_mutex_lock(&sem->lock);

    sem->count++;

    thread_cond_signal(&sem->cond);
    thread_mutex_unlock(&sem->lock);
}

void sem_wait(struct sem *sem) 
{
    thread_mutex_lock(&sem->lock);

    while (sem->count == 0) 
    {
      thread_cond_wait(&sem->cond, &sem->lock);
    }

    sem->count--;
    thread_mutex_unlock(&sem->lock);
}

void *pld;

struct q 
{
   struct sem sem; 
   void *ptr;
};


// Thread 1 (sender)
void*
send(struct q *q, void *p)
{
  thread_mutex_lock(&q->sem.lock);

  // wait for the payload pipe to be empty
  while(q->ptr != 0);

  // send the payload
  q->ptr = p;

  // wake up the receiver to take the payload
  sem_post(&q->sem);

  thread_mutex_unlock(&q->sem.lock);
}


// Thread 2 (receiver)
void*
recv(struct q *q)
{
  void *p;

  thread_mutex_lock(&q->sem.lock);

  // wait for the sender to send the payload
  while((p = q->ptr) == 0)
    sem_wait(&q->sem);
  
  // reset the payload pipe
  q->ptr = 0;

  thread_mutex_unlock(&q->sem.lock);

  // receive the payload
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

  sem_init(&q->sem, 0);

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
