/**
 *  Author: Michael Hegglin
 *  Date: 04/15/2020
 *    
 *  Description: 
 *    This library contains my own versions 
 *    of malloc, calloc, free, and realloc
 *    along with several helper methods
 *

 #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
 **/

#include "big_bird.h"

struct chunk_hdr * head = NULL;

/**
 *  Description:
 *    prints out data for
 *    debugging without using
 *    system's malloc
 **/
void debugging(char * debug_out)
{
  char buf[100];
  snprintf(buf, 100, "%s\n", debug_out);
  fputs(buf, stderr);
}

/**
 *  Description:
 *    This function gets
 *    more memory using sbrk and 
 *    returns a pointer to the start of it
 **/
void *get_more_memory(int multiple)
{
  void * sbrk_request;
  char * str_debug = "get_more_memory() Error\n";

  sbrk_request = sbrk(CHUNK_SIZE * multiple);

  if (sbrk_request == (void *) -1) 
  {
      debugging(str_debug);
      return NULL;
    }

    return sbrk_request;
}

int get_multiple(size)
{
  int multiple = 1;

  while ((multiple * CHUNK_SIZE) < size)
  {
    multiple++;
  }

  return multiple;
}

/**
 *  Description:
 *    allocates memory like the std
 *    calloc c library (Or, I guess I tried to)
 **/
void *calloc(size_t nmemb, size_t size)
{
  char buf[100];
  size_t temp = size * nmemb;
  void * ptr = malloc(temp);
  memset(ptr, 0, temp);

  if (getenv("DEBUG_MALLOC") != NULL)
  {
    snprintf(buf, 100, "MALLOC: calloc(%zd, %zd)      =>  (ptr=%p, size=%zd)",
              nmemb, size, ptr, temp);
    debugging(buf);
  }

  return ptr;
}

/**
 *  Description:
 *    reallocated memory like the std
 *    realloc c library (Or, I guess I tried to)
 **/
