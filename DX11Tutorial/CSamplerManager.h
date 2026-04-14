#pragma once
#include "CSamplerState.h"

class CSamplerManager
{
public:
	CSamplerManager() = default;
	~CSamplerManager() = default;

	void Initialize(ID3D11Device& refDevice);
	uint64_t Create(SAMPLER_TYPE eType);

	CSamplerState* Get(SAMPLER_TYPE eType);
	const CSamplerState* Get(SAMPLER_TYPE eType) const;

	CSamplerState* Get(uint64_t id);
	const CSamplerState* Get(uint64_t id) const;

private:
	ID3D11Device* m_pDevice = nullptr;

	vector<unique_ptr<CSamplerState>> m_vecSamplers;
	unordered_map<SAMPLER_TYPE, uint64_t> m_mapping;
};