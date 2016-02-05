#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "memory.h"

typedef union {
    long l_dummy;
    double d_dummy;
    void *p_dummy;
} Cell;

#define CELL_SIZE (sizeof(Cell))

// DEFAULT_PAGE_SIZE: the number of cells
#define DEFAULT_PAGE_SIZE (1024)

typedef struct MemoryPage_tag MemoryPage;
typedef MemoryPage *MemoryPageList;

struct MemoryPage_tag {
    int cell_num;         // 总内存块数量
    int use_cell_num;     // 已经分配出去的内存块数量
    MemoryPageList next;  // 链表结构
    Cell cell[1];         // 内存块指针
};

struct MEM_Storage_tag {
    MemoryPageList page_list;  // 内存页链表
    int current_page_size;     // 一个内存页所拥有的最少 cell 数量
};

#define max(a, b) (((a) > (b)) ? (a) : (b))

// Storage 模块位于 MEM_Controller 的管理之下
// 本质上是一个构造函数
// 只申请了存储器结构体所需要的空间, 没有申请存储器拥有的数据区空间
// page_size > 0, 如果 page_size 为 0, 则设置默认的 page_size
MEM_Storage
MEM_open_storage_func(MEM_Controller controller, const char *filename, int line, int page_size)
{
    MEM_Storage storage;

    storage = MEM_malloc_func(controller, filename, line, sizeof(struct MEM_Storage_tag));

    storage->page_list = NULL;
    assert(page_size >= 0);

    page_size = (page_size > 0) ? page_size : DEFAULT_PAGE_SIZE;
    storage->current_page_size = page_size;

    return storage;
}

void *
MEM_storage_malloc_func(MEM_Controller controller, const char *filename, int line, MEM_Storage storage, size_t size)
{
    int cell_num = ((size - 1) / CELL_SIZE) + 1;  // (size / CELL_SIZE) 上去整

    void *p;
    if (storage->page_list != NULL
            && (storage->page_list->use_cell_num + cell_num < storage->page_list->cell_num)) {
        // 剩余内存块足够, 直接分配
        p = &(storage->page_list->cell[storage->page_list->use_cell_num]);
        storage->page_list->use_cell_num += cell_num;
    }
    else {
        // 现在要申请一块新的内存页, 如果申请的块大小超过了统一的页大小, 那么就按申请的来.
        // 否则每个页的大小都是一样的.
        int alloc_cell_num = max(cell_num, storage->current_page_size);
        MemoryPage *new_page = MEM_malloc_func(controller, filename, line,
                sizeof(MemoryPage) + CELL_SIZE * (alloc_cell_num - 1));
        // 页头 + 内存块占用空间. 因为页头中最后一个成员已经占有了一个 cell, 所以要 -1.
        // 头部插入链表
        new_page->next = storage->page_list;
        storage->page_list = new_page;

        new_page->cell_num = alloc_cell_num;
        new_page->use_cell_num = cell_num;
        p = &(new_page->cell[0]);
    }

    return p;
}

void
MEM_dispose_storage_func(MEM_Controller controller, MEM_Storage storage)
{
    MemoryPage *page;
    while (storage->page_list) {
        page = storage->page_list;
        storage->page_list = storage->page_list->next;
        MEM_free_func(controller, page);
    }
    MEM_free_func(controller, storage);
}

