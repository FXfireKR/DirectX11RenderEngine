#pragma once
#include "VoxelTypes.h"
#include "ModelTypes.h"
#include "ModelUtil.h"

struct ModelEntry
{
	string key;
	bool bRawLoaded = false;	
	ModelRaw raw;

	bool bResolved = false;
	ModelResolved resolved;

	bool bBaked = false;
	BakedModel baked;
};

class CModelDB
{
public:
	void Initialize(const string& resourceRoot);
	void Load();
	void Clear();

	bool SubmitToLoad(const char* modelKey);
	MODEL_ID LoadModel(const char* modelKey);

	const MODEL_ID FindModelID(uint64_t modelHash) const;
	const MODEL_ID FindModelID(const char* modelKey) const;

	const BakedModel* GetBakedModel(MODEL_ID id) const;

	const BakedModel* FindBakedModel(uint64_t modelHash) const;
	const BakedModel* FindBakedModel(const char* modelKey) const;

	inline const unordered_set<string>& GetUsedTextureKeys() const { return m_usedTextureKeys; }

private:
	bool _LoadRawModelJSON(IN const char* modelKey, OUT ModelRaw& modelRaw);
	bool _ResolveModel(IN const char* modelKey, OUT ModelResolved& modelResolved);
	bool _ResolveParentRecursive(IN const char* modelKey, ModelResolved& modelResolved, unordered_set<string>& visitedStack);

	string _ResolveTextureRef(const ModelResolved& model, const string& texRef, int depth = 0);
	void _BakeElements(IN const ModelResolved& modelResolved, OUT BakedModel& bakedModel);
	void _BakeOneElementFace(IN const ModelResolved& modelResolved, const ModelElement& modelElem, int faceDir, OUT BakedModel& bakedModel);

	string _BuildModelPath(string resourceRoot, string modelKey);
	XMFLOAT3 RotatePointAroundOrigin01(const XMFLOAT3& p, const XMFLOAT3& origin01, ROT_AXIS axis, float angleDeg);

private:
	string m_strResourceRoot;
	vector<unique_ptr<ModelEntry>> m_vecEntries;
	unordered_map<uint64_t, MODEL_ID> m_mapKeyToID;

	queue<string> m_queueModelList;

	unordered_set<string> m_usedTextureKeys;
};