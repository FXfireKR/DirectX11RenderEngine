#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "VoxelTypes.h"

using BlockPropHashMap = std::unordered_map<PROP_HASH, VALUE_HASH>;

struct GlobalPropertyRegistry
{
	std::unordered_map<PROP_HASH, PROPERTY_ID> mapNameHashToID;
	std::vector<std::string> vecIDToName;

	PROPERTY_ID GetOrCreate(const std::string& name);
	bool Find(const std::string& name, PROPERTY_ID& outPropID) const;
	bool FindByHash(PROP_HASH nameHash, PROPERTY_ID& outPropID) const;
};

struct PropertyDomain
{
	PROPERTY_ID propID = 0;
	std::unordered_set<VALUE_HASH> pendingValueHashes;
	std::unordered_map<VALUE_HASH, VALUE_INDEX> mapValueHashToIndex;
	std::vector<VALUE_HASH> vecIndexToValueHash;

	void CollectValueHash(VALUE_HASH vHash) { pendingValueHashes.insert(vHash); }
	const size_t DomainSize() const { return vecIndexToValueHash.size(); }
};

struct BlockTypeDef
{
	BLOCK_ID blockID = 0;
	std::vector<PropertyDomain> vecProps;
	std::unordered_map<PROPERTY_ID, uint8_t> mapPropToSlot;
	std::vector<uint16_t> stride;
	uint16_t totalStateCount = 1;
};

struct AppliedModel
{
	std::string modelKey;
	uint64_t modelHash = 0;
	int x = 0;
	int y = 0;
	bool rotate = false;
	bool uvlock = false;
	uint16_t weight = 1;
};

struct Term
{
	PROPERTY_ID propID = 0;
	VALUE_INDEX valueIndex = 0;
};

struct VariantRule
{
	std::vector<Term> vecAndTerms;
	std::vector<AppliedModel> vecChoices;
};

struct BlockStateDef
{
	std::vector<VariantRule> vecVariants;
};