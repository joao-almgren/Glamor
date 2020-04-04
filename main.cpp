#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <memory>
#include <functional>

#define CUSTOMFVF (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE)
struct CUSTOMVERTEX
{
	D3DVECTOR position;
	D3DVECTOR normal;
	D3DCOLOR color;
};

const CUSTOMVERTEX vertices[] =
{
	 -5.0f, -5.0f, 5.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0),
	 5.0f, -5.0f, 5.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0),
	 -5.0f, 5.0f, 5.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0),
	 5.0f, 5.0f, 5.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0),
	 -5.0f, -5.0f, -5.0f, 0.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(255, 0, 0),
	 -5.0f, 5.0f, -5.0f, 0.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(255, 0, 0),
	 5.0f, -5.0f, -5.0f, 0.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(255, 0, 0),
	 5.0f, 5.0f, -5.0f, 0.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(255, 0, 0),
	 -5.0f, 5.0f, -5.0f, 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255),
	 -5.0f, 5.0f, 5.0f, 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255),
	 5.0f, 5.0f, -5.0f, 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255),
	 5.0f, 5.0f, 5.0f, 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255),
	 -5.0f, -5.0f, -5.0f, 0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 0),
	 5.0f, -5.0f, -5.0f, 0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 0),
	 -5.0f, -5.0f, 5.0f, 0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 0),
	 5.0f, -5.0f, 5.0f, 0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 0),
	 5.0f, -5.0f, -5.0f, 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 255),
	 5.0f, 5.0f, -5.0f, 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 255),
	 5.0f, -5.0f, 5.0f, 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 255),
	 5.0f, 5.0f, 5.0f, 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 255),
	 -5.0f, -5.0f, -5.0f, -1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255),
	 -5.0f, -5.0f, 5.0f, -1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255),
	 -5.0f, 5.0f, -5.0f, -1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255),
	 -5.0f, 5.0f, 5.0f, -1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255)
};

const short indices[] =
{
	0, 1, 2,
	2, 1, 3,
	4, 5, 6,
	6, 5, 7,
	8, 9, 10,
	10, 9, 11,
	12, 13, 14,
	14, 13, 15,
	16, 17, 18,
	18, 17, 19,
	20, 21, 22,
	22, 21, 23,
};

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	const wchar_t* const window_title = L"D3DTest";
	const int screen_width = 800;
	const int screen_height = 600;

	WNDCLASSEX wc
	{
		.cbSize = sizeof(WNDCLASSEX),
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
		.lpszClassName = window_title,
	};
	if (!RegisterClassEx(&wc))
		return 0;

	RECT window_rect = { 0, 0, screen_width, screen_height };
	AdjustWindowRectEx(&window_rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
	const int window_width = window_rect.right - window_rect.left;
	const int window_height = window_rect.bottom - window_rect.top;

	HWND hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, window_title, window_title, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height, 0, 0, hInstance, nullptr);
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
				.BackBufferWidth = screen_width,
				.BackBufferHeight = screen_height,
				.BackBufferFormat = D3DFMT_A8R8G8B8,
				.BackBufferCount = 1,
				.SwapEffect = D3DSWAPEFFECT_DISCARD,
				.Windowed = TRUE,
				.EnableAutoDepthStencil = TRUE,
				.AutoDepthStencilFormat = D3DFMT_D16,
			};

			IDirect3DDevice9* pDevice;
			if (FAILED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &pDevice)))
				return nullptr;

			pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
			pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

			D3DXMATRIX matProjection;
			D3DXMatrixPerspectiveFovLH(&matProjection, D3DXToRadian(90), (float)screen_width / (float)screen_height, 1.0f, 100.0f);
			pDevice->SetTransform(D3DTS_PROJECTION, &matProjection);

			pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
			pDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(50, 50, 50));

			D3DLIGHT9 light
			{
				.Type = D3DLIGHT_DIRECTIONAL,
				.Diffuse = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f),
				.Direction = D3DXVECTOR3(-1.0f, -0.3f, -1.0f),
			};
			pDevice->SetLight(0, &light);
			pDevice->LightEnable(0, TRUE);

			D3DMATERIAL9 material
			{
				.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f),
				.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f),
			};
			pDevice->SetMaterial(&material);

			return pDevice;
		}(),
		[](IDirect3DDevice9* pDevice)
		{
			pDevice->Release();
		}
	);
	if (!pDevice)
		return 0;

	std::unique_ptr<IDirect3DVertexBuffer9, std::function<void(IDirect3DVertexBuffer9*)>> pVertexBuffer
	(
		[&pDevice]() -> IDirect3DVertexBuffer9*
		{
			IDirect3DVertexBuffer9* pVertexBuffer;
			if (FAILED(pDevice->CreateVertexBuffer(24 * sizeof(CUSTOMVERTEX), 0, CUSTOMFVF, D3DPOOL_MANAGED, &pVertexBuffer, nullptr)))
				return nullptr;

			void* pData;
			if (FAILED(pVertexBuffer->Lock(0, 0, &pData, 0)))
			{
				pVertexBuffer->Release();
				return nullptr;
			}
			memcpy(pData, vertices, sizeof(vertices));
			pVertexBuffer->Unlock();

			return pVertexBuffer;
		}(),
		[](IDirect3DVertexBuffer9* pVertexBuffer)
		{
			pVertexBuffer->Release();
		}
	);
	if (!pVertexBuffer)
		return 0;

	std::unique_ptr<IDirect3DIndexBuffer9, std::function<void(IDirect3DIndexBuffer9*)>> pIndexBuffer
	(
		[&pDevice]() -> IDirect3DIndexBuffer9*
		{
			IDirect3DIndexBuffer9* pIndexBuffer;
			if (FAILED(pDevice->CreateIndexBuffer(36 * sizeof(short), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pIndexBuffer, nullptr)))
				return nullptr;

			void* pData;
			if (FAILED(pIndexBuffer->Lock(0, 0, &pData, 0)))
			{
				pIndexBuffer->Release();
				return nullptr;
			}
			memcpy(pData, indices, sizeof(indices));
			pIndexBuffer->Unlock();

			return pIndexBuffer;
		}(),
		[](IDirect3DIndexBuffer9* pIndexBuffer)
		{
			pIndexBuffer->Release();
		}
	);
	if (!pIndexBuffer)
		return 0;

	float angle = 0.0f;

	MSG msg{};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			pDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

			if (SUCCEEDED(pDevice->BeginScene()))
			{
				D3DXMATRIX matView;
				const D3DXVECTOR3 eye(15.0f, 0.0f, 0.0f);
				const D3DXVECTOR3 at(0.0f, 0.0f, 0.0f);
				const D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
				D3DXMatrixLookAtLH(&matView, &eye, &at, &up);
				pDevice->SetTransform(D3DTS_VIEW, &matView);

				angle += 0.025f;
				D3DXMATRIX matRotZ, matRotY, matRotX;
				D3DXMatrixRotationZ(&matRotZ, angle);
				D3DXMatrixRotationY(&matRotY, angle);
				D3DXMatrixRotationX(&matRotX, angle);
				D3DXMATRIX matWorld = matRotZ * matRotY * matRotX;
				pDevice->SetTransform(D3DTS_WORLD, &matWorld);

				pDevice->SetFVF(CUSTOMFVF);
				pDevice->SetStreamSource(0, pVertexBuffer.get(), 0, sizeof(CUSTOMVERTEX));
				pDevice->SetIndices(pIndexBuffer.get());
				pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 24, 0, 12);

				pDevice->EndScene();
			}

			pDevice->Present(nullptr, nullptr, 0, nullptr);
		}
	}

	return 0;
}
