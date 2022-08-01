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

	//��ȡ�Ӷ���span��ӳ��
	Span* MapObjectToSpan(void* obj);
	//��ȡ�Ӷ���Size��ӳ��
	size_t MapObjectToSize(void* obj);

	//�ͷſ���span�ص�Pagecache���ϲ����ڵ�span
	void ReleaseSpanToPageCache(Span* span);

	size_t GetIdToSize(PAGE_ID id);
	void SetIdToSize(PAGE_ID id, size_t size);

	//��ȡһ��Kҳ��span
	Span* NewSpan(size_t k);

	std::mutex _pageMtx;

private:
	SpanList _spanLists[NPAGES];
	ObjectPool<Span> _spanPool;

	//std::unordered_map<PAGE_ID, Span*> _idSpanMap;
	//std::map<PAGE_ID, Span*> _idSpanMap;

	TCMalloc_PageMapSize<32 - PAGE_SHIFT> _idSizeMap;//ӳ���ַ��size

	//ֱ�ӵ�ַ��ӳ�䣬ҳ��ӳ��span
	//32λ�£�ӳ��ҳ�� ���� = 2^32 / 2^12 = 2 ^ 20; 
	TCMalloc_PageMap1<32 - PAGE_SHIFT> _idSpanMap; //һ��


	PageCache()
	{}
	PageCache(const PageCache&) = delete;

	static PageCache _sInst;
};