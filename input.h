#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

class Input
{
public:
	Input();
	~Input();

	bool init(const HWND hwnd, const HINSTANCE hinstance);
	bool update();

	DIMOUSESTATE mouseState{};
	unsigned char keyState[256]{};

private:
	bool initMouse(const HWND hwnd);
	bool initKeyboard(const HWND hwnd);

	bool updateMouse();
	bool updateKeyboard();

	IDirectInput* mDevice;
	IDirectInputDevice* mMouse;
	IDirectInputDevice* mKeyboard;
};
