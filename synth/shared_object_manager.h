#ifndef __SHARED_OBJECT_MANAGER__
#define __SHARED_OBJECT_MANAGER__

void* my_dlopen(string file);
void dlref_inc(void* handle);
void dlref_dec(void* handle);

#endif
