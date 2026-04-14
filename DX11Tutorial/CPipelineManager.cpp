#include "pch.h"
#include "CPipelineManager.h"

uint64_t CPipelineManager::Create(uint64_t id)
{
	auto iter = m_mapPipeline.find(id);
	if (iter == m_mapPipeline.end()) {
		m_mapPipeline.insert(make_pair(id, make_unique<CPipeline>()));
	}
	return id;
}

CPipeline* CPipelineManager::Ensure(uint64_t id)
{
	auto iter = m_mapPipeline.find(id);
	if (iter == m_mapPipeline.end()) {
		m_mapPipeline.insert(make_pair(id, make_unique<CPipeline>()));
	}
	return m_mapPipeline[id].get();
}

CPipeline* CPipelineManager::Get(uint64_t id)
{
	auto iter = m_mapPipeline.find(id);
	if (iter != m_mapPipeline.end()) {
		return iter->second.get();
	}
	return nullptr;
}

const CPipeline* CPipelineManager::Get(uint64_t id) const
{
	auto iter = m_mapPipeline.find(id);
	if (iter != m_mapPipeline.end()) {
		return iter->second.get();
	}
	return nullptr;
}