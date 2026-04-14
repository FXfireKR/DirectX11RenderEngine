#pragma once
#include "CPipeline.h"

class CPipelineManager
{
public:
	CPipelineManager() = default;
	~CPipelineManager() = default;

	uint64_t Create(uint64_t id);
	CPipeline* Ensure(uint64_t id);
	CPipeline* Get(uint64_t id);
	const CPipeline* Get(uint64_t id) const;

private:
	unordered_map<uint64_t, unique_ptr<CPipeline>> m_mapPipeline;
};