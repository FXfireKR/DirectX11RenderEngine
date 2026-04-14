#include "pch.h"
#include "CObject.h"

void CObject::Init()
{
	for (const auto& iter : m_arrComponents) 
	{
		if (!iter) 
			continue;

		iter->Init();
	}
}

void CObject::Start()
{
	CommitStart();
}

void CObject::CommitStart()
{
	for (const auto& iter : m_arrComponents) 
	{
		if (!iter) 
			continue;

		if (!iter->GetAlive() || !iter->GetEnable()) 
			continue;

		if (iter->GetStarted()) 
			continue;

		iter->Start();
		iter->SetStarted(true);
	}
}

void CObject::FixedUpdate(float fDelta)
{
	for (const auto& iter : m_arrComponents) 
	{
		if (!iter) 
			continue;

		if (iter->GetAlive() && iter->GetEnable()) 
		{
			iter->FixedUpdate(fDelta);
		}
	}
}

void CObject::Update(float fDelta)
{
	for (const auto& iter : m_arrComponents) 
	{
		if (!iter) 
			continue;

		if (iter->GetAlive() && iter->GetEnable()) 
		{
			iter->Update(fDelta);
		}
	}
}

void CObject::LateUpdate(float fDelta)
{
	for (const auto& iter : m_arrComponents) 
	{
		if (!iter) 
			continue;

		if (iter->GetAlive() && iter->GetEnable()) 
		{
			iter->LateUpdate(fDelta);
		}
	}
}

void CObject::Build()
{
	for (const auto& iter : m_arrComponents)
	{
		if (!iter)
			continue;

		if (iter->GetAlive() && iter->GetEnable())
		{
			iter->Build();
		}
	}
}

void CObject::Render()
{
	for (const auto& iter : m_arrComponents) 
	{
		if (!iter) 
			continue;

		if (iter->GetAlive() && iter->GetEnable()) 
		{
			iter->Render();
		}
	}
}

void CObject::Destroy()
{
	m_bAlive = false;
	m_bPeddingDestroy = true;
}

void CObject::AddChild(OBJECT_ID uChildID_)
{
	if (std::find(m_vecChildren.begin(), m_vecChildren.end(), uChildID_) == m_vecChildren.end()) 
	{
		m_vecChildren.push_back(uChildID_);
	}
}