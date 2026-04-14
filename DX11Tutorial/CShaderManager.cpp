#include "pch.h"
#include "CShaderManager.h"

void CShaderManager::Initialize(ID3D11Device& refDevice_)
{
	m_pDevice = &refDevice_;

	_LoadShaderDescs();
	_LoadErrorFallbackShader();
}

void CShaderManager::Compile()
{
	for (uint32_t i = 0; i < m_uMaxShaderCompileCount; ++i) 
	{
		if (m_queCompileWait.empty()) return;

		const ShaderKey& shaderKey = m_queCompileWait.front();
		auto iter = m_mapShaders.find(shaderKey);
		if (iter != m_mapShaders.end()) 
		{
			CShader* pShader = iter->second.get();
			if (nullptr != pShader) 
			{
				assert(SHADER_COMPILE_STATE::NOT_READY == pShader->GetCompileState());
				HRESULT hr = pShader->Compile(m_pDevice);
#ifdef DEBUG_LOG
				if (FAILED(hr)) {
					cout << "[Shader Compile Error] ID : " << shaderKey.uBaseShaderID << ", Flags : " << shaderKey.uMacroFlags << endl;
				}
#endif // DEBUG_LOG
			}
		}
		m_queCompileWait.pop();
	}
}

const CShader* CShaderManager::CreateShader(uint64_t uShaderID_, uint32_t uShaderMacroFlags_)
{
	const auto& iter = m_mapShaderDesc.find(uShaderID_);
	if (iter == m_mapShaderDesc.end()) {
		assert(false && "Shader desc not found");
		return nullptr;
	}

	ShaderKey toMakeShaderKey{ uShaderID_, uShaderMacroFlags_ };
	auto iterShader = m_mapShaders.find(toMakeShaderKey);

	if (iterShader == m_mapShaders.end()) {
		const SHADER_DESC& desc = iter->second;
		iterShader = m_mapShaders.insert(make_pair(toMakeShaderKey, make_unique<CShader>(desc, _ConvertFlagToShaderMacro(uShaderMacroFlags_)))).first;

		// 이건 중복으로 들어갈 일이 없어...
		m_queCompileWait.push(toMakeShaderKey);
	}
	
	return iterShader->second.get();
}

const CShader* CShaderManager::CreateShaderByName(const string& strName_, uint32_t uShaderMacroFlags_)
{
	const auto& iter = m_mapShaderNameToID.find(strName_);
	if (iter == m_mapShaderNameToID.end()) {
		assert(false && "Shader desc not found");
		return nullptr;
	}

	return CreateShader(iter->second, uShaderMacroFlags_);
}

CShader* CShaderManager::Get(uint64_t uShaderID_, uint32_t uShaderMacroFlags_)
{
	ShaderKey toFindShaderKey{ uShaderID_, uShaderMacroFlags_ };
	auto iterShader = m_mapShaders.find(toFindShaderKey);
	if (iterShader == m_mapShaders.end()) {
		assert(false && "Shader not found");
		return nullptr;
	}
	return iterShader->second.get();
}

const CShader* CShaderManager::Get(uint64_t uShaderID_, uint32_t uShaderMacroFlags_) const
{
	ShaderKey toFindShaderKey {uShaderID_, uShaderMacroFlags_};
	auto iterShader = m_mapShaders.find(toFindShaderKey);
	if (iterShader == m_mapShaders.end()) {
		assert(false && "Shader not found");	
		return nullptr;
	}
	return iterShader->second.get();
}

CShader* CShaderManager::Get(const string& strName_, uint32_t uShaderMacroFlags_)
{
	auto shaderIDPair = m_mapShaderNameToID.find(strName_);
	if (shaderIDPair == m_mapShaderNameToID.end()) {
		assert(false && "Shader not found");
		return nullptr;
	}
	return Get(shaderIDPair->second, uShaderMacroFlags_);
}

const CShader* CShaderManager::Get(const string& strName_, uint32_t uShaderMacroFlags_) const
{
	auto shaderIDPair = m_mapShaderNameToID.find(strName_);
	if (shaderIDPair == m_mapShaderNameToID.end()) {
		assert(false && "Shader not found");
		return nullptr;
	}
	return Get(shaderIDPair->second, uShaderMacroFlags_);
}

CShader* CShaderManager::GetErrorShader()
{
	return Get(m_uErrorShaderID, 0);
}

const CShader* CShaderManager::GetErrorShader() const
{
	return Get(m_uErrorShaderID, 0);
}

bool CShaderManager::GetShaderDesc(__in uint64_t uShaderID_, __out SHADER_DESC& refShaderDesc_)
{
	refShaderDesc_ = SHADER_DESC{};

	auto iterShaderDesc = m_mapShaderDesc.find(uShaderID_);
	if (iterShaderDesc != m_mapShaderDesc.end()) {
		refShaderDesc_ = iterShaderDesc->second;
		return true;
	}
	return false;
}

bool CShaderManager::GetShaderDescByName(__in const string& strName_, __out SHADER_DESC& refShaderDesc_)
{
	auto shaderIDPair = m_mapShaderNameToID.find(strName_);
	if (shaderIDPair != m_mapShaderNameToID.end()) {
		return GetShaderDesc(shaderIDPair->second, refShaderDesc_);
	}

	refShaderDesc_ = SHADER_DESC{};
	return false;
}

