#include "input.h"

Input::Input()
	: mDevice{ nullptr }
	, mMouse{ nullptr }
	, mKeyboard{ nullptr }
{
}

Input::~Input()
{
	if (mMouse)
	{
		mMouse->Unacquire();
		mMouse->Release();
	}

	if (mKeyboard)
	{
		mKeyboard->Unacquire();
		mKeyboard->Release();
	}

	if (mDevice)
	{
		mDevice->Release();
	}
}

bool Input::init(HWND hwnd, HINSTANCE hinstance)
{
	if (DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8W, (void**)&mDevice, nullptr) != DI_OK)
		return false;

	return (initMouse(hwnd) && initKeyboard(hwnd));
}

bool Input::update()
{
	return (updateMouse() && updateKeyboard());
}

bool Input::initMouse(HWND hwnd)
{
	if (mDevice->CreateDevice(GUID_SysMouse, &mMouse, 0) != DI_OK)
		return false;

	if (mMouse->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE) != DI_OK)
		return false;

	if (mMouse->SetDataFormat(&c_dfDIMouse) != DI_OK)
		return false;

	HRESULT hr = mMouse->Acquire();
	if (hr != DI_OK && hr != S_FALSE)
		return false;

	return true;
}

bool Input::updateMouse()
{
	if (mMouse->GetDeviceState(sizeof DIMOUSESTATE, (LPVOID)&mouseState) != DI_OK)
		return false;

	return true;
}

bool Input::initKeyboard(HWND hwnd)
{
	if (mDevice->CreateDevice(GUID_SysKeyboard, &mKeyboard, 0) != DI_OK)
		return false;

	if (mKeyboard->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE) != DI_OK)
		return false;

	if (mKeyboard->SetDataFormat(&c_dfDIKeyboard) != DI_OK)
		return false;

	HRESULT hr = mKeyboard->Acquire();
	if (hr != DI_OK && hr != S_FALSE)
		return false;

	return true;
}

bool Input::updateKeyboard()
{
	if (mKeyboard->GetDeviceState(256, (void *)&keyState) != DI_OK)
		return false;

	return true;
}
