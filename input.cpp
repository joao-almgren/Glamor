#include "input.h"

//*********************************************************************************************************************

Input::Input()
	: pDI{ nullptr }
	, mouse{ nullptr }
	, keyboard{ nullptr }
{
}

//*********************************************************************************************************************

Input::~Input()
{
	if (mouse)
	{
		mouse->Unacquire();
		mouse->Release();
	}

	if (keyboard)
	{
		keyboard->Unacquire();
		keyboard->Release();
	}

	if (pDI)
	{
		pDI->Release();
	}
}

//*********************************************************************************************************************

bool Input::init(const HWND hwnd, const HINSTANCE hinstance)
{
	if (DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8W, (void**)&pDI, nullptr) != DI_OK)
		return false;

	return (initMouse(hwnd) && initKeyboard(hwnd));
}

//*********************************************************************************************************************

bool Input::update()
{
	return (updateMouse() && updateKeyboard());
}

//*********************************************************************************************************************

bool Input::initMouse(const HWND hwnd)
{
	if (pDI->CreateDevice(GUID_SysMouse, &mouse, 0) != DI_OK)
		return false;

	if (mouse->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE) != DI_OK)
		return false;

	if (mouse->SetDataFormat(&c_dfDIMouse) != DI_OK)
		return false;

	HRESULT hr = mouse->Acquire();
	if (hr != DI_OK && hr != S_FALSE)
		return false;

	return true;
}

//*********************************************************************************************************************

bool Input::updateMouse()
{
	if (mouse->GetDeviceState(sizeof DIMOUSESTATE, (LPVOID)&mouseState) != DI_OK)
		return false;

	return true;
}

//*********************************************************************************************************************

bool Input::initKeyboard(const HWND hwnd)
{
	if (pDI->CreateDevice(GUID_SysKeyboard, &keyboard, 0) != DI_OK)
		return false;

	if (keyboard->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE) != DI_OK)
		return false;

	if (keyboard->SetDataFormat(&c_dfDIKeyboard) != DI_OK)
		return false;

	HRESULT hr = keyboard->Acquire();
	if (hr != DI_OK && hr != S_FALSE)
		return false;

	return true;
}

//*********************************************************************************************************************

bool Input::updateKeyboard()
{
	if (keyboard->GetDeviceState(256, (void *)&keyState) != DI_OK)
		return false;

	return true;
}

//*********************************************************************************************************************
