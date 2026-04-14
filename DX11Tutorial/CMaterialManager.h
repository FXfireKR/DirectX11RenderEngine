#pragma once
#include "CMaterial.h"

class CMaterialManager
{
public:
	CMaterialManager() = default;
	~CMaterialManager() = default;

	uint64_t Create(uint64_t id);
	CMaterial* Ensure(uint64_t id);
	CMaterial* Get(uint64_t id);
	const CMaterial* Get(uint64_t id) const;

private:
	unordered_map<uint64_t, unique_ptr<CMaterial>> m_mapMaterial;
};