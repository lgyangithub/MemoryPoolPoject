
template <class T>
class ObjectPool
{
public:
	T* New()
	{
		T* obj = nullptr;
		//如果自由链表有对象，直接取一个
		if (_freeList)
		{
			void* next = *((void**)_freeList);
			obj = (T*)_freeList;
			_freeList = *((void**)_freeList);
		}
		else
		{
			//剩余内存不够一个对象大小时，重开开大块空间
			if (_remains < sizeof(T))
			{
				_remains = 128 * 1024;
				//_memory = (char*)malloc(_remains);
				_memory = (char*)SystemAlloc(_remains >> 13); //除8k
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

		//对已经有的一块空间初始化，使用定位new显示调用T的构造函数初始化
		new(obj)T;
		return obj;
	}

	void Delete(T* obj)
	{
		//显示调用T的析构函数
		obj->~T();
		//头插
		*((void**)obj) = _freeList;
		_freeList = obj;
	}
private:
	char* _memory = nullptr; //指向内存块的指针
	int _remains = 0; //内存块中剩余字节数
	void* _freeList = nullptr; //管理换回来的自由链表
};