void *realloc(void *ptr, size_t size)
{
  char buf[100];
  struct chunk_hdr * temp_node;
  struct chunk_hdr * new_node;
  void * last_resort_malloc;

  if (size == 0)
  {
    free(ptr);
  }

  if (ptr == NULL)
  {
    last_resort_malloc = malloc(size);

    if (getenv("DEBUG_MALLOC") != NULL)
    {
      snprintf(buf,100, "MALLOC: realloc(%p, %zd)      =>  (ptr=%p, size=%zd)",
              ptr, size, last_resort_malloc, size);
      debugging(buf);
    }

    return last_resort_malloc;
  }

  temp_node = head;
  while (temp_node != NULL)
  {
    /* here we are just finding the header that corresponds to the ptr */
    if ((void *)temp_node <= ptr && 
      ptr < (void *)((char *)temp_node + HEADER_SIZE + temp_node->size))
    {
      /* The current temp_node is the header that contains the
         chunk of memory that we want.   */

      if (size > temp_node->size)
      {
        /* the new space in memory has to
           be bigger than the previous space in memory

           1) we check the surrounding nodes to see if
              we can free up enough space next to us 
              to not copy data

           2) we go through the list to see if there 
              is a free spot that fits our data size 
              if there is copy data over to it

           3) we try to malloc more data to create 
              a new space and copy current data over
        */

        /* 1) see if neighbors have enough space to 
              make things work */
        if (temp_node->next->free_flag == FREE &&
          (temp_node->next->size + temp_node->size + HEADER_SIZE) >= size)
        {

          /* Freeing the free chunk in front of it will create
             enough space for the realloc */

          temp_node->size += HEADER_SIZE + temp_node->next->size;

          if (temp_node->next->next != NULL)
          {
            temp_node->next = temp_node->next->next;
            temp_node->next->prev = temp_node;
          }
          else
          {
            new_node = (struct chunk_hdr *)
                       ((char *)temp_node + HEADER_SIZE + size);
            new_node->size = temp_node->size - HEADER_SIZE - size;
            new_node->free_flag = FREE;

            temp_node->size = size;

            temp_node->next = new_node;
            new_node->prev = temp_node;
          }
          
          if (getenv("DEBUG_MALLOC") != NULL)
          {
            snprintf(buf,100, 
              "MALLOC: realloc(%p, %zd)      =>  (ptr=%p, size=%zd)",
              ptr, size, ((void *)((char *)temp_node + HEADER_SIZE)), size);
            debugging(buf);
          }

          return ((void *)((char *)temp_node + HEADER_SIZE));
        }

        else if (temp_node->prev != NULL &&
          temp_node->prev->free_flag == FREE &&
          (temp_node->prev->size + temp_node->size + HEADER_SIZE) >= size)

        {
          /* Freeing the free chunk behind it will create
             enough space for the realloc */

          temp_node->prev->free_flag = NOT_FREE;
          temp_node->prev->size += HEADER_SIZE + temp_node->size;
          temp_node->prev->next = temp_node->next;
          temp_node->next->prev = temp_node->prev;

          memcpy((char *)temp_node->prev + HEADER_SIZE, 
            (char *)temp_node + HEADER_SIZE, temp_node->size);

          if (getenv("DEBUG_MALLOC") != NULL)
          {
            snprintf(buf, 100, 
              "MALLOC: realloc(%p, %zd)      =>  (ptr=%p, size=%zd)",
              ptr, size, 
              ((void *)((char *)temp_node->prev + HEADER_SIZE)), size);
            debugging(buf);
          }


          return ((void *)((char *)temp_node->prev + HEADER_SIZE));
        }

        else if (temp_node->prev != NULL &&
          temp_node->prev->free_flag == FREE &&
          temp_node->next->free_flag == FREE &&
          (temp_node->prev->size + temp_node->size + 
            temp_node->next->size + 2 * HEADER_SIZE) >= size)

        {
          /* Freeing the free chunk behind it and in 
             front of it will create enough space for 
             the realloc */

          temp_node->size += HEADER_SIZE + temp_node->next->size;

          if (temp_node->next->next != NULL)
          {
            temp_node->next = temp_node->next->next;
            temp_node->next->prev = temp_node;
          }
          else
          {
            new_node = (struct chunk_hdr *)
                       ((char *)temp_node + HEADER_SIZE + size);
            new_node->size = temp_node->size - HEADER_SIZE - size;
            new_node->free_flag = FREE;

            temp_node->size = size;

            temp_node->next = new_node;
            new_node->prev = temp_node;
          }

          temp_node->prev->free_flag = NOT_FREE;
          temp_node->prev->size += HEADER_SIZE + temp_node->size;
          temp_node->prev->next = temp_node->next;
          temp_node->next->prev = temp_node->prev;

          memcpy((char *)temp_node->prev + HEADER_SIZE, 
            (char *)temp_node + HEADER_SIZE, temp_node->size);

          if (getenv("DEBUG_MALLOC") != NULL)
          {
            snprintf(buf, 100, 
              "MALLOC: realloc(%p, %zd)      =>  (ptr=%p, size=%zd)",
              ptr, size, 
              ((void *)((char *)temp_node->prev + HEADER_SIZE)), size);
            debugging(buf);
          }

          return ((void *)((char *)temp_node->prev + HEADER_SIZE));
        }

        /* 2) go through the list to check for spaces */
        new_node = head;
        while (new_node != NULL)
        {
          if (new_node->free_flag == FREE && new_node->size >= size)
          {
            new_node->free_flag = NOT_FREE;

            memcpy((void *)((char *)new_node + HEADER_SIZE), 
              (char *)temp_node + HEADER_SIZE, temp_node->size);

            if (getenv("DEBUG_MALLOC") != NULL)
            {
              snprintf(buf, 100, 
                "MALLOC: realloc(%p, %zd)      =>  (ptr=%p, size=%zd)",
                ptr, size, 
                ((void *)((char *)new_node + HEADER_SIZE)), size);
              debugging(buf);
            }

            return ((void *)((char *)new_node + HEADER_SIZE));
          }

          new_node = new_node->next;
        }

        /* 3) call malloc for more memory */
        /* malloc should automaticall link
           this new node to the end of the list 
           and return us the spot we eventually want
           to return after copying data over*/

        last_resort_malloc = malloc(size);
        
        if (last_resort_malloc != NULL)
        {
          memcpy(last_resort_malloc, 
          (char *)temp_node + HEADER_SIZE, temp_node->size);

          if (getenv("DEBUG_MALLOC") != NULL)
          {
            snprintf(buf,100, 
              "MALLOC: realloc(%p, %zd)      =>  (ptr=%p, size=%zd)",
              ptr, size, last_resort_malloc, size);
            debugging(buf);
          }

          return last_resort_malloc;
        }
        else
        {
          if (getenv("DEBUG_MALLOC") != NULL)
          {
            snprintf(buf,100, 
              "MALLOC: realloc(%p, %zd)      =>  (ptr=%p, size=%zd)",
              ptr, size, (void *)((char *)temp_node + HEADER_SIZE), size);
            debugging(buf);
          }

          return (void *)((char *)temp_node + HEADER_SIZE);
        }
      }
      else if (size < temp_node->size)
      {
        /* the new space in memory has to
           be smaller than the previous space in memory
           currently I don't do this */

        if (getenv("DEBUG_MALLOC") != NULL)
        {
          snprintf(buf,100, 
            "MALLOC: realloc(%p, %zd)      =>  (ptr=%p, size=%zd)",
            ptr, size, (void *)((char *)temp_node + HEADER_SIZE), size);
          debugging(buf);
        }

        return (void *)((char *)temp_node + HEADER_SIZE);
      }
      else
      {
        if (getenv("DEBUG_MALLOC") != NULL)
        {
          snprintf(buf,100, 
            "MALLOC: realloc(%p, %zd)      =>  (ptr=%p, size=%zd)",
            ptr, size, (void *)((char *)temp_node + HEADER_SIZE), size);
          debugging(buf);
        }
        /* the user has requested the same size buffer,
           so just return a pointer to the beginning again */
        return (void *)((char *)temp_node + HEADER_SIZE);
      }
    }

    temp_node = temp_node->next;
  }

  return NULL;
}

