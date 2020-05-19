
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioc_secret.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


int main(int argc, char ** argv)
{
  int fd, res, uid;
  char * a = "apples";

  fd = open("/dev/secret", O_WRONLY);
  printf("Opening...  fd=%d\n",fd);

  res = write(fd, a, strlen(a));
  printf("Writing...  res=%d\n",res);

  /* try grant */
  if( argc > 1 && 0 != ( uid = atoi(argv[1]) ) )
  {
    if( res = ioctl(fd, SSGRANT, &uid) )
      perror("ioctl");

    printf("Trying to change owner to %d...res=%d\n",uid, res);
  }

  res = close(fd);
  return 0;
}

