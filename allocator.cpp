#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allocator.hpp"

unsigned long Allocator::align(unsigned long n) {
    int align = 8;
    if (n % 4 == 0 && n != 0) return n;
    if (n < 4) return 4;
    while (n > align) align = align + 4;
    return align;
}

Page* Allocator::findPage(unsigned long memSize){
    Page* dividedPages = m_dividedPages;
    Page* emptyPages = m_emptyPage;
    Page* page = dividedPages;
    while (page != NULL){
        if(page->size >= memSize && page->blockAmount > 0){
            return page;
        }
        page = page->nextPage;
    }
    return emptyPages;
}

void* Allocator::first_fit(Page* page){
    Block* block = page->block;
    UsedBlock * usedBlocks = m_usedBlocks;
    while (block != NULL) {
        if (block->isFree) {
            if (usedBlocks == NULL) {
                UsedBlock * usedBlock = (UsedBlock *) malloc(sizeof(Block));
                usedBlock->nextBlock = NULL;
                usedBlock->prevBlock = NULL;
                usedBlock->originalBlock = block;
                usedBlocks = usedBlock;
            } else {
                while (usedBlocks->nextBlock != NULL) usedBlocks = usedBlocks->nextBlock;
                UsedBlock* newBlock = (UsedBlock *) malloc(sizeof(Block));
                newBlock->prevBlock = usedBlocks;
                newBlock->nextBlock = NULL;
                newBlock->originalBlock = block;
            }
            page->blockAmount--;
            block->isFree = false;
            return block->addr;
        }
        block = block->nextBlock;
    }
    return NULL;
}

void Allocator::dividePage(Page *page, unsigned long blockSize) {
    Block* block = (Block*) malloc(sizeof(Block));
    int blockAmount = (int) (page->size / blockSize);
    block->size = blockSize;
    block->addr = page->buffer;
    block->isFree = true;
    page->block = block;
    page->blockAmount++;
    page->size = blockSize;

    for (int i = 1; i < blockAmount; ++i) {
        Block* newBlock = (Block*) malloc(sizeof(Block));
        newBlock->size = blockSize;
        newBlock->addr = (char*) page->buffer + (i * blockSize);
        newBlock->nextBlock = NULL;
        newBlock->prevBlock = block;
        newBlock->isFree = true;// 1

        block->nextBlock = newBlock;
        block = newBlock;

        page->blockAmount++;
    }
}

Allocator::Allocator(int numPages, unsigned long size){
    if (numPages < 1) return;
    size = align(size);
    Page* page = (Page*) malloc(sizeof(Page));
    page->size = size;
    page->mode = EMPTY;
    page->nextPage = NULL;
    page->buffer = malloc(sizeof(char) * size);
    page->blockAmount = 0;
    page->block = NULL;

    m_emptyPage = page;
    m_allocationUsed = false;
    m_pageSize = size;
    m_fullSizePage = NULL;
    m_dividedPages = NULL;
    m_usedBlocks = NULL;

    for (int i = 1; i < numPages; ++i) {
        Page *nextPage = (Page*) malloc(sizeof(Page));
        nextPage->size = size;
        nextPage->buffer = malloc(sizeof(char) * size);
        nextPage->mode = EMPTY;
        nextPage->nextPage = NULL;
        nextPage->blockAmount = 0;
        nextPage->block = NULL;

        page->nextPage = nextPage;
        page = nextPage;
    }
}

void* Allocator::mem_alloc(unsigned long size){
    Page * page;
    size = align(size);
    if (!m_allocationUsed) {
        m_allocationUsed = true;
        page = m_emptyPage;
        if (size > (m_pageSize / 2)) {
            page->mode = PAGED;
            m_emptyPage = m_emptyPage->nextPage;
            m_fullSizePage = page;
            m_fullSizePage->nextPage = NULL;
            return page->buffer;
        }
        dividePage(page, size);

        page->mode = DIVIDED;

        m_emptyPage = m_emptyPage->nextPage;
        m_dividedPages = page;
        m_dividedPages->nextPage = NULL;
        return first_fit(page);
    }
    page = findPage(size);
    if (page == NULL) {
        return NULL;
    } else if (page->mode == DIVIDED) {
        return first_fit(page);
    } else if (page->mode == EMPTY && size > (page->size / 2)) {
        page->mode = PAGED;
        page->nextPage = m_fullSizePage;
        m_emptyPage = m_emptyPage->nextPage;
        return page->buffer;
    } else {
        dividePage(page, size);

        page->nextPage = m_dividedPages;
        page->mode = DIVIDED;

        m_dividedPages = page;
        m_emptyPage = m_emptyPage->nextPage;
        return first_fit(page);
    }
}

void Allocator::mem_free(void* addr) {
    UsedBlock * block = m_usedBlocks;
    Page* page = m_fullSizePage;
    while (block != NULL) {
        if (block->originalBlock->addr == addr && !block->originalBlock->isFree) {
            block->originalBlock->isFree = true;
            UsedBlock * prevBlock = block->prevBlock;
            UsedBlock* nextBlock = block->nextBlock;
            if (prevBlock != NULL) { prevBlock->nextBlock = nextBlock; }
            if (nextBlock != NULL) { nextBlock->prevBlock = prevBlock; }
            return;
        }
        block = block->nextBlock;
    }
    while (page != NULL) {
        if(page->buffer == addr) {
            page->mode = EMPTY;
            return;
        }
        page = page->nextPage;
    }
}

void* Allocator::mem_realoc(void* addr, unsigned long size) {
    void* newAddr = mem_alloc(size);
    if (newAddr == NULL) return NULL;
    memcpy(newAddr, addr, size);
    mem_free(addr);
    return newAddr;
}

void mem_dump(Allocator *allocator){}
