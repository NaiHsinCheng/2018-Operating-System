#include "mm.h"

#define NALLOC 1024 /* minimum units to request */

/* block header */
union header
{ 
	struct
	{
		union header *next_ptr; /* next block if on free list */
		size_t size; /* size of this block */
	} s;

	long x; /* force alignment of blocks, size of union is same as long */
};

typedef union header Header;

static Header base; /* empty list to get started */
static Header *freep = NULL; /* start of free list */

/* morecore: ask system for more memory */
static Header *morecore(size_t new_units)
{
	char *p;           /* p : current pointer */
	Header *upper_ptr;
	
	if (new_units < NALLOC)
		new_units = NALLOC;
	
	p = sbrk(new_units * sizeof(Header));
	
	/* no space at all */
	if (p == (char *) -1) 
		return NULL;
	
	/* make up equal to cp and set size */
	upper_ptr = (Header *)p;
	upper_ptr->s.size = new_units;
	
	/* put up into free list */
	myfree((void *)(upper_ptr+1));
	
	return freep;
}

/* mymalloc: general-purpose storage allocator */
void *mymalloc(size_t size)
{
	/* p: current pointer of free list, prevp: prev pointer of free list */
	Header *p, *prevp;
	Header *morecore(size_t);
	
	size_t new_units; /* a number of Header we need to get */
	
	new_units = (size + sizeof(Header)-1) / sizeof(Header) + 1; /* round up with integer division */

	/* no free list yet */
	if ((prevp = freep) == NULL)
	{ 
		base.s.next_ptr = freep = prevp = &base;
		base.s.size = 0;
	}

	for (p = prevp->s.next_ptr ; ; prevp = p, p = p->s.next_ptr)
	{
		/* big enough */
		if(p->s.size >= new_units)
		{
			if (p->s.size == new_units) /* exactly */
				prevp->s.next_ptr = p->s.next_ptr;
			else   /* allocate tail end */
			{
				p->s.size -= new_units;
				p += p->s.size;
				p->s.size = new_units;
			}
	
			freep = prevp;
			return (void *)(p+1);
		}

		/* wrapped around free list */
		if (p == freep)
			if ((p = morecore(new_units)) == NULL) /* none left */
				return NULL;
	}
}

/* free: put block ap in free list */
void myfree(void *ptr)
{
	Header *bp, *p;

	bp = (Header *)ptr - 1;            /* point to block header */

	for (p = freep ; !(bp > p && bp < p->s.next_ptr) ; p = p->s.next_ptr)
	{
		/* freed block at start or end of arena */
		if (p >= p->s.next_ptr && (bp > p || bp < p->s.next_ptr))
			break; 
	}
	
	/* join to upper neighbor */
	if (bp + bp->s.size == p->s.next_ptr)
	{ 
		bp->s.size += p->s.next_ptr->s.size;
		bp->s.next_ptr = p->s.next_ptr->s.next_ptr;
	}
	else
		bp->s.next_ptr = p->s.next_ptr;
	
	/* join to lower neighbor */
	if (p + p->s.size == bp)
	{
		p->s.size += bp->s.size;
		p->s.next_ptr = bp->s.next_ptr;
	}
	else
		p->s.next_ptr = bp;
	
	freep = p;
}

void *myrealloc(void *ptr, size_t size)
{
	Header *bp;
	size_t new_units;

	if(ptr == NULL)                /* if the ptr points to null, malloc new one */
		return mymalloc(size);

	bp = (Header *)ptr - 1;        /* point to block header */
	new_units = (size + sizeof(Header) - 1) / sizeof(Header) + 1;   /* round up with integer division */

	if(bp->s.size >= new_units)
		return ptr;

	void *new_ptr = mymalloc(size);

	if(new_ptr == NULL)
		return NULL;

	memcpy(new_ptr, ptr, (bp->s.size - 1) * sizeof(Header));

	myfree(ptr);

	return new_ptr;
}

void *mycalloc(size_t nmemb, size_t size)
{
	size_t all = nmemb * size;
	void *new_ptr = mymalloc(all);
	if(new_ptr == NULL)
		return NULL;
	memset(new_ptr,0,all);
	return new_ptr;
}
