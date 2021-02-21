#include <mm/pmm.h>

extern uint64_t __kernel_end;
uint64_t* bitmap = (uint64_t*)&__kernel_end;

uint64_t totalmem;
uint64_t bitmapEntries;

static spinlock_t pmm_lock;

void* pmalloc(size_t pages) {
	spinlock_lock(&pmm_lock);

	uint64_t firstBit = 0;
	uint64_t concurrentBits = 0;
	uint64_t totalBitsInBitmap = totalmem / PAGESIZE;

	for (uint64_t i = 1; i < totalBitsInBitmap; i++) {

		if (getAbsoluteBit(bitmap, i) == 0) {
			if (concurrentBits == 0) {
				firstBit = i;
			}

			concurrentBits++;

			if (pages == concurrentBits) {
				goto alloc;
			}
		} else {
			firstBit = 0;
			concurrentBits = 0;

			continue;
		}
	}

	spinlock_release(&pmm_lock);

	return NULL;

alloc:
	// iterate over bits now that a block has been found
	for (uint64_t i = firstBit; i < firstBit + pages; i++) {
		setAbsoluteBit(bitmap, i);
	}

	spinlock_release(&pmm_lock);
	return (void*)(firstBit * PAGESIZE);
}

void pmfree(void* ptr, size_t pages) {
	spinlock_lock(&pmm_lock);

	uint64_t firstBit = (uint64_t)ptr / PAGESIZE;
	uint64_t totalBitsInBitmap = totalmem / PAGESIZE;

	for (uint64_t i = 0; i < totalBitsInBitmap; i++) {
		if (i == firstBit) {
			for (uint64_t j = 0; j < pages; j++) {
				clearAbsoluteBit(bitmap, j);
			}
			goto done;
		}
	}

done:
	spinlock_release(&pmm_lock);
	return;
}

void* pmrealloc(void* ptr, size_t oldSize, size_t newSize) {
	spinlock_release(&pmm_lock);

	if (newSize < PAGESIZE)
		return ptr;

	uint64_t* newBuffer = (uint64_t*)pmalloc(newSize);
	memcpy(newBuffer, ptr, oldSize);
	pmfree(ptr, oldSize);

	spinlock_release(&pmm_lock);
	return newBuffer;
}
