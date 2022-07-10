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

	// ��ϵͳ����kҳ�ڴ�ҵ���������
	void* SystemAllocPage(size_t k);

	Span* NewSpan(size_t k);

	// ��ȡ�Ӷ���span��ӳ��
	Span* MapObjectToSpan(void* obj);

	size_t GetIdToSize(PageID id);
	void SetIdToSize(PageID id, size_t size);


	// �ͷſ���span�ص�Pagecache�����ϲ����ڵ�span
	void ReleaseSpanToPageCache(Span* span);
private:
	SpanList _spanList[NPAGES];	// ��ҳ��ӳ��

	TCMalloc_PageMap2<32 - PAGE_SHIFT> _idSpanMap;
	TCMalloc_PageMap1<32 - PAGE_SHIFT> _idSizeMap;

	// tcmalloc ������  Ч�ʸ���

	std::recursive_mutex _mtx;


private:
	PageCache()
	{}

	PageCache(const PageCache&) = delete;

	// ����
	static PageCache _sInst;
};
