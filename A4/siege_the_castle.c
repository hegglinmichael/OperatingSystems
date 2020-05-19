/**
*	Author: Michael Hegglin
*	Date: May 22, 2020
*	Description: 
*		This is a minix device driver
*		written for Operating Systems at Cal Poly
**/

#include <minix/drivers.h>
#include <minix/driver.h>
#include <sys/ioc_secret.h>
#include <stdio.h>
#include <stdlib.h>
#include <minix/ds.h>
#include "secret.h"
#include <sys/ucred.h>
#include <unistd.h>

#define SECRET_SIZE 8192
#define RBIT 0000004
#define WBIT 0000002

/*
--------------------------------------------------------------------------------
START SECRET FUNCTION DECLARATIONS
*/

FORWARD _PROTOTYPE( char * secret_name, (void) );
FORWARD _PROTOTYPE( int secret_open, (struct driver *d, message *m) );
FORWARD _PROTOTYPE( int secret_close, (struct driver *d, message *m) );
FORWARD _PROTOTYPE( void secret_geometry, (struct partition *entry) );
FORWARD _PROTOTYPE( struct device * secret_prepare, (int dev) );
FORWARD _PROTOTYPE( int secret_transfer, (
                        int procnr,
                        int opcode,
                        u64_t position,
                        iovec_t *iov,
                        unsigned nr_req
                     ) );

FORWARD _PROTOTYPE( int secret_ioctl, (struct driver *d, message *m) );

/*
END SECRET FUNCTION DECLARATIONS
--------------------------------------------------------------------------------
START SEF FUNCTION DECLARATIONS
*/

FORWARD _PROTOTYPE( int lu_state_restore, (void) );
FORWARD _PROTOTYPE( void sef_local_startup, (void) );
FORWARD _PROTOTYPE( int sef_cb_init, 
                  ( int type, sef_init_info_t *UNUSED(info)) );
FORWARD _PROTOTYPE( int sef_cb_lu_state_save, 
                  ( int UNUSED(state) ) );

/*
END SEF FUNCTION DECLARATION
--------------------------------------------------------------------------------
START PROG VARIABLES
*/

/* Counts the number of times the device has been opened */
PRIVATE int open_counter;

PRIVATE struct device secret_device;  /* Represents the /dev/Secret device */
PRIVATE char secret_data[SECRET_SIZE];  /* Buffer to hold the message */
PRIVATE char * secret_ptr = secret_data;
PRIVATE uid_t grantee;  /* contains the file owner's UID */

/* comparitor to see if file is owned by anyone */
uid_t empty;
PRIVATE int someone_writing = 0;

/* the distance between the below is how much secret I have */
PRIVATE int last_read;  /* Last read byte by the owner */
PRIVATE int last_written; /* Last written byte by the owner */

/* Entry points to the secret driver. */
PRIVATE struct driver secret_tab =
{
  secret_name,
  secret_open,
  secret_close,
  secret_ioctl,
  secret_prepare,
  secret_transfer,
  nop_cleanup,
  secret_geometry,
  nop_alarm,
  nop_cancel,
  nop_select,
  secret_ioctl,
  do_nop,
};


/*
END PROG VARIABLES
--------------------------------------------------------------------------------
START MAIN
*/

PUBLIC int main(void)
{
  /* Initialize SEF */
  sef_local_startup();

  /* runs the main loop */
  driver_task(&secret_tab, DRIVER_STD);
  return OK;
}

/*
END MAIN
--------------------------------------------------------------------------------
START SEF FUNCTIONS
*/

/**
 *  Description:
 *    Register SEF Callbacks and initializes them,
 *    Means that initial execution will always be the same
 *    for: fresh, live update, restart
 *
 *    includes callbacks for live updates as well
 *    if event is triggered by RS(Reincarnation server)
 *    RS will send message to old state to prepare
 **/
PRIVATE void sef_local_startup()
{
  /**
   *  Register init callbacks. Use the same function for all event types
   **/
  sef_setcb_init_fresh(sef_cb_init);
  sef_setcb_init_lu(sef_cb_init);
  sef_setcb_init_restart(sef_cb_init);

  /**
   *  Register live update callbacks
   *    - Agree to update immediately when LU is requested in a valid state.
   *    - Support live update starting from any standard state.
   *    - Register a custom routine to save the state.
   **/
  sef_setcb_lu_prepare(sef_cb_lu_prepare_always_ready);
  sef_setcb_lu_state_isvalid(sef_cb_lu_state_isvalid_standard);
  sef_setcb_lu_state_save(sef_cb_lu_state_save);

  /**
   *  Let SEF perform startup.
   **/
  sef_startup();
}

