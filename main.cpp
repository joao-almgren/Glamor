#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "d3dwrap.h"
#include "skybox.h"
#include "cube.h"
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

	auto hWnd = CreateWindowEx
	(
		WS_EX_OVERLAPPEDWINDOW,
		windowTitle,
		windowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
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
				.AutoDepthStencilFormat = D3DFMT_D16,
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

			pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
			pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);

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

			//pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
			//pDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(50, 50, 50));

			//D3DLIGHT9 light
			//{
			//	.Type = D3DLIGHT_DIRECTIONAL,
			//	.Diffuse = D3DXCOLOR(0.75f, 0.75f, 0.75f, 0),
			//	.Direction = D3DXVECTOR3(0.57735f, 0.57735f, 0.57735f),
			//};
			//pDevice->SetLight(0, &light);
			//pDevice->LightEnable(0, TRUE);

			//D3DMATERIAL9 material
			//{
			//	.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f),
			//	.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f),
			//};
			//pDevice->SetMaterial(&material);

			return pDevice;
		}(),
		[](IDirect3DDevice9* pDevice)
		{
			pDevice->Release();
		}
	);
	if (!pDevice)
		return 0;

	Cube cube(pDevice.get());
	if (!cube.init())
		return 0;

	Skybox skybox(pDevice.get());
	if (!skybox.init())
		return 0;

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
			cube.update(1.0f);
			skybox.update(1.0f);

			pDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(60, 68, 85), 1.0f, 0);

			if (SUCCEEDED(pDevice->BeginScene()))
			{
				cube.draw();
				skybox.draw();

				pDevice->EndScene();
			}

			pDevice->Present(nullptr, nullptr, nullptr, nullptr);
		}
	}

	return 0;
}

//*********************************************************************************************************************

