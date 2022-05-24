#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "constants.h"
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
#include "tree.h"
#include "fish.h"
#include "statue.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#include <memory>
#include <functional>

constexpr auto FOURCC_INTZ = static_cast<D3DFORMAT>(MAKEFOURCC('I', 'N', 'T', 'Z'));
constexpr auto FOURCC_NULL = static_cast<D3DFORMAT>(MAKEFOURCC('N', 'U', 'L', 'L'));

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPWSTR /*lpCmdLine*/, _In_ int /*nShowCmd*/)
{
	const auto windowTitle{ L"Glamor" };

	WNDCLASSEX wc
	{
		.cbSize = sizeof(WNDCLASSEX),
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

	RECT windowRect{ .right = SCREEN_WIDTH, .bottom = SCREEN_HEIGHT };
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

	Input input;
	if (!input.init(hWnd, hInstance))
		return 0;

	std::unique_ptr<IDirect3D9, std::function<void(IDirect3D9*)>> pD3D
	(
		[]() -> IDirect3D9*
		{
			return Direct3DCreate9(D3D_SDK_VERSION);
		}(),
		[](IDirect3D9* me)
		{
			me->Release();
		}
	);
	if (!pD3D)
		return 0;

	std::unique_ptr<IDirect3DDevice9, std::function<void(IDirect3DDevice9*)>> pDevice
	(
		[&pD3D, &hWnd]() -> IDirect3DDevice9*
		{
			if (FAILED(pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, FOURCC_INTZ)))
			{
				return nullptr;
			}

			if (FAILED(pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, FOURCC_NULL)))
			{
				return nullptr;
			}

			D3DPRESENT_PARAMETERS d3dpp
			{
				.BackBufferWidth = SCREEN_WIDTH,
				.BackBufferHeight = SCREEN_HEIGHT,
				.BackBufferFormat = D3DFMT_A8R8G8B8,
				.BackBufferCount = 1,
				.SwapEffect = D3DSWAPEFFECT_DISCARD,
				.Windowed = TRUE,
				.EnableAutoDepthStencil = TRUE,
				.AutoDepthStencilFormat = D3DFMT_D24S8,
			};

			IDirect3DDevice9* me;
			if (FAILED(pD3D->CreateDevice
			(
				D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL,
				hWnd,
				D3DCREATE_HARDWARE_VERTEXPROCESSING,
				&d3dpp,
				&me
			)))
				return nullptr;

			D3DCAPS9 caps;
			me->GetDeviceCaps(&caps);
			if (caps.PixelShaderVersion < D3DPS_VERSION(3, 0)
				|| !(caps.DevCaps2 & D3DDEVCAPS2_STREAMOFFSET))
			{
				me->Release();
				return nullptr;
			}

			me->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

			return me;
		}(),
		[](IDirect3DDevice9* me)
		{
			me->Release();
		}
	);
	if (!pDevice)
		return 0;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX9_Init(pDevice.get());

	enum { DEFAULT_RTT, DEFAULT_Z, REFLECT_RTT, REFRACT_RTT, REFRACT_Z, SURFACE_RTT, SURFACE_Z, FLIP_RTT, BOUNCE1_RTT, BOUNCE2_RTT, SHADOW_RTT, SHADOW_Z, SURFACE_COUNT };
	Texture rtReflect, rtRefract, rtRefractZ, rtSurfaceZ, rtFlip, rtBounce1, rtBounce2, rtShadowZ;
	Surface surface[SURFACE_COUNT];
	{
		// default surfaces
		IDirect3DSurface9* pSurface;
		if (FAILED(pDevice->GetRenderTarget(0, &pSurface)))
			return 0;
		surface[DEFAULT_RTT] = makeSurface();
		surface[DEFAULT_RTT].reset(pSurface);

		if (FAILED(pDevice->GetDepthStencilSurface(&pSurface)))
			return 0;
		surface[DEFAULT_Z] = makeSurface();
		surface[DEFAULT_Z].reset(pSurface);

		// reflection rtt
		IDirect3DTexture9* pTexture;
		if (FAILED(pDevice->CreateTexture(WATER_TEX_SIZE, WATER_TEX_SIZE, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtReflect = makeTexture();
		rtReflect.reset(pTexture);
		if (FAILED(rtReflect->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[REFLECT_RTT] = makeSurface();
		surface[REFLECT_RTT].reset(pSurface);

		// refraction rtt
		if (FAILED(pDevice->CreateTexture(WATER_TEX_SIZE, WATER_TEX_SIZE, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtRefract = makeTexture();
		rtRefract.reset(pTexture);
		if (FAILED(rtRefract->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[REFRACT_RTT] = makeSurface();
		surface[REFRACT_RTT].reset(pSurface);

		// refraction depth rtt
		if (FAILED(pDevice->CreateTexture(WATER_TEX_SIZE, WATER_TEX_SIZE, 1, D3DUSAGE_DEPTHSTENCIL, FOURCC_INTZ, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtRefractZ = makeTexture();
		rtRefractZ.reset(pTexture);
		if (FAILED(rtRefractZ->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[REFRACT_Z] = makeSurface();
		surface[REFRACT_Z].reset(pSurface);

		// surface rtt
		if (FAILED(pDevice->CreateRenderTarget(WATER_TEX_SIZE, WATER_TEX_SIZE, FOURCC_NULL, D3DMULTISAMPLE_NONE, 0, FALSE, &pSurface, nullptr)))
			return 0;
		surface[SURFACE_RTT] = makeSurface();
		surface[SURFACE_RTT].reset(pSurface);

		// surface depth rtt
		if (FAILED(pDevice->CreateTexture(WATER_TEX_SIZE, WATER_TEX_SIZE, 1, D3DUSAGE_DEPTHSTENCIL, FOURCC_INTZ, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtSurfaceZ = makeTexture();
		rtSurfaceZ.reset(pTexture);
		if (FAILED(rtSurfaceZ->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[SURFACE_Z] = makeSurface();
		surface[SURFACE_Z].reset(pSurface);

		// flip rtt
		if (FAILED(pDevice->CreateTexture(SCREEN_WIDTH, SCREEN_HEIGHT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtFlip = makeTexture();
		rtFlip.reset(pTexture);
		if (FAILED(rtFlip->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[FLIP_RTT] = makeSurface();
		surface[FLIP_RTT].reset(pSurface);

		// bounce1 rtt
		if (FAILED(pDevice->CreateTexture(SCREEN_WIDTH, SCREEN_HEIGHT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtBounce1 = makeTexture();
		rtBounce1.reset(pTexture);
		if (FAILED(rtBounce1->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[BOUNCE1_RTT] = makeSurface();
		surface[BOUNCE1_RTT].reset(pSurface);

		// bounce2 rtt
		if (FAILED(pDevice->CreateTexture(BOUNCE_TEX_SIZE, BOUNCE_TEX_SIZE, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtBounce2 = makeTexture();
		rtBounce2.reset(pTexture);
		if (FAILED(rtBounce2->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[BOUNCE2_RTT] = makeSurface();
		surface[BOUNCE2_RTT].reset(pSurface);

		// shadow rtt
		if (FAILED(pDevice->CreateRenderTarget(SHADOW_TEX_SIZE, SHADOW_TEX_SIZE, FOURCC_NULL, D3DMULTISAMPLE_NONE, 0, FALSE, &pSurface, nullptr)))
			return 0;
		surface[SHADOW_RTT] = makeSurface();
		surface[SHADOW_RTT].reset(pSurface);

		// shadow depth rtt
		if (FAILED(pDevice->CreateTexture(SHADOW_TEX_SIZE, SHADOW_TEX_SIZE, 1, D3DUSAGE_DEPTHSTENCIL, FOURCC_INTZ, D3DPOOL_DEFAULT, &pTexture, nullptr)))
			return 0;
		rtShadowZ = makeTexture();
		rtShadowZ.reset(pTexture);
		if (FAILED(rtShadowZ->GetSurfaceLevel(0, &pSurface)))
			return 0;
		surface[SHADOW_Z] = makeSurface();
		surface[SHADOW_Z].reset(pSurface);
	}

	Post post(pDevice.get());
	if (!post.init())
		return 0;

	Camera camera(pDevice.get(), D3DXVECTOR3(0, 25, 5), 0, 0);

	Skybox skybox(pDevice.get(), &camera);
	if (!skybox.init())
		return 0;

	Sea sea(pDevice.get(), &camera, rtReflect.get(), rtRefract.get(), rtRefractZ.get(), rtSurfaceZ.get(), rtShadowZ.get());
	if (!sea.init())
		return 0;

	Scape scape(pDevice.get(), &camera, rtShadowZ.get());
	if (!scape.init())
		return 0;

	auto getScapeHeight = [&scape](const float x, const float z) -> float
	{
		return scape.height(x, z);
	};

	auto getScapeAngle = [&scape](const float x, const float z) -> float
	{
		return scape.angle(x, z);
	};

	Tree tree(pDevice.get(), &camera, rtShadowZ.get());
	if (!tree.init(getScapeHeight, getScapeAngle))
		return 0;

	Rock rock(pDevice.get(), &camera, rtShadowZ.get());
	if (!rock.init(getScapeHeight, getScapeAngle))
		return 0;

	Grass grass(pDevice.get(), &camera, rtShadowZ.get());
	if (!grass.init(getScapeHeight, getScapeAngle))
		return 0;

	Statue statue(pDevice.get(), &camera, rtShadowZ.get());
	if (!statue.init())
		return 0;

	Fish fish(pDevice.get());
	if (!fish.init())
		return 0;

	Butterfly butterfly(pDevice.get(), &camera, rtShadowZ.get());
	if (!butterfly.init())
		return 0;

	bool keyDownM = false;
	bool mouseToggle = true;

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

				float speed = 0.05f;
				if (input.keyState[DIK_LSHIFT])
					speed *= 6;
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

				if (input.keyState[DIK_M])
				{
					if (!keyDownM)
						keyDownM = true;
				}
				else if (keyDownM)
				{
					keyDownM = false;
					mouseToggle = !mouseToggle;
					ShowCursor(mouseToggle);
				}

				camera.setProjection();
				camera.setView();
				camera.setFrustum();

				scape.update();
				skybox.update();
				sea.update();
				rock.update();
				butterfly.update();
				grass.update();
				tree.update();
				fish.update();
				statue.update();
			}

			D3DXMATRIX matLightViewProj;

			// update shadow
			{
				D3DXMATRIX matLightProj;
				D3DXMatrixOrthoLH(&matLightProj, 280, 340, 1, 200);
				pDevice->SetTransform(D3DTS_PROJECTION, &matLightProj);

				D3DXMATRIX matLightView;
				D3DXVECTOR3 light(1, 2, 1);
				D3DXVec3Normalize(&light, &light);
				D3DXVECTOR3 pos(170 * light);
				D3DXVECTOR3 at(pos - light);
				D3DXVECTOR3 yup(0, 1, 0);
				D3DXMatrixLookAtLH(&matLightView, &pos, &at, &yup);
				pDevice->SetTransform(D3DTS_VIEW, &matLightView);

				matLightViewProj = matLightView * matLightProj;

				pDevice->SetRenderTarget(0, surface[SHADOW_RTT].get());
				pDevice->SetDepthStencilSurface(surface[SHADOW_Z].get());
				pDevice->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

				if (SUCCEEDED(pDevice->BeginScene()))
				{
					statue.draw(StatueRenderMode::CASTER, matLightViewProj);
					tree.draw(TreeRenderMode::CASTER, matLightViewProj);
					rock.draw(RockRenderMode::CASTER, matLightViewProj);
					scape.draw(ScapeRenderMode::CASTER, matLightViewProj);

					pDevice->EndScene();
				}
			}

			D3DXMATRIX matRTTProj;
			D3DXMatrixPerspectiveFovLH(&matRTTProj, (D3DX_PI / 2), 1.0f, NEAR_PLANE, FAR_PLANE);
			pDevice->SetTransform(D3DTS_PROJECTION, &matRTTProj);

			if (camera.getPos().y > 0)
			{
				// update reflection
				{
					pDevice->SetRenderTarget(0, surface[REFLECT_RTT].get());
					pDevice->Clear(0, nullptr, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, 1.0f, 0);

					if (SUCCEEDED(pDevice->BeginScene()))
					{
						camera.setView();

						D3DXMATRIX matView;
						pDevice->GetTransform(D3DTS_VIEW, &matView);

						D3DXMATRIX matReflect;
						D3DXMatrixScaling(&matReflect, 1, -1, 1);

						D3DXMATRIX matReflectView = matReflect * matView;
						pDevice->SetTransform(D3DTS_VIEW, &matReflectView);

						statue.draw(StatueRenderMode::REFLECT, matLightViewProj);
						tree.draw(TreeRenderMode::ALPHA_CLIP, matLightViewProj);
						rock.draw(RockRenderMode::REFLECT, matLightViewProj);
						scape.draw(ScapeRenderMode::REFLECT, matLightViewProj);
						skybox.draw();

						pDevice->EndScene();
					}
				}

				camera.setView();

				// update refraction
				{
					pDevice->SetRenderTarget(0, surface[REFRACT_RTT].get());
					pDevice->SetDepthStencilSurface(surface[REFRACT_Z].get());
					pDevice->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

					if (SUCCEEDED(pDevice->BeginScene()))
					{
						fish.draw(FishRenderMode::NORMAL);
						rock.draw(RockRenderMode::REFRACT, matLightViewProj);
						scape.draw(ScapeRenderMode::SIMPLE, matLightViewProj);

						pDevice->EndScene();
					}
				}

				// update surface depth
				{
					pDevice->SetRenderTarget(0, surface[SURFACE_RTT].get());
					pDevice->SetDepthStencilSurface(surface[SURFACE_Z].get());
					pDevice->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

					if (SUCCEEDED(pDevice->BeginScene()))
					{
						sea.draw(SeaRenderMode::PLAIN, matRTTProj, matLightViewProj);

						pDevice->EndScene();
					}

					pDevice->SetDepthStencilSurface(surface[DEFAULT_Z].get());
				}
			}
			else if (camera.getPos().y < 0)
			{
				// update reflection
				{
					pDevice->SetRenderTarget(0, surface[REFLECT_RTT].get());
					pDevice->Clear(0, nullptr, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, 1.0f, 0);

					if (SUCCEEDED(pDevice->BeginScene()))
					{
						camera.setView();

						D3DXMATRIX matView;
						pDevice->GetTransform(D3DTS_VIEW, &matView);

						D3DXMATRIX matReflect;
						D3DXMatrixScaling(&matReflect, 1, -1, 1);

						D3DXMATRIX matReflectView = matReflect * matView;
						pDevice->SetTransform(D3DTS_VIEW, &matReflectView);

						fish.draw(FishRenderMode::REFLECT);
						rock.draw(RockRenderMode::UNDERWATER_REFLECT, matLightViewProj);
						scape.draw(ScapeRenderMode::UNDERWATER_REFLECT, matLightViewProj);

						pDevice->EndScene();
					}
				}

				camera.setView();

				// update refraction
				{
					pDevice->SetRenderTarget(0, surface[REFRACT_RTT].get());
					pDevice->Clear(0, nullptr, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, 1.0f, 0);

					if (SUCCEEDED(pDevice->BeginScene()))
					{
						statue.draw(StatueRenderMode::SIMPLE, matLightViewProj);
						tree.draw(TreeRenderMode::ALPHA_CLIP, matLightViewProj);
						rock.draw(RockRenderMode::NORMAL, matLightViewProj);
						scape.draw(ScapeRenderMode::SIMPLE, matLightViewProj);
						skybox.draw();

						pDevice->EndScene();
					}
				}
			}

			camera.setProjection();
			camera.setView();

			// render
			{
				pDevice->SetRenderTarget(0, surface[FLIP_RTT].get());
				pDevice->SetRenderTarget(1, surface[BOUNCE1_RTT].get());
				pDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, 1.0f, 0);

				if (SUCCEEDED(pDevice->BeginScene()))
				{
					if (camera.getPos().y > 0)
					{
						statue.draw(StatueRenderMode::NORMAL, matLightViewProj);
						fish.draw(FishRenderMode::NORMAL);
						butterfly.draw(matLightViewProj);
						tree.draw(TreeRenderMode::ALPHA_CLIP, matLightViewProj);
						rock.draw(RockRenderMode::NORMAL, matLightViewProj);
						grass.draw(GrassRenderMode::PLAIN, matLightViewProj);
						scape.draw(ScapeRenderMode::SHADOW, matLightViewProj);
						sea.draw(SeaRenderMode::NORMAL, matRTTProj, matLightViewProj);
						skybox.draw();
						tree.draw(TreeRenderMode::ALPHA_BLEND, matLightViewProj);
						grass.draw(GrassRenderMode::BLEND, matLightViewProj);
					}
					else
					{
						fish.draw(FishRenderMode::NORMAL);
						rock.draw(RockRenderMode::REFRACT, matLightViewProj);
						scape.draw(ScapeRenderMode::UNDERWATER, matLightViewProj);
						sea.draw(SeaRenderMode::UNDERWATER, matRTTProj, matLightViewProj);
					}

					pDevice->EndScene();
				}

				pDevice->SetRenderTarget(1, nullptr);
			}

			// imgui
			//static ImVec4 dear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
			//static float dear_float = 0.0f;
			//static bool dear_flip = false;
			{
				ImGui_ImplDX9_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();

				ImGui::Begin("Debug");
				ImGui::Text("%.1f FPS", (double)ImGui::GetIO().Framerate);
				//ImGui::SliderFloat("Float", &dear_float, 0.0f, 1.0f);
				//ImGui::ColorEdit3("Colour", (float*)&dear_color);
				//ImGui::Checkbox("Flip", &dear_flip);
				ImGui::End();

				ImGui::Begin("RTT");
				ImGui::Image(rtReflect.get(), ImVec2(128, 128));
				ImGui::Image(rtRefract.get(), ImVec2(128, 128));
				//ImGui::Image(rtRefractZ.get(), ImVec2(128, 128));
				//ImGui::Image(rtSurfaceZ.get(), ImVec2(128, 128));
				//ImGui::Image(rtFlip.get(), ImVec2(128, 128));
				ImGui::Image(rtBounce1.get(), ImVec2(128, 128));
				//ImGui::Image(rtBounce2.get(), ImVec2(128, 128));
				ImGui::Image(rtShadowZ.get(), ImVec2(128, 128));
				ImGui::End();

				ImGui::EndFrame();
			}

			// post
			{
				pDevice->SetRenderTarget(0, surface[BOUNCE2_RTT].get());

				if (SUCCEEDED(pDevice->BeginScene()))
				{
					post.draw(PostRenderMode::DOWN, { rtBounce1.get() });

					pDevice->EndScene();
				}

				pDevice->SetRenderTarget(0, surface[DEFAULT_RTT].get());

				if (SUCCEEDED(pDevice->BeginScene()))
				{
					post.draw(PostRenderMode::ADD, { rtFlip.get(), rtBounce2.get() });

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
