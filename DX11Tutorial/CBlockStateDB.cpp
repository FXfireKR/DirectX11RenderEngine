#include "pch.h"
#include "CBlockStateDB.h"
#include "CBlockDefDB.h"
#include "CModelDB.h"
#include "ResourceUtil.h"
#include "BlockStateParseUtil.h"

void CBlockStateDB::Initialize(const char* path, const CBlockDefDB* pBlockDefDB, CModelDB* pModelDB)
{
	// path 에서 base 경로를 잡으면 이하 경로를 찾아감.
	m_strRoot = path;
	m_pBlockDefDB = pBlockDefDB;
	m_pModelDB = pModelDB;

	Clear();

	if (!_ScanBlockStateFiles(m_vecFiles)) assert(false && "_ScanBlockStateFiles");
	m_uMaxFiles = m_vecFiles.size();

	//if (_Pass1_CollectDomains(m_vecFiles))
	//_FinalizeAllBockTypes();
	//_Pass2_CompileRules(m_vecFiles);
}

void CBlockStateDB::Clear()
{
	m_vecFiles.clear();
	m_mapBlockState.clear();
	m_mapBlockType.clear();
	m_propRegistry = GlobalPropertyRegistry{};
	m_eStep = LOAD_PROGRESS_STEP::INIT;
	m_uMaxFiles = 0;
	m_uLoadedFiles = 0;
}

void CBlockStateDB::Load()
{
	switch (m_eStep)
	{
		case LOAD_PROGRESS_STEP::INIT:
		{
			m_eStep = LOAD_PROGRESS_STEP::COLLECT_DOMAINS;
		} break;

		case LOAD_PROGRESS_STEP::COLLECT_DOMAINS:
		{
			for (size_t i = 0; i < MAX_FILE_LOAD_CNT; ++i) {
				if (m_uLoadedFiles >= m_uMaxFiles) {
					m_eStep = LOAD_PROGRESS_STEP::FINALIZE_ALL;
					return;
				}
				_Pass1_CollectDomains(m_vecFiles[m_uLoadedFiles]);
				++m_uLoadedFiles;
			}
		} break;

		case LOAD_PROGRESS_STEP::FINALIZE_ALL:
		{
			_FinalizeAllBlockTypes();
			m_eStep = LOAD_PROGRESS_STEP::COMPILE_RULES;
			m_uLoadedFiles = 0;
		} break;

		case LOAD_PROGRESS_STEP::COMPILE_RULES:
		{
			for (size_t i = 0; i < MAX_FILE_LOAD_CNT; ++i) {
				if (m_uLoadedFiles >= m_uMaxFiles) {
					m_eStep = LOAD_PROGRESS_STEP::END;
					return;
				}
				_Pass2_CompileRules(m_vecFiles[m_uLoadedFiles]);
				++m_uLoadedFiles;
			}
		} break;

		case LOAD_PROGRESS_STEP::END:
		{
			m_vecFiles.clear();
			m_vecFiles.shrink_to_fit();
		} break;
	}
}

bool CBlockStateDB::GetAppliedModels(IN BLOCK_ID blockID, STATE_INDEX stateIndex, OUT vector<AppliedModel>& vecAppliedModels) const
{
	PROFILE_SCOPE();

	vecAppliedModels.clear();

	const BlockTypeDef* typeDef = _FindBlockType(blockID);
	const BlockStateDef* stateDef = _FindBlockState(blockID);
	if (!typeDef || !stateDef)
		return false;

	// decode
	std::vector<VALUE_INDEX> decoded;
	_DecodeStateIndex(*typeDef, stateIndex, decoded);

	// match
	for (const auto& rule : stateDef->vecVariants)
	{
		bool ok = true;
		for (const auto& t : rule.vecAndTerms)
		{
			auto slotIt = typeDef->mapPropToSlot.find(t.propID);
			if (slotIt == typeDef->mapPropToSlot.end())
			{
				ok = false;
				break;
			}

			VALUE_INDEX cur = decoded[slotIt->second];
			if (cur != t.valueIndex)
			{
				ok = false;
				break;
			}
		}

		if (!ok)
			continue;

		// 선택: 1차는 첫 번째
		vecAppliedModels.push_back(rule.vecChoices[0]);
		return true;
	}



	// 매칭 없음: 빈 반환
	return true;
}

