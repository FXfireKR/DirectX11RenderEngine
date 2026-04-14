#include "pch.h"
#include "CModelDB.h"
#include "ModelUtil.h"
#include "ResourceUtil.h"

void CModelDB::Initialize(const string& resourceRoot)
{
    m_strResourceRoot = resourceRoot;
}

void CModelDB::Load()
{
    while (!m_queueModelList.empty())
    {
        LoadModel(m_queueModelList.front().c_str());
        m_queueModelList.pop();
    }
}

void CModelDB::Clear()
{
    m_vecEntries.clear();
    m_mapKeyToID.clear();
    m_strResourceRoot.clear();
    m_usedTextureKeys.clear();
}

bool CModelDB::SubmitToLoad(const char* modelKey)
{
    const uint64_t hash = fnv1a_64(modelKey);
    auto it = m_mapKeyToID.find(hash);
    if (it != m_mapKeyToID.end()) 
        return false;

    m_queueModelList.push(modelKey);
    return true;
}

MODEL_ID CModelDB::LoadModel(const char* modelKey)
{
    const uint64_t h = fnv1a_64(modelKey);
    auto it = m_mapKeyToID.find(h);
    if (it != m_mapKeyToID.end()) 
        return it->second;

    MODEL_ID newID = static_cast<MODEL_ID>(m_vecEntries.size());
    auto entry = std::make_unique<ModelEntry>();
    entry->key = modelKey;

    ModelResolved resolved;
    if (!_ResolveModel(modelKey, resolved))
    {
        // fail
        return UINT64_ERROR;
    }
    else
    {
        entry->bResolved = true;
        entry->resolved = std::move(resolved);
        entry->bBaked = true;
        _BakeElements(entry->resolved, entry->baked);
    }

    m_vecEntries.push_back(std::move(entry));
    m_mapKeyToID[h] = newID;
    return newID;
}

const MODEL_ID CModelDB::FindModelID(uint64_t modelHash) const
{
    auto it = m_mapKeyToID.find(modelHash);
    if (it != m_mapKeyToID.end()) 
        return it->second;

    return UINT64_ERROR;
}

const MODEL_ID CModelDB::FindModelID(const char* modelKey) const
{
    const uint64_t modelHash = fnv1a_64(modelKey);
    auto it = m_mapKeyToID.find(modelHash);
    if (it != m_mapKeyToID.end()) 
        return it->second;

    return UINT64_ERROR;
}

const BakedModel* CModelDB::GetBakedModel(MODEL_ID id) const
{
    if (id >= m_vecEntries.size()) return nullptr;
    const auto& e = m_vecEntries[id];
    if (!e || !e->bBaked) 
        return nullptr;

    return &(e->baked);
}

const BakedModel* CModelDB::FindBakedModel(uint64_t modelHash) const
{
    MODEL_ID modelID = FindModelID(modelHash);
    if (modelID == UINT64_ERROR) 
        return nullptr;

    return GetBakedModel(modelID);
}

const BakedModel* CModelDB::FindBakedModel(const char* modelKey) const
{
    MODEL_ID modelID = FindModelID(modelKey);
    if (modelID == UINT64_ERROR) 
        return nullptr;

    return GetBakedModel(modelID);
}

