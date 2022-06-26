#pragma once

#include <stdlib.h>

struct noop_deleter { void operator()(void*) const {} };
struct free_deleter { void operator()(void* p) const { ::free(p); } };
template <class T>
struct array_deleter { void operator()(void* p) const { delete[] static_cast<T*>(p); } };