bool CBlockStateDB::GetAppliedModels(IN BLOCK_ID blockID, STATE_INDEX stateIndex, OUT const vector<AppliedModel>*& vecAppliedModels) const
{
	PROFILE_SCOPE();

	//vecAppliedModels.clear();

	const BlockTypeDef* typeDef = _FindBlockType(blockID);
	const BlockStateDef* stateDef = _FindBlockState(blockID);
	if (!typeDef || !stateDef) 
		return false;

	// decode
	std::vector<VALUE_INDEX> decoded;
	_DecodeStateIndex(*typeDef, stateIndex, decoded);

	// match
	for (const auto& rule : stateDef->vecVariants)
	{
		bool ok = true;
		for (const auto& t : rule.vecAndTerms)
		{
			auto slotIt = typeDef->mapPropToSlot.find(t.propID);
			if (slotIt == typeDef->mapPropToSlot.end()) 
			{ 
				ok = false; 
				break; 
			}

			VALUE_INDEX cur = decoded[slotIt->second];
			if (cur != t.valueIndex) 
			{ 
				ok = false; 
				break; 
			}
		}

		if (!ok) 
			continue;

		// 선택: 1차는 첫 번째
		//vecAppliedModels.push_back(rule.vecChoices[0]);
		vecAppliedModels = &(rule.vecChoices);
		return true;
	}

	

	// 매칭 없음: 빈 반환
	return true;
}

bool CBlockStateDB::EncodeStateIndex(IN BLOCK_ID blockID, const BlockPropHashMap& props, OUT STATE_INDEX& stateIndex) const
{
	stateIndex = 0;

	const BlockTypeDef* typeDef = _FindBlockType(blockID);
	if (!typeDef) return false;

	// 기본값은 전부 0 (즉, 각 property domain의 0번 값) props에 들어온 것만 덮어쓴다.
	uint32_t state = 0;

	for (const auto& kv : props)
	{
		const uint64_t propHash = kv.first;
		const uint64_t valueHash = kv.second;

		PROPERTY_ID pid = 0;
		if (!m_propRegistry.FindByHash(propHash, pid))
		{
			// 이 블럭에서 쓰지 않는 Prop이 들어온 경우 무시
			continue;
		}

		auto slotIt = typeDef->mapPropToSlot.find(pid);
		if (slotIt == typeDef->mapPropToSlot.end())
		{
			// 이 blockID에 없는 property면 무시
			continue;
		}

		const uint8_t slot = slotIt->second;
		if (slot >= typeDef->vecProps.size()) continue;

		const PropertyDomain& dom = typeDef->vecProps[slot];

		auto vit = dom.mapValueHashToIndex.find(valueHash);
		if (vit == dom.mapValueHashToIndex.end())
		{
			// value가 domain에 없음(예: "powered=maybe" 같은 잘못된 값)
			// 여기서는 무시(기본값 0 유지). 디버그 로그는 선택.
#ifdef _DEBUG
			// std::cerr << "EncodeStateIndex: unknown valueHash for propHash\n";
#endif // _DEBUG
			continue;
		}

		const VALUE_INDEX vIdx = vit->second;
		const uint16_t stride = typeDef->stride[slot];

		state += (uint32_t)vIdx * (uint32_t)stride;
	}

	// clamp (STATE_INDEX가 uint16_t 가정)
	if (state > 65535u) state = 65535u;

	stateIndex = (STATE_INDEX)state;
	return true;
}

bool CBlockStateDB::TryGetStateValueHash(BLOCK_ID blockID, STATE_INDEX stateIndex, PROP_HASH propHash, VALUE_HASH& outValueHash) const
{
	outValueHash = 0;

	const BlockTypeDef* typeDef = _FindBlockType(blockID);
	if (!typeDef)
		return false;

	PROPERTY_ID pid = 0;
	if (!m_propRegistry.FindByHash(propHash, pid))
		return false;

	auto slotIt = typeDef->mapPropToSlot.find(pid);
	if (slotIt == typeDef->mapPropToSlot.end())
		return false;

	const uint8_t slot = slotIt->second;
	if (slot >= typeDef->vecProps.size())
		return false;

	std::vector<VALUE_INDEX> decoded;
	_DecodeStateIndex(*typeDef, stateIndex, decoded);

	if (slot >= decoded.size())
		return false;

	const PropertyDomain& dom = typeDef->vecProps[slot];
	const VALUE_INDEX vIdx = decoded[slot];

	if (vIdx >= dom.vecIndexToValueHash.size())
		return false;

	outValueHash = dom.vecIndexToValueHash[vIdx];
	return true;
}

