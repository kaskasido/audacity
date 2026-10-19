// -*- mode: c++ -*-
// Threads fuer MULTITHREADED-sbsms.
// POSIX: die echten pthreads. Windows (MSVC hat kein pthread.h) oder mit
// SBSMS_STD_THREADS: die kleine Teilmenge der pthread-API, die sbsms nutzt,
// nachgebaut mit std::thread, std::mutex und std::condition_variable (C++11).
#ifndef SBSMS_PTHREAD_H
#define SBSMS_PTHREAD_H

#if !defined(_WIN32) && !defined(SBSMS_STD_THREADS)
#include <pthread.h>
#else

#include <condition_variable>
#include <mutex>
#include <thread>

namespace _sbsms_ {

typedef std::thread *pthread_t;
typedef std::mutex pthread_mutex_t;
typedef std::condition_variable pthread_cond_t;

inline int pthread_create(pthread_t *t, const void *, void *(*fn)(void *), void *arg)
{
  *t = new std::thread(fn, arg);
  return 0;
}

inline int pthread_join(pthread_t t, void **)
{
  t->join();
  delete t;
  return 0;
}

// sbsms ruft pthread_exit(NULL) nur direkt vor "return NULL" am Ende der
// Thread-Funktionen; dort ist ein normales Zurueckkehren gleichwertig.
inline void pthread_exit(void *) {}

// std::mutex/std::condition_variable sind beim Anlegen fertig; die Objekte
// werden nicht kopiert oder verschoben, init ist daher leer.
inline int pthread_mutex_init(pthread_mutex_t *, const void *) { return 0; }
inline int pthread_mutex_lock(pthread_mutex_t *m) { m->lock(); return 0; }
inline int pthread_mutex_unlock(pthread_mutex_t *m) { m->unlock(); return 0; }
inline int pthread_cond_init(pthread_cond_t *, const void *) { return 0; }
inline int pthread_cond_broadcast(pthread_cond_t *c) { c->notify_all(); return 0; }

inline int pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m)
{
  // Der Aufrufer haelt m bereits (pthread-Semantik): uebernehmen, warten,
  // danach wieder an den Aufrufer zurueckgeben.
  std::unique_lock<std::mutex> lock(*m, std::adopt_lock);
  c->wait(lock);
  lock.release();
  return 0;
}

}

#endif
#endif
