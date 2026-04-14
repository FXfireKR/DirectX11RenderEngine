#include "pch.h"
#include "CObjectManager.h"

CObjectManager::CObjectManager()
{
	m_vecObjects.reserve(MAX_OBJECT_COUNT);
}

CObjectManager::~CObjectManager()
{
}

OBJECT_ID CObjectManager::Add(const string& strName_, CScene* const pOwnScene_)
{
	uint64_t strHash = fnv1a_64(strName_);
	OBJECT_ID uObjectID = INVALID_OBJECT_ID;

	if (true == m_hashIDMap.contains(strHash))
	{
		uObjectID = m_hashIDMap.find(strHash)->second;
	}
	else // (false == m_hashIDMap.contains(strHash))
	{
		if (false == m_vecFreeIDs.empty())
		{
			uObjectID = m_vecFreeIDs.back();
			m_vecFreeIDs.pop_back();
		}
		else // (true == m_vecFreeIDs.empty())
		{
			uObjectID = static_cast<OBJECT_ID>(m_vecSparse.size());
			m_vecSparse.push_back(INVALID_OBJECT_ID); // insert dummy-value
		}
		m_hashIDMap.insert(make_pair(strHash, uObjectID));

		// setting new data
		auto newObject = make_unique<CObject>();
		newObject->m_strName = strName_;
		newObject->m_uID = uObjectID;
		newObject->m_pOwnScene = pOwnScene_;

		uint32_t uDenseIndex = static_cast<uint32_t>(m_vecObjects.size());
		m_vecObjects.push_back(std::move(newObject));
		m_vecSparse[uObjectID] = uDenseIndex;
	}
	return uObjectID;
}

void CObjectManager::Destroy(const string& strName_)
{
	uint64_t strHash = fnv1a_64(strName_);
	auto iter = m_hashIDMap.find(strHash);

	if (iter == m_hashIDMap.end()) 
		return;

	Destroy(iter->second);
}

void CObjectManager::Destroy(OBJECT_ID uObjectID_)
{
	if (uObjectID_ >= m_vecSparse.size())
		return;

	uint32_t denseIndex = m_vecSparse[uObjectID_];
	if (denseIndex >= m_vecObjects.size())
		return;

	CObject* pObject = m_vecObjects[denseIndex].get();
	if (nullptr == pObject || pObject->GetID() != uObjectID_)
		return;

	pObject->Destroy();
}

void CObjectManager::CommitPendingStart()
{
	PROFILE_SCOPE();

	ForEachAliveEnabled([&](CObject& obj) {
		obj.CommitStart();
	});
}

void CObjectManager::ProcessPeddingDestroy()
{
	PROFILE_SCOPE();

	size_t count = 0;
	for (int i = static_cast<int>(m_vecObjects.size() - 1); i >= 0; --i) 
	{
		if (true == m_vecObjects[i]->GetPeddingDestroy()) 
		{
			_RemoveObjectAtIndex(i);
		}
	}
}

void CObjectManager::_RemoveObjectAtIndex(uint32_t uIndex_)
{
	if (uIndex_ >= m_vecObjects.size())
		return;

	OBJECT_ID removedID = m_vecObjects[uIndex_]->GetID();
	uint64_t removedHash = fnv1a_64(m_vecObjects[uIndex_]->m_strName);

	const uint32_t lastIndex = static_cast<uint32_t>(m_vecObjects.size() - 1);

	if (uIndex_ != lastIndex)
	{
		std::swap(m_vecObjects[uIndex_], m_vecObjects[lastIndex]);

		OBJECT_ID movedID = m_vecObjects[uIndex_]->GetID();
		m_vecSparse[movedID] = uIndex_;
	}

	m_vecObjects.pop_back();

	m_hashIDMap.erase(removedHash);
	m_vecSparse[removedID] = UINT32_MAX;   // invalid mark
	m_vecFreeIDs.push_back(removedID);
}

CObject* CObjectManager::Get(const string& strName_)
{
	uint64_t strHash = fnv1a_64(strName_);

	auto iter = m_hashIDMap.find(strHash);
	if (iter == m_hashIDMap.end())
		return nullptr;

	return Get(iter->second);
}

CObject* CObjectManager::Get(OBJECT_ID uObjectID_)
{
	if (uObjectID_ >= m_vecSparse.size())
		return nullptr;

	uint32_t denseIndex = m_vecSparse[uObjectID_];
	if (denseIndex == UINT32_MAX || denseIndex >= m_vecObjects.size())
		return nullptr;

	CObject* pObj = m_vecObjects[denseIndex].get();
	if (nullptr == pObj || pObj->GetID() != uObjectID_)
		return nullptr;

	return pObj;
}

const CObject* CObjectManager::Get(const string& strName_) const
{
	uint64_t strHash = fnv1a_64(strName_);
	auto iter = m_hashIDMap.find(strHash);
	if (iter == m_hashIDMap.end())
		return nullptr;

	return Get(iter->second);
}

const CObject* CObjectManager::Get(OBJECT_ID uObjectID_) const
{
	if (uObjectID_ >= m_vecSparse.size())
		return nullptr;

	uint32_t denseIndex = m_vecSparse[uObjectID_];
	if (denseIndex == UINT32_MAX || denseIndex >= m_vecObjects.size())
		return nullptr;

	const CObject* pObj = m_vecObjects[denseIndex].get();
	if (nullptr == pObj || pObj->GetID() != uObjectID_)
		return nullptr;

	return pObj;
}