bool CBlockStateDB::_ScanBlockStateFiles(vector<filesystem::path>& outFiles) const
{
	outFiles.clear();

	filesystem::path root(m_strRoot);
	if (!filesystem::exists(root)) return false;

	for (auto& it : filesystem::recursive_directory_iterator(root))
	{
		if (!it.is_regular_file()) continue;
		auto path = it.path();
		if (path.extension() != ".json") continue;

		auto str = path.generic_string();
		if (str.find("/assets/") == std::string::npos) continue;
		if (str.find("/blockstates/") == std::string::npos) continue;

		outFiles.push_back(path);
	}
	return true;
}

//bool CBlockStateDB::_Pass1_CollectDomains(const vector<filesystem::path>& files)
bool CBlockStateDB::_Pass1_CollectDomains(const filesystem::path& file)
{
	string blockKey;
	if (!BuildBlockKeyFromPath(file, blockKey)) 
		return false;

	BLOCK_ID blockID = m_pBlockDefDB->FindBlockID(blockKey.c_str());
	if (blockID == INVALID_BLOCK_ID)
	{
#ifdef DEBUG_LOG 
		cout << "[BlockStateDB] Unknown block key in registry : " << blockKey << "\n";
#endif // DEBUG_LOG 
		return true; // skip
	}

	Document doc;
	if (!_ReadJson(file, doc)) 
		return false;

	if (doc.HasMember("variants"))
	{
		const auto& variants = doc["variants"];
		if (!_ReadVariants_Pass1(blockID, variants)) 
			return false;
	}

	return true;
}

bool CBlockStateDB::_FinalizeAllBlockTypes()
{
	for (auto& kv : m_mapBlockType)
	{
		BlockTypeDef& def = kv.second;

		// 1) 각 domain의 pendingValueHashes -> vecIndexToValueHash 확정
		for (auto& domain : def.vecProps)
		{
			domain.vecIndexToValueHash.clear();
			domain.vecIndexToValueHash.reserve(domain.pendingValueHashes.size());

			for (auto h : domain.pendingValueHashes)
				domain.vecIndexToValueHash.push_back(h);

			std::sort(domain.vecIndexToValueHash.begin(), domain.vecIndexToValueHash.end()); // hash 기준 정렬

			domain.mapValueHashToIndex.clear();
			for (size_t i = 0; i < domain.vecIndexToValueHash.size(); ++i)
				domain.mapValueHashToIndex[domain.vecIndexToValueHash[i]] = (VALUE_INDEX)i;

			domain.pendingValueHashes.clear();
		}

		// 2) propID -> slot 맵 구축
		def.mapPropToSlot.clear();
		for (size_t i = 0; i < def.vecProps.size(); ++i)
		{
			def.mapPropToSlot[def.vecProps[i].propID] = (uint8_t)i;
		}

		// 3) stride/totalStateCount 계산
		def.stride.assign(def.vecProps.size(), 1);
		uint32_t total = 1;
		for (size_t i = 0; i < def.vecProps.size(); ++i)
		{
			if (i == 0) def.stride[i] = 1;
			else
			{
				uint32_t prevSize = (uint32_t)def.vecProps[i - 1].DomainSize();
				def.stride[i] = (uint16_t)(def.stride[i - 1] * prevSize);
			}
		}

		for (auto& d : def.vecProps)
		{
			total *= (uint32_t)std::max<size_t>(1, d.DomainSize());
			if (total > 65535) {
				// 1.21 바닐라 기준 대부분 이 안에 들어가지만, 혹시 몰라 경고, 여기서는 clamp만
				total = 65535;
				break;
			}
		}

		def.totalStateCount = (uint16_t)total;
	}
	return true;
}

//bool CBlockStateDB::_Pass2_CompileRules(const vector<filesystem::path>& files)
bool CBlockStateDB::_Pass2_CompileRules(const filesystem::path& file)
{
	std::string blockKey;
	if (!BuildBlockKeyFromPath(file, blockKey)) return false;

	BLOCK_ID blockID = m_pBlockDefDB->FindBlockID(blockKey.c_str());
	if (blockID == INVALID_BLOCK_ID)
	{
#ifdef DEBUG_LOG 
		cout << "[BlockStateDB] Unknown block key in registry : " << blockKey << "\n";
#endif // DEBUG_LOG 
		return true; // skip
	}

	rapidjson::Document doc;
	if (!_ReadJson(file, doc)) return false;

	if (doc.HasMember("variants"))
	{
		const auto& variants = doc["variants"];
		if (!_ReadVariants_Pass2(blockID, variants)) return false;
	}

	return true;
}


