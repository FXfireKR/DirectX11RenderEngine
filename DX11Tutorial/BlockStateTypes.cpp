#include "pch.h"
#include "BlockStateTypes.h"

PROPERTY_ID GlobalPropertyRegistry::GetOrCreate(const std::string& name)
{
	uint64_t hash = fnv1a_64(name);

	auto it = mapNameHashToID.find(hash);
	if (it != mapNameHashToID.end()) 
		return it->second;

	PROPERTY_ID newID = (PROPERTY_ID)vecIDToName.size();
	mapNameHashToID[hash] = newID;
	vecIDToName.push_back(name);

	return newID;
}

bool GlobalPropertyRegistry::Find(const std::string& name, PROPERTY_ID& outPropID) const
{
	uint64_t hash = fnv1a_64(name);
	return FindByHash(hash, outPropID);
}

bool GlobalPropertyRegistry::FindByHash(PROP_HASH nameHash, PROPERTY_ID& outPropID) const
{
	auto it = mapNameHashToID.find(nameHash);
	if (it == mapNameHashToID.end()) 
		return false;

	outPropID = it->second;
	return true;
}