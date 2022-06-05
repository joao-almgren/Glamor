#pragma once

namespace Config
{
	extern int SCREEN_WIDTH;
	extern int SCREEN_HEIGHT;

	extern float NEAR_PLANE;
	extern float FAR_PLANE;
	extern float FOV;

	extern int WATER_TEX_SIZE;
	extern int BOUNCE_TEX_SIZE;
	extern int SHADOW_TEX_SIZE;

	bool load() noexcept;
}
