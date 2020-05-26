#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "input.h"
#include "camera.h"
#include "d3dwrap.h"
#include "skybox.h"
#include "cube.h"
#include "scape.h"
#include "sea.h"
#include <memory>
#include <functional>

//*********************************************************************************************************************

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPWSTR /*lpCmdLine*/, _In_ int /*nShowCmd*/)
{
	const auto windowTitle{ L"D3D9Test" };
	const auto screenWidth{ 1024 };
	const auto screenHeight{ 768 };

	WNDCLASSEX wc
	{
		.cbSize = sizeof(WNDCLASSEX),
		// https://devblogs.microsoft.com/oldnewthing/20150220-00/?p=44623
		// NOLINTNEXTLINE
		.lpfnWndProc = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
		{
			switch (message)
			{
			case WM_KEYDOWN:
				switch (wParam)
				{
				case VK_ESCAPE:
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					return 0;
				}
				break;
			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;
			}
			return DefWindowProc(hWnd, message, wParam, lParam);
		},
		.hInstance = hInstance,
		.lpszClassName = windowTitle,
	};
	if (!RegisterClassEx(&wc))
		return 0;

	RECT windowRect{ .right = screenWidth, .bottom = screenHeight };
	AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
	const auto windowWidth{ windowRect.right - windowRect.left };
	const auto windowHeight{ windowRect.bottom - windowRect.top };
	const auto windowLeft = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2;
	const auto windowTop = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2;

	auto hWnd = CreateWindowEx
	(
		WS_EX_OVERLAPPEDWINDOW,
		windowTitle,
		windowTitle,
		WS_OVERLAPPEDWINDOW,
		windowLeft,
		windowTop,
		windowWidth,
		windowHeight,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);
	if (!hWnd)
		return 0;

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	SetFocus(hWnd);
	ShowCursor(FALSE);

	Input input;
	if (!input.init(hWnd, hInstance))
		return 0;

	std::unique_ptr<IDirect3D9, std::function<void(IDirect3D9*)>> pD3D
	(
		Direct3DCreate9(D3D_SDK_VERSION),
		[](IDirect3D9* pD3D)
		{
			pD3D->Release();
		}
	);
	if (!pD3D)
		return 0;

	std::unique_ptr<IDirect3DDevice9, std::function<void(IDirect3DDevice9*)>> pDevice
	(
		[&pD3D, &hWnd]() -> IDirect3DDevice9*
		{
			D3DPRESENT_PARAMETERS d3dpp
			{
				.BackBufferWidth = screenWidth,
				.BackBufferHeight = screenHeight,
				.BackBufferFormat = D3DFMT_A8R8G8B8,
				.BackBufferCount = 1,
				.SwapEffect = D3DSWAPEFFECT_DISCARD,
				.Windowed = TRUE,
				.EnableAutoDepthStencil = TRUE,
				.AutoDepthStencilFormat = D3DFMT_D24S8,
			};

			IDirect3DDevice9* pDevice;
			if (FAILED(pD3D->CreateDevice
			(
				D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL,
				hWnd,
				D3DCREATE_HARDWARE_VERTEXPROCESSING,
				&d3dpp,
				&pDevice
			)))
				return nullptr;

			return pDevice;
		}(),
		[](IDirect3DDevice9* pDevice)
		{
			pDevice->Release();
		}
	);
	if (!pDevice)
		return 0;

	auto resetDevice = [&pDevice]()
	{
		pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

		D3DXMATRIX matProjection;
		D3DXMatrixPerspectiveFovLH
		(
			&matProjection,
			D3DXToRadian(60),
			static_cast<float>(screenWidth) / static_cast<float>(screenHeight),
			1.0f,
			1000.0f
		);
		pDevice->SetTransform(D3DTS_PROJECTION, &matProjection);
	};
	resetDevice();

	enum { DEFAULT_RTT, DEFAULT_Z, REFLECT_RTT, REFLECT_Z, REFRACT_RTT, REFRACT_Z, SURFACE_COUNT };

	Texture rtReflect, rtRefract;
	Surface surface[SURFACE_COUNT];
	{
		// save default surfaces
		IDirect3DSurface9* pSurface;
		if (FAILED(pDevice->GetRenderTarget(0, &pSurface)))
			return 0;
		surface[DEFAULT_RTT].reset(pSurface);

		if (FAILED(pDevice->GetDepthStencilSurface(&pSurface)))
			return 0;
		surface[DEFAULT_Z].reset(pSurface);

		// create reflection rtt
		IDirect3DTexture9* pTexture;
		if (FAILED(pDevice->CreateTexture(512, 512, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtReflect.reset(pTexture);
		if (FAILED(rtReflect->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[REFLECT_RTT].reset(pSurface);

		// create reflection zbuffer
		if (FAILED(pDevice->CreateDepthStencilSurface(512, 512, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, TRUE, &pSurface, nullptr)))
			return 0;
		surface[REFLECT_Z].reset(pSurface);

		// create refraction rtt
		if (FAILED(pDevice->CreateTexture(512, 512, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtRefract.reset(pTexture);
		if (FAILED(rtRefract->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[REFRACT_RTT].reset(pSurface);

		// create refraction zbuffer
		if (FAILED(pDevice->CreateDepthStencilSurface(512, 512, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, TRUE, &pSurface, nullptr)))
			return 0;
		surface[REFRACT_Z].reset(pSurface);
	}

	Cube cube(pDevice.get());
	if (!cube.init())
		return 0;

	cube.setPos(D3DXVECTOR3(16, 50.0f, 16));

	Scape scape(pDevice.get());
	if (!scape.init())
		return 0;

	Skybox skybox(pDevice.get());
	if (!skybox.init())
		return 0;

	Sea sea(pDevice.get(), rtReflect.get(), rtRefract.get());
	if (!sea.init())
		return 0;

	Camera camera(D3DXVECTOR3(0, 25, 0), 0, 0);

	MSG msg{};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// tick
			{
				input.update();

				POINT currMouse{ input.mouseState.lX, input.mouseState.lY };
				camera.rotate((float)-currMouse.y / 256.0f, (float)-currMouse.x / 256.0f);

				const float speed = 0.2f;
				if (input.keyState[DIK_D] || input.keyState[DIK_RIGHT])
					camera.moveRight(speed);
				else if (input.keyState[DIK_A] || input.keyState[DIK_LEFT])
					camera.moveRight(-speed);
				if (input.keyState[DIK_W] || input.keyState[DIK_UP] || input.mouseState.rgbButtons[0])
					camera.moveForward(speed);
				else if (input.keyState[DIK_S] || input.keyState[DIK_DOWN])
					camera.moveForward(-speed);
				if (input.keyState[DIK_Q])
					camera.moveUp(speed);
				else if (input.keyState[DIK_Z])
					camera.moveUp(-speed);

				D3DXVECTOR3 pos = camera.getPos();
				scape.setPos(D3DXVECTOR3(pos.x, 0.0f, pos.z));
				skybox.setPos(pos);

				cube.update();
				scape.update();
				sea.update();
				skybox.update();
			}

			D3DXMATRIX matRTTProj;
			D3DXMatrixPerspectiveFovLH(&matRTTProj, (D3DX_PI / 2), 1.0f, 1.0f, 1000.0f);
			pDevice->SetTransform(D3DTS_PROJECTION, &matRTTProj);

			// update reflection
			{
				// set render target to reflect surfaces
				pDevice->SetRenderTarget(0, surface[REFLECT_RTT].get());
				pDevice->SetDepthStencilSurface(surface[REFLECT_Z].get());

				pDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(60, 68, 85), 1.0f, 0);
				if (SUCCEEDED(pDevice->BeginScene()))
				{
					camera.setView(pDevice.get());

					D3DXMATRIX matView;
					pDevice->GetTransform(D3DTS_VIEW, &matView);

					D3DXMATRIX matReflect;
					D3DXMatrixScaling(&matReflect, 1, -1, 1);

					D3DXMATRIX matReflectView = matReflect * matView;
					pDevice->SetTransform(D3DTS_VIEW, &matReflectView);

					scape.draw(ScapeRenderMode::Above);
					cube.draw();
					skybox.draw();

					pDevice->EndScene();
				}
			}

			// update refraction
			{
				// set render target to refract surfaces
				pDevice->SetRenderTarget(0, surface[REFRACT_RTT].get());
				pDevice->SetDepthStencilSurface(surface[REFRACT_Z].get());

				pDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(60, 68, 85), 1.0f, 0);
				if (SUCCEEDED(pDevice->BeginScene()))
				{
					camera.setView(pDevice.get());

					scape.draw(ScapeRenderMode::Below);
					cube.draw();
					skybox.draw();

					pDevice->EndScene();
				}
			}

			// reset device
			pDevice->SetRenderTarget(0, surface[DEFAULT_RTT].get());
			pDevice->SetDepthStencilSurface(surface[DEFAULT_Z].get());
			resetDevice();

			// render
			{
				pDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_XRGB(60, 68, 85), 1.0f, 0);

				if (SUCCEEDED(pDevice->BeginScene()))
				{
					camera.setView(pDevice.get());
					cube.draw();
					scape.draw();
					sea.draw(matRTTProj);
					skybox.draw();

					pDevice->EndScene();
				}
			}

			pDevice->Present(nullptr, nullptr, nullptr, nullptr);
		}
	}

	return 0;
}

//*********************************************************************************************************************
