#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

class Input
{
public:
	Input();
	~Input();

	bool init(HWND hwnd, HINSTANCE hinstance);
	bool update();

	DIMOUSESTATE mouseState{};
	unsigned char keyState[256]{};

private:
	bool initMouse(HWND hwnd);
	bool initKeyboard(HWND hwnd);

	bool updateMouse();
	bool updateKeyboard();

	IDirectInput* mDevice;
	IDirectInputDevice* mMouse;
	IDirectInputDevice* mKeyboard;
};
