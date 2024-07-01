// Internal includes
#include "graphics/dx12_backend.h"
#include "graphics/dx12_containers.h"
#include "graphics/event_collector.h"
#include "tools/string_utilities.h"
#include "tools/security.h"

// System incldues
#include <windowsx.h>

namespace d3d12
{
	namespace window
	{
		void handle_messages(RenderWindow)
		{
			// Poll and handle messages
			MSG msg = {};
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			// We push the raw event anyway
			if (WM_PAINT != message)
				event_collector::push_event({ FrameEvent::Raw, message, wParam, lParam });

			switch (message)
			{
				case WM_PAINT:
					event_collector::request_draw();
					break;
				case WM_CLOSE:
					event_collector::push_event({FrameEvent::Close, 0, 0});
					break;
				case WM_DESTROY:
					event_collector::push_event({FrameEvent::Destroy, 0, 0});
					break;
				case WM_MOUSEMOVE:
				{
					POINT pt;
					GetCursorPos(&pt);
					ScreenToClient(hwnd, &pt);
					event_collector::push_event({ FrameEvent::MouseMovement, (uint32_t)pt.x, (uint32_t)pt.y });
				}
				break;
				case WM_MBUTTONDOWN:
					event_collector::push_event({ FrameEvent::MouseButton, (uint32_t)MK_MBUTTON, 1 });
				break;
				case WM_MBUTTONUP:
					event_collector::push_event({ FrameEvent::MouseButton, (uint32_t)MK_MBUTTON, 0 });
				break;

				case WM_RBUTTONDOWN:
					event_collector::push_event({ FrameEvent::MouseButton, (uint32_t)MK_RBUTTON, 1 });
				break;
				case WM_RBUTTONUP:
					event_collector::push_event({ FrameEvent::MouseButton, (uint32_t)MK_RBUTTON, 0 });
				break;

				case WM_LBUTTONDOWN:
					event_collector::push_event({ FrameEvent::MouseButton, (uint32_t)MK_LBUTTON, 1 });
				break;
				case WM_LBUTTONUP:
					event_collector::push_event({ FrameEvent::MouseButton, (uint32_t)MK_LBUTTON, 0 });
				break;

				case WM_MOUSEWHEEL:
					event_collector::push_event({ FrameEvent::MouseWheel, (uint32_t)GET_WHEEL_DELTA_WPARAM(wParam), 0 });
					break;
				case WM_KEYDOWN:
					event_collector::push_event({ FrameEvent::KeyDown, (uint32_t)wParam, 0 });
					break;
				case WM_KEYUP:
					event_collector::push_event({ FrameEvent::KeyUp, (uint32_t)wParam, 0 });
					break;
				default:
					return DefWindowProc(hwnd, message, wParam, lParam); // add this
			}

			return 0;
		}

		// Function to register the window
		void RegisterWindowClass(HINSTANCE hInst, const char* windowClassName)
		{
			// Register a window class for creating our render window with.
			WNDCLASSEXA windowClass = {};

			windowClass.cbSize = sizeof(WNDCLASSEX);
			windowClass.style = CS_HREDRAW | CS_VREDRAW;
			windowClass.lpfnWndProc = &WndProc;
			windowClass.cbClsExtra = 0;
			windowClass.cbWndExtra = 0;
			windowClass.hInstance = hInst;
			windowClass.hIcon = LoadIcon(hInst, NULL);
			windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
			windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			windowClass.lpszMenuName = NULL;
			windowClass.lpszClassName = windowClassName;
			windowClass.hIconSm = LoadIcon(hInst, NULL);

			// Register the window
			assert_msg(RegisterClassExA(&windowClass) > 0, "RegisterClassExA failed.");
		}

		void EvaluateWindowParameters(uint32_t width, uint32_t height, int32_t& windowWidth, int32_t& windowHeight, int32_t& windowX, int32_t& windowY)
		{
			// Get the size of the monitor
			int screenWidth = GetSystemMetrics(SM_CXSCREEN);
			int screenHeight = GetSystemMetrics(SM_CYSCREEN);

			// Calculates the required size of the window rectangle, based on the desired client-rectangle size.
			RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
			assert_msg(AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE), "AdjustWindowRect failed.");

			windowWidth = windowRect.right - windowRect.left;
			windowHeight = windowRect.bottom - windowRect.top;

