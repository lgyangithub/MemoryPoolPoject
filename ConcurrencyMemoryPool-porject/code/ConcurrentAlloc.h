#pragma once

#include "Common.h"
#include "ThreadCache.h"
#include "PageCache.h"
#include "ObjetcPool.h"

static void* ConcurrentAlloc(size_t size)
{
	if (size > MAX_BYTES)
	{
		size_t alignSize = SizeClass::RoundUp(size);
		size_t kpage = alignSize >> PAGE_SHIFT;

		PageCache::GetInstance()->GetInstance()->_pageMtx.lock();
		Span* span = PageCache::GetInstance()->NewSpan(kpage);
		span->_objSize = size;
		PageCache::GetInstance()->GetInstance()->_pageMtx.unlock();

		void* ptr = (void*)(span->_pageId << PAGE_SHIFT);
		return ptr;
	}
	else
	{
		//通过TLS 每个线程无锁地获取自己专有的threadcache
		if (pTLSThreadCache == nullptr)
		{
			static ObjectPool<ThreadCache> tcPool;
			//pTLSThreadCache = new ThreadCache;
			pTLSThreadCache = tcPool.New();
		}

		//cout << std::this_thread::get_id() << ":" << pTLSThreadCache << endl;

		return pTLSThreadCache->Allocate(size);
	}
}

static void ConcurrentFree(void* ptr)
{
	//Span* span = PageCache::GetInstance()->MapObjectToSpan(ptr);
	//size_t size = span->_objSize;
	PAGE_ID id = (PAGE_ID)ptr >> PAGE_SHIFT;
	size_t size = PageCache::GetInstance()->GetIdToSize(id);

	if (size > MAX_BYTES)
	{
		Span* span = PageCache::GetInstance()->MapObjectToSpan(ptr);
		size_t size = span->_objSize;
		PageCache::GetInstance()->_pageMtx.lock();
		PageCache::GetInstance()->ReleaseSpanToPageCache(span);
		PageCache::GetInstance()->_pageMtx.unlock();
	}
	else
	{
		assert(pTLSThreadCache);
		pTLSThreadCache->Deallocate(ptr, size);
	}
}
