#ifndef ESX_SHELL_EXT_REGISTER_H_
#define ESX_SHELL_EXT_REGISTER_H_
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // !WIN32_LEAN_AND_MEAN
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif  // !_WINSOCKAPI_
#include <windows.h>

namespace esx {
typedef bool(__stdcall* PFN_REGISTER)();
typedef bool(__stdcall* PFN_UNREGISTER)();
typedef bool(__stdcall* PFN_IS_REGISTER)();
typedef bool(__stdcall* PFN_IS_ELEVATED)();

inline bool Register(const wchar_t* dllPath) {
    bool result = false;
    HMODULE hDll = NULL;

    do {
        if (!dllPath || !dllPath[0]) {
            break;
        }

        hDll = LoadLibraryW(dllPath);
        if (!hDll) {
            break;
        }

        PFN_IS_ELEVATED pfnIsElevated = (PFN_IS_ELEVATED)GetProcAddress(hDll, "IsElevated");
        if (!pfnIsElevated) {
            break;
        }

        if (!pfnIsElevated()) {
            break;
        }

        PFN_REGISTER pfnRegister = (PFN_REGISTER)GetProcAddress(hDll, "RegisterShellExt");
        if (!pfnRegister) {
            break;
        }

        result = pfnRegister();
    } while (false);

    if (hDll) {
        FreeLibrary(hDll);
    }
    return result;
}

inline bool Unregister(const wchar_t* dllPath) {
    bool result = false;
    HMODULE hDll = NULL;

    do {
        if (!dllPath || !dllPath[0]) {
            break;
        }

        hDll = LoadLibraryW(dllPath);
        if (!hDll) {
            break;
        }

        PFN_IS_ELEVATED pfnIsElevated = (PFN_IS_ELEVATED)GetProcAddress(hDll, "IsElevated");
        if (!pfnIsElevated) {
            break;
        }

        if (!pfnIsElevated()) {
            break;
        }

        PFN_UNREGISTER pfnUnregister = (PFN_UNREGISTER)GetProcAddress(hDll, "UnregisterShellExt");
        if (!pfnUnregister) {
            break;
        }

        result = pfnUnregister();
    } while (false);

    if (hDll) {
        FreeLibrary(hDll);
    }
    return result;
}

inline bool IsRegister(const wchar_t* dllPath) {
    bool result = false;
    HMODULE hDll = NULL;

    do {
        if (!dllPath || !dllPath[0]) {
            break;
        }

        hDll = LoadLibraryW(dllPath);
        if (!hDll) {
            break;
        }

        PFN_IS_REGISTER pfnIsRegister = (PFN_IS_REGISTER)GetProcAddress(hDll, "IsRegister");
        if (!pfnIsRegister) {
            break;
        }

        result = pfnIsRegister();
    } while (false);

    if (hDll) {
        FreeLibrary(hDll);
    }
    return result;
}
}  // namespace esx

#endif  // !ESX_SHELL_EXT_REGISTER_H_
