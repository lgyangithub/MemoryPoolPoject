#pragma once

#include "Common.h"

class ThreadCache
{
public:
	//������ͷ��ڴ����
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);

	//�����Ļ����ȡ����
	void* FetchFromCentralCache(size_t index, size_t size);

	//�ͷŶ���ʱ����������ʱ�������ڴ浽���Ļ���
	void ListTooLong(FreeList& list, size_t size);
private:
	FreeList _freeLists[NFREE_LIST];
};

//TLS thread local storage
static _declspec(thread) ThreadCache* pTLSThreadCache = nullptr;

