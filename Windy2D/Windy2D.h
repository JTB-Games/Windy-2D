#pragma once
#include <algorithm>  
#include <windows.h>
#include<vector>
#include<string>
#include<list>
//some drawing functions are tooken from OLC console game engine
namespace Windy {
	typedef char s8;
	typedef unsigned char u8;
	typedef short s16;
	typedef unsigned short u16;
	typedef int s32;
	typedef unsigned int u32;
	typedef long long s64;
	typedef unsigned long long u64;
#define WindyMain WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)


	inline int clamp(int min, int val, int max) {
		if (val < min) return min;
		if (val > max) return max;
		return val;
	}


	static bool running = true;

	struct Render_State {
		int height, width;
		void* memory;

		BITMAPINFO bitmap_info;
	};
	u32 ClearColor;
	static Render_State Screen_Data;
	HWND window;
	HDC hdc;
	bool Focused;
	void ClearScreen(u32 color) {
		unsigned int* pixel = (u32*)Screen_Data.memory;
		for (int y = 0; y < Screen_Data.height; y++) {
			for (int x = 0; x < Screen_Data.width; x++) {
				*pixel++ = color;
			}
		}
	}
	void DrawFilledRect(int x0, int y0, int x1, int y1, u32 color) {
		//Oclussion culling?
		x0 = clamp(0, x0, Screen_Data.width);
		x1 = clamp(0, x1, Screen_Data.width);
		y0 = clamp(0, y0, Screen_Data.height);
		y1 = clamp(0, y1, Screen_Data.height);

		for (int y = y0; y < y1; y++) {
			u32* pixel = (u32*)Screen_Data.memory + x0 + y * Screen_Data.width;
			for (int x = x0; x < x1; x++) {
				*pixel++ = color;
			}
		}
	}
	void DrawPixel(int x, int y, u32 color) {
		DrawFilledRect(x, y, x + 1, y + 1, color);
	}
	//Adapted from OLC console game engine
	void DrawLine(int x1, int y1, int x2, int y2, u32 color)
	{
		int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
		dx = x2 - x1; dy = y2 - y1;
		dx1 = abs(dx); dy1 = abs(dy);
		px = 2 * dy1 - dx1;	py = 2 * dx1 - dy1;
		if (dy1 <= dx1)
		{
			if (dx >= 0)
			{
				x = x1; y = y1; xe = x2;
			}
			else
			{
				x = x2; y = y2; xe = x1;
			}

			DrawPixel(x, y, color);

			for (i = 0; x < xe; i++)
			{
				x = x + 1;
				if (px < 0)
					px = px + 2 * dy1;
				else
				{
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) y = y + 1; else y = y - 1;
					px = px + 2 * (dy1 - dx1);
				}
				DrawPixel(x, y, color);
			}
		}
		else
		{
			if (dy >= 0)
			{
				x = x1; y = y1; ye = y2;
			}
			else
			{
				x = x2; y = y2; ye = y1;
			}

			DrawPixel(x, y, color);

			for (i = 0; y < ye; i++)
			{
				y = y + 1;
				if (py <= 0)
					py = py + 2 * dx1;
				else
				{
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) x = x + 1; else x = x - 1;
					py = py + 2 * (dx1 - dy1);
				}
				DrawPixel(x, y, color);
			}
		}
	}
	LRESULT CALLBACK windy_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		LRESULT result = 0;

		switch (uMsg) {
		case WM_CLOSE:
		case WM_DESTROY: {
			running = false;
		} break;
		case WM_SIZE: {

			RECT rect;
			GetClientRect(hwnd, &rect);
			Screen_Data.width = rect.right - rect.left;
			Screen_Data.height = rect.bottom - rect.top;
			int size = Screen_Data.width * Screen_Data.height * sizeof(unsigned int);
			if (Screen_Data.memory) VirtualFree(Screen_Data.memory, 0, MEM_RELEASE);
			Screen_Data.memory = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			Screen_Data.bitmap_info.bmiHeader.biSize = sizeof(Screen_Data.bitmap_info.bmiHeader);
			Screen_Data.bitmap_info.bmiHeader.biWidth = Screen_Data.width;
			Screen_Data.bitmap_info.bmiHeader.biHeight = Screen_Data.height;
			Screen_Data.bitmap_info.bmiHeader.biPlanes = 1;
			Screen_Data.bitmap_info.bmiHeader.biBitCount = 32;
			Screen_Data.bitmap_info.bmiHeader.biCompression = BI_RGB;


		} break;
		case WM_SETFOCUS: {
			Focused = true;
		}break;

		case WM_KILLFOCUS: {
			Focused = false;
		}break;
		default: {
			result = DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		}
		return result;
	}

	//Update Windy Window
	void WindyUpdate() {
		ClearScreen(ClearColor);
		MSG message;
		while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}
	//Draw render data to screen
	void WindyDraw()
	{

		//Draw render data to screen
		StretchDIBits(hdc, 0, 0, Screen_Data.width, Screen_Data.height, 0, 0, Screen_Data.width, Screen_Data.height, Screen_Data.memory, &Screen_Data.bitmap_info, DIB_RGB_COLORS, SRCCOPY);
	}
	// Create Window
	void WindyCreateWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd, int Width, int Height, LPCWSTR WindowName, u32 ClearBackground, bool Fullscreen) {
		// Create Window Class
		WNDCLASS window_class = {};
		window_class.style = CS_HREDRAW | CS_VREDRAW;
		window_class.lpszClassName = L"Window Class";
		window_class.lpfnWndProc = windy_callback;

		// Register Class
		RegisterClass(&window_class);

		// Create Window
		ClearColor = ClearBackground;
		int x = Width;
		int y = Height;
		//Create the window
		if (!Fullscreen)
			window = CreateWindow(window_class.lpszClassName, WindowName, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, x, y, 0, 0, hInstance, 0);
		else
			window = CreateWindow(window_class.lpszClassName, WindowName, WS_MAXIMIZE | WS_POPUPWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, x, y, 0, 0, hInstance, 0);
		hdc = GetDC(window);
	}
	void DrawTriangleOutline(int p1x, int p1y, int p2x, int p2y, int p3x, int p3y, u32 col)
	{
		DrawLine(p1x, p1y, p2x, p2y, col);
		DrawLine(p2x, p2y, p3x, p3y, col);
		DrawLine(p3x, p3y, p1x, p1y, col);
	}
	// Adapted from OLC console game engine
	void DrawFilledTriangle(int p1x, int p1y, int p2x, int p2y, int p3x, int p3y, u32 col)
	{
		auto drawline = [&](int sx, int ex, int ny) { for (int i = sx; i <= ex; i++) DrawPixel(i, ny, col); };

		int t1x, t2x, y, minx, maxx, t1xp, t2xp;
		bool changed1 = false;
		bool changed2 = false;
		int signx1, signx2, dx1, dy1, dx2, dy2;
		int e1, e2;
		// Sort vertices
		if (p1y > p2y) { std::swap(p1y, p2y); std::swap(p1x, p2x); }
		if (p1y > p3y) { std::swap(p1y, p3y); std::swap(p1x, p3x); }
		if (p2y > p3y) { std::swap(p2y, p3y); std::swap(p2x, p3x); }

		t1x = t2x = p1x; y = p1y;   // Starting points
		dx1 = (int)(p2x - p1x);
		if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
		else signx1 = 1;
		dy1 = (int)(p2y - p1y);

		dx2 = (int)(p3x - p1x);
		if (dx2 < 0) { dx2 = -dx2; signx2 = -1; }
		else signx2 = 1;
		dy2 = (int)(p3y - p1y);

		if (dy1 > dx1) { std::swap(dx1, dy1); changed1 = true; }
		if (dy2 > dx2) { std::swap(dy2, dx2); changed2 = true; }

		e2 = (int)(dx2 >> 1);
		// Flat top, just process the second half
		if (p1y == p2y) goto next;
		e1 = (int)(dx1 >> 1);

		for (int i = 0; i < dx1;) {
			t1xp = 0; t2xp = 0;
			if (t1x < t2x) { minx = t1x; maxx = t2x; }
			else { minx = t2x; maxx = t1x; }
			// process first line until y value is about to change
			while (i < dx1) {
				i++;
				e1 += dy1;
				while (e1 >= dx1) {
					e1 -= dx1;
					if (changed1) t1xp = signx1;//t1x += signx1;
					else          goto next1;
				}
				if (changed1) break;
				else t1x += signx1;
			}
			// Move line
		next1:
			// process second line until y value is about to change
			while (1) {
				e2 += dy2;
				while (e2 >= dx2) {
					e2 -= dx2;
					if (changed2) t2xp = signx2;//t2x += signx2;
					else          goto next2;
				}
				if (changed2)     break;
				else              t2x += signx2;
			}
		next2:
			if (minx > t1x) minx = t1x;
			if (minx > t2x) minx = t2x;
			if (maxx < t1x) maxx = t1x;
			if (maxx < t2x) maxx = t2x;
			drawline(minx, maxx, y);    // Draw line from min to max points found on the y
										// Now increase y
			if (!changed1) t1x += signx1;
			t1x += t1xp;
			if (!changed2) t2x += signx2;
			t2x += t2xp;
			y += 1;
			if (y == p2y) break;
		}
	next:
		// Second half
		dx1 = (int)(p3x - p2x); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
		else signx1 = 1;
		dy1 = (int)(p3y - p2y);
		t1x = p2x;

		if (dy1 > dx1) {   // swap values
			std::swap(dy1, dx1);
			changed1 = true;
		}
		else changed1 = false;

		e1 = (int)(dx1 >> 1);

		for (int i = 0; i <= dx1; i++) {
			t1xp = 0; t2xp = 0;
			if (t1x < t2x) { minx = t1x; maxx = t2x; }
			else { minx = t2x; maxx = t1x; }
			// process first line until y value is about to change
			while (i < dx1) {
				e1 += dy1;
				while (e1 >= dx1) {
					e1 -= dx1;
					if (changed1) { t1xp = signx1; break; }//t1x += signx1;
					else          goto next3;
				}
				if (changed1) break;
				else   	   	  t1x += signx1;
				if (i < dx1) i++;
			}
		next3:
			// process second line until y value is about to change
			while (t2x != p3x) {
				e2 += dy2;
				while (e2 >= dx2) {
					e2 -= dx2;
					if (changed2) t2xp = signx2;
					else          goto next4;
				}
				if (changed2)     break;
				else              t2x += signx2;
			}
		next4:

			if (minx > t1x) minx = t1x;
			if (minx > t2x) minx = t2x;
			if (maxx < t1x) maxx = t1x;
			if (maxx < t2x) maxx = t2x;
			drawline(minx, maxx, y);
			if (!changed1) t1x += signx1;
			t1x += t1xp;
			if (!changed2) t2x += signx2;
			t2x += t2xp;
			y += 1;
			if (y > p3y) return;
		}
	}
	void DrawFilledCircle(int xc, int yc, int r, u32 color)
	{
		// Taken from wikipedia
		int x = 0;
		int y = r;
		int p = 3 - 2 * r;
		if (!r) return;

		auto drawline = [&](int sx, int ex, int ny)
		{
			for (int i = sx; i <= ex; i++)
				DrawPixel(i, ny, color);
		};

		while (y >= x)
		{
			// Modified to draw scan-lines instead of edges by OLC
			drawline(xc - x, xc + x, yc - y);
			drawline(xc - y, xc + y, yc - x);
			drawline(xc - x, xc + x, yc + y);
			drawline(xc - y, xc + y, yc + x);
			if (p < 0) p += 4 * x++ + 6;
			else p += 4 * (x++ - y--) + 10;
		}
	}
	union RGBColor
	{
		u32 Color;
		byte bytes[3];
	};
}