/**
 *  Description:
 *    allocates memory like the std
 *    malloc c library (Or, I guess I tried to)
 **/
void *malloc(size_t size)
{
  char buf[100];
  int multiple;
  void * hunk_of_memory;
  struct chunk_hdr * new_node;
  struct chunk_hdr * temp_node;

  if (size <= 0)
  {
    return NULL;
  }

  /* round size of to a multiple of 32 */
  while (size % HEADER_SIZE != 0)
  {
    size++;
  }

  if (head == NULL)
  {
    /* if no memory has been allocated yet
       allocate some for the program,
       then add a header to the start of 
       this new memory. */

    /* find out if you need to take a 
       bigger chunk of memory on start */
    multiple = get_multiple(size);

    hunk_of_memory = get_more_memory(multiple+HEADER_SIZE+SPACE_FOR_NEW_NODE);
    new_node = hunk_of_memory;
    head = new_node;

    new_node->free_flag = NOT_FREE;
    new_node->size = size;
    new_node->prev = NULL;

    hunk_of_memory = ((char *)hunk_of_memory + HEADER_SIZE);

    /* now that there is some memory lopped off
       to help keep track, create a new header
       that contains the remaining amount of memory
       you currently have and say that space is free.
       Then link the head node to this free space node */

    temp_node = (struct chunk_hdr *)((char *)hunk_of_memory + size);

    temp_node->free_flag = FREE;
    temp_node->size = (multiple * CHUNK_SIZE) - HEADER_SIZE - size;

    new_node->next = temp_node;
    temp_node->prev = new_node;
    temp_node->next = NULL;
  }
  else
  {
    /* 
    If memory has already been 
    allocated in the prgram there
    are 2 cases:

    1) look through the linked list for a 
       spot <= the size of the current spot.
       Unless more memory is needed, the last 
       header will hold the rest of the current chunk
       of memory which should always be big enough

       reformat the header, and if necessary create
       a new tail header containing the size of the
       rest of the unused memory

    2) use sbrk to lop off even more memory
       for the program
    */

    /* 1) look in the linked list */
    temp_node = head;
    while (temp_node != NULL)
    {
      if (temp_node->free_flag == FREE && temp_node->size >= size)
      {
        if (temp_node->next == NULL)
        {
          /* this header is the tail header
             and a new tail header must be made
             or sbrk must be called in order to 
             make a new tail header */

          if (temp_node->size >= size + SPACE_FOR_NEW_NODE + HEADER_SIZE)
          {
            /* This means that there is still room in the
               remaining memory to add another header as
               the new tail */

            temp_node->free_flag = NOT_FREE;

            new_node = (struct chunk_hdr *)
                       ((char *)temp_node + HEADER_SIZE + size);
            new_node->free_flag = FREE;
            new_node->size = temp_node->size - size - HEADER_SIZE;

            temp_node->size = size;
            new_node->prev = temp_node;
            temp_node->next = new_node;
            new_node->next = NULL;
          }
          else
          {
            /* This means that we need to call sbrk
               again to get more memory allocated to us.
               we will use this new memory for a new tail,
               and for the current malloc request */
            /*multiple = get_multiple(size + HEADER_SIZE 
            + SPACE_FOR_NEW_NODE);    Error is here 
            hunk_of_memory = get_more_memory(multiple);

            new_node
            temp_node->free_flag = FREE;

            new_node = (struct chunk_hdr *)hunk_of_memory;
            new_node->free_flag = NOT_FREE;
            new_node->size = CHUNK_SIZE * multiple;*/
            break;
          }
        }
        else
        {
          /* This is a random spot that is
             free and that we can use*/

          temp_node->free_flag = NOT_FREE;
        }

        /* returning plus one header because we need to 
           return the memory chunk that comes after 
           the header.  I want this to increment by 32 bytes
           for both types of systems */
        if (getenv("DEBUG_MALLOC") != NULL)
        {
          snprintf(buf, 100, "MALLOC: malloc(%zd)      =>  (ptr=%p, size=%zd)",
                    size, ((void *)((char *)temp_node + HEADER_SIZE)), size);
          debugging(buf);
        }

        return ((void *)((char *)temp_node + HEADER_SIZE));
      }

      temp_node = temp_node->next;
    }

    /* 2) lop off another chunk of data from the sbrk chunk
          beacuse currently there is no chunk big enough 
          to handle the requested malloc */

    /* we are going to get the last node of
       the current list here before calling
       sbrk */

    temp_node = head;
    while (temp_node->next != NULL)
    {
      temp_node = temp_node->next;
    }

    multiple = get_multiple(size + HEADER_SIZE + SPACE_FOR_NEW_NODE);
    hunk_of_memory = get_more_memory(multiple);

    new_node = (struct chunk_hdr *)hunk_of_memory;
    new_node->size = size;
    new_node->free_flag = NOT_FREE;

    temp_node->next = new_node;
    new_node->prev = temp_node;

    temp_node = (struct chunk_hdr *)((char *)new_node + HEADER_SIZE + size);
    temp_node->free_flag = FREE;
    temp_node->size = (multiple * CHUNK_SIZE) - HEADER_SIZE - size;

    temp_node->prev = new_node;
    new_node->next = temp_node;
    temp_node->next = NULL;
  }

  if (getenv("DEBUG_MALLOC") != NULL)
  {
    snprintf(buf, 100, "MALLOC: malloc(%zd)      =>  (ptr=%p, size=%zd)",
             size, ((void *)((char *)new_node + HEADER_SIZE)), size);
    debugging(buf);
  }

  return ((void *)((char *)new_node + HEADER_SIZE));
}

