#pragma once
#include "CShader.h"

/*	[ Shader life-time ]
*	- Never release during runtime.
*	- Released only at engine shutdown.
*/

constexpr const char* ERROR_SHADER_NAME = "Error";

class CShaderManager
{
public:
	CShaderManager() = default;
	~CShaderManager() = default;

	void Initialize(ID3D11Device& refDevice_);
	void Compile();

	const CShader* CreateShader(uint64_t uShaderID_, uint32_t uShaderMacroFlags_);
	const CShader* CreateShaderByName(const string& strName_, uint32_t uShaderMacroFlags_);

	CShader* Get(uint64_t uShaderID_, uint32_t uShaderMacroFlags_);
	const CShader* Get(uint64_t uShaderID_, uint32_t uShaderMacroFlags_) const;

	CShader* Get(const string& strName_, uint32_t uShaderMacroFlags_);
	const CShader* Get(const string& strName_, uint32_t uShaderMacroFlags_) const;

	CShader* GetErrorShader();
	const CShader* GetErrorShader() const;

	bool GetShaderDesc(__in uint64_t uShaderID_, __out SHADER_DESC& refShaderDesc_);
	bool GetShaderDescByName(__in const string& strName_, __out SHADER_DESC& refShaderDesc_);

	void IsCompileDone() const;
public:
	inline size_t GetCurrentCompileWait() const { return m_queCompileWait.size(); }
	inline void SetMaxShaderCompileCount(uint32_t uCount) { m_uMaxShaderCompileCount = std::max(uCount, (uint32_t)1); }

private:
	void _LoadShaderDescs();
	void _LoadErrorFallbackShader();
	vector<D3D_SHADER_MACRO> _ConvertFlagToShaderMacro(uint32_t uMacroFlags_);

private:
	ID3D11Device* m_pDevice = nullptr; // not-own

	unordered_map<string, uint64_t> m_mapShaderNameToID;
	unordered_map<uint64_t, SHADER_DESC> m_mapShaderDesc;
	unordered_map<ShaderKey, unique_ptr<CShader>, ShaderKeyHash> m_mapShaders;

	// shader compile queue
	queue<ShaderKey> m_queCompileWait;
	uint64_t m_uMaxShaderCompileCount = 5; // 한번에 컴파일 할 쉐이더 최대 갯수

	uint64_t m_uErrorShaderID = UINT64_MAX;
};