void CShaderManager::IsCompileDone() const
{
	assert(m_queCompileWait.empty());
}

void CShaderManager::_LoadShaderDescs()
 {
	 // 하드 경로는 나중에 ini로 빼자
	 filesystem::path shaderDataPath = std::filesystem::current_path().parent_path();
	 shaderDataPath = shaderDataPath / "Shader/shaders.json";

	 ifstream jsonFile(shaderDataPath.string(), std::ios::binary);
	 if (false == jsonFile.is_open()) return;

	 std::stringstream jsonBuffer;
	 jsonBuffer << jsonFile.rdbuf();

	 Document docs;
	 // available comments & Trailing commas
	 docs.Parse<kParseCommentsFlag|kParseTrailingCommasFlag>(jsonBuffer.str().c_str());

	 // close file when end parsing
	 jsonFile.close();

	 if (docs.HasParseError()) {
		 ParseErrorCode ecode = docs.GetParseError();
#ifdef DEBUG_LOG
		 cout << "Shaders.json parsing error!";
#endif // DEBUG_LOG
		 return;
	 }

	 // if it was not OBJECT eject
	 if (!docs.IsObject()) return;

	 for (auto& shaderMember : docs.GetObj()) {
		 string strShaderName(shaderMember.name.GetString());
		 const Value& shaderObject = shaderMember.value;

		 if (!shaderObject.IsObject()) {
#ifdef DEBUG_LOG
			 cout << strShaderName.c_str() << " is not JSON Object!" << endl;
#endif // DEBUG_LOG
			 continue;
		 }

		 uint64_t uShaderID = fnv1a_64(strShaderName);
		 if (m_mapShaderNameToID.contains(strShaderName)) {
#ifdef DEBUG_LOG
			 cout << strShaderName.c_str() << " is already Exist in Shader-map!" << endl;
#endif // DEBUG_LOG
			 continue;
		 }

		 m_mapShaderNameToID.insert(make_pair(strShaderName, uShaderID));
		 SHADER_DESC newShaderDESC;

		 // read VS
		 if (true == shaderObject.HasMember("VS")) {
			 const Value& vs = shaderObject["VS"];
			 if (true == vs.IsObject()) {
				 newShaderDESC.strVertexShaderPath = string(SHADER_BASE_PATH) + string(vs["path"].GetString());
				 newShaderDESC.strVertexShaderEntry = string(vs["entry"].GetString());
				 newShaderDESC.strVertexShaderVersion = string(vs["version"].GetString());
			 }
		 }

		 // read PS
		 if (true == shaderObject.HasMember("PS")) {
			 const Value& ps = shaderObject["PS"];
			 if (true == ps.IsObject()) {
				 newShaderDESC.strPixelShaderPath = string(SHADER_BASE_PATH) + string(ps["path"].GetString());
				 newShaderDESC.strPixelShaderEntry = string(ps["entry"].GetString());
				 newShaderDESC.strPixelShaderVersion = string(ps["version"].GetString());
			 }
		 }
		 m_mapShaderDesc.insert(make_pair(uShaderID, newShaderDESC));
	 }
 }

void CShaderManager::_LoadErrorFallbackShader()
{
	auto iter = m_mapShaderNameToID.find(ERROR_SHADER_NAME);
	assert(iter != m_mapShaderNameToID.end());

	uint64_t shaderID = iter->second;

	// cashing
	m_uErrorShaderID = shaderID;

	auto descIter = m_mapShaderDesc.find(shaderID);
	assert(descIter != m_mapShaderDesc.end());

	ShaderKey toMakeShaderKey{ shaderID, 0 };
	auto iterShader = m_mapShaders.find(toMakeShaderKey);

	if (iterShader == m_mapShaders.end()) {
		const SHADER_DESC& desc = descIter->second;
		iterShader = m_mapShaders.insert(make_pair(toMakeShaderKey, make_unique<CShader>(desc, _ConvertFlagToShaderMacro(toMakeShaderKey.uMacroFlags)))). first;

		// 에러 쉐이더는 바로 컴파일
		HRESULT hr = iterShader->second.get()->Compile(m_pDevice);
		assert(SUCCEEDED(hr));
	}
}

vector<D3D_SHADER_MACRO> CShaderManager::_ConvertFlagToShaderMacro(uint32_t uMacroFlags_)
{
	vector<D3D_SHADER_MACRO> vecMacros;

	// instancing
	if (uMacroFlags_ & static_cast<uint32_t>(SHADER_MACRO::USE_INSTANCING)) {
		vecMacros.push_back({ "USE_INSTANCING", "" });
	}

	if (uMacroFlags_ & static_cast<uint32_t>(SHADER_MACRO::USE_SKINNING)) {
		vecMacros.push_back({ "USE_SKINNING", "" });
	}

	if (uMacroFlags_ & static_cast<uint32_t>(SHADER_MACRO::USE_SHADOW)) {
		vecMacros.push_back({ "USE_SHADOW", "" });
	}

	vecMacros.push_back({ nullptr, nullptr });
	return vecMacros;
}