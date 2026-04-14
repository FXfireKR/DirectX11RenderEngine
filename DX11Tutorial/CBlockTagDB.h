#pragma once
#include "BlockTagTypes.h"

class CBlockDefDB;

class CBlockTagDB
{
public:
	CBlockTagDB() = default;
	~CBlockTagDB() = default;

	void Initialize(const char* resourceRoot, const CBlockDefDB* pBlockDefDB);
	bool Load();
	void Clear();

	bool HasTag(BLOCK_ID blockID, const char* tagName) const;
	bool HasTagHash(BLOCK_ID blockID, BLOCK_TAG_HASH tagHash) const;

	const BlockTagDef* FindTagDef(const char* tagName) const;
	const BlockTagRaw* FindTagRaw(const char* tagName) const;

	BLOCK_TAG_HASH FindTagHash(const char* tagName) const;

	inline const unordered_map<string, BlockTagDef>& GetTagDefs() const { return m_mapTagDefs; }

private:
	bool _LoadTagFiles();
	bool _LoadOneTagFile(const filesystem::path& filePath);

	bool _ReadJson(const filesystem::path& path, rapidjson::Document& outDoc) const;
	bool _ParseTagRaw(const rapidjson::Value& root, const string& tagName, BlockTagRaw& outRaw);

	bool _BuildRuntimeTags();
	bool _ResolveTag(const string& tagName, BlockTagDef& outDef, unordered_set<string>& visiting);

	void _BuildReverseLookup();

private:
	static bool _IsTagReference(const string& value);
	static string _NormalizeTagPathToName(const filesystem::path& filePath);
	static string _StripTagPrefix(const string& value);
	static BLOCK_TAG_HASH _HashTag(const char* tag);


private:
	string m_strResourceRoot;
	const CBlockDefDB* m_pBlockDefDB = nullptr;

	//raw
	unordered_map<string, BlockTagRaw> m_mapTagRaws;

	// runtime
	unordered_map<string, BlockTagDef> m_mapTagDefs;
	unordered_map<BLOCK_TAG_HASH, string> m_mapHashToTagName;

	// reverse cache : blockID -> tagHashes
	vector<unordered_set<BLOCK_TAG_HASH>> m_vecBlockTagHashes;
};