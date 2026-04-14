#include "pch.h"
#include "CChunkColumn.h"

void CChunkColumn::Initialize(int cx, int cz)
{
	m_coord = { cx, cz };
	m_eResidency = EChunkResidency::RESIDENT;
	m_bGenerated = false;
	m_bModified = false;
	m_uLastAccessTick = 0;
}

void CChunkColumn::ResetSection(int sy)
{
	if (sy < 0 || sy >= CHUNK_SECTION_COUNT)
		return;

	m_sections[sy].reset();
}

void CChunkColumn::ResetBlockLightSection(int sy)
{
	if (sy < 0 || sy >= CHUNK_SECTION_COUNT)
		return;

	m_blockLightSections[sy].reset();
}

CChunkSection* CChunkColumn::GetSection(int sy)
{
	if (sy < 0 || sy >= CHUNK_SECTION_COUNT)
		return nullptr;

	return m_sections[sy].get();
}

const CChunkSection* CChunkColumn::GetSection(int sy) const
{
	if (sy < 0 || sy >= CHUNK_SECTION_COUNT)
		return nullptr;

	return m_sections[sy].get();
}

CChunkSection* CChunkColumn::EnsureSection(int sy)
{
	if (sy < 0 || sy >= CHUNK_SECTION_COUNT)
		return nullptr;

	if (!m_sections[sy])
		m_sections[sy] = make_unique<CChunkSection>();

	return m_sections[sy].get();
}

CChunkLightSection* CChunkColumn::GetBlockLightSection(int sy)
{
	if (sy < 0 || sy >= CHUNK_SECTION_COUNT)
		return nullptr;

	return m_blockLightSections[sy].get();
}

const CChunkLightSection* CChunkColumn::GetBlockLightSection(int sy) const
{
	if (sy < 0 || sy >= CHUNK_SECTION_COUNT)
		return nullptr;

	return m_blockLightSections[sy].get();
}

CChunkLightSection* CChunkColumn::EnsureBlockLightSection(int sy)
{
	if (sy < 0 || sy >= CHUNK_SECTION_COUNT)
		return nullptr;

	if (!m_blockLightSections[sy])
		m_blockLightSections[sy] = make_unique<CChunkLightSection>();

	return m_blockLightSections[sy].get();
}
