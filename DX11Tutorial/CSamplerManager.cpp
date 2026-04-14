#include "pch.h"
#include "CSamplerManager.h"

void CSamplerManager::Initialize(ID3D11Device& refDevice)
{
	m_pDevice = &refDevice;
}

uint64_t CSamplerManager::Create(SAMPLER_TYPE eType)
{
	auto iter = m_mapping.find(eType);
	if (iter != m_mapping.end()) return iter->second;

	auto sam = make_unique<CSamplerState>();
	if (false == sam->Create(m_pDevice, eType)) return UINT64_MAX;
	
	uint64_t newID = m_vecSamplers.size();
	m_vecSamplers.push_back(std::move(sam));

	m_mapping.insert(make_pair(eType, newID));
	return newID;
}

CSamplerState* CSamplerManager::Get(SAMPLER_TYPE eType)
{
	auto iter = m_mapping.find(eType);
	if (iter != m_mapping.end()) {
		if (iter->second < m_vecSamplers.size()) {
			return m_vecSamplers[iter->second].get();
		}
	}
	return nullptr;
}

const CSamplerState* CSamplerManager::Get(SAMPLER_TYPE eType) const
{
	auto iter = m_mapping.find(eType);
	if (iter != m_mapping.end()) {
		if (iter->second < m_vecSamplers.size()) {
			return m_vecSamplers[iter->second].get();
		}
	}
	return nullptr;
}

CSamplerState* CSamplerManager::Get(uint64_t id)
{
	if (id < m_vecSamplers.size()) {
		return m_vecSamplers[id].get();
	}
	return nullptr;
}

const CSamplerState* CSamplerManager::Get(uint64_t id) const
{
	if (id < m_vecSamplers.size()) {
		return m_vecSamplers[id].get();
	}
	return nullptr;
}