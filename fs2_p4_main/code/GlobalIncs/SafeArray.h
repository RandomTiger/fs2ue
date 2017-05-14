#ifndef __SAFEARRAY_H__
#define __SAFEARRAY_H__

#include <assert.h>

template<class T>
class SafeArray
{
public:
	const int m_size;

	SafeArray(const int size) :
		m_size(size)
	{
		m_array = new T[size];
		Clear();
	}
	~SafeArray()
	{
		delete m_array;
	}

	T &operator[](const int index)
	{
		assert(index >= 0 && index < m_size);
		return m_array[index];
	}

	void DuplicateData(const int dest, const int source)
	{
		m_array[dest] = m_array[source];
	}

	T *GetArray()
	{
		return m_array;
	}

	void MemCpy(const int offset, const void * const source, const int count)
	{
		assert(offset >= 0 && (offset + count) < m_size);
		memcpy(m_array + offset, source, sizeof(T) * count);
	}

	void MemCpy(const int offset, const SafeArray<T> &source, const int count)
	{
		assert(offset >= 0 && (offset + count) < m_size);
		assert(count < source.m_size);
		memcpy(m_array + offset, source.m_array, sizeof(T) * count);
	}

	void Clear()
	{
		memset(m_array, 0, sizeof(T) * m_size);
	}

private:
	T *m_array;

	SafeArray(SafeArray<T> &arrayCopy) {};
	SafeArray & operator = (const SafeArray & other) {}
};

#endif