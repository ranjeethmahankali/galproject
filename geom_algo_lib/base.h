#pragma once

#define PINVOKE extern "C" __declspec(dllexport)

PINVOKE int Test_GetSquare(int n);