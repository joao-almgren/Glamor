#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "input.h"
#include "camera.h"
#include "d3dwrap.h"
#include "skybox.h"
#include "scape.h"
#include "sea.h"
#include "rock.h"
#include "post.h"
#include "butterfly.h"
#include "grass.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#include <memory>
#include <functional>

//*********************************************************************************************************************

// The enum type D3DFORMAT is unscoped. Prefer enum class over enum.
#pragma warning( push )
#pragma warning( disable : 26812 )
constexpr auto FOURCC_INTZ = ((D3DFORMAT)(MAKEFOURCC('I', 'N', 'T', 'Z')));
constexpr auto FOURCC_NULL = ((D3DFORMAT)(MAKEFOURCC('N', 'U', 'L', 'L')));
#pragma warning( pop )

//*********************************************************************************************************************

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
			if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
				return true;

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
	//ShowCursor(FALSE);

	Input input;
	if (!input.init(hWnd, hInstance))
		return 0;

	std::unique_ptr<IDirect3D9, std::function<void(IDirect3D9*)>> pD3D
	(
		[]() -> IDirect3D9*
		{
			IDirect3D9* pD3D;
			pD3D = Direct3DCreate9(D3D_SDK_VERSION);

			if (FAILED(pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, FOURCC_INTZ)))
			{
				pD3D->Release();
				return nullptr;
			}

			if (FAILED(pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, FOURCC_NULL)))
			{
				pD3D->Release();
				return nullptr;
			}

			return pD3D;
		}(),
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

			D3DCAPS9 caps;
			pDevice->GetDeviceCaps(&caps);
			if (caps.PixelShaderVersion < D3DPS_VERSION(3, 0)
				|| !(caps.DevCaps2 & D3DDEVCAPS2_STREAMOFFSET))
			{
				pDevice->Release();
				return nullptr;
			}

			pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

			return pDevice;
		}(),
		[](IDirect3DDevice9* pDevice)
		{
			pDevice->Release();
		}
	);
	if (!pDevice)
		return 0;

	auto resetProjection = [&pDevice]()
	{
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
	resetProjection();

	enum { DEFAULT_RTT, DEFAULT_Z, REFLECT_RTT, REFRACT_RTT, REFRACT_Z, SURFACE_RTT, SURFACE_Z, FLIP_RTT, BOUNCE1_RTT, BOUNCE2_RTT, SURFACE_COUNT };
	Texture rtReflect, rtRefract, rtRefractZ, rtSurfaceZ, rtFlip, rtBounce1, rtBounce2;
	Surface surface[SURFACE_COUNT];
	{
		// default surfaces
		IDirect3DSurface9* pSurface;
		if (FAILED(pDevice->GetRenderTarget(0, &pSurface)))
			return 0;
		surface[DEFAULT_RTT].reset(pSurface);

		if (FAILED(pDevice->GetDepthStencilSurface(&pSurface)))
			return 0;
		surface[DEFAULT_Z].reset(pSurface);

		// reflection rtt
		IDirect3DTexture9* pTexture;
		if (FAILED(pDevice->CreateTexture(512, 512, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtReflect.reset(pTexture);
		if (FAILED(rtReflect->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[REFLECT_RTT].reset(pSurface);

		// refraction rtt
		if (FAILED(pDevice->CreateTexture(512, 512, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtRefract.reset(pTexture);
		if (FAILED(rtRefract->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[REFRACT_RTT].reset(pSurface);

		// refraction depth rtt
		if (FAILED(pDevice->CreateTexture(512, 512, 1, D3DUSAGE_DEPTHSTENCIL, FOURCC_INTZ, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtRefractZ.reset(pTexture);
		if (FAILED(rtRefractZ->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[REFRACT_Z].reset(pSurface);

		// surface rtt
		if (FAILED(pDevice->CreateRenderTarget(512, 512, FOURCC_NULL, D3DMULTISAMPLE_NONE, 0, FALSE, &pSurface, nullptr)))
			return 0;
		surface[SURFACE_RTT].reset(pSurface);

		// surface depth rtt
		if (FAILED(pDevice->CreateTexture(512, 512, 1, D3DUSAGE_DEPTHSTENCIL, FOURCC_INTZ, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtSurfaceZ.reset(pTexture);
		if (FAILED(rtSurfaceZ->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[SURFACE_Z].reset(pSurface);

		// flip rtt
		if (FAILED(pDevice->CreateTexture(screenWidth, screenHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtFlip.reset(pTexture);
		if (FAILED(rtFlip->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[FLIP_RTT].reset(pSurface);

		// bounce1 rtt
		if (FAILED(pDevice->CreateTexture(screenWidth, screenHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtBounce1.reset(pTexture);
		if (FAILED(rtBounce1->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[BOUNCE1_RTT].reset(pSurface);

		// bounce2 rtt
		if (FAILED(pDevice->CreateTexture(256, 256, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtBounce2.reset(pTexture);
		if (FAILED(rtBounce2->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[BOUNCE2_RTT].reset(pSurface);
	}

	Scape scape(pDevice.get());
	if (!scape.init())
		return 0;

	Skybox skybox(pDevice.get());
	if (!skybox.init())
		return 0;

	Sea sea(pDevice.get(), rtReflect.get(), rtRefract.get(), rtRefractZ.get(), rtSurfaceZ.get());
	if (!sea.init())
		return 0;

	auto getScapeHeight = [&scape](float x, float z) -> float
	{
		return scape.height(x, z);
	};

	auto getScapeAngle = [&scape](float x, float z) -> float
	{
		return scape.angle(x, z);
	};

	Rock rock(pDevice.get());
	if (!rock.init(getScapeHeight, getScapeAngle))
		return 0;

	Post post(pDevice.get());
	if (!post.init())
		return 0;

	Butterfly butterfly(pDevice.get());
	if (!butterfly.init())
		return 0;

	Grass grass(pDevice.get());
	if (!grass.init(getScapeHeight, getScapeAngle))
		return 0;

	Camera camera(D3DXVECTOR3(0, 25, 0), 0, 0);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX9_Init(pDevice.get());

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
			//if (!io.WantCaptureMouse && !io.WantCaptureKeyboard)
			{
				input.update();

				POINT currMouse{ input.mouseState.lX, input.mouseState.lY };
				camera.rotate((float)-currMouse.y / 256.0f, (float)-currMouse.x / 256.0f);

				const float speed = 0.2f;
				if (input.keyState[DIK_D] || input.keyState[DIK_RIGHT])
					camera.moveRight(speed);
				else if (input.keyState[DIK_A] || input.keyState[DIK_LEFT])
					camera.moveRight(-speed);
				if (input.keyState[DIK_W] || input.keyState[DIK_UP])
					camera.moveForward(speed);
				else if (input.keyState[DIK_S] || input.keyState[DIK_DOWN])
					camera.moveForward(-speed);
				if (input.keyState[DIK_Q])
					camera.moveUp(speed);
				else if (input.keyState[DIK_Z])
					camera.moveUp(-speed);
			}

			// update
			{
				scape.update();
				skybox.update();
				sea.update();
				rock.update();
				butterfly.update();
				grass.update(camera.getPos());
			}

			D3DXMATRIX matRTTProj;
			D3DXMatrixPerspectiveFovLH(&matRTTProj, (D3DX_PI / 2), 1.0f, 1.0f, 1000.0f);
			pDevice->SetTransform(D3DTS_PROJECTION, &matRTTProj);

			// update reflection
			{
				pDevice->SetRenderTarget(0, surface[REFLECT_RTT].get());
				pDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

				if (SUCCEEDED(pDevice->BeginScene()))
				{
					camera.setView(pDevice.get());

					D3DXMATRIX matView;
					pDevice->GetTransform(D3DTS_VIEW, &matView);

					D3DXMATRIX matReflect;
					D3DXMatrixScaling(&matReflect, 1, -1, 1);

					D3DXMATRIX matReflectView = matReflect * matView;
					pDevice->SetTransform(D3DTS_VIEW, &matReflectView);

					rock.draw(RockRenderMode::Reflect);
					scape.draw(ScapeRenderMode::Reflect, camera.getPos());
					skybox.draw(camera.getPos());

					pDevice->EndScene();
				}
			}

			// update refraction
			{
				pDevice->SetRenderTarget(0, surface[REFRACT_RTT].get());
				pDevice->SetDepthStencilSurface(surface[REFRACT_Z].get());
				pDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

				if (SUCCEEDED(pDevice->BeginScene()))
				{
					camera.setView(pDevice.get());

					rock.draw(RockRenderMode::Refract);
					scape.draw(ScapeRenderMode::Normal, camera.getPos());

					pDevice->EndScene();
				}
			}

			// update surface
			{
				pDevice->SetRenderTarget(0, surface[SURFACE_RTT].get());
				pDevice->SetDepthStencilSurface(surface[SURFACE_Z].get());
				pDevice->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

				if (SUCCEEDED(pDevice->BeginScene()))
				{
					camera.setView(pDevice.get());

					sea.draw(SeaRenderMode::Plain, matRTTProj, camera.getPos());

					pDevice->EndScene();
				}

				pDevice->SetDepthStencilSurface(surface[DEFAULT_Z].get());
			}

			resetProjection();

			// render
			{
				pDevice->SetRenderTarget(0, surface[FLIP_RTT].get());
				pDevice->SetRenderTarget(1, surface[BOUNCE1_RTT].get());
				pDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

				if (SUCCEEDED(pDevice->BeginScene()))
				{
					camera.setView(pDevice.get());
					scape.draw(ScapeRenderMode::Normal, camera.getPos());
					rock.draw(RockRenderMode::Normal);
					grass.draw(GrassRenderMode::Plain);
					butterfly.draw();
					sea.draw(SeaRenderMode::Normal, matRTTProj, camera.getPos());
					skybox.draw(camera.getPos());
					grass.draw(GrassRenderMode::Blend);

					pDevice->EndScene();
				}

				pDevice->SetRenderTarget(1, nullptr);
			}

			// imgui
			{
				//static ImVec4 dear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
				//static float dear_float = 0.0f;
				//static bool dear_flip = false;

				ImGui_ImplDX9_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();

				ImGui::Begin("Debug");
				ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
				//ImGui::SliderFloat("Float", &dear_float, 0.0f, 1.0f);
				//ImGui::ColorEdit3("Colour", (float*)&dear_color);
				//ImGui::Checkbox("Flip", &dear_flip);
				ImGui::End();

				ImGui::EndFrame();
			}

			// post
			{
				pDevice->SetRenderTarget(0, surface[BOUNCE2_RTT].get());
				pDevice->Clear(0, nullptr, D3DCLEAR_TARGET, 0, 0, 0);

				if (SUCCEEDED(pDevice->BeginScene()))
				{
					post.draw(PostRenderMode::Down, { rtBounce1.get() });

					pDevice->EndScene();
				}

				pDevice->SetRenderTarget(0, surface[DEFAULT_RTT].get());
				pDevice->Clear(0, nullptr, D3DCLEAR_TARGET, 0, 0, 0);

				if (SUCCEEDED(pDevice->BeginScene()))
				{
					post.draw(PostRenderMode::Add, { rtFlip.get(), rtBounce2.get() });

					ImGui::Render();
					ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

					pDevice->EndScene();
				}
			}

			pDevice->Present(nullptr, nullptr, nullptr, nullptr);
		}
	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	return 0;
}

//*********************************************************************************************************************
