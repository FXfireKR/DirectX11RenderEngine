#pragma once
#include "ShaderTypes.h"

constexpr const char* SHADER_BASE_PATH = "../Shader/";

enum class SHADER_COMPILE_STATE
{
	NOT_READY,
	COMPILING,
	READY,
	FAIL,
};

struct SHADER_DESC
{
	string strVertexShaderPath = "";
	string strVertexShaderVersion = "vs_5_0";
	string strVertexShaderEntry = "VS";
	string strPixelShaderPath = "";
	string strPixelShaderVersion = "ps_5_0";
	string strPixelShaderEntry = "PS";
};

class CShader
{
public:
	CShader(const CShader&) = delete;
	CShader& operator=(const CShader&) = delete;

	CShader(SHADER_DESC shaderDesc_, vector<D3D_SHADER_MACRO>&& vecMacros_);
	~CShader();

	void Apply(ID3D11DeviceContext* pContext_) const;
	HRESULT Compile(ID3D11Device* const pDevice_);
	void ReleaseBlobs();
	
public:
	inline bool GetCompiled() { return m_eCompileState == SHADER_COMPILE_STATE::FAIL || m_eCompileState == SHADER_COMPILE_STATE::READY; }
	inline bool IsUsable() { return m_eCompileState == SHADER_COMPILE_STATE::READY; }
	inline SHADER_COMPILE_STATE GetCompileState() { return m_eCompileState; }

	inline ID3D11VertexShader* GetVertexShader() const { return m_pVertexShader.Get(); }
	inline ID3D11PixelShader* GetPixelShader() const { return m_pPixelShader.Get(); }
	inline ID3DBlob* GetVertexBlob() const { return m_pVertexBlob.Get(); }
	inline ID3DBlob* GetPixelBlob() const { return m_pPixelBlob.Get(); }
	
private:
	ComPtr<ID3D11VertexShader> m_pVertexShader = nullptr;
	ComPtr<ID3D11PixelShader> m_pPixelShader = nullptr;
	ComPtr<ID3DBlob> m_pVertexBlob = nullptr;
	ComPtr<ID3DBlob> m_pPixelBlob = nullptr;
	
	vector<ComPtr<ID3D11Buffer>> m_vecVertexConstBuffers;
	vector<ComPtr<ID3D11Buffer>> m_vecPixelConstBuffers;

	SHADER_DESC m_shaderDesc;
	vector<D3D_SHADER_MACRO> m_vecShaderMacro;
	SHADER_COMPILE_STATE m_eCompileState = SHADER_COMPILE_STATE::NOT_READY;
};