#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <memory>
#include <functional>
#include <fstream>

const auto vertexFVF{ D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_TEXCOORDSIZE2(0) };

struct Vertex
{
	float x, y, z;
	float nx, ny, nz;
	DWORD c;
	float u, v;
};

const Vertex cubeVertex[]
{
	{-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0), 0, 0},
	{1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0), 1, 0},
	{-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0), 0, 1},
	{1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0), 1, 1},
	{-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(255, 0, 0), 0, 0},
	{-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(255, 0, 0), 0, 1},
	{1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(255, 0, 0), 1, 0},
	{1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(255, 0, 0), 1, 1},
	{-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255), 0, 0},
	{-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255), 0, 1},
	{1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255), 1, 0},
	{1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255), 1, 1},
	{-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 0), 0, 0},
	{1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 0), 1, 0},
	{-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 0), 0, 1},
	{1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 0), 1, 1},
	{1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 255), 0, 0},
	{1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 255), 1, 0},
	{1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 255), 0, 1},
	{1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 255), 1, 1},
	{-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255), 0, 0},
	{-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255), 0, 1},
	{-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255), 1, 0},
	{-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255), 1, 1}
};

const int16_t cubeIndex[]
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
	22, 21, 23
};

IDirect3DVertexBuffer9* CreateVertexBuffer(IDirect3DDevice9* pDevice, const Vertex* vertices, const unsigned int count)
{
	IDirect3DVertexBuffer9* pVertexBuffer;
	if (FAILED(pDevice->CreateVertexBuffer
	(
		count * sizeof(Vertex),
		0,
		vertexFVF,
		D3DPOOL_MANAGED,
		&pVertexBuffer,
		nullptr
	)))
		return nullptr;

	void* pData;
	if (FAILED(pVertexBuffer->Lock(0, 0, &pData, 0)))
	{
		pVertexBuffer->Release();
		return nullptr;
	}

	memcpy(pData, vertices, count * sizeof(Vertex));
	pVertexBuffer->Unlock();

	return pVertexBuffer;
}

IDirect3DIndexBuffer9* CreateIndexBuffer(IDirect3DDevice9* pDevice, const int16_t* indices, const unsigned int count)
{
	IDirect3DIndexBuffer9* pIndexBuffer;
	if (FAILED(pDevice->CreateIndexBuffer
	(
		count * sizeof(int16_t),
		0,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&pIndexBuffer,
		nullptr
	)))
		return nullptr;

	void* pData;
	if (FAILED(pIndexBuffer->Lock(0, 0, &pData, 0)))
	{
		pIndexBuffer->Release();
		return nullptr;
	}

	memcpy(pData, indices, count * sizeof(int16_t));
	pIndexBuffer->Unlock();

	return pIndexBuffer;
}

ID3DXEffect* CreateEffect(IDirect3DDevice9* pDevice, wchar_t* filename)
{
	ID3DXEffect* pEffect;
	ID3DXBuffer* pBufferErrors{};

	if (FAILED(D3DXCreateEffectFromFile
	(
		pDevice,
		filename,
		nullptr,
		nullptr,
		0,
		nullptr,
		&pEffect,
		&pBufferErrors
	)))
	{
		if (pBufferErrors != nullptr)
		{
			void* pErrors = pBufferErrors->GetBufferPointer();
			std::ofstream fout(L"fxlog.txt", std::ios_base::app);
			fout << static_cast<char*>(pErrors) << std::endl;
			fout.close();
		}
		return nullptr;
	}

	return pEffect;
}

IDirect3DTexture9* CreateTexture(IDirect3DDevice9* pDevice, wchar_t* filename)
{
	IDirect3DTexture9* pTexture;
	if (FAILED(D3DXCreateTextureFromFile(pDevice, filename, &pTexture)))
		return nullptr;

	return pTexture;
}

int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	const auto windowTitle = L"D3D9Test";
	const auto screenWidth = 800;
	const auto screenHeight = 600;

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
		.lpszClassName = windowTitle,
	};
	if (!RegisterClassEx(&wc))
		return 0;

	RECT windowRect{ 0, 0, screenWidth, screenHeight };
	AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
	const auto windowWidth = windowRect.right - windowRect.left;
	const auto windowHeight = windowRect.bottom - windowRect.top;

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
		CreateVertexBuffer(pDevice.get(), cubeVertex, 24),
		[](IDirect3DVertexBuffer9* pVertexBuffer)
		{
			pVertexBuffer->Release();
		}
	);
	if (!pVertexBuffer)
		return 0;

	std::unique_ptr<IDirect3DIndexBuffer9, std::function<void(IDirect3DIndexBuffer9*)>> pIndexBuffer
	(
		CreateIndexBuffer(pDevice.get(), cubeIndex, 36),
		[](IDirect3DIndexBuffer9* pIndexBuffer)
		{
			pIndexBuffer->Release();
		}
	);
	if (!pIndexBuffer)
		return 0;

	std::unique_ptr<ID3DXEffect, std::function<void(ID3DXEffect*)>> pEffect
	(
		CreateEffect(pDevice.get(), L"test.fx"),
		[](ID3DXEffect* pEffect)
		{
			pEffect->Release();
		}
	);
	if (!pEffect)
		return 0;

	std::unique_ptr<IDirect3DTexture9, std::function<void(IDirect3DTexture9*)>> pTexture
	(
		CreateTexture(pDevice.get(), L"test.bmp"),
		[](IDirect3DTexture9* pTexture)
		{
			pTexture->Release();
		}
	);
	if (!pTexture)
		return 0;

	auto angle = 0.0f;

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
			pDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

			if (SUCCEEDED(pDevice->BeginScene()))
			{
				D3DXMATRIX matProjection;
				D3DXMatrixPerspectiveFovLH
				(
					&matProjection,
					D3DXToRadian(90),
					static_cast<float>(screenWidth) / static_cast<float>(screenHeight),
					1.0f,
					100.0f
				);
				pDevice->SetTransform(D3DTS_PROJECTION, &matProjection);

				D3DXMATRIX matView;
				const D3DXVECTOR3 eye(3.0f, 0.0f, 0.0f);
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

				pEffect->SetTechnique("Technique0");

				D3DXMATRIX worldViewProjection = matWorld * matView * matProjection;
				D3DXMatrixTranspose(&worldViewProjection, &worldViewProjection);
				pEffect->SetMatrix("worldViewProj", &worldViewProjection);

				pEffect->SetTexture("testTexture", pTexture.get());

				unsigned int uPasses;
				if (SUCCEEDED(pEffect->Begin(&uPasses, 0)))
				{
					for (unsigned int uPass = 0; uPass < uPasses; uPass++)
					{
						pEffect->BeginPass(uPass);

						pDevice->SetFVF(vertexFVF);
						pDevice->SetStreamSource(0, pVertexBuffer.get(), 0, sizeof(Vertex));
						pDevice->SetIndices(pIndexBuffer.get());
						pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 24, 0, 12);

						pEffect->EndPass();
					}

					pEffect->End();
				}

				pDevice->EndScene();
			}
			
			pDevice->Present(nullptr, nullptr, nullptr, nullptr);
		}
	}

	return 0;
}
