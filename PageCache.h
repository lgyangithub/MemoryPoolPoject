#pragma once
#include "Common.h"
#include "PageMap.h"

class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &_sInst;
	}

	// 向系统申请k页内存挂到自由链表
	void* SystemAllocPage(size_t k);

	Span* NewSpan(size_t k);

	// 获取从对象到span的映射
	Span* MapObjectToSpan(void* obj);

	size_t GetIdToSize(PageID id);
	void SetIdToSize(PageID id, size_t size);


	// 释放空闲span回到Pagecache，并合并相邻的span
	void ReleaseSpanToPageCache(Span* span);
private:
	SpanList _spanList[NPAGES];	// 按页数映射

	TCMalloc_PageMap2<32 - PAGE_SHIFT> _idSpanMap;
	TCMalloc_PageMap1<32 - PAGE_SHIFT> _idSizeMap;

	// tcmalloc 基数树  效率更高

	std::recursive_mutex _mtx;


private:
	PageCache()
	{}

	PageCache(const PageCache&) = delete;

	// 单例
	static PageCache _sInst;
};
