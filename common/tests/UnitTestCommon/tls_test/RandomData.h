#pragma once
//#include <algorithm>
#include <memory>
#include <chrono>
#include <random>
#include <list>

#include <cstdlib>
#include <cstdint>

/* random data class */
class RandomData
{
public:
	/* list of shared pointers to random data */
	typedef std::list<std::shared_ptr<RandomData>> List;
	typedef std::shared_ptr<RandomData> Ptr;

	/* create new random data */
	RandomData(size_t size, std::default_random_engine &gen)
		: size(size)
	{
		/* random number generator */
		std::uniform_int_distribution<uint32_t> dist(0, UINT8_MAX);

		/* allocate storage */
		data = new uint8_t[size];
		/* fill with random values */
		for (size_t i = 0; i < size; i++)
			data[i] = (uint8_t)dist(gen);
	}

	/* create random data container from existing one */
	RandomData(void *extern_data, size_t data_size)
	{
		data = new uint8_t[data_size];
		size = data_size;

		if (size == 0)
		{
			data = NULL;
		}

		memcpy(data, extern_data, data_size);
	}

	/* comapare with raw random data */
	bool equal(void *extern_data, size_t data_size) const
	{
		if (data_size != size)
			return false;

		return (memcmp(data, extern_data, data_size) == 0 ? true : false);
	}

	const void *getData() const
	{
		return data;
	}

	size_t getSize() const
	{
		return size;
	}

	/* equivalence operator */
	bool operator==(const RandomData &v) const
	{
		if (size != v.size)
			return false;

		return (memcmp(data, v.data, size) == 0 ? true : false);
	}

	~RandomData()
	{
		delete[] data;
	}
private:
	uint8_t *data;
	size_t size;
};
