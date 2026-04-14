#include "pch.h"
#include "CInitializeScene.h"
#include "CRenderWorld.h"

void CInitializeScene::Awake()
{
	BlockDB.Initialize("../Resource/");
	BlockResDB.Initialize("../Resource/", GetRenderWorld().GetDevice());
}

void CInitializeScene::Update(float fDelta)
{
#ifdef IMGUI_ACTIVATE
	ImGui::Begin("Initialize");
	ImGui::Text("BlockDB        : %s", m_iCurrentLoadStep > static_cast<uint8_t>(INITIALIZE_STEP::BLOCK_DB_LOAD) ? "DONE" : "LOADING");
	ImGui::Text("BlockRes       : %s", m_iCurrentLoadStep > static_cast<uint8_t>(INITIALIZE_STEP::BLOCK_RESOURCE_DB_LOAD) ? "DONE" : "LOADING");
	ImGui::Text("Render Warmup  : %s", m_iCurrentLoadStep > static_cast<uint8_t>(INITIALIZE_STEP::RENDER_WARM) ? "DONE" : "WARMING");
	ImGui::End();
#endif // IMGUI_ACTIVATE

	switch (static_cast<INITIALIZE_STEP>(m_iCurrentLoadStep))
	{
		case CInitializeScene::INITIALIZE_STEP::BLOCK_DB_LOAD:
		{
			BlockDB.Load();
			if (BlockDB.IsLoadedComplete())
				++m_iCurrentLoadStep;
		} break;
		case CInitializeScene::INITIALIZE_STEP::BLOCK_RESOURCE_DB_LOAD:
		{
			BlockResDB.RegisterTextureKeys(BlockDB.GetUsedTextureKeys());
			BlockResDB.Load();

			++m_iCurrentLoadStep;
		} break;
		case CInitializeScene::INITIALIZE_STEP::SOUND_PRE_LOAD:
		{
			auto& preloadQueue = BlockResDB.GetPreLoadQueue();
			if (preloadQueue.empty())
				++m_iCurrentLoadStep;
			else
			{
				int curBudget = 4;
				while (curBudget > 0 && !preloadQueue.empty())
				{
					const auto& elem = preloadQueue.front();
					GetAudioSystem().LoadSound(elem.id, elem.objectPath.c_str(), elem.desc.b3D, elem.desc.bLoop);
					preloadQueue.pop();

					--curBudget;
				}
			}
		} break;
		case CInitializeScene::INITIALIZE_STEP::RENDER_WARM:
		{
			_WarmupGameSceneRenderResources();
			++m_iCurrentLoadStep;
		} break;
		case CInitializeScene::INITIALIZE_STEP::END:
		{
			m_bChangeSceneReq = true;
			m_eNextSceneReq = SCENE_TYPE::GAME_SCENE;
		} break;
	}
}

