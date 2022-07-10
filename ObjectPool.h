
template <class T>
class ObjectPool
{
public:
	T* New()
	{
		T* obj = nullptr;
		//������������ж���ֱ��ȡһ��
		if (_freeList)
		{
			void* next = *((void**)_freeList);
			obj = (T*)_freeList;
			_freeList = *((void**)_freeList);
		}
		else
		{
			//ʣ���ڴ治��һ�������Сʱ���ؿ������ռ�
			if (_remains < sizeof(T))
			{
				_remains = 128 * 1024;
				//_memory = (char*)malloc(_remains);
				_memory = (char*)SystemAlloc(_remains >> 13); //��8k
				if (_memory == nullptr)
				{
					throw std::bad_alloc();
				}
			}

			obj = (T*)_memory;
			size_t objSize = sizeof(T) > sizeof(void*) ? sizeof(T) : sizeof(void*);
			_memory += objSize;
			_remains -= objSize;
		}

		//���Ѿ��е�һ��ռ��ʼ����ʹ�ö�λnew��ʾ����T�Ĺ��캯����ʼ��
		new(obj)T;
		return obj;
	}

	void Delete(T* obj)
	{
		//��ʾ����T����������
		obj->~T();
		//ͷ��
		*((void**)obj) = _freeList;
		_freeList = obj;
	}
private:
	char* _memory = nullptr; //ָ���ڴ���ָ��
	int _remains = 0; //�ڴ����ʣ���ֽ���
	void* _freeList = nullptr; //������������������
};