/**
 *  Description:
 *    This initializes the secret driver
 *    It holds the exact instructions for what to
 *    do when the following states happen:
 *      - initialization
 *      - live update
 *      - restart
 **/
PRIVATE int sef_cb_init(int type, sef_init_info_t *UNUSED(info))
{
  int do_announce_driver = TRUE;
  open_counter = 0;

  switch(type)
  {
    case SEF_INIT_FRESH:
      printf("secret driver is starting fresh\n");
      last_written = 0;
      last_read = 0;
      empty = -1;
      grantee = empty;
      break;

    case SEF_INIT_LU:
      lu_state_restore();
      do_announce_driver = FALSE;
      printf("secret driver has had a live update\n");
      break;

    case SEF_INIT_RESTART:
      printf("secret driver as just restarted\n");
      break;
  }

  /* Announce that we are up when necessary */
  if (do_announce_driver) 
  {
    driver_announce();
  }

  /* initialization was successful */
  return OK;
}

/**
 *  Description:
 *    Restore the state by restoring variables:
 *      - open_counter
 *      - last_read
 *      - last_written
 *      - grantee
 *      - secret
 **/
PRIVATE int lu_state_restore()
{
  char temp[SECRET_SIZE];
  int curr_size;
  u32_t value;

  curr_size = last_written - last_read;

  ds_retrieve_u32("open_counter", &value);
  ds_delete_u32("open_counter");
  open_counter = (int) value;

  ds_retrieve_u32("someone_writing", &value);
  ds_delete_u32("someone_writing");
  someone_writing = (int) value;

  ds_retrieve_u32("last_read", &value);
  ds_delete_u32("last_read");
  last_read = (int) value;

  ds_retrieve_u32("last_written", &value);
  ds_delete_u32("last_written");
  last_written = (int) value;

  ds_retrieve_str("secret_ptr", secret_ptr, curr_size);
  ds_delete_str("secret_ptr");

  ds_retrieve_u32("grantee", &value);
  ds_delete_u32("grantee");
  grantee = (uid_t) value;

  return OK;
}

/**
 *  Description:
 *    Saves the variables by publishing them to the 
 *    Data store
 *      - open_counter
 *      - last_read
 *      - last_written
 *      - grantee
 *      - secret
 **/
PRIVATE int sef_cb_lu_state_save(int UNUSED(state))
{
  ds_publish_u32("open_counter", open_counter, DSF_OVERWRITE);
  ds_publish_u32("someone_writing", someone_writing, DSF_OVERWRITE);
  ds_publish_u32("last_read", last_read, DSF_OVERWRITE);
  ds_publish_u32("last_written", last_written, DSF_OVERWRITE);
  ds_publish_str("secret_ptr", secret_ptr, DSF_OVERWRITE);
  ds_publish_u32("grantee", grantee, DSF_OVERWRITE);
  return OK;
}

/*
END SEF FUNCTIONS
--------------------------------------------------------------------------------
START SECRET FUNCTIONS
*/

/**
 *  Description:
 *    returns the name of the device
 **/
PRIVATE char * secret_name()
{
  printf("secret_name()\n");
  return "secret";
}

/**
 *  Description:
 *    
 *    
 **/
PRIVATE int secret_open(struct driver *d, message *m)
{
  /* I think I want to put the message into 
  the buffer here, not sure.  Definitely want to
  check who the user is */
  
  struct ucred u;
  int success;

  printf("secret_open()\n");
  success = getnucred(m->IO_ENDPT, &u);

  if ((m->COUNT & WBIT) == WBIT)
  { /* process is trying to write */
    if (someone_writing == 0)
    {
      someone_writing = 1;
      grantee = u.uid;
    }
    else if (someone_writing && u.uid != grantee)
    {
      return EACCES;
    }
    else
    {
      return ENOSPC;
    }
  }
  else if ((m->COUNT & RBIT) == RBIT)
  { /* process is trying to read */
    if (someone_writing == 0) 
    { 
      return OK; 
    }
    else if (someone_writing == 1 && grantee != u.uid) 
    { 
      return EACCES; 
    }
    else if (grantee == u.uid) 
    { 
      grantee = empty; 
      someone_writing = 0;
    }
  }
  else
  { /* user is trying to read and write */
    return EACCES;
  }

/*
  switch(m->COUNT)
  {
    case 2: 
      if (someone_writing == 0)
      {
        someone_writing = 1;
        grantee = u.uid;
      }
      else if (someone_writing && u.uid != grantee)
      {
        return EACCES;
      }
      else
      {
        return ENOSPC;
      }

      break;

    case 4: 
      if (someone_writing == 0) 
      { 
        return OK; 
      }
      else if (someone_writing == 1 && grantee != u.uid) 
      { 
        return EACCES; 
      }
      else if (grantee == u.uid) 
      { 
        grantee = empty; 
        someone_writing = 0;
      }
      break;
    case 6: 
      return EACCES;
  }*/

  return OK;
}

