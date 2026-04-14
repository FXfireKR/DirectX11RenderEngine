#include "pch.h"
#include "CInventoryComponent.h"

void CInventoryComponent::Init()
{
	m_arrayHotBar.fill({ {0, 0}, 0 });
	m_iSelectedSlot = 0;

	TrySetSlot(0, "minecraft:torch");
	TrySetSlot(1, "minecraft:glass");
	TrySetSlot(2, "minecraft:oak_leaves");
	TrySetSlot(3, "minecraft:cobblestone");
	TrySetSlot(4, "minecraft:dirt");
	TrySetSlot(5, "minecraft:oak_log");
	TrySetSlot(6, "minecraft:crafting_table");
	TrySetSlot(7, "minecraft:sand");
	TrySetSlot(8, "minecraft:oak_planks");
}

void CInventoryComponent::Start()
{
}

int CInventoryComponent::GetSelectedSlotIndex() const
{
	return m_iSelectedSlot;
}

void CInventoryComponent::SetSelectedSlotIndex(int index)
{
	if (index < 0 || index >= HOTBAR_SIZE) 
		return;
	m_iSelectedSlot = index;
}

const InventorySlot* CInventoryComponent::GetSelectedSlot() const
{
	return &m_arrayHotBar[m_iSelectedSlot];
}

InventorySlot* CInventoryComponent::GetSelectedSlotMutable()
{
	return &m_arrayHotBar[m_iSelectedSlot];
}

const InventorySlot* CInventoryComponent::GetSlot(int index) const
{
	if (index < 0 || index >= HOTBAR_SIZE) 
		return nullptr;

	return &m_arrayHotBar[index];
}

InventorySlot* CInventoryComponent::GetSlotMutable(int index)
{
	if (index < 0 || index >= HOTBAR_SIZE) 
		return nullptr;

	return &m_arrayHotBar[index];
}

BlockCell CInventoryComponent::GetSelectedPlaceBlock() const
{
	const InventorySlot& slot = m_arrayHotBar[m_iSelectedSlot];
	if (slot.IsEmpty()) 
		return { 0, 0 };

	return slot.block;
}

bool CInventoryComponent::TrySetSlot(int index, string blockName)
{
	if (index < 0 || index >= HOTBAR_SIZE)
		return false;

	BlockPropHashMap props;
	BLOCK_ID blockID = BlockDB.FindBlockID(blockName.c_str());
	STATE_INDEX sidx;
	if (!BlockDB.EncodeStateIndex(blockID, props, sidx))
		return false;

	m_arrayHotBar[index].block.blockID = blockID;
	m_arrayHotBar[index].block.stateIndex = sidx;
	m_arrayHotBar[index].count = 999;
	return true;
}

bool CInventoryComponent::TryConsumeSelectedOne()
{
	InventorySlot& slot = m_arrayHotBar[m_iSelectedSlot];
	if (slot.IsEmpty()) return false;

	if (slot.count > 0)
	{
		--slot.count;
		if (slot.count == 0)
		{
			slot.block = { 0, 0 };
		}
		return true;
	}
	return false;
}
