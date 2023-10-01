// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  //先定义一个指针
  struct run *r;

  //锁住内存
  acquire(&kmem.lock);
  //指针指向表头
  r = kmem.freelist;
  //还有空闲内存就将表头指向下一个内存块
  if(r)
    kmem.freelist = r->next;
  //解锁内存
  release(&kmem.lock);

  //把新申请的空间的数据全部设置为5
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

uint64
count_freemem(void)
{
  //锁内存管理结构，防止统计时有进程访问free-list
  acquire(&kmem.lock);
  //统计空闲页数，乘上页大小PGSIZE，就是空闲字节数
  uint64 mem_bytes=0;
  //xv6中，空闲内存页的记录方式是将空闲内存页本身作为链表的节点，形成一个空闲页链表
  struct run *r=kmem.freelist;
  //遍历free-list
  while (r)
  {
    mem_bytes+=PGSIZE;
    r=r->next;
  }
  //解锁
  release(&kmem.lock);

  return mem_bytes;
  
}