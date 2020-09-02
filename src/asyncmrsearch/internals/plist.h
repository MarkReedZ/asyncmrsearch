/*
  Pointer List. 

  The pointer list returns a type pointer that you can access as a normal array ( p[0], p[1], ) while taking care of the allocating and resizing for you.

  USAGE:

  int *lst = plist_new();
  for( int x=0; x < 20; x++ ) {
    int *a = malloc(sizeof(int)); *a = 1;

    plist_add(&lst, *a); // Must pass a reference as we may grow the array
    printf("  %d\n", lst[x]);
  }
  plist_free(lst);

*/
#ifndef _PLIST_H
#define _PLIST_H


#define plist_add( pl, x ) ({ uint32_t *ip = (uint32_t*)*pl; uint32_t max = ip[-1]; uint32_t *off = ip-2; if ( *off >= max ) { _plist_grow((void**)pl); ip = (uint32_t*)*pl; off = ip-2; printf("DELME off %d\n",*off);} *pl[*off] = *x; *off += 1; })
#define plist_free( pl ) ({ char *p = ((char*)pl)-8; free(p); })

void *plist_new(void);

void _plist_grow(void **pl);


#endif 

