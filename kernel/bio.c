// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 13
#define NENTRY 5

struct entry
{
	struct spinlock lock;
	struct buf buf[NENTRY];
};

struct entry bcache[NBUCKET];

inline static int hash(int key)
{
	return key % NBUCKET;
}

void binit(void)
{
	for (int i = 0; i < NBUCKET; i++)
	{
		initlock(&bcache[i].lock, "bcache");
		for (int j = 0; i < NENTRY; i++)
		{
			initsleeplock(&bcache[i].buf[j].lock, "bcache entry");
		}
	}
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf *bget(uint dev, uint blockno)
{
	struct buf *b;
	int key = hash(blockno);

	acquire(&bcache[key].lock);

	// Is the block already cached?
	for (int i = 0; i < NENTRY; i++)
	{
		b = &bcache[key].buf[i];
		if (b->dev == dev && b->blockno == blockno)
		{
			b->refcnt++;
			release(&bcache[key].lock);
			acquiresleep(&b->lock);
			return b;
		}
	}

	// Not cached.
	// Recycle the least recently used (LRU) unused buffer.
	int min_idx = 0;
	for (int i = 0; i < NENTRY; i++)
	{
		b = &bcache[key].buf[i];
		if (b->refcnt == 0)
		{
			b->dev = dev;
			b->blockno = blockno;
			b->valid = 0;
			b->refcnt = 1;
			release(&bcache[key].lock);
			acquiresleep(&b->lock);
			return b;
		}
		else
		{
			min_idx = (b->refcnt < bcache[min_idx].buf[i].refcnt) ? i : min_idx;
		}
	}

	b = &bcache[key].buf[min_idx];
	b->dev = dev;
	b->blockno = blockno;
	b->valid = 0;
	b->refcnt = 1;
	release(&bcache[key].lock);
	acquiresleep(&b->lock);
	return b;
}

// Return a locked buf with the contents of the indicated block.
struct buf *
bread(uint dev, uint blockno)
{
	struct buf *b;

	b = bget(dev, blockno);
	if (!b->valid)
	{
		virtio_disk_rw(b, 0);
		b->valid = 1;
	}
	return b;
}

// Write b's contents to disk.  Must be locked.
void bwrite(struct buf *b)
{
	if (!holdingsleep(&b->lock))
		panic("bwrite");
	virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void brelse(struct buf *b)
{
	if (!holdingsleep(&b->lock))
		panic("brelse");

	releasesleep(&b->lock);

	int key = hash(b->blockno);
	acquire(&bcache[key].lock);
	b->refcnt--;
	release(&bcache[key].lock);
}

void bpin(struct buf *b)
{
	int key = hash(b->blockno);

	acquire(&bcache[key].lock);
	b->refcnt++;
	release(&bcache[key].lock);
}

void bunpin(struct buf *b)
{
	int key = hash(b->blockno);

	acquire(&bcache[key].lock);
	b->refcnt--;
	release(&bcache[key].lock);
}
