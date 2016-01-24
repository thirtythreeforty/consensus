extern "C" {
#include <pebble.h>
}

#include <new>

void* operator new(size_t size)
{
  return malloc(size);
}

void operator delete(void *ptr) throw()
{
  free(ptr);
}

void operator delete(void *ptr, unsigned int) throw()
{
  free(ptr);
}

void* operator new[](size_t size)
{
  return malloc(size);
}

void operator delete[](void *ptr) throw()
{
  free(ptr);
}

void operator delete[](void *ptr, unsigned int) throw()
{
  free(ptr);
}

extern "C" void __cxa_pure_virtual() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Pure virtual function called!");
}
