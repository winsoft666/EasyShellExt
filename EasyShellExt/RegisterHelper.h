#pragma once
#include <string>

namespace esx {
class RegisterHelper {
   public:
    static bool IsRegistered();

    static bool IsHKCR(const std::wstring& key);

    static bool IsContextMenu();

    // register COM-object Add HKCR\CLSID\{<CLSID>} key.
    static bool RegisterInprocServer(const wchar_t* dllPath, const wchar_t* clsid, const wchar_t* value);

    // Require administrator's rights!
    static bool Register(const wchar_t* dllPath, const wchar_t* companyName, const wchar_t* shellExtName);

    // Require administrator's rights!
    static bool Unregister( const wchar_t* shellExtName);

    // NOTE: The function add or removes the {{<CLSID>}} key under
    // HKCU\Software\Classes\<type>\shellex\ContextMenuHandlers in the registry.
    static bool RegisterContextMenuHandler(const wchar_t* fileType, const wchar_t* shellExtName, bool bRegister);

    // Update Registry
    // true	 = Register the context menu handler.
    // false = Unregister the context menu handler.
    static bool RegisterContextMenuHandler(bool isRegister, const wchar_t* shellExtName);
};
}  // namespace esx
