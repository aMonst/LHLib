#include "LH_Win.h"
#include "LHHeap.h"
#include "LHObject.h"

#define new new(__WFILE__,__LINE__)
CLHHeap CLHObject::ms_Heap;