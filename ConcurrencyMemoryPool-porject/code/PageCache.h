#pragma once

#include "Common.h"
#include "ObjetcPool.h"
#include "PageMap.h"

class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &_sInst;
	}

	//获取从对象到span的映射
	Span* MapObjectToSpan(void* obj);
	//获取从对象到Size的映射
	size_t MapObjectToSize(void* obj);

	//释放空闲span回到Pagecache，合并相邻的span
	void ReleaseSpanToPageCache(Span* span);

	size_t GetIdToSize(PAGE_ID id);
	void SetIdToSize(PAGE_ID id, size_t size);

	//获取一个K页的span
	Span* NewSpan(size_t k);

	std::mutex _pageMtx;

private:
	SpanList _spanLists[NPAGES];
	ObjectPool<Span> _spanPool;

	//std::unordered_map<PAGE_ID, Span*> _idSpanMap;
	//std::map<PAGE_ID, Span*> _idSpanMap;

	TCMalloc_PageMapSize<32 - PAGE_SHIFT> _idSizeMap;//映射地址和size

	//直接地址法映射，页号映射span
	//32位下，映射页号 数量 = 2^32 / 2^12 = 2 ^ 20; 
	TCMalloc_PageMap1<32 - PAGE_SHIFT> _idSpanMap; //一层


	PageCache()
	{}
	PageCache(const PageCache&) = delete;

	static PageCache _sInst;
};