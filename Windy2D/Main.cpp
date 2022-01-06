#include"Windy2D.h"
using namespace Windy;
int WindyMain{
	WindyCreateWindow(hInstance, hPrevInstance, lpCmdLine, 1, 1080, 720, L"Windy2D Application", 0x4283b8, true);
	while (running)
	{

		WindyUpdate();
		if (Focused) {
			DrawFilledRect(0, 0, 256, 256, 0x000000);
			WindyDraw();
		}

	}
}