bool CModelDB::_LoadRawModelJSON(IN const char* modelKey, OUT ModelRaw& modelRaw)
{
    // model key 입력 예시 "minecraft:block/stone"
    // 변환 예시 "assets/minecraft/models/block/stone.json"

    string path = _BuildModelPath(m_strResourceRoot, modelKey);
    std::stringstream jsonBuffer;
    IFileStreamWrapper::ReadAllStream(path, jsonBuffer);

    // available comments & Trailing commas
    Document docs;
    docs.Parse<kParseCommentsFlag | kParseTrailingCommasFlag>(jsonBuffer.str().c_str());
    
    if (docs.HasParseError() || !docs.IsObject()) 
    {
        ParseErrorCode ecode = docs.GetParseError();
#ifdef DEBUG_LOG
        cout << "Shaders.json parsing error!";
#endif // DEBUG_LOG
        return false;
    }

    modelRaw = ModelRaw{};

    // parent
    if (docs.HasMember("parent"))
    {
        const auto& parent = docs["parent"];
        if (parent.IsString())
        {
            modelRaw.bHasParent = true;
            modelRaw.parentKey = parent.GetString();
        }
    }

    // texture
    if (docs.HasMember("textures"))
    {
        const auto& textures = docs["textures"];
        if (textures.IsObject())
        {
            for (auto it = textures.MemberBegin(); it != textures.MemberEnd(); ++it)
            {
                const auto& key = it->name;
                const auto& value = it->value;
                if (!key.IsString() || !value.IsString()) continue;

                modelRaw.textures[key.GetString()] = value.GetString();
            }
        }
    }
    
    // elements
    if (docs.HasMember("elements"))
    {
        const auto& elements = docs["elements"];
        if (elements.IsArray())
        {
            modelRaw.elements.reserve(elements.Size());

            for (rapidjson::SizeType i = 0; i < elements.Size(); ++i) 
            {
                const auto& elem = elements[i];
                if (!elem.IsObject()) continue;

                ModelElement modelElem{};

                if (elem.HasMember("from")) CRapidJsonParsorWrapper::ReadVector3(elem["from"], modelElem.from);
                if (elem.HasMember("to")) CRapidJsonParsorWrapper::ReadVector3(elem["to"], modelElem.to);

                // rotation
                if (elem.HasMember("rotation"))
                {
                    const auto& r = elem["rotation"];
                    if (r.IsObject())
                    {
                        modelElem.bHasRotation = true;

                        if (r.HasMember("origin"))
                            CRapidJsonParsorWrapper::ReadVector3(r["origin"], modelElem.rotOrigin);

                        if (r.HasMember("axis") && r["axis"].IsString())
                        {
                            ROT_AXIS eAxis;
                            if (true == TryParseAxis(r["axis"].GetString(), eAxis))
                            {
                                modelElem.rotAxis = static_cast<uint8_t>(eAxis);
                            }
                        }

                        if (r.HasMember("angle") && r["angle"].IsNumber())
                            modelElem.rotAngleDeg = r["angle"].GetFloat();

                        if (r.HasMember("rescale") && r["rescale"].IsBool())
                            modelElem.bRescale = r["rescale"].GetBool();
                    }
                }

                // face
                if (elem.HasMember("faces"))
                {
                    const auto& faces = elem["faces"];
                    if (faces.IsObject())
                    {
                        for (auto it = faces.MemberBegin(); it != faces.MemberEnd(); ++it)
                        {
                            if (!it->name.IsString())
                                continue;

                            const char* faceName = it->name.GetString();

                            FACE_DIR eDir;
                            if (!TryParseFaceDir(faceName, eDir))
                                continue;

                            uint8_t dir = static_cast<uint8_t>(eDir);

                            const auto& f = it->value;
                            if (!f.IsObject())
                                continue;

                            modelElem.bHasFace[dir] = true;
                            ModelFace face{};

                            // texture 필수
                            if (f.HasMember("texture") && f["texture"].IsString())
                                face.textureRef = f["texture"].GetString();

                            // uv optional
                            if (f.HasMember("uv"))
                            {
                                const auto& uv = f["uv"];
                                if (CRapidJsonParsorWrapper::ReadUV4(uv, face.uv))
                                    face.bHasUV = true;
                            }

                            // cullface optional
                            if (f.HasMember("cullface") && f["cullface"].IsString())
                            {
                                if (!TryParseFaceDir(f["cullface"].GetString(), eDir)) 
                                    continue;

                                const int cdir = static_cast<int>(eDir);
                                if (cdir >= 0 && cdir < (int)FACE_DIR::COUNT)
                                {
                                    face.bHasCullFace = true;
                                    face.cullFaceDir = (uint8_t)cdir;
                                }
                            }

                            // rotation optional (0/90/180/270)
                            if (f.HasMember("rotation") && f["rotation"].IsInt())
                                face.rotation = (uint8_t)f["rotation"].GetInt();

                            // tintindex optional
                            if (f.HasMember("tintindex") && f["tintindex"].IsInt())
                                face.tintIndex = f["tintindex"].GetInt();

                            modelElem.faces[dir] = std::move(face);
                        }
                    }
                }

                modelRaw.elements.push_back(std::move(modelElem));
            }
        }
    }
    return true;
}

