#include "pch.h"
#include "RegisterHelper.h"
#include <versionhelpers.h>
#include "WinRegistry.h"
#include "shellext_h.h"
#include "Utils.h"
#include "CustomImpl.h"

#define HKCR_CONTEXTMENUHANDLERS L"\\shellex\\ContextMenuHandlers\\"
#define HKLM_CURRENTVERSION L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion"
#define HKLM_APPROVED HKLM_CURRENTVERSION L"\\Shell Extensions\\Approved"

namespace esx {
bool RegisterHelper::IsRegistered() {
    return IsContextMenu();
}

bool RegisterHelper::IsHKCR(const std::wstring& key) {
    bool ret = false;
    WinRegistry reg(HKEY_CLASSES_ROOT, L"CLSID\\" + key);
    if (SUCCEEDED(reg.open(KEY_READ, false))) {
        ret = true;
        reg.close();
    }
    return ret;
}

bool RegisterHelper::IsContextMenu() {
    return IsHKCR(CLSIDToString(CLSID_EasyShellExt));
}

// register COM-object Add HKCR\CLSID\{<CLSID>} key.
bool RegisterHelper::RegisterInprocServer(const wchar_t* dllPath, const wchar_t* clsid, const wchar_t* value) {
    bool ret = false;
    const std::wstring subKey = std::wstring(L"CLSID\\") + clsid;

    do {
        WinRegistry reg(HKEY_CLASSES_ROOT, subKey);
        if (FAILED(reg.open(KEY_READ | KEY_WRITE, true))) {
            break;
        }

        if (FAILED(reg.setSZValue(nullptr, value))) {
            reg.close();
            break;
        }
        reg.close();

        WinRegistry regInproc(HKEY_CLASSES_ROOT, subKey + L"\\InprocServer32");
        if (FAILED(regInproc.open(KEY_READ | KEY_WRITE, true))) {
            break;
        }
        if (FAILED(regInproc.setSZValue(nullptr, dllPath))) {
            regInproc.close();
            break;
        }
        if (FAILED(regInproc.setSZValue(L"ThreadingModel", L"Apartment"))) {
            regInproc.close();
            break;
        }
        regInproc.close();
        ret = true;
    } while (false);

    if (!ret) {
        ::RegDeleteTreeW(HKEY_CLASSES_ROOT, subKey.c_str());
    }

    return ret;
}

// Require administrator's rights!
bool RegisterHelper::Register(const wchar_t* dllPath, const wchar_t* companyName, const wchar_t* shellExtName) {
    if (!dllPath || !dllPath[0])
        return false;

    if (!::IsWindows7OrGreater())
        return false;

    std::wstring clsid = CLSIDToString(CLSID_EasyShellExt);

    bool ret = false;
    std::wstring appCompanyName = std::wstring(companyName) + L"." + std::wstring(shellExtName);
    // register COM-object for shortcut menu handler
    if (RegisterInprocServer(dllPath, clsid.c_str(), appCompanyName.c_str())) {
        WinRegistry reg(HKEY_LOCAL_MACHINE, HKLM_APPROVED);
        if (SUCCEEDED(reg.open(KEY_READ | KEY_WRITE, true))) {
            if (SUCCEEDED(reg.setSZValue(clsid.c_str(), shellExtName))) {
                if (RegisterContextMenuHandler(true, shellExtName)) {
                    ret = true;
                }
            }
            reg.close();
        }
    }

    return ret;
}

// Require administrator's rights!
bool RegisterHelper::Unregister(const wchar_t* shellExtName) {
    bool ret = true;

    std::wstring clsid = CLSIDToString(CLSID_EasyShellExt);

    LSTATUS status = WinRegistry::DeleteKey(HKEY_LOCAL_MACHINE, HKLM_APPROVED, clsid.c_str());
    if (status != ERROR_SUCCESS && status != ERROR_FILE_NOT_FOUND) {
        return false;
    }

    status = ::RegDeleteTreeW(HKEY_CLASSES_ROOT, (L"CLSID\\" + clsid).c_str());
    if (status != ERROR_SUCCESS && status != ERROR_FILE_NOT_FOUND) {
        return false;
    }

    if (!RegisterContextMenuHandler(false, shellExtName))
        ret = false;

    return ret;
}

// NOTE: The function add or removes the {{<CLSID>}} key under
// HKCU\Software\Classes\<type>\shellex\ContextMenuHandlers in the registry.
bool RegisterHelper::RegisterContextMenuHandler(const wchar_t* fileType, const wchar_t* shellExtName, bool bRegister) {
    if (!fileType || !fileType[0])
        return false;

    std::wstring defaultValue;
    std::wstring strFileType = fileType;
    if (*fileType == L'.') {
        WinRegistry reg(HKEY_CLASSES_ROOT, fileType);
        if (SUCCEEDED(reg.open(KEY_READ, false))) {
            if (SUCCEEDED(reg.getSZValue(nullptr, defaultValue))) {
                strFileType = defaultValue;
            }
            reg.close();
        }
    }

    std::wstring clsid = CLSIDToString(CLSID_EasyShellExt);
    std::wstring subKey = strFileType;
    subKey += HKCR_CONTEXTMENUHANDLERS;
    subKey += shellExtName;

    bool ret = false;
    if (bRegister) {
        // key HKCR\<Types>\shellex\ContextMenuHandlers\{{<CLSID>}}
        WinRegistry reg(HKEY_CLASSES_ROOT, subKey);
        if (SUCCEEDED(reg.open(KEY_READ | KEY_WRITE, true))) {
            if (SUCCEEDED(reg.setSZValue(nullptr, clsid.c_str()))) {
                ret = true;
            }
            reg.close();
        }
    }
    else {
        LSTATUS status = ::RegDeleteTreeW(HKEY_CLASSES_ROOT, subKey.c_str());
        if (status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND) {
            ret = true;
        }
    }

    return ret;
}

// Update Registry
// true	 = Register the context menu handler.
// false = Unregister the context menu handler.
bool RegisterHelper::RegisterContextMenuHandler(bool isRegister, const wchar_t* shellExtName) {
    int res = 0;
    // The context menu handler is associated with the classes.
    res += RegisterContextMenuHandler(L"*", shellExtName, isRegister);
    res += RegisterContextMenuHandler(L"Directory", shellExtName, isRegister);
    res += RegisterContextMenuHandler(L"Drive", shellExtName, isRegister);
    res += RegisterContextMenuHandler(L"Folder", shellExtName, isRegister);
    res += RegisterContextMenuHandler(L"Directory\\Background", shellExtName, isRegister);
    res += RegisterContextMenuHandler(L"DesktopBackground", shellExtName, isRegister);
    res += RegisterContextMenuHandler(L"LibraryFolder", shellExtName, isRegister);
    res += RegisterContextMenuHandler(L"LibraryFolder\\Background", shellExtName, isRegister);
    return res > 0;
}
}  // namespace esx