#pragma once
#include "CObject.h"

constexpr size_t MAX_OBJECT_COUNT = 100000ull;

class CScene;

class CObjectManager
{
public:
	CObjectManager();
	~CObjectManager();

	OBJECT_ID Add(const string& strName_, CScene* const pOwnScene_);
	void Destroy(const string& strName_);
	void Destroy(OBJECT_ID uObjectID_);

	void CommitPendingStart();
	void ProcessPeddingDestroy();

	CObject* Get(const string& strName_);
	const CObject* Get(const string& strName_) const;

	CObject* Get(OBJECT_ID uObjectID_);
	const CObject* Get(OBJECT_ID uObjectID_) const;

	template<typename Fn>
	void ForEachAliveEnabled(Fn&& fn) 
	{
		for (auto& obj : m_vecObjects)
		{
			CObject* pObject = obj.get();
			if (nullptr == pObject)
				continue;

			if (!pObject->GetAlive() || !pObject->GetEnable())
				continue;

			fn(*pObject);
		}
	}

	vector<uint32_t> GetAllObject() { return m_vecSparse; }

private:
	void _RemoveObjectAtIndex(uint32_t uIndex_);

private:
	unordered_map<uint64_t, OBJECT_ID> m_hashIDMap;
	vector<uint32_t> m_vecSparse;
	vector<unique_ptr<CObject>> m_vecObjects;
	vector<OBJECT_ID> m_vecFreeIDs;
	size_t m_uDestroyBudget = 8;
};