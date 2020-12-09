#ifndef allocator_hpp
#define allocator_hpp

#define EMPTY 0
#define DIVIDED 1
#define PAGED 2

struct Block {
    Block* nextBlock;
    Block* prevBlock;
    void* addr;
    unsigned long size;
    bool isFree;
};

struct UsedBlock {
    UsedBlock* nextBlock;
    UsedBlock* prevBlock;
    Block* originalBlock;
};

struct Page {
    int blockAmount;
    unsigned long size;
    char mode;
    void* buffer;
    Page* nextPage;
    Block* block;
};

class Allocator {
    bool m_allocationUsed;
    unsigned long m_pageSize;
    UsedBlock * m_usedBlocks;
    Page* m_emptyPage;
    Page* m_dividedPages;
    Page* m_fullSizePage;
    
    void *first_fit(Page* page);
    void dividePage(Page *page, unsigned long blockSize);
    unsigned long align(unsigned long n);
    Page* findPage(unsigned long memSize);
public:
    Allocator(int numPages, unsigned long size);
    void* mem_alloc(unsigned long size);
    void mem_free(void* addr);
    void* mem_realoc(void* addr, unsigned long size);
};
#endif
