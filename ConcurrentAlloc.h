#pragma once

#include "Common.h"
#include "ThreadCache.h"
#include "PageCache.h"
#include "ObjectPool.h"

//void* tcmalloc(size_t size)
static void* ConcurrentAlloc(size_t size)
{
	try
	{
		if (size > MAX_BYTES)
		{
			size_t npage = SizeClass::RoundUp(size) >> PAGE_SHIFT;
			Span* span = PageCache::GetInstance()->NewSpan(npage);
			span->_objsize = size;

			void* ptr = (void*)(span->_pageId << PAGE_SHIFT);
			return ptr;
		}
		else
		{
			if (tls_threadcache == nullptr)
			{
				//tls_threadcache = new ThreadCache;
				static ObjectPool<ThreadCache> ObjPool;
				tls_threadcache = ObjPool.New();
			}
			return tls_threadcache->Allocate(size);
		}
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
	return nullptr;
}

static void ConcurrentFree(void* ptr)
{
	try
	{
		PageID id = (ADDRES_TYPE)ptr >> PAGE_SHIFT;
		size_t size = PageCache::GetInstance()->GetIdToSize(id);

		//Span* span = PageCache::GetInstance()->MapObjectToSpan(ptr);
		//size_t size = span->_objsize;

		if (size > MAX_BYTES)
		{
			Span* span = PageCache::GetInstance()->MapObjectToSpan(ptr);
			PageCache::GetInstance()->ReleaseSpanToPageCache(span);
		}
		else
		{
			assert(tls_threadcache);
			tls_threadcache->Deallocate(ptr, size);
		}
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
}