bool CModelDB::_ResolveModel(IN const char* modelKey, OUT ModelResolved& modelResolved)
{
    modelResolved = ModelResolved{};
    unordered_set<string> visit;
    return _ResolveParentRecursive(modelKey, modelResolved, visit);
}

bool CModelDB::_ResolveParentRecursive(IN const char* modelKey, ModelResolved& modelResolved, unordered_set<string>& visitedStack)
{
    // cycle detect
    if (visitedStack.contains(modelKey)) return false;

    visitedStack.insert(modelKey);

    ModelRaw raw;
    if (!_LoadRawModelJSON(modelKey, raw)) return false;

    // parent first resolve
    ModelResolved parentResolved;
    bool bHasParent = raw.bHasParent;

    if (bHasParent)
    {
        if (!_ResolveParentRecursive(raw.parentKey.c_str(), parentResolved, visitedStack)) return false;

        modelResolved.textures = parentResolved.textures;

        modelResolved.elements = parentResolved.elements;
    }

    // child-run zzzz
    for (auto& kv : raw.textures)
        modelResolved.textures[kv.first] = kv.second;

    // elem
    if (!raw.elements.empty())
        modelResolved.elements = raw.elements;

    visitedStack.erase(modelKey);
    return true;
}

string CModelDB::_ResolveTextureRef(const ModelResolved& model, const string& texRef, int depth)
{
    if (depth > 16) return std::string(); // fail
    if (texRef.empty()) return std::string();

    // "#alias"
    if (texRef[0] == '#')
    {
        string alias = texRef.substr(1);
        auto it = model.textures.find(alias);
        if (it == model.textures.end()) return std::string();

        return _ResolveTextureRef(model, it->second, depth + 1);
    }
    return texRef;
}

void CModelDB::_BakeElements(IN const ModelResolved& modelResolved, OUT BakedModel& bakedModel)
{
    bakedModel.quads.clear();
    bakedModel.quads.reserve(modelResolved.elements.size() * 6);

    for (auto& elem : modelResolved.elements)
    {
        for (int d = 0; d < (int)FACE_DIR::COUNT; ++d)
        {
            if (!elem.bHasFace[d]) continue;

            _BakeOneElementFace(modelResolved, elem, d, bakedModel);
        }
    }
}

