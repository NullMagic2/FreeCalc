#include "..//headers//memory.h"

/*
 * allocateAdditionalMemory()
 *
 * Purpose:
 *     Expands the calculator's custom heap when there is not enough contiguous
 *     space to accommodate a new memory allocation request. It attempts to
 *     reserve a new region of virtual memory and commit a portion of it
 *     to the heap.
 *
 * Parameters:
 *     requestedSize: The size in bytes of the memory block that could not
 *                    be allocated from the existing heap. This value is used
 *                    to determine how much additional memory to allocate.
 *
 * Return Value:
 *     int: 0 if the heap expansion was successful, -1 if it failed.
 *
 * Remarks:
 *     - The function uses VirtualAlloc() to reserve a block of virtual memory.
 *     - If the reservation succeeds, the function calls allocateBufferForLargeNumbers()
 *       to commit (allocate) a portion of the reserved memory to the heap.
 *     - The `currentAllocationSize` variable keeps track of the current heap size.
 *     - If the initial allocation size (`currentAllocationSize`) is less than
 *       the minimum allocation size (`minimumAllocationSize`), the heap is
 *       doubled in size. Otherwise, it's expanded to fit the requested size.
 *     - If `VirtualAlloc()` fails, the function returns -1 to indicate an error.
 */
DWORD allocateAdditionalMemory(DWORD requestedSize)
{
    // Calculate aligned size based on page size (assuming 4KB pages)
    uint alignedSize = (currentAllocationSize + 0xFFF) & 0xFFFFF000;

    // Double the allocation size if it's less than minimum
    if (currentAllocationSize < minimumAllocationSize) {
        currentAllocationSize *= 2;
    }
    // If the aligned size is still less than requested, use requested size
    if (alignedSize < requestedSize) {
        alignedSize = requestedSize;
    }

    // Reserve virtual memory using VirtualAlloc()
    LPVOID allocatedMemory = VirtualAlloc(NULL, alignedSize, MEM_RESERVE, PAGE_READWRITE);

    if (allocatedMemory != NULL) {
        // Memory reservation successful, commit a portion for the heap
        int allocationResult = allocateBufferForLargeNumbers(0, requestedSize);
        if (allocationResult == 0) {
            // Committed memory successfully, return success 
            return 0;
        }
        // If committing memory fails, free the reserved region
        VirtualFree(allocatedMemory, 0, MEM_RELEASE);
    }

    // Memory reservation failed or committing memory failed, return error
    return -1;
}


/*
 * allocateMemoryFromHeap()
 *
 * Purpose:
 *     Allocates a block of memory from the calculator's custom heap.
 *     This function manages a linked list of memory blocks and attempts to
 *     find a suitable free block. If no free block is large enough, it attempts
 *     to expand the heap.
 *
 * Parameters:
 *     requestedSize: The size in bytes of the memory block to allocate.
 *
 * Return Value:
 *     int:  The address of the allocated memory block (which is an offset
 *           from the beginning of the custom heap), or 0 if the allocation fails.
 *
 * Remarks:
 *     - The function searches for a free block that is large enough to accommodate
 *       the requested size.
 *     - If a suitable block is found, it is marked as 'in use' and its address is returned.
 *     - If no suitable block is found, the function attempts to expand the heap
 *       by calling allocateAdditionalMemory().
 *     - If the heap expansion fails, the function calls handleMemoryAllocationError()
 *       to report the error.
 */
DWORD allocateMemoryFromHeap(DWORD requestedSize)
{
    int** foundBlock;
    int allocationResult;

    if (requestedSize < MAX_HEAP_ALLOCATION_SIZE) { // Use MAX_HEAP_ALLOCATION_SIZE
        uint alignedSize = requestedSize + 3 & ALIGNMENT_MASK;  // Use ALIGNMENT_MASK

        do {
            foundBlock = findAvailableMemoryBlock(alignedSize);
            if (foundBlock != (int**)0x0) {
                // Found a suitable block
                if ((((*foundBlock)[1] & ALIGNMENT_MASK_UNSIGNED) -  // Use ALIGNMENT_MASK_UNSIGNED
                    ((DWORD)foundBlock[1] & ALIGNMENT_MASK)) -
                    alignedSize != 4)
                {
                    // Split the block if it's much larger than needed
                    splitMemoryBlock(foundBlock, alignedSize);
                }
                foundBlock[1] = (int*)((uint)foundBlock[1] & ALIGNMENT_MASK); // Use ALIGNMENT_MASK
                memoryBlockListHead = (void*)*foundBlock;
                return ((DWORD)foundBlock[1] & ALIGNMENT_MASK) + 4; // Use ALIGNMENT_MASK
            }

            // No suitable block found, try to allocate more memory
            allocationResult = allocateAdditionalMemory(alignedSize);
            if (allocationResult == -1) {
                break;  // Allocation failed
            }

            // Try to find a block again after allocation 
            foundBlock = findAvailableMemoryBlock(alignedSize);
            if (foundBlock == (int**)0x0) {
                handleMemoryAllocationError();
            }

        } while (allocationResult != -1); // Keep trying until allocation fails 
    }

    return NULL; // Allocation failed 
}