/**
 *  Description:
 *    If no one is writing a secret,
 *    delete the secret information, then exit
 **/
PRIVATE int secret_close(struct driver *d, message *m)
{
  printf("secret_close()\n");

  if (!someone_writing)
  {
    memset(secret_ptr, 0, SECRET_SIZE);
  }

  return OK;
}

/**
 *  Description:
 *    Apparently nothing is going to use the variables set here
 *    They numbers represent the geometry of the device 
 **/
PRIVATE void secret_geometry(struct partition *entry)
{
  printf("hello_geometry()\n");
  entry->cylinders = 0;
  entry->heads = 0;
  entry->sectors = 0;
}

/**
 *  Description:
 *    Apparently nothing is going to use the variables set here
 *    They numbers represent the geometry of the device 
 **/
PRIVATE struct device * secret_prepare(int dev)
{
  secret_device.dv_base = make64(0, 0);
  secret_device.dv_size = make64(SECRET_SIZE, 0);
  return &secret_device;
}

/**
 *  Description:
 *    This method transfers data in and out of the
 *    device.  
 *
 *CHANGE: I need to write the incoming transfer
 **/
PRIVATE int secret_transfer(
    int procnr,
    int opcode,
    u64_t position,
    iovec_t *iov,
    unsigned nr_req
  )
{
  int ruh_roh = 0;
  int bytes, ret, curr_size;
  curr_size = last_written - last_read;

  if (nr_req != 1)
  {
    printf("The thing that shouldn't trigger did\n");
  }

  /* If we are reading */
  if (opcode == DEV_GATHER_S)
  {
    printf("secret_transfer(): READ\n");
    bytes = curr_size < iov->iov_size ?
            curr_size : iov->iov_size;
  }
  else if (opcode = DEV_SCATTER_S) /* If we are writing */
  {
    printf("secret_transfer(): WRITE\n");
    /* test for overflow */
    if (curr_size + iov->iov_size <= SECRET_SIZE)
    {
      bytes = iov->iov_size;
    } 
    else 
    {
      printf("Ruh Roh\n");
      ruh_roh = 1;
      bytes = SECRET_SIZE - curr_size;
    }
  }
  
  if (bytes <= 0) { return OK; }

  switch(opcode)
  {
    case DEV_GATHER_S:
      someone_writing = 0;
      ret = sys_safecopyto(
          procnr,
          (cp_grant_id_t) iov->iov_addr,
          0,
          (vir_bytes) (secret_ptr + position.lo),
          bytes, D
        );

      last_read += bytes;
      break;

    case DEV_SCATTER_S:
      someone_writing = 1;
      ret = sys_safecopyfrom(
          procnr,
          (cp_grant_id_t) iov->iov_addr,
          0,
          (vir_bytes) (secret_ptr + curr_size),
          bytes, D
        );

        last_written += bytes;
      break;

    default:
      return EINVAL;
  }

  iov->iov_size -= bytes;

  if (ruh_roh) { return ENOSPC; }
  return ret;
}

/*
END SECRET FUNCTIONS
--------------------------------------------------------------------------------
START MISC
*/

PRIVATE int secret_ioctl(struct driver *d, message *m)
{
  uid_t new_usr; 
  int success, res;
  struct ucred u;

  success = getnucred(m->IO_ENDPT, &u);
  res = sys_safecopyfrom(
      m->IO_ENDPT, 
      (vir_bytes)m->IO_GRANT,
      0, 
      (vir_bytes) &new_usr, 
      sizeof(new_usr), D
    );

  if (m->REQUEST == SSGRANT && u.uid == grantee) 
  {
    grantee = new_usr;
    return OK;
  }

  return ENOTTY;
}

/*
END MISC
--------------------------------------------------------------------------------
*/






























