
#include "plist.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void *plist_new(void) {
  char *p = malloc( 8 + 16 * sizeof(void*) );
  uint32_t *ip = (uint32_t*)p; 
  ip[0] = 0; ip[1] = 16; // len, max
  return ip+2;
}

void _plist_grow(void **pl) {

  uint32_t *ip = (uint32_t*)*pl;
  uint32_t max = ip[-1]; 
  max <<= 1;
  char *orig = ((char*)(*pl))-8;
  char *p = realloc( orig, 8 + max * sizeof(void*) );
  ip = (uint32_t*)p;
  ip[1] = max;
  *pl = p+8;

}

