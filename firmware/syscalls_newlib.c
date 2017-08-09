
//MIT License
//
//Copyright (c) 2017 tvelliott
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <errno.h>
#include "syscalls_newlib.h"


extern int32_t _heap;
static unsigned char *heap_ptr = NULL;
//int errno;
//int _errno;

//calling pow() function causes errors with newlib
//including <errno.h> didn't help
//newlib was complaining about __error not being defined
//no idea if this is right, but compiles
//int32_t error = 0;
//int * __errno( void )
//{
// return &error;
//}

//////////////////////////////////////////////////////////////
//  this is used by libc (newlib) when calling malloc / free
//  the heap area is defined in main.ld
//////////////////////////////////////////////////////////////
caddr_t _sbrk(int32_t incr)
{
  unsigned char *prev_heap;

  if(heap_ptr == NULL) {
    heap_ptr = (unsigned char *)&_heap;
  }

  prev_heap = heap_ptr;
  heap_ptr += incr;

  return (caddr_t) prev_heap;
}

//    sbrk
//    Increase program data space.
//    As malloc and related functions depend on this, it is useful to have a working implementation.
//
//    The following suffices for a standalone system;
//    it exploits the symbol end automatically defined by the GNU linker.
//
//    caddr_t sbrk(int incr){
//      extern char end;    /* Defined by the linker */
//      static char *heap_end;
//      char *prev_heap_end;
//
//      if (heap_end == 0) {
//        heap_end = &end;
//      }
//     prev_heap_end = heap_end;
//      if (heap_end + incr > &_stack)
//        {
//          _write (1, "Heap and stack collision\n", 25);
//          abort ();
//        }
//
//      heap_end += incr;
//      return (caddr_t) prev_heap_end;
//    }


int _close(int file)
{
  return -1;
}

char *__env[1] = { 0 };
char **environ = __env;

int _execve(char *name, char **argv, char **env)
{
  errno=ENOMEM;
  return -1;
}

int _fork()
{
  errno=EAGAIN;
  return -1;
}

int _fstat(int file, struct stat *st)
{
  //st->st_mode = S_IFCHR;
  return 0;
}

int _getpid()
{
  return 1;
}

int _isatty(int file)
{
  return 1;
}

int _kill(int pid, int sig)
{
  errno=EINVAL;
  return(-1);
}

int _link(char *old, char *new)
{
  errno=EMLINK;
  return -1;
}

void _exit(int status)
{
}

int _lseek(int file, int ptr, int dir)
{
  return 0;
}

int _open(const char *name, int flags, int mode)
{
  return -1;
}

int _read(int file, char *ptr, int len)
{
  return 0;
}


//int _stat(const char *file, struct stat *st)
//{
// st->st_mode = S_IFCHR;
//return 0;
//}

//clock_t _times(struct tms *buf)
//{
// return -1;
//}

int _unlink(char *name)
{
  errno=ENOENT;
  return -1;
}

int _wait(int *status)
{
  errno=ECHILD;
  return -1;
}

int _write(int file, char *ptr, int len)
{
  //int todo;

  //for (todo = 0; todo < len; todo++) {
  //   writechar(*ptr++);
  //}
  //return len;
  return 0;
}

void abort()
{
  //do_system_reset();
  while(1);//don't know, force wdt
}
