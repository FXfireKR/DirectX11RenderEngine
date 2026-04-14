#pragma once
#include <cstdint>

enum class LOADING_PROGRESS : uint8_t
{
	INITIALIZE,
	PROGRESS,
	ENDING,
};

class ILoadProgress
{
public:
	ILoadProgress() = default;
	virtual ~ILoadProgress() = default;

	virtual void Initialize() = 0;
	virtual void LoadPrograss() = 0;

protected:
	//bool m_b
};