/**
 *  Description:
 *    frees space in memory.
 *    if the pointer is not
 *    pointing at the start I find it
 *    (Or, I guess I tried to)
 **/
void free(void *ptr)
{
  char buf[100];
  struct chunk_hdr * temp_node = head;

  if (ptr == NULL)
  {
    return;
  }

  if (getenv("DEBUG_MALLOC") != NULL)
  {
    snprintf(buf, 100, "MALLOC: free(%p)", ptr);
    debugging(buf);
  }

  while (temp_node != NULL)
  {
    if ((void *)temp_node <= ptr && 
      ptr < (void *)((char *)temp_node + HEADER_SIZE + temp_node->size))
    {
      /* if the above is true, then the ptr is in this
         chunk of memory and we can free it */

      temp_node->free_flag = FREE;

      /* Now we need to handle the case of
         fragmentation and check the surrounding
         nodes to see if they're also free.  This
         way we can combine memory chunks */

      if (temp_node->next != NULL && temp_node->next->free_flag == FREE)
      {
        /* the next space is free and we can combine it */
        temp_node->size += temp_node->next->size + HEADER_SIZE;
        temp_node->next = temp_node->next->next;

        if (temp_node->next != NULL)
        {
          temp_node->next->prev = temp_node;
        }
      }

      if (temp_node->prev != NULL && temp_node->prev->free_flag == FREE)
      {
        /* the prev space is free and we can combine it */
        temp_node->prev->size += temp_node->size + HEADER_SIZE;
        temp_node->prev->next = temp_node->next;

        if (temp_node->next != NULL)
        {
          temp_node->next->prev = temp_node->prev;
        }
      }

      return;
    }

    temp_node = temp_node->next;
  }
}
















