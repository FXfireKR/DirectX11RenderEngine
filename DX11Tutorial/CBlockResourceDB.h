#pragma once
#include "singleton.h"

#include "CRuntimeAtlas.h"
#include "CRuntimeAtlasBuilder.h"
#include "CSoundDataBase.h"

struct BlockCell;

class CBlockResourceDB : public singleton<CBlockResourceDB>
{
public:
	void Initialize(const char* resourceRoot, ID3D11Device* pDevice);
	bool Load();
	void Clear();

public: // RuntimeAtlas

	bool RegisterTextureKey(const char* textureKey);
	bool RegisterTextureKeys(const vector<string>& keys);
	bool RegisterTextureKeys(const unordered_set<string>& keys);

	const AtlasRegion* FindAtlasRegion(const char* textureKey) const;
	bool TryGetRegion(const char* textureKey, AtlasRegion& outRegion) const;
	bool TryGetBlockParticleRegion(const char* textureKey, XMFLOAT2& outMinUV, XMFLOAT2& outMaxUV) const;

	inline ID3D11ShaderResourceView* GetAtlasTextureView() const { return m_runtimeAtlas.GetShaderResourceView(); }

public: // SoundDataBase

	inline const SoundEventDef* FindEvent(SoundEventID eventID) const { return m_soundDatabase.FindEvent(eventID); }
	inline const SoundEventDef* FindEvent(const char* eventName) const { return m_soundDatabase.FindEvent(eventName); }
	inline bool ResolveEvent(SoundEventID eventID, ResolvedSound& outResolved) const { return m_soundDatabase.ResolveEvent(eventID, outResolved); }
	inline bool ResolveEvent(const char* eventName, ResolvedSound& outResolved) const { return m_soundDatabase.ResolveEvent(eventName, outResolved); }
	inline bool ResolveBlock(BLOCK_ID blockID, EBlockSoundUsage usage, ResolvedSound& outResolved) const { return m_soundDatabase.ResolveBlock(blockID, usage, outResolved); }

	inline queue<AudioLoadElemDesc>& GetPreLoadQueue() { return m_soundDatabase.GetPreLoadQueue(); }

private:
	bool _ResolveTextureFilePath(const char* textureKey, string& outPath) const;
	bool _BuildAtlasInputs(vector<AtlasBuildInput>& outInputs) const;


private:
	ID3D11Device* m_pDevice = nullptr;
	string m_strRoot;

	bool m_bInit = false;
	bool m_bLoadComplete = false;

	CRuntimeAtlasBuilder m_runtimeAtlasBuilder;
	CRuntimeAtlas m_runtimeAtlas;
	CSoundDataBase m_soundDatabase;

	unordered_set<string> m_setTextureKeys;
};