bool CInitializeScene::_WarmupGameSceneRenderResources()
{
	CRenderWorld& rw = GetRenderWorld();

	auto& shaderManager = rw.GetShaderManager();
	auto& ilManager = rw.GetIALayoutManager();
	auto& pipelineManager = rw.GetPipelineManager();
	auto& meshManager = rw.GetMeshManager();
	auto& materialManager = rw.GetMaterialManager();
	auto& textureManager = rw.GetTextureManager();
	auto& samplerManager = rw.GetSamplerManager();

	const uint64_t normalShaderID = fnv1a_64("NormalImageForward");
	const uint64_t cutoutShaderID = fnv1a_64("NormalImageCutout");
	const uint64_t shadowShaderID = fnv1a_64("ShadowDepth");
	const uint64_t highlightShaderID = fnv1a_64("Highlight");
	const uint64_t uiShaderID = fnv1a_64("UIInvertMask");
	const uint64_t skyShaderID = fnv1a_64("SkyBillboard");
	const uint64_t skyCloudShaderID = fnv1a_64("SkyCloud");
	const uint64_t skyStarShaderID = fnv1a_64("SkyStar");

	// 1) shader queue 등록
	shaderManager.CreateShader(normalShaderID, 0);
	shaderManager.CreateShader(cutoutShaderID, 0);
	shaderManager.CreateShader(shadowShaderID, 0);
	shaderManager.CreateShader(highlightShaderID, 0);
	shaderManager.CreateShader(uiShaderID, 0);
	shaderManager.CreateShader(skyShaderID, 0);
	shaderManager.CreateShader(skyCloudShaderID, 0);
	shaderManager.CreateShader(skyStarShaderID, 0);

	// 2) 큐를 여기서 다 소모
	shaderManager.SetMaxShaderCompileCount(64);
	while (shaderManager.GetCurrentCompileWait() > 0)
	{
		shaderManager.Compile();
	}

	// 3) input layout
	const uint32_t normalLayoutID = ilManager.Create(
		VERTEX_POSITION_NORMAL_UV_COLOR::GetLayout(), { normalShaderID, 0 },
		shaderManager.Get(normalShaderID, 0)->GetVertexBlob());

	const uint32_t cutoutLayoutID = ilManager.Create(
		VERTEX_POSITION_NORMAL_UV_COLOR::GetLayout(), { cutoutShaderID, 0 },
		shaderManager.Get(cutoutShaderID, 0)->GetVertexBlob());

	const uint32_t shadowLayoutID = ilManager.Create(
		VERTEX_POSITION_NORMAL_UV_COLOR::GetLayout(), { shadowShaderID, 0 },
		shaderManager.Get(shadowShaderID, 0)->GetVertexBlob());

	const uint32_t highlightLayoutID = ilManager.Create(
		VERTEX_POSITION::GetLayout(), { highlightShaderID, 0 },
		shaderManager.Get(highlightShaderID, 0)->GetVertexBlob());

	const uint32_t uiLayoutID = ilManager.Create(
		VERTEX_POSITION_UV::GetLayout(), { uiShaderID, 0 },
		shaderManager.Get(uiShaderID, 0)->GetVertexBlob());

	const uint32_t skyLayoutID = ilManager.Create(
		VERTEX_POSITION_UV::GetLayout(), { skyShaderID, 0 },
		shaderManager.Get(skyShaderID, 0)->GetVertexBlob());

	const uint32_t skyCloudLayoutID = ilManager.Create(
		VERTEX_POSITION_UV::GetLayout(), { skyCloudShaderID, 0 },
		shaderManager.Get(skyCloudShaderID, 0)->GetVertexBlob());

	const uint32_t skyStarLayoutID = ilManager.Create(
		VERTEX_POSITION_UV::GetLayout(), { skyStarShaderID, 0 },
		shaderManager.Get(skyStarShaderID, 0)->GetVertexBlob());

	// 4) sampler
	const uint64_t pointWrapSamplerID = samplerManager.Create(SAMPLER_TYPE::POINT_WRAP);
	const uint64_t shadowSamplerID = samplerManager.Create(SAMPLER_TYPE::SHADOWCOMPARISON);
	const uint64_t linearWarpSamplerID = samplerManager.Create(SAMPLER_TYPE::LINEAR_WARP);
	const uint64_t linearClampSamplerID = samplerManager.Create(SAMPLER_TYPE::LINEAR_CLAMP);

	// 5) chunk pipelines
	{
		auto opaquePipeID = pipelineManager.Create(fnv1a_64("ChunkPipeline"));
		auto* opaquePipeline = pipelineManager.Get(opaquePipeID);
		opaquePipeline->SetShader(shaderManager.Get(normalShaderID, 0));
		opaquePipeline->SetInputLayout(ilManager.Get(normalLayoutID));
		opaquePipeline->CreateOpaqueState(rw.GetDevice());
		opaquePipeline->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		auto cutoutPipeID = pipelineManager.Create(fnv1a_64("ChunkCutoutPipeline"));
		auto* cutoutPipeline = pipelineManager.Get(cutoutPipeID);
		cutoutPipeline->SetShader(shaderManager.Get(cutoutShaderID, 0));
		cutoutPipeline->SetInputLayout(ilManager.Get(cutoutLayoutID));
		cutoutPipeline->CreateCutoutAlphaTestState(rw.GetDevice(), false);
		cutoutPipeline->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		auto transPipeID = pipelineManager.Create(fnv1a_64("ChunkTransparentPipeline"));
		auto* transPipeline = pipelineManager.Get(transPipeID);
		transPipeline->SetShader(shaderManager.Get(normalShaderID, 0));
		transPipeline->SetInputLayout(ilManager.Get(normalLayoutID));
		transPipeline->CreateTransparentAlphaState(rw.GetDevice(), false);
		transPipeline->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		auto shadowPipeID = pipelineManager.Create(fnv1a_64("ChunkShadowPipeline"));
		auto* shadowPipeline = pipelineManager.Get(shadowPipeID);
		shadowPipeline->SetShader(shaderManager.Get(shadowShaderID, 0));
		shadowPipeline->SetInputLayout(ilManager.Get(shadowLayoutID));
		shadowPipeline->CreateOpaqueState(rw.GetDevice());
		shadowPipeline->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	// 6) chunk materials
	{
		auto opaqueMatID = materialManager.Create(fnv1a_64("ChunkMaterial"));
		auto* opaqueMat = materialManager.Get(opaqueMatID);
		opaqueMat->SetTexture(0, BlockResDB.GetAtlasTextureView());
		opaqueMat->SetSampler(0, samplerManager.Get(pointWrapSamplerID)->Get());
		opaqueMat->SetTexture(1, rw.GetShadowMapSRV());
		opaqueMat->SetSampler(1, samplerManager.Get(shadowSamplerID)->Get());

		auto cutoutMatID = materialManager.Create(fnv1a_64("ChunkCutoutMaterial"));
		auto* cutoutMat = materialManager.Get(cutoutMatID);
		cutoutMat->SetTexture(0, BlockResDB.GetAtlasTextureView());
		cutoutMat->SetSampler(0, samplerManager.Get(pointWrapSamplerID)->Get());
		cutoutMat->SetTexture(1, rw.GetShadowMapSRV());
		cutoutMat->SetSampler(1, samplerManager.Get(shadowSamplerID)->Get());

		auto transMatID = materialManager.Create(fnv1a_64("ChunkTransparentMaterial"));
		auto* transMat = materialManager.Get(transMatID);
		transMat->SetTexture(0, BlockResDB.GetAtlasTextureView());
		transMat->SetSampler(0, samplerManager.Get(pointWrapSamplerID)->Get());
		transMat->SetTexture(1, rw.GetShadowMapSRV());
		transMat->SetSampler(1, samplerManager.Get(shadowSamplerID)->Get());
	}

	// 7) highlight
	{
		auto pipeID = pipelineManager.Create(fnv1a_64("HighlightPipeline"));
		auto* pipeline = pipelineManager.Get(pipeID);
		pipeline->SetShader(shaderManager.Get(highlightShaderID, 0));
		pipeline->SetInputLayout(ilManager.Get(highlightLayoutID));
		pipeline->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		pipeline->CreateOpaqueState(rw.GetDevice());

		meshManager.CreateAABBLine(fnv1a_64("AABBHighlight"));
		materialManager.Create(fnv1a_64("HighlightMaterial"));
	}

	// 8) crosshair
	{
		auto pipeID = pipelineManager.Create(fnv1a_64("UICrosshairPipeline"));
		auto* pipeline = pipelineManager.Get(pipeID);
		pipeline->SetShader(shaderManager.Get(uiShaderID, 0));
		pipeline->SetInputLayout(ilManager.Get(uiLayoutID));
		pipeline->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pipeline->CreateUIInvertState(rw.GetDevice(), true);

		meshManager.CreateQuad(fnv1a_64("UICrosshairQuad"));

		auto matID = materialManager.Create(fnv1a_64("UICrosshairMaterial"));
		auto* mat = materialManager.Get(matID);

		const uint64_t texID = textureManager.LoadTexture2D(
			fnv1a_64("ui/crosshair"),
			"../Resource/assets/minecraft/textures/gui/sprites/hud/crosshair.png",
			TEXTURE_USAGE::StaticColor);

		mat->SetSampler(0, samplerManager.Get(linearWarpSamplerID)->Get());
		mat->SetTexture(0, textureManager.GetTexture(texID)->GetShaderResourceView());
	}

	// 9) sky billboard
	{
		auto pipeID = pipelineManager.Create(fnv1a_64("BillbaordPipeline"));
		auto* pipeline = pipelineManager.Get(pipeID);
		pipeline->SetShader(shaderManager.Get(skyShaderID, 0));
		pipeline->SetInputLayout(ilManager.Get(skyLayoutID));
		pipeline->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pipeline->CreateSkyAlphaState(rw.GetDevice(), true);

		meshManager.CreateQuad(fnv1a_64("SkyBillboardQuad"));

		auto* sunMat = materialManager.Ensure(fnv1a_64("SunBillboardMaterial"));
		auto* moonMat = materialManager.Ensure(fnv1a_64("MoonBillboardMaterial"));

		const uint64_t sunTexID = textureManager.LoadTexture2D(
			fnv1a_64("sun"),
			"../Resource/assets/minecraft/textures/environment/celestial/sun.png",
			TEXTURE_USAGE::StaticColor);

		const uint64_t moonTexID = textureManager.LoadTexture2D(
			fnv1a_64("moon"),
			"../Resource/assets/minecraft/textures/environment/celestial/moon/full_moon.png",
			TEXTURE_USAGE::StaticColor);

		sunMat->SetSampler(0, samplerManager.Get(pointWrapSamplerID)->Get());
		sunMat->SetTexture(0, textureManager.GetTexture(sunTexID)->GetShaderResourceView());

		moonMat->SetSampler(0, samplerManager.Get(pointWrapSamplerID)->Get());
		moonMat->SetTexture(0, textureManager.GetTexture(moonTexID)->GetShaderResourceView());
	}

	return true;
}