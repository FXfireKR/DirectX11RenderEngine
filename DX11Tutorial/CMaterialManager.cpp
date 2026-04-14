#include "pch.h"
#include "CMaterialManager.h"

uint64_t CMaterialManager::Create(uint64_t id)
{
	auto iter = m_mapMaterial.find(id);
	if (iter == m_mapMaterial.end()) {
		m_mapMaterial.insert(make_pair(id, make_unique<CMaterial>()));
	}
	return id;
}

CMaterial* CMaterialManager::Ensure(uint64_t id)
{
	auto iter = m_mapMaterial.find(id);
	if (iter == m_mapMaterial.end()) {
		m_mapMaterial.insert(make_pair(id, make_unique<CMaterial>()));
	}
	return m_mapMaterial[id].get();
}

CMaterial* CMaterialManager::Get(uint64_t id)
{
	auto iter = m_mapMaterial.find(id);
	if (iter != m_mapMaterial.end()) {
		return iter->second.get();
	}
	return nullptr;
}

const CMaterial* CMaterialManager::Get(uint64_t id) const
{
	auto iter = m_mapMaterial.find(id);
	if (iter != m_mapMaterial.end()) {
		return iter->second.get();
	}
	return nullptr;
}