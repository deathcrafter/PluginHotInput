#pragma once

#include <Windows.h>
#include <stdlib.h>
#include "../API/RainmeterAPI.h"
#include <string>
#include <regex>

#define CONTROL_DOWN (GetKeyState(VK_CONTROL) & 0x8000)
#define SHIFT_DOWN (GetKeyState(VK_SHIFT) & 0x8000)

struct Measure
{
	void* rm;
	void* skin;
	HWND skinWnd;

	bool useLuaEsc;
	std::wstring inputAction;
	std::wstring navigateAction;

	Measure(void* p_rm) :
		rm(p_rm),
		skin(RmGetSkin(p_rm)),
		skinWnd(RmGetSkinWindow(p_rm)),
		useLuaEsc(false),
		inputAction(),
		navigateAction()
	{}
};

// function handler for WH_GETMESSAGE hook
LRESULT CALLBACK GetMessageProc(int, WPARAM, LPARAM);

void SetMeasure(Measure*);
void RemoveMeasure(Measure*);

void Log(LPCWSTR, int);
void Execute(int, std::wstring, HWND);
std::wstring ReplaceBang(std::wstring, const std::wstring&, const std::wstring&);
std::wstring EscLuaChars(std::wstring);