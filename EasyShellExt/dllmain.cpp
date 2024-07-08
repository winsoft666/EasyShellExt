#include "pch.h"
#include "resource.h"
// Generated files
#include "shellext_h.h"
#include "shellext_i.c"
#include "Utils.h"
#include "LoggerHelper.h"
#include <ShlObj.h>
#include "RegisterHelper.h"
#include "CustomImpl.h"

HMODULE gCurrentModule = NULL;

class EasyShellExtModule : public ATL::CAtlDllModuleT<EasyShellExtModule> {
   public:
    DECLARE_LIBID(LIBID_EASYSHELLEXTENSIONLib)
    //DECLARE_REGISTRY_APPID_RESOURCEID(IDR_EASYSHELLEXT, "{EB26EA8E-3B98-4DED-AE59-255C3BA725C3}")
};

class EasyShellExtModule _AtlModule;

// Returns a class factory to create an object of the requested type.
STDAPI DllGetClassObject(_In_ REFCLSID clsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv) {
    std::string clsid_str = esx::GuidToInterfaceName(clsid);
    std::string riid_str = esx::GuidToInterfaceName(riid);

    HRESULT hr = _AtlModule.DllGetClassObject(clsid, riid, ppv);

    if (hr == CLASS_E_CLASSNOTAVAILABLE)
        ESX_LOG(ERROR) << __FUNCTION__ << "(), ClassFactory " << clsid_str << " not found!";
    else if (FAILED(hr))
        ESX_LOG(ERROR) << __FUNCTION__ << "(), unknown interface " << riid_str;
    else
        ESX_LOG(INFO) << __FUNCTION__ << "(), found interface " << riid_str << ", ppv=" << esx::ToHexString(*ppv);
    return hr;
}

// Used to determine whether the DLL can be unloaded by OLE.
STDAPI DllCanUnloadNow(void) {
    ESX_LOG(INFO) << __FUNCTION__ << "() " << esx::GetProcessContextDesc();

    HRESULT hr = _AtlModule.DllCanUnloadNow();

    if (hr == S_OK) {
        ESX_LOG(INFO) << __FUNCTION__ << "() -> Yes";
        return S_OK;
    }
    ESX_LOG(INFO) << __FUNCTION__ << "() -> No.";
    return S_FALSE;
}

// DllRegisterServer - Adds entries to the system registry.
STDAPI DllRegisterServer(void) {
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer();

    if (SUCCEEDED(hr)) {
        // Notify the Shell to pick the changes:
        // https://docs.microsoft.com/en-us/windows/desktop/shell/reg-shell-exts#predefined-shell-objects
        // Any time you create or change a Shell extension handler, it is important to notify the system that you have made a change.
        // Do so by calling SHChangeNotify, specifying the SHCNE_ASSOCCHANGED event.
        // If you do not call SHChangeNotify, the change might not be recognized until the system is rebooted.
        SHChangeNotify(SHCNE_ASSOCCHANGED, 0, 0, 0);
    }

    return hr;
}

// DllUnregisterServer - Removes entries from the system registry.
STDAPI DllUnregisterServer(void) {
    // unregisters object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllUnregisterServer();

    if (SUCCEEDED(hr)) {
        // Notify the Shell to pick the changes:
        // https://docs.microsoft.com/en-us/windows/desktop/shell/reg-shell-exts#predefined-shell-objects
        // Any time you create or change a Shell extension handler, it is important to notify the system that you have made a change.
        // Do so by calling SHChangeNotify, specifying the SHCNE_ASSOCCHANGED event.
        // If you do not call SHChangeNotify, the change might not be recognized until the system is rebooted.
        SHChangeNotify(SHCNE_ASSOCCHANGED, 0, 0, 0);
    }

    return hr;
}

STDAPI_(bool)
IsElevated() {
    struct ADMIN {
        BOOL isAdmin = FALSE;
        PSID pAdminSID = nullptr;  // AdministratorsGroup
        ~ADMIN() {
            if (pAdminSID)
                ::FreeSid(pAdminSID);
        }
    } admin;

    // Allocate and initialize a SID of the administrators group.
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (::AllocateAndInitializeSid(&NtAuthority, 2,
                                   SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                   &admin.pAdminSID)) {
        // Determine whether the SID of administrators group is enabled
        // in the primary access token of the process.
        if (admin.pAdminSID)
            ::CheckTokenMembership(nullptr, admin.pAdminSID, &admin.isAdmin);
    }
    return admin.isAdmin;
}

STDAPI_(bool)
IsRegister() {
    return esx::RegisterHelper::IsRegistered();
}

STDAPI_(bool)
RegisterShellExt() {
    WCHAR szDllPath[MAX_PATH] = {0};
    if (GetModuleFileNameW(gCurrentModule, szDllPath, MAX_PATH) == 0) {
        ESX_LOG(ERROR) << __FUNCTION__ << "(), unable to get dll path!";
        return false;
    }

    return esx::RegisterHelper::Register(szDllPath, COMPANY_NAME, SHELL_EXT_NAME);
}

STDAPI_(bool)
UnregisterShellExt() {
    return esx::RegisterHelper::Unregister(SHELL_EXT_NAME);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            gCurrentModule = hModule;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
