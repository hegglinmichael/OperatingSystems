/**
 *  Author Michael Hegglin
 *  Due Date: 4 - 27 - 2020
 *
 *  Description:
 *    This program creates a library to keep track of
 *    light weight processes (threads) for the user
 *
 *  from passing 2 to 28 we made it
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "thread_ahead.h"

/*
Line Length Limit
--------------------------------------------------------------------------------
START scheduler variables
*/

/* scheduler for the whole shebang is kernal sanders */
struct scheduler chicken = {NULL, NULL, rr_admit, rr_remove, rr_next};
/* and what does kernal sanders have to make schedules?  His chicken */
scheduler kernal_sanders = &chicken;

/* 
 * I first wrote the sceduler to pop a node off when next was called
 * but having to re-enter threads I felt wasn't the best way, so I changed
 * it to a circular linked list, relying on myself to call remove 
 * properly when exiting
 */

thread head = NULL;  /* ptr to the array in memory malloced */
thread tail = NULL;  /* ptr to the tail of the array of threads */
thread next = NULL;  /* points to the next in scheduler to be returned */

/* 
 * Below is just for the tid to work, I'm going to save everything
 * because clearly I'm not supposed to use sched_one & two for it.
 * I hate how I solved this.  I could malloc and realloc more 
 * depending on list size, but even then I don't like that solution
 * of having to create an independent list
 */

thread all_threads[MAX_THREADS];

/*
END scheduler variables
--------------------------------------------------------------------------------
START LWP variables
*/

/* This method increments upwards to assign new tid */
tid_t give_lwp_id = 0;
/* keeps track of the current thread running */
thread curr_thread = NULL;
/* hold the current values to be used */
rfile *regs = NULL;

/*
END LWP variables
--------------------------------------------------------------------------------
START LWP Methods
*/

/**
 *  Description:
 *    mallocs data for the stack
 *    creats a new thread
 *    gives the thread an id
 *    puts arguments in registers
 *    puts function reference on the stack
 *    makes the stack look suspended
 *    sets the base ptr 
 **/
tid_t lwp_create(lwpfun func_ref, void *args, size_t size)
{
  if (DEBUG) { printf("MAKING THREAD:\n"); fflush(stdout); }
  unsigned long *stack_ptr;

  /* error checking */
  if (func_ref == NULL || size < 0)
  {
    return -1;
  }

  /* mallocing data for the thread context */
  thread my_new_thread = (thread)malloc(THREAD_SIZE);

  /* mallocing data for the stack */
  my_new_thread->stack = (unsigned long *)malloc(size * LONG_SIZE);

  /* error checking */
  if (my_new_thread == NULL || my_new_thread->stack == NULL)
  {
    fprintf(stderr, "Failed to malloc data when adding thread\n");
    return -1;
  }

  /* point this ptr to the start of the stack, you said go backwards */
  stack_ptr = (my_new_thread->stack + size) - 1;

  my_new_thread->tid = give_lwp_id++;
  my_new_thread->stacksize = size;

  /* Need to set default value per directions */
  my_new_thread->state.fxsave = FPU_INIT;
  /* I think this is where I save the argument reference, confirmed in class */
  my_new_thread->state.rdi = (unsigned long)args;

  /* 
   * Make the stack look like it's suspended? 
   * I have no idea if this is correct so check here if failing 
   *
   * Yeah, I was wrong a bunch
   *   - don't increment the stack
   *   - you need to add lwp_exit before the function
   *   - point the rbp at end of stack
   *  
   * I'm unsure if I should make the stack go up or down
   * guess we'll see in testing.  Nvmind you said it in lecture
   */
  *stack_ptr = (unsigned long)lwp_exit;
  stack_ptr--;
  *stack_ptr = (unsigned long)func_ref;
  stack_ptr--;

  /* need to set the base pointer now I think its down here */
  my_new_thread->state.rbp = (unsigned long)stack_ptr;

  /* add the thread to the scheduler */
  kernal_sanders->admit(my_new_thread);
  /* add it to all_threads just for tid method */
  all_threads[my_new_thread->tid] = my_new_thread;

  if (DEBUG) { printf("THREAD CREATED: %lu\n", give_lwp_id); fflush(stdout); }
  return (my_new_thread->tid);
}

/**
 *  Description:
 *    You said make a real exit, now to figure out how it works
 *    I don't understand why I should have this separate, only 
 *    thing I'd say I really don't understand.
 *
 *    Solved: it's beyond the reach of the stack now
 *
 *  Will the real exit please stand up
 **/
