#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

class Input
{
public:
	Input() noexcept;
	~Input();

	bool init(HWND hwnd, HINSTANCE hinstance) noexcept;
	bool update() noexcept;

	DIMOUSESTATE mouseState{};
	unsigned char keyState[256]{};

private:
	bool initMouse(HWND hwnd) noexcept;
	bool initKeyboard(HWND hwnd) noexcept;

	bool updateMouse() noexcept;
	bool updateKeyboard() noexcept;

	IDirectInput* mDevice;
	IDirectInputDevice* mMouse;
	IDirectInputDevice* mKeyboard;
};
