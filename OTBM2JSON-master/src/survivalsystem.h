////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __SURVIVALTHREAD_H__
#define __SURVIVALTHREAD_H__

#if defined WIN32 || defined __WINDOWS__

#include <windows.h>
#include <process.h>    // Thread
#include <stddef.h>
#include <stdlib.h>
#include <conio.h>
#include <sys/timeb.h>

#include "definitions.h"

#define SURVIVALSYS_THREAD_RETURN  void
#define SURVIVALSYS_CREATE_THREAD(a, b) _beginthread(a, 0, b)
#define SURVIVALSYS_THREAD_LOCKVAR CRITICAL_SECTION
#define SURVIVALSYS_THREAD_LOCKVARINIT(a) InitializeCriticalSection(&a);
#define SURVIVALSYS_THREAD_LOCK(a)        EnterCriticalSection(&a);
#define SURVIVALSYS_THREAD_UNLOCK(a)      LeaveCriticalSection(&a);
#define SURVIVALSYS_THREAD_TIMEOUT			  WAIT_TIMEOUT
#define SURVIVALSYS_THREAD_SIGNALVARINIT(a) a = CreateEvent(NULL, FALSE, FALSE, NULL)
#define SURVIVALSYS_THREAD_SIGNAL_SEND(a)   SetEvent(a);

typedef HANDLE SURVIVALSYS_THREAD_SIGNALVAR;

inline __int64 SURVIVALSYS_TIME()
{
  _timeb t;
  _ftime(&t);
  return ((__int64)t.millitm) + ((__int64)t.time) * 1000;
}

inline int SURVIVALSYS_THREAD_WAITSIGNAL(SURVIVALSYS_THREAD_SIGNALVAR& signal, SURVIVALSYS_THREAD_LOCKVAR& lock)
{
  LeaveCriticalSection(&lock);
  WaitForSingleObject(signal, INFINITE);
  EnterCriticalSection(&lock);

  return -0x4711;
}

inline void SURVIVALSYS_SLEEP(uint32_t t){
	Sleep(t);
}


inline int SURVIVALSYS_THREAD_WAITSIGNAL_TIMED(SURVIVALSYS_THREAD_SIGNALVAR& signal, SURVIVALSYS_THREAD_LOCKVAR& lock, __int64 cycle)
{
  __int64 tout64 = (cycle - SURVIVALSYS_TIME());
  
  DWORD tout = 0;
  if (tout64 > 0)
    tout = (DWORD)(tout64);

  LeaveCriticalSection(&lock);
  int ret = WaitForSingleObject(signal, tout);
  EnterCriticalSection(&lock);

  return ret;
}

typedef int socklen_t;

#else  // #if defined WIN32 || defined __WINDOWS__

#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>


#define SURVIVALSYS_THREAD_RETURN void*

inline void SURVIVALSYS_CREATE_THREAD(void *(*a)(void*), void *b)
{
  pthread_attr_t attr;
  pthread_t id;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
  pthread_create(&id, &attr, a, b);
}

typedef pthread_mutex_t SURVIVALSYS_THREAD_LOCKVAR;

inline void SURVIVALSYS_THREAD_LOCKVARINIT(SURVIVALSYS_THREAD_LOCKVAR& l) {
		  pthread_mutexattr_t attr;
		  pthread_mutexattr_init(&attr);
		  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
		  pthread_mutex_init(&l, &attr);
}
#define SURVIVALSYS_THREAD_LOCK(a)          pthread_mutex_lock(&a);
#define SURVIVALSYS_THREAD_UNLOCK(a)        pthread_mutex_unlock(&a);
#define SURVIVALSYS_THREAD_TIMEOUT			  ETIMEDOUT
#define SURVIVALSYS_THREAD_SIGNALVARINIT(a) pthread_cond_init(&a, NULL);
#define SURVIVALSYS_THREAD_SIGNAL_SEND(a)   pthread_cond_signal(&a);

typedef pthread_cond_t SURVIVALSYS_THREAD_SIGNALVAR;

inline void SURVIVALSYS_SLEEP(int t)
{
  timespec tv;
  tv.tv_sec  = t / 1000;
  tv.tv_nsec = (t % 1000)*1000000;
  nanosleep(&tv, NULL);
}

inline __int64 SURVIVALSYS_TIME()
{
  timeb t;
  ftime(&t);
  return ((__int64)t.millitm) + ((__int64)t.time) * 1000;
}

#define SURVIVALSYS_THREAD_WAITSIGNAL(a,b) pthread_cond_wait(&a, &b)

inline int SURVIVALSYS_THREAD_WAITSIGNAL_TIMED(SURVIVALSYS_THREAD_SIGNALVAR& signal, SURVIVALSYS_THREAD_LOCKVAR& lock, __int64 cycle) {
		  timespec tv;
		  tv.tv_sec = (__int64)(cycle / 1000);
		  // tv_nsec is in nanoseconds while we only store microseconds...
		  tv.tv_nsec = (__int64)(cycle % 1000) * 1000000;
		  return pthread_cond_timedwait(&signal, &lock, &tv);
}


#ifndef SOCKET
#define SOCKET int
#endif

#ifndef closesocket
#define closesocket close
#endif


#endif // #if defined WIN32 || defined __WINDOWS__




#endif // #ifndef __SURVIVALTHREAD_H__