bool CBlockStateDB::_ReadJson(const std::filesystem::path& path, rapidjson::Document& outDoc) const
{
	stringstream jsonBuffer;
	if (false == IFileStreamWrapper::ReadAllStream(path.generic_string(), jsonBuffer)) return false;

	outDoc.Parse<kParseCommentsFlag | kParseTrailingCommasFlag>(jsonBuffer.str().c_str());

	if (outDoc.HasParseError() || !outDoc.IsObject()) {
		ParseErrorCode ecode = outDoc.GetParseError();
		return false;
	}
	return true;
}

bool CBlockStateDB::_ReadVariants_Pass1(BLOCK_ID blockID, const rapidjson::Value& variants)
{
	if (!variants.IsObject()) return false;

	BlockTypeDef& typeDef = _GetOrCreateBlockType(blockID);

	for (auto it = variants.MemberBegin(); it != variants.MemberEnd(); ++it)
	{
		if (!it->name.IsString()) continue;
		const char* keyStr = it->name.GetString();

		vector<pair<string, string>> terms;
		if (!BlockStateParseUtil::ParsePredicate(keyStr, terms))
		{
#ifdef DEBUG_LOG
			cerr << "Malformed variant key: " << keyStr << "\n";
#endif // DEBUG_LOG
			continue;
		}

		for (auto& kv : terms)
		{
			PROPERTY_ID pid = m_propRegistry.GetOrCreate(kv.first);
			PropertyDomain& dom = _GetOrCreateDomain(typeDef, pid);

			uint64_t vhash = fnv1a_64(kv.second);
			dom.CollectValueHash(vhash);
		}
	}
	return true;
}

bool CBlockStateDB::_ReadVariants_Pass2(BLOCK_ID blockID, const rapidjson::Value& variants)
{
	if (!variants.IsObject()) return false;

	const BlockTypeDef* typeDef = _FindBlockType(blockID);
	if (!typeDef) return false;

	BlockStateDef& stateDef = m_mapBlockState[blockID];
	stateDef.vecVariants.clear();

	for (auto it = variants.MemberBegin(); it != variants.MemberEnd(); ++it)
	{
		if (!it->name.IsString()) continue;
		const char* keyStr = it->name.GetString();
		const auto& value = it->value;

		VariantRule rule;

		// keyStr -> terms (propID,valueIndex)
		if (!_CompileKeyTerms_ToTerms(*typeDef, keyStr, rule.vecAndTerms))
		{
#ifdef DEBUG_LOG
			std::cerr << "Compile terms failed: " << keyStr << "\n";
#endif // DEBUG_LOG
			continue;
		}

		// value -> choices
		if (value.IsObject())
		{
			AppliedModel m;
			if (_ReadModelSpec(value, m)) {
				m_pModelDB->SubmitToLoad(m.modelKey.c_str());
				rule.vecChoices.push_back(std::move(m));
			}

		}
		else if (value.IsArray())
		{
			for (auto& elem : value.GetArray())
			{
				if (!elem.IsObject()) continue;
				AppliedModel m;
				if (_ReadModelSpec(elem, m)) {
					m_pModelDB->SubmitToLoad(m.modelKey.c_str());
					rule.vecChoices.push_back(std::move(m));
				}
			}
		}
		else
		{
			continue;
		}

		if (rule.vecChoices.empty())
			continue;

		// 정렬(선택): propID 기준 정렬로 매칭 안정/간편
		std::sort(rule.vecAndTerms.begin(), rule.vecAndTerms.end(),
			[](const Term& a, const Term& b) { return a.propID < b.propID; });

		stateDef.vecVariants.push_back(std::move(rule));
	}

	return true;
}

bool CBlockStateDB::_ParseKeyTerms_ToHashes(const char* keyStr, std::vector<std::pair<uint64_t, uint64_t>>& out) const
{
	// ParsePredicate
	return false;
}