			// Center the window within the screen. Clamp to 0, 0 for the top-left corner.
			windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
			windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);
		}

		// Function to create the window
		HWND CreateWindowInternal(const wchar_t* windowClassName, HINSTANCE hInst, const wchar_t* windowTitle, uint32_t width, uint32_t height)
		{
			// Evaluate the actual size and location of the window
			int32_t windowWidth = 1, windowHeight = 1, windowX = 0, windowY = 0;
			EvaluateWindowParameters(width, height, windowWidth, windowHeight, windowX, windowY);

			// Center the window within the screen.
			HWND hWnd = CreateWindowExW(NULL, windowClassName, windowTitle, WS_OVERLAPPEDWINDOW, windowX, windowY, windowWidth, windowHeight, NULL, NULL, hInst, nullptr);
			assert_msg(hWnd != nullptr, "Failed to create window");

			// Return the created window
			return hWnd;
		}

		RenderWindow create_window(uint64_t hInstance, uint32_t width, uint32_t height, const char* windowName)
		{
			// Create the window internal structure
			DX12Window* dx12_window = new DX12Window();

			// Convert the name to wide
			const std::wstring& wc = convert_to_wide(windowName);

			// Grab the instance
			HINSTANCE hInst = (HINSTANCE)hInstance;

			// Register the window class
			RegisterWindowClass(hInst, "BacasableWindow");

			// Handle the mouse
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.hwndTrack = dx12_window->window;
			tme.dwFlags = TME_HOVER | TME_LEAVE;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);

			// Evaluate the actual size and location of the window
			int32_t windowWidth = 1, windowHeight = 1, windowX = 0, windowY = 0;
			EvaluateWindowParameters(width, height, windowWidth, windowHeight, windowX, windowY);

			// Create the window
			dx12_window->window = CreateWindowExA(
				0,												// Optional window styles.
				"BacasableWindow",								// Window class
				windowName,										// Window text
				WS_OVERLAPPEDWINDOW,							// Window style

				// Size and position
				windowX, windowY, windowWidth, windowHeight,

				NULL,       // Parent window    
				NULL,       // Menu
				hInst,		// Instance handle
				NULL        // Additional application data
			);
			assert_msg(dx12_window->window != nullptr, "Failed to create window.");

			// Cast the window to the opaque type
			return (RenderWindow)dx12_window;
		}

		void destroy_window(RenderWindow renderWindow)
		{
			// Grab the internal windows structure
			DX12Window* dx12_window = (DX12Window*)renderWindow;

			// Destroy the actual window
			assert_msg(DestroyWindow(dx12_window->window), "Failed to destroy window.");

			// Detroy the internal window structure
			delete dx12_window;
		}

		void show(RenderWindow renderWindow)
		{
			DX12Window* dx12_window = (DX12Window*)renderWindow;
			ShowWindow(dx12_window->window, SW_SHOWDEFAULT);
		}

		void hide(RenderWindow renderWindow)
		{
			DX12Window* dx12_window = (DX12Window*)renderWindow;
			ShowWindow(dx12_window->window, SW_HIDE);
		}

		void set_cursor_visibility(bool state)
		{
			ShowCursor(state);
		}

		void set_cursor_pos(RenderWindow window, uint2 position)
		{
			DX12Window* dx12_window = (DX12Window*)window;
			POINT pt = { (long)position.x, (long)position.y };
			ClientToScreen(dx12_window->window, &pt);
			SetCursorPos(pt.x, pt.y);
		}

		uint2 window_center(RenderWindow window)
		{
			DX12Window* dx12_window = (DX12Window*)window;
			uint4 windowBounds;
			window::window_bounds(window, windowBounds);
			POINT windowCenter;
			windowCenter.x = (windowBounds.x + windowBounds.z) / 2;
			windowCenter.y = (windowBounds.y + windowBounds.w) / 2;
			ScreenToClient(dx12_window->window, &windowCenter);
			return uint2({ (uint32_t)windowCenter.x, (uint32_t)windowCenter.y});
		}

		void window_size(RenderWindow window, uint2& size)
		{
			DX12Window* dx12_window = (DX12Window*)window;
			RECT rect;
			GetWindowRect(dx12_window->window, &rect);
			size = uint2({ (uint32_t)rect.right - (uint32_t)rect.left, (uint32_t)rect.bottom - (uint32_t)rect.top });
		}

		void window_bounds(RenderWindow renderWindow, uint4& bounds)
		{
			DX12Window* dx12_window = (DX12Window*)renderWindow;
			RECT rect;
			GetWindowRect(dx12_window->window, &rect);
			bounds = uint4({ (uint32_t)rect.left, (uint32_t)rect.top, (uint32_t)rect.right, (uint32_t)rect.bottom });
		}

		void viewport_size(RenderWindow window, uint2& size)
		{
			DX12Window* dx12_window = (DX12Window*)window;
			RECT rect;
			GetClientRect(dx12_window->window, &rect);
			size = uint2({ (uint32_t)rect.right - (uint32_t)rect.left, (uint32_t)rect.bottom - (uint32_t)rect.top});
		}

		uint2 viewport_center(RenderWindow window)
		{
			uint4 viewportBounds;
			window::viewport_bounds(window, viewportBounds);
			return uint2({ (viewportBounds.x + viewportBounds.z) / 2, (viewportBounds.y + viewportBounds.w) / 2 });
		}

		void viewport_bounds(RenderWindow renderWindow, uint4& bounds)
		{
			DX12Window* dx12_window = (DX12Window*)renderWindow;
			RECT rect;
			GetClientRect(dx12_window->window, &rect);
			bounds = uint4({ (uint32_t)rect.left, (uint32_t)rect.top, (uint32_t)rect.right, (uint32_t)rect.bottom });
		}
	}
}
