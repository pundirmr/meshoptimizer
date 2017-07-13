// This file is part of meshoptimizer library; see meshoptimizer.hpp for version/license details
#include "meshoptimizer.hpp"

#include <cassert>
#include <vector>

namespace meshopt
{

template <typename T>
static VertexCacheStatistics analyzeVertexCacheImpl(const T* indices, size_t index_count, size_t vertex_count, unsigned int cache_size)
{
	assert(index_count % 3 == 0);
	assert(cache_size >= 3);

	VertexCacheStatistics result = {};

	std::vector<unsigned int> cache_timestamps(vertex_count, 0);
	unsigned int timestamp = cache_size + 1;

	for (size_t i = 0; i < index_count; ++i)
	{
		T index = indices[i];
		assert(index < vertex_count);

		if (timestamp - cache_timestamps[index] > cache_size)
		{
			// cache miss
			cache_timestamps[index] = timestamp++;
			result.vertices_transformed++;
		}
	}

	result.acmr = index_count == 0 ? 0 : float(result.vertices_transformed) / float(index_count / 3);
	result.atvr = vertex_count == 0 ? 0 : float(result.vertices_transformed) / float(vertex_count);

	return result;
}

VertexCacheStatistics analyzeVertexCache(const unsigned short* indices, size_t index_count, size_t vertex_count, unsigned int cache_size)
{
	return analyzeVertexCacheImpl(indices, index_count, vertex_count, cache_size);
}

VertexCacheStatistics analyzeVertexCache(const unsigned int* indices, size_t index_count, size_t vertex_count, unsigned int cache_size)
{
	return analyzeVertexCacheImpl(indices, index_count, vertex_count, cache_size);
}

} // namespace meshopt