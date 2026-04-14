#pragma once
#include "CComponentBase.h"
#include "ChunkTypes.h"

struct InventorySlot
{
	BlockCell block{};
	uint32_t count = 0;

	bool IsEmpty() const
	{
		return count == 0 || block.IsAir();
	}
};

class CInventoryComponent : public CComponentBase<CInventoryComponent, COMPONENT_TYPE::INVENTORY>
{
public:
	static constexpr int HOTBAR_SIZE = 9;

	CInventoryComponent() = default;
	~CInventoryComponent() override = default;

	void Init() override;
	void Start() override;

	int GetSelectedSlotIndex() const;
	void SetSelectedSlotIndex(int index);
	const InventorySlot* GetSelectedSlot() const;
	InventorySlot* GetSelectedSlotMutable();

	const InventorySlot* GetSlot(int index) const;
	InventorySlot* GetSlotMutable(int index);

	BlockCell GetSelectedPlaceBlock() const;
	bool TrySetSlot(int index, string blockName);
	bool TryConsumeSelectedOne();



private:
	array<InventorySlot, HOTBAR_SIZE> m_arrayHotBar;
	int m_iSelectedSlot;
};