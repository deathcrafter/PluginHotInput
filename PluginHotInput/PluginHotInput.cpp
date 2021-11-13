/*
Copyright (C) 2021 Shaktijeet Sahoo


This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include "PluginHotInput.h"



// global variables
HINSTANCE g_instance = NULL;
HHOOK g_hook = nullptr;
bool g_hookActive = false;
Measure* g_measure = nullptr;

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD ul_reason_for_call, LPWORD lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		g_instance = hinst;

		// Disable DLL_THREAD_ATTACH and DLL_THREAD_DETACH notification calls : saves resources
		DisableThreadLibraryCalls((HMODULE)hinst);
		break;
	}

	return TRUE;
}

PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
	Measure* measure = new Measure(rm);
	*data = measure;
}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue)
{
	Measure* measure = (Measure*)data;

	measure->useLuaEsc = RmReadInt(rm, L"UseLuaEscape", 0) == 1;
	measure->inputAction = RmReadString(rm, L"InputAction", NULL);
	measure->navigateAction = RmReadString(rm, L"NavigateAction", NULL);
}

PLUGIN_EXPORT double Update(void* data)
{
	Measure* measure = (Measure*)data;
	return 0.0;
}

PLUGIN_EXPORT void Finalize(void* data)
{
	Measure* measure = (Measure*)data;
	RemoveMeasure(measure);
	delete measure;
}

PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args)
{
	Measure* measure = (Measure*)data;
	
	std::wstring arg(args);

	if (arg.empty()) return;

	if (_wcsicmp(arg.c_str(), L"Start") == 0) {
		SetMeasure(measure);
	}
	else if (_wcsicmp(arg.c_str(), L"Stop") == 0) {
		RemoveMeasure(measure);
	}
}

LPCWSTR getClipboard(HWND hwnd)
{
	LPCWSTR strData{};

	if (OpenClipboard(hwnd))
	{
			HANDLE hClipboardData = GetClipboardData(CF_UNICODETEXT);
			if (hClipboardData)
			{
				WCHAR* pchData = (WCHAR*)GlobalLock(hClipboardData);
				if (pchData)
				{
					strData = pchData;
					GlobalUnlock(hClipboardData);
				}
			}
		CloseClipboard();
	}
	return strData;
}

PLUGIN_EXPORT LPCWSTR GetClipboard(void* data, const int argc, const WCHAR* argv[])
{
	Measure* measure = (Measure*)data;

	if (argc >= 0)
	{
		return getClipboard(measure->skinWnd);
	}

	return L"";
}

PLUGIN_EXPORT LPCWSTR IsCtrlDown(void* data, const int argc, const WCHAR* argv[])
{
	Measure* measure = (Measure*)data;

	if (CONTROL_DOWN)
		return L"1";
	return L"0";
}

PLUGIN_EXPORT LPCWSTR IsShiftDown(void* data, const int argc, const WCHAR* argv[])
{
	Measure* measure = (Measure*)data;

	if (SHIFT_DOWN)
		return L"1";
	return L"0";
}

// ----------------------------------------------------------------------------------------------------------------

// =========================================================
// Hook functions
// =========================================================

void SetMeasure(Measure* measure) {
	g_measure = measure;

	if (g_measure != nullptr && !g_hookActive) {
		if (!g_hook) {
			g_hook = SetWindowsHookEx(WH_GETMESSAGE, GetMessageProc, g_instance, GetWindowThreadProcessId(measure->skinWnd, NULL));

			if (g_hook) {
				RmLog(measure->rm, LOG_DEBUG, L"Successfully started message hook!");
				g_hookActive = true;
			}
			else {
				RmLog(measure->rm, LOG_ERROR, L"Unable to start message hook!");
			}
		}
	}
};

void RemoveMeasure(Measure* measure) {
	g_measure = nullptr;

	if (g_measure == nullptr && g_hookActive) {
		while (g_hook && UnhookWindowsHookEx(g_hook) == FALSE) {
			RmLog(measure->rm, LOG_ERROR, L"Can't unhook message hook!");
		}
		RmLog(measure->rm, LOG_DEBUG, L"Unhooked message hook successfully!");
		g_hook = nullptr;
		g_hookActive = false;
	}
}

LRESULT CALLBACK GetMessageProc(int nCode, WPARAM wParam, LPARAM lParam) {

	if (nCode >= 0) {
		MSG* msg = (MSG*)lParam;
		if (msg->hwnd != g_measure->skinWnd)
			return CallNextHookEx(g_hook, nCode, wParam, lParam);

		switch (msg->message) {
		case WM_CHAR:
			if (!CONTROL_DOWN) {

				switch (msg->wParam)
				{
					// First, handle non-displayable characters.
				case VK_BACK:  // backspace.
					Execute(0, L"BACKSPACE", msg->hwnd);
					break;
				case VK_TAB:  // tab.
					Execute(0, L"TAB", msg->hwnd);
					break;
				case 0x0A:  // linefeed : not working apparently
					Execute(0, L"LINEFEED", msg->hwnd);
					break;
				case VK_RETURN:  // carriage return.
					if (GetAsyncKeyState(VK_SHIFT)) {
						Execute(0, L"LINEFEED", msg->hwnd);
					}
					else {
						Execute(0, L"RETURN", msg->hwnd);
					}
					break;
				case VK_ESCAPE:  // escape.
					Execute(0, L"ESC", msg->hwnd);
					break;
				case VK_SPACE:  // space.
					Execute(0, L" ", msg->hwnd);
					break;
					// Next, handle displayable characters by appending them to our string.
				default:
					std::wstring chr(1, (wchar_t)msg->wParam);
					if (g_measure->useLuaEsc) {
						chr = EscLuaChars(chr);
					}
					Execute(0, chr, msg->hwnd);
					break;
				}
			}
		case WM_KEYDOWN:
			switch (msg->wParam) {
			case VK_LEFT:
				if (CONTROL_DOWN) {
					Execute(1, L"CTRLLEFT", msg->hwnd);
				}
				else if (SHIFT_DOWN) {
					Execute(1, L"SHIFTLEFT", msg->hwnd);
				}
				else {
					Execute(1, L"LEFT", msg->hwnd);
				}
				break;
			case VK_RIGHT:
				if (CONTROL_DOWN) {
					Execute(1, L"CTRLRIGHT", msg->hwnd);
				}
				else if (SHIFT_DOWN) {
					Execute(1, L"SHIFTRIGHT", msg->hwnd);
				}
				else {
					Execute(1, L"RIGHT", msg->hwnd);
				}
				break;
			case VK_UP:
				if (CONTROL_DOWN) {
					Execute(1, L"CTRLUP", msg->hwnd);
				}
				else if (SHIFT_DOWN) {
					Execute(1, L"SHIFTUP", msg->hwnd);
				}
				else {
					Execute(1, L"UP", msg->hwnd);
				}
				break;
			case VK_DOWN:
				if (CONTROL_DOWN) {
					Execute(1, L"CTRLDOWN", msg->hwnd);
				}
				else if (SHIFT_DOWN) {
					Execute(1, L"SHIFTDOWN", msg->hwnd);
				}
				else {
					Execute(1, L"DOWN", msg->hwnd);
				}
				break;
			case 0x43:
				if (CONTROL_DOWN) {
					Execute(0, L"COPY", msg->hwnd);
				}
				break;
			case 0x56:
				if (CONTROL_DOWN) {
					Execute(0, L"PASTE", msg->hwnd);
				}
				break;
			case 0x58:
				if (CONTROL_DOWN) {
					Execute(0, L"CUT", msg->hwnd);
				}
				break;
			case 0x59:
				if (CONTROL_DOWN) {
					Execute(0, L"REDO", msg->hwnd);
				}
				break;
			case 0x5A:
				if (CONTROL_DOWN) {
					Execute(0, L"UNDO", msg->hwnd);
				}
				break;
			}
		}
	}

	// pass this message to target application
	return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

// --------------------------------------------------------------------------------------------------------------

// =========================================================
// Helper functions
// =========================================================

void Execute(int bang, std::wstring replaceStr, HWND skinWnd) {
	if (g_measure != nullptr) {
		if (bang == 0) {
			if (g_measure->skinWnd == skinWnd && !g_measure->inputAction.empty()) {
				std::wstring bng = ReplaceBang(g_measure->inputAction, L"\\$input\\$", replaceStr);
				RmExecute(g_measure->skin, bng.c_str());
			}
		}
		else {
			if (g_measure->skinWnd == skinWnd && !g_measure->navigateAction.empty()) {
				std::wstring bng = ReplaceBang(g_measure->navigateAction, L"\\$navigation\\$", replaceStr);
				RmExecute(g_measure->skin, bng.c_str());
			}
		}
	}
}

std::wstring EscLuaChars(std::wstring str) {
	if (str == L"\\") {
		str = L"\\\\";
	}
	else if (str == L"'") {
		str = L"\\'";
	}
	else if (str == L"\"") {
		str = L"\\\"";
	}
	else if (str == L"[") {
		str = L"\\[";
	}
	else if (str == L"]") {
		str = L"\\]";
	}

	return str;
}

std::wstring ReplaceBang(std::wstring str, const std::wstring& from, const std::wstring& to) {
	if (str.empty() || from.empty())
		return str;
	std::wstring buff = std::regex_replace(str, std::wregex(from, std::regex::icase), to);
	return buff;
}

// --------------------------------------------------------------------------------------------------------------