void CModelDB::_BakeOneElementFace(IN const ModelResolved& modelResolved, const ModelElement& modelElem, int faceDir, OUT BakedModel& bakedModel)
{
    const FACE_DIR eDir = static_cast<FACE_DIR>(faceDir);
    const ModelFace& face = modelElem.faces[faceDir];

    // texture resolve
    string texKey = _ResolveTextureRef(modelResolved, face.textureRef);
    if (texKey.empty()) return;

    string normalizedTextureKey = texKey;
    if (!NormalizeTextureKey(texKey.c_str(), normalizedTextureKey))
        return;

    // 1) Build positions with Minecraft face orientation
    XMFLOAT3 p[4];
    BuildFaceQuadPositions01(modelElem, eDir, p);

    XMFLOAT3 faceNorm = FaceToNormalFloat3(eDir);

    // element rotation (optional)
    if (modelElem.bHasRotation)
    {
        XMFLOAT3 origin01 =
        {
            modelElem.rotOrigin[0] / 16.0f,
            modelElem.rotOrigin[1] / 16.0f,
            modelElem.rotOrigin[2] / 16.0f
        };

        for (int i = 0; i < 4; ++i)
        {
            p[i] = RotatePointAroundOrigin01(p[i], origin01, static_cast<ROT_AXIS>(modelElem.rotAxis), modelElem.rotAngleDeg);
        }

        XMVECTOR v0 = XMLoadFloat3(&p[0]);
        XMVECTOR v1 = XMLoadFloat3(&p[1]);
        XMVECTOR v2 = XMLoadFloat3(&p[2]);

        XMVECTOR e0 = XMVectorSubtract(v1, v0);
        XMVECTOR e1 = XMVectorSubtract(v2, v0);
        XMVECTOR n = XMVector3Normalize(XMVector3Cross(e0, e1));

        XMFLOAT3 base = FaceToNormalFloat3(eDir);
        XMVECTOR baseN = XMLoadFloat3(&base);

        if (XMVectorGetX(XMVector3Dot(n, baseN)) < 0.0f)
            n = XMVectorNegate(n);

        XMStoreFloat3(&faceNorm, n);
    }

    // 2) Resolve uv01: (u0,v0,u1,v1) in 0..1, with top-left origin (0,0)
    float uv01[4];
    if (face.bHasUV)
    {
        uv01[0] = face.uv[0] / 16.0f;
        uv01[1] = face.uv[1] / 16.0f;
        uv01[2] = face.uv[2] / 16.0f;
        uv01[3] = face.uv[3] / 16.0f;
    }
    else
    {
        ComputeFaceUVDefault(modelElem, eDir, uv01);
    }

    float u0 = uv01[0], v0 = uv01[1], u1 = uv01[2], v1 = uv01[3];

    // 3) Map uv corners to match p[0..3] corner meaning
    // p[0]=LB, p[1]=RB, p[2]=RT, p[3]=LT
    // top-left UV space: LT=(u0,v0), RB=(u1,v1)
    XMFLOAT2 uv[4] =
    {
        { u1, v1 }, // 0: LB
        { u0, v1 }, // 1: RB
        { u0, v0 }, // 2: RT
        { u1, v0 }, // 3: LT
    };

    // 4) Apply Minecraft face.rotation (0/90/180/270)
    ApplyUVRotation(uv, face.rotation);

    // 5) Emit baked quad
    BakedQuad q{};
    q.textureHash = fnv1a_64(normalizedTextureKey);
//#ifdef _DEBUG
    q.debugTextureKey = normalizedTextureKey;
//#endif // _DEBUG
    q.dir = static_cast<uint8_t>(eDir);
    q.bHasCullFace = face.bHasCullFace;
    q.cullFaceDir = face.bHasCullFace ? face.cullFaceDir : static_cast<uint8_t>(FACE_DIR::COUNT);
    q.tintIndex = face.tintIndex;

    if (!normalizedTextureKey.empty())
    {
        m_usedTextureKeys.insert(normalizedTextureKey);
    }

    q.verts[0] = { p[0], faceNorm, uv[0], 0xFFFFFFFF };
    q.verts[1] = { p[1], faceNorm, uv[1], 0xFFFFFFFF };
    q.verts[2] = { p[2], faceNorm, uv[2], 0xFFFFFFFF };
    q.verts[3] = { p[3], faceNorm, uv[3], 0xFFFFFFFF };

    bakedModel.quads.push_back(std::move(q));
}

string CModelDB::_BuildModelPath(string resourceRoot, string modelKey)
{
    string nameSpace = "minecraft";
    string path = modelKey;

    size_t colon = modelKey.find(':');
    if (colon != string::npos)
    {
        nameSpace = modelKey.substr(0, colon);
        path = modelKey.substr(colon + 1);
    }

    // 이미 "models/" 붙어있으면 중복 방지
    if (path.rfind("models/", 0) == 0)
        path = path.substr(7);

    return resourceRoot + "assets/" + nameSpace + "/models/" + path + ".json";
}

XMFLOAT3 CModelDB::RotatePointAroundOrigin01(const XMFLOAT3& p, const XMFLOAT3& origin01, ROT_AXIS axis, float angleDeg)
{
    XMVECTOR v = XMVectorSet(
        p.x - origin01.x,
        p.y - origin01.y,
        p.z - origin01.z,
        0.0f);

    const float rad = XMConvertToRadians(angleDeg);

    XMMATRIX R = XMMatrixIdentity();
    switch (axis)
    {
        case ROT_AXIS::X: R = XMMatrixRotationX(rad); break;
        case ROT_AXIS::Y: R = XMMatrixRotationY(rad); break;
        case ROT_AXIS::Z: R = XMMatrixRotationZ(rad); break;
        default: break;
    }

    v = XMVector3TransformCoord(v, R);
    v = XMVectorAdd(v, XMVectorSet(origin01.x, origin01.y, origin01.z, 0.0f));

    XMFLOAT3 out{};
    XMStoreFloat3(&out, v);
    return out;
}