bool CBlockStateDB::_CompileKeyTerms_ToTerms(const BlockTypeDef& typeDef, const char* keyStr, std::vector<Term>& outTerms) const
{
	outTerms.clear();
	if (!keyStr) return false;

	std::string s(keyStr);
	if (s.empty()) return true; // Always (terms empty)

	std::vector<std::pair<std::string, std::string>> terms;
	if (!BlockStateParseUtil::ParsePredicate(keyStr, terms))
		return false;

	for (auto& kv : terms)
	{
		// property name -> pid (이미 Pass1에서 생성되어 있을 가능성이 큼)
		PROPERTY_ID pid = 0;
		m_propRegistry.Find(kv.first, pid);

		auto slotIt = typeDef.mapPropToSlot.find(pid);
		if (slotIt == typeDef.mapPropToSlot.end()) return false;

		const PropertyDomain& dom = typeDef.vecProps[slotIt->second];

		uint64_t vhash = fnv1a_64(kv.second);
		auto vit = dom.mapValueHashToIndex.find(vhash);
		if (vit == dom.mapValueHashToIndex.end()) return false;

		outTerms.push_back({ pid, vit->second });
	}

	return true;
}

int CBlockStateDB::_NormalizeRot(int val)
{
	int ret = (((val % 360) + 360) % 360);
	if (ret % 90 != 0)
	{
#ifdef DEBUG_LOG
		std::cout << ret << " Normalize rotation not multiple of 90\n";
#endif // DEBUG_LOG
		return 0;
	}
	return ret;
}

bool CBlockStateDB::_ReadModelSpec(const rapidjson::Value& value, AppliedModel& outModel)
{
	if (!value.IsObject()) return false;
	outModel = AppliedModel{};

	if (!value.HasMember("model") || !value["model"].IsString()) return false;
	outModel.modelKey = value["model"].GetString();
	outModel.modelHash = fnv1a_64(outModel.modelKey);

	outModel.rotate = false;

	if (value.HasMember("x") && value["x"].IsInt()) {
		outModel.x = _NormalizeRot(value["x"].GetInt());
		outModel.rotate = true;
	}

	if (value.HasMember("y") && value["y"].IsInt()) {
		outModel.y = _NormalizeRot(value["y"].GetInt());
		outModel.rotate = true;
	}

	if (value.HasMember("uvlock") && value["uvlock"].IsBool())
		outModel.uvlock = value["uvlock"].GetBool();


	if (value.HasMember("weight") && value["weight"].IsInt())
	{
		int w = value["weight"].GetInt();
		if (w < 1) w = 1;
		if (w > 65535) w = 65535;
		outModel.weight = (uint16_t)w;
	}
	return true;
}

BlockTypeDef& CBlockStateDB::_GetOrCreateBlockType(BLOCK_ID blockID)
{
	auto it = m_mapBlockType.find(blockID);
	if (it != m_mapBlockType.end()) return it->second;

	BlockTypeDef def;
	def.blockID = blockID;
	auto [it2, _] = m_mapBlockType.emplace(blockID, std::move(def));
	return it2->second;
}

PropertyDomain& CBlockStateDB::_GetOrCreateDomain(BlockTypeDef& typeDef, PROPERTY_ID pid)
{
	// 선형 탐색(초기엔 충분). 최적화는 나중에 mapPropToSlot을 Pass1에도 쓰는 방식으로.
	for (auto& d : typeDef.vecProps)
		if (d.propID == pid) return d;

	PropertyDomain d;
	d.propID = pid;
	typeDef.vecProps.push_back(std::move(d));
	return typeDef.vecProps.back();
}

const BlockTypeDef* CBlockStateDB::_FindBlockType(BLOCK_ID blockID) const
{
	auto it = m_mapBlockType.find(blockID);
	if (it == m_mapBlockType.end()) return nullptr;
	return &it->second;
}

const BlockStateDef* CBlockStateDB::_FindBlockState(BLOCK_ID blockID) const
{
	auto it = m_mapBlockState.find(blockID);
	if (it == m_mapBlockState.end()) return nullptr;
	return &it->second;
}

void CBlockStateDB::_DecodeStateIndex(const BlockTypeDef& typeDef, STATE_INDEX sidx, std::vector<VALUE_INDEX>& outValueIndexPerProp) const
{
	PROFILE_SCOPE();

	outValueIndexPerProp.assign(typeDef.vecProps.size(), 0);

	// 뒤에서 앞으로
	for (int i = (int)typeDef.vecProps.size() - 1; i >= 0; --i)
	{
		uint16_t st = typeDef.stride[i];
		size_t domainSize = std::max<size_t>(1, typeDef.vecProps[i].DomainSize());
		VALUE_INDEX v = (VALUE_INDEX)((sidx / st) % domainSize);
		outValueIndexPerProp[i] = v;
	}
}