#pragma once
#include <windows.h>
#include "Overlay.h"

INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmdShow) {
	Overlay::init(instance, cmdShow, "WindowName", yourRenderfunction);
	Overlay::run();
	Overlay::release();
}
