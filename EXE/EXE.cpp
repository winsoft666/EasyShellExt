#include <iostream>
#include "../EasyShellExtRegister.hpp"
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

int main(int argc, char** argv) {
    WCHAR szDllPath[MAX_PATH] = {0};
    GetModuleFileNameW(NULL, szDllPath, MAX_PATH);
    PathRemoveFileSpecW(szDllPath);
    PathCombineW(szDllPath, szDllPath, L"EasyShellExt.dll");

    if (argc < 2)
        return 1;

    if (strcmp(argv[1], "r") == 0) {
        // register
        if (esx::Register(szDllPath)) {
            std::wcout << L"Register " << szDllPath << L" Success.";
        }
        else {
            std::wcout << L"Register " << szDllPath << L" Failed.";
        }
    }
    else if (strcmp(argv[1], "u") == 0) {
        // unregister
        if (esx::Unregister(szDllPath)) {
            std::wcout << L"Unregister " << szDllPath << L" Success.";
        }
        else {
            std::wcout << L"Unregister " << szDllPath << L" Failed.";
        }
    }
    else if (strcmp(argv[1], "q") == 0) {
        if (esx::IsRegister(szDllPath)) {
            std::wcout << szDllPath << L" has registered.";
        }
        else {
            std::wcout << szDllPath << L" not be registered.";
        }
    }

    return 0;
}
