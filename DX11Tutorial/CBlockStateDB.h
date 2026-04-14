#pragma once
#include "ModelTypes.h"
#include "BlockStateTypes.h"

/*
* VALUE가 INDEX로 저장되므로, 단계를 나눈다.
* Pass_1 : variants key 문자열들을 모아서 peding 시킴
* Finalize : 정렬해서 Hash-map 구축, stride, totalState 계산
* Pass_2 : variants key 문자열들을 기반으로 컴파일된 VariantRule 생성
*/

class CBlockDefDB;
class CModelDB;

class CBlockStateDB
{
private:
	enum class LOAD_PROGRESS_STEP
	{
		INIT,
		COLLECT_DOMAINS,
		FINALIZE_ALL,
		COMPILE_RULES,
		END,
	};

public:
	void Initialize(const char* path, const CBlockDefDB* pBlockDefDB, CModelDB* pModelDB);
	void Clear();
	void Load();

	bool GetAppliedModels(IN BLOCK_ID blockID, STATE_INDEX stateIndex, OUT vector<AppliedModel>& vecAppliedModels) const;
	bool GetAppliedModels(IN BLOCK_ID blockID, STATE_INDEX stateIndex, OUT const vector<AppliedModel>*& vecAppliedModels) const;
	bool EncodeStateIndex(IN BLOCK_ID blockID, const BlockPropHashMap& props, OUT STATE_INDEX& stateIndex) const;
	bool TryGetStateValueHash(BLOCK_ID blockID, STATE_INDEX stateIndex, PROP_HASH propHash, VALUE_HASH& outValueHash) const;

private:
	bool _ScanBlockStateFiles(vector<filesystem::path>& outFiles) const;

	bool _Pass1_CollectDomains(const filesystem::path& file);
	bool _FinalizeAllBlockTypes();
	bool _Pass2_CompileRules(const filesystem::path& file);

	//bool _Pass1_CollectDomains(const vector<filesystem::path>& files);
	//bool _FinalizeAllBockTypes();
	//bool _Pass2_CompileRules(const vector<filesystem::path>& files);

	bool _ReadJson(const std::filesystem::path& path, rapidjson::Document& outDoc) const;
	bool _ReadVariants_Pass1(BLOCK_ID blockID, const rapidjson::Value& variants);
	bool _ReadVariants_Pass2(BLOCK_ID blockID, const rapidjson::Value& variants);

	// keyStr 파서
	bool _ParseKeyTerms_ToHashes(const char* keyStr, std::vector<std::pair<uint64_t, uint64_t>>& out) const; // (propNameHash, valueHash)
	bool _CompileKeyTerms_ToTerms(const BlockTypeDef& typeDef, const char* keyStr, std::vector<Term>& outTerms) const;

	// ModelSpec 파서(네가 이미 구현한 걸 붙이되, 아래 시그니처 유지 추천)
	static int  _NormalizeRot(int val);
	static bool _ReadModelSpec(const rapidjson::Value& value, AppliedModel& outModel);

	// registry/get-create
	BlockTypeDef& _GetOrCreateBlockType(BLOCK_ID blockID);
	PropertyDomain& _GetOrCreateDomain(BlockTypeDef& typeDef, PROPERTY_ID pid);

	const BlockTypeDef* _FindBlockType(BLOCK_ID blockID) const;
	const BlockStateDef* _FindBlockState(BLOCK_ID blockID) const;

	// stateIndex decode (valueIndex per prop slot)
	void _DecodeStateIndex(const BlockTypeDef& typeDef, STATE_INDEX sidx, std::vector<VALUE_INDEX>& outValueIndexPerProp) const;

public:
	inline const size_t& GetCurrentLoadedCnt() { return m_uLoadedFiles; }
	inline const size_t& GetMaxLoadedCnt() { return m_uMaxFiles; }
	inline bool IsLoadComplete() { return m_eStep == LOAD_PROGRESS_STEP::END; }

private:
	const CBlockDefDB* m_pBlockDefDB = nullptr;
	CModelDB* m_pModelDB = nullptr;

	string m_strRoot;
	GlobalPropertyRegistry m_propRegistry;

	unordered_map<BLOCK_ID, BlockStateDef> m_mapBlockState;
	unordered_map<BLOCK_ID, BlockTypeDef> m_mapBlockType;

	LOAD_PROGRESS_STEP m_eStep = LOAD_PROGRESS_STEP::INIT;
	vector<filesystem::path> m_vecFiles;
	size_t m_uMaxFiles = 0;
	size_t m_uLoadedFiles = 0;
	const size_t MAX_FILE_LOAD_CNT = 3;
};