void real_exit(void)
{
  kernal_sanders->remove(curr_thread);
  curr_thread = kernal_sanders->next();

  (curr_thread == NULL) ?
  (swap_rfiles(NULL, regs)) :
  (swap_rfiles(NULL, &(curr_thread->state)));
}

/**
 *  Description:
 *    Sets registers, then tells the
 *    real exit to please stand up
 **/
void lwp_exit(void)
{
  if (DEBUG) { printf("START LWP EXIT:\n"); fflush(stdout); }
  SetSP(regs->rsp);
  real_exit();
  if (DEBUG) { printf("END LWP EXIT:\n"); fflush(stdout); }
}

/**
 *  Description:
 *    returns the lwp of current thread,
 *    and NO_THREAD if not called by a lwp
 **/
tid_t lwp_gettid(void)
{
  tid_t t;
  (curr_thread == NULL) ? (t = NO_THREAD) : (t = curr_thread->tid);
  return t;
}

void lwp_yield(void)
{
  if (DEBUG) { printf("YEILDING:\n"); fflush(stdout); }
  /* if I'm swaping I need the old chicken too */
  thread temp = curr_thread;

  /* 
   * instr say "control another lwp", so first
   * lets start by getting another chicken
   */
  curr_thread = kernal_sanders->next();

  /* 
   * check for null.
   *
   * need to swap files if I get a new one, ah which means
   * I need to save the old one
   */
  if (curr_thread == NULL)
  {
    swap_rfiles(NULL, regs);
  }

  swap_rfiles(&(temp->state), &(curr_thread->state));
  if (DEBUG) { printf("YELT:\n"); fflush(stdout); }
}

/**
 *  Description:
 *    this method mallocs some memory for the programs
 *    registers, gets the first piece of chicken(thread)
 *    from kernal sanders, and does the swap_rfiles
 ***/
void lwp_start(void)
{
  if (DEBUG) { printf("STARTING THREAD\n"); fflush(stdout);}
  /* malloc space for all the registers, start should only be called once? */
  regs = (rfile *)malloc(RFILE_SIZE);
  /* getting a piece of chicken from kernal sanders */
  curr_thread = kernal_sanders->next();

  /* return if kernal sanders has no chicken */
  if (curr_thread == NULL)
  {
    return;
  }

  /* 
   * the instructions say swap it so that's a good start 
   * I'm 50% sure that the args are correct, so that's higher than normal
   */
  swap_rfiles(regs, &(curr_thread->state));
  if (DEBUG) { printf("STARTED THREAD\n"); fflush(stdout);}
}

/**
 *  Description:
 *    Swaps everything back to the start
 **/
void lwp_stop(void)
{
  if (DEBUG) { printf("STOPPING LWP\n"); fflush(stdout);}
  swap_rfiles(&(curr_thread->state), regs);
  if (DEBUG) { printf("STOPPED LWP\n"); fflush(stdout);}
}

/*
END LWP Methods
--------------------------------------------------------------------------------
START Scheduler Methods
*/

/**
 *  Description:
 *    Method copies a thread.  I wanted rr_remove to 
 *    free the thread when it was removed, so you have to make
 *    a new one.  Could probably just not free, but whatever.
 *    
 *    Nevermind probably best to copy since you have to mess with
 *    the stack pointer. That one sucked to figure out.
 **/
void copy_a_thread_over(thread old, thread new)
{
  void *stack_ptr;

  new->tid = old->tid;
  new->state = old->state;
  new->stacksize = old->stacksize;
  new->sched_one = old->sched_one;
  new->sched_two = old->sched_two;
  memcpy(new->stack, old->stack, LONG_SIZE * old->stacksize);

  /* I discovered that I need to change the location of the stack ptr */
  stack_ptr = new->stack;
  stack_ptr += (old->state.rsp - (unsigned long)old->stack);
  new->state.rsp = (unsigned long)stack_ptr;

  /* 
   * I'm adding this in here because I have no idea what these
   * variables are for.  I'm just putting them here because I 
   * figure if they ever point to something, when I make a new one
   * I should also bring whatever this is over
   */
  new->lib_one = old->lib_one;
  new->lib_two = old->lib_two;
}

/**
 *  Description:
 *    sets a new sceduler for the 
 *    program, or returns to default
 *
 *  So this method was way harder than I thought it would be
 **/
