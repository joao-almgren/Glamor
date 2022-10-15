#include "config.h"
#include "json/json.h"

namespace Config
{
	int SCREEN_WIDTH{ 1280 };
	int SCREEN_HEIGHT{ 800 };
	bool FULL_SCREEN{ false };

	float NEAR_PLANE{ 1.0f };
	float FAR_PLANE{ 1000.0f };
	float FOV{ 60.0f };

	int WATER_TEX_SIZE{ 512 };
	int BOUNCE_TEX_SIZE{ 256 };
	int SHADOW_TEX_SIZE{ 4096 };

	bool load() noexcept
	{
		try
		{
			auto configJson = Json::LoadFile("config.json");

			SCREEN_WIDTH = std::stoi(configJson["SCREEN_WIDTH"].value);
			SCREEN_HEIGHT = std::stoi(configJson["SCREEN_HEIGHT"].value);
			FULL_SCREEN = static_cast<bool>(std::stoi(configJson["FULL_SCREEN"].value));

			NEAR_PLANE = std::stof(configJson["NEAR_PLANE"].value);
			FAR_PLANE = std::stof(configJson["FAR_PLANE"].value);
			FOV = std::stof(configJson["FOV"].value);

			WATER_TEX_SIZE = std::stoi(configJson["WATER_TEX_SIZE"].value);
			BOUNCE_TEX_SIZE = std::stoi(configJson["BOUNCE_TEX_SIZE"].value);
			SHADOW_TEX_SIZE = std::stoi(configJson["SHADOW_TEX_SIZE"].value);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}
}