void lwp_set_scheduler(scheduler fun)
{
  if (DEBUG) { printf("SETTING SCHEDULER\n"); fflush(stdout);}

  thread temp, remember, new;
  scheduler temp_sch = &chicken;

  if (fun != NULL && fun->init != NULL)
  {
    fun->init();
  }

  temp = kernal_sanders->next();
  remember = temp;

  while (temp != NULL)
  {
    new = (thread)malloc(THREAD_SIZE);
    new->stack = (unsigned long *)malloc(LONG_SIZE * temp->stacksize);
    
    /* error checking malloc */
    if (new == NULL || new->stack == NULL)
    {
      fprintf(stderr, "Failed to malloc data when switching schedulers\n");
      return;
    }

    copy_a_thread_over(temp, new);

    /* 
     * why does this order matter?  I could pass test case 24 without
     * cahnging the order.  Is it only for testing purposes
     */
    kernal_sanders->remove(temp);
    (fun != NULL) ? (fun->admit(new)) : (temp_sch->admit(new));
    temp = kernal_sanders->next();

    if (temp == remember)
    {
      break;
    }
  }

  if (fun != NULL && kernal_sanders->shutdown != NULL)
  {
    kernal_sanders->shutdown();
  }

  (fun != NULL) ? (kernal_sanders = fun) : (kernal_sanders = temp_sch);
  if (DEBUG) { printf("SET SCHEDULER\n"); fflush(stdout);}
}

/**
 *  Description:
 *    gets the current sceduler  
 *    for the program
 **/
scheduler lwp_get_scheduler(void)
{
  return kernal_sanders;
}

/**
 *  Description:
 *    Allows round robin scheduling setup
 *    I don't think I need anything in this method
 **/
void rr_init(void) {}

/**
 *  Description:
 *    Allows round robin scheduling setup
 *    to free every node in the list before exiting
 *
 *  Not Sure I want yet
 **/
void rr_shutdown(void) {}

/**
 *  Description:
 *    This method will add a new process to
 *    the round robbin list
 **/
void rr_admit(thread new)
{
  if (new == NULL)
  {
    return;
  }

  if (head == NULL)
  {
    tail = new;
    head = new;

    new->sched_one = NULL;
    new->sched_two = NULL;
    next = head;
  }
  else
  {
    new->sched_one = tail;
    new->sched_two = head;

    tail->sched_two = new;
    tail = new;

    head->sched_one = tail;
  }
}

/**
 *  Description:
 *    This method will remove a process
 *    from the round robbin list
 **/
void rr_remove(thread victim)
{
  int i = 0;
  thread temp = head;

  if (victim == NULL)
  {
    return;
  }

  while (temp != NULL && i <= give_lwp_id)
  {
    if (temp->tid == victim->tid)
    {
      /* If there is only 1 element in the list */
      if (tail->tid == head->tid)
      {
        head = NULL;
        tail = NULL;
        break;
      }
      /* if the head is the thread to remove */
      else if (head->tid == victim->tid)
      {
        head = head->sched_two;

        tail->sched_two = head;
        head->sched_one = tail;
        break;
      }
      /* if the tail is the thread to remove */
      else if (tail->tid == victim->tid)
      {
        tail = tail->sched_one;

        tail->sched_two = head;
        head->sched_one = tail;
      }
      /* if a node in the center is the thread to remove */
      else
      {
        temp->sched_one->sched_two = temp->sched_two;
        temp->sched_two->sched_one = temp->sched_one;
        break;
      }

      free(temp->stack);
      free(temp);
    }

    i++;
    temp = temp->sched_two;
  }
}

/**
 *  Description:
 *    This method will return a process
 *    from the round robbin list
 **/
thread rr_next(void)
{
  thread remember = NULL;

  if (head == NULL || next == NULL)
  {
    return NULL;
  }

  if (head->sched_two == NULL || head->sched_two->tid == head->tid)
  {
    return head;
  }
  else
  {
    remember = next;
    next = next->sched_two;
  }

  return remember;
}

/*
END Scheduler Methods
--------------------------------------------------------------------------------
*/

/**
 *  Description:
 *    This method will return a thread
 *    that corresponds to the given thread id
 *
 *  How can I be stuck on this method after everything else - FIXED
 *  Why can't we just count on the schedulers to be correct?
 **/
thread tid2thread(tid_t tid)
{
  if (DEBUG) { printf("FINDING THREAD\n"); fflush(stdout);}

  int i;

  for (i = 0; i < give_lwp_id; i++)
  {
    if (all_threads[i]->tid == tid)
    {
      return all_threads[i];
    }
  }

  /*
   * Old code that I liked more
   *
   * int i = 0;
   * thread temp = curr_thread;

   * while (temp != NULL && i <= give_lwp_id)
   * {
   *   if (temp->tid == tid)
   *   {
   *     return temp;
   *   }

   *   i++;
   *   temp = temp->sched_two;
   * }
   */

  if (DEBUG) { printf("FOUND THREAD\n"); fflush(stdout);}
  return NULL;
}




