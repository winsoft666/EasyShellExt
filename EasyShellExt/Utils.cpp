#include "pch.h"
#include "Utils.h"
#include <ShlObj.h>  // for IID_IShellExtInit, IID_IContextMenu
#include <strsafe.h>
#include "StringEncode.h"
// Generated files
#include "shellext_h.h"
#include "shellext_i.c"

namespace esx {
std::string GuidToString(GUID guid) {
    char buffer[64];
    sprintf_s(buffer, sizeof(buffer), "{%08X-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X}", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    return buffer;
}

std::string GuidToInterfaceName(GUID guid) {
    if (IsEqualGUID(guid, IID_IUnknown))
        return "IUnknown";  //{00000000-0000-0000-C000-000000000046}
    if (IsEqualGUID(guid, IID_IClassFactory))
        return "IClassFactory";  //{00000001-0000-0000-C000-000000000046}
    if (IsEqualGUID(guid, IID_IShellExtInit))
        return "IShellExtInit";  //{000214E8-0000-0000-C000-000000000046}
    if (IsEqualGUID(guid, IID_IContextMenu))
        return "IContextMenu";  //{000214E4-0000-0000-C000-000000000046}
    if (IsEqualGUID(guid, IID_IContextMenu2))
        return "IContextMenu2";  //{000214F4-0000-0000-C000-000000000046}
    if (IsEqualGUID(guid, IID_IContextMenu3))
        return "IContextMenu3";  //{BCFCE0A0-EC17-11D0-8D10-00A0C90F2719}
    if (IsEqualGUID(guid, IID_IObjectWithSite))
        return "IObjectWithSite";  //{FC4801A3-2BA9-11CF-A229-00AA003D7352}
    if (IsEqualGUID(guid, IID_IInternetSecurityManager))
        return "IInternetSecurityManager";  //{79EAC9EE-BAF9-11CE-8C82-00AA004BA90B}

    if (IsEqualGUID(guid, CLSID_EasyShellExt))
        return "EasyShellExt";

    //unknown GUID, return the string representation:
    //ie: CLSID_UNDOCUMENTED_01, {924502A7-CC8E-4F60-AE1F-F70C0A2B7A7C}
    return GuidToString(guid);
}

bool StringPrintfV(const char* format, va_list argList, std::string& output) noexcept {
    if (!format)
        return false;

    bool ret = false;
    char* pMsgBuffer = NULL;
    size_t iMsgBufCount = 0;

    HRESULT hr = STRSAFE_E_INSUFFICIENT_BUFFER;
    try {
        while (hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
            iMsgBufCount += 1024;
            if (pMsgBuffer) {
                free(pMsgBuffer);
                pMsgBuffer = NULL;
            }

            pMsgBuffer = (char*)malloc(iMsgBufCount * sizeof(char));
            if (!pMsgBuffer) {
                break;
            }
            hr = StringCchVPrintfA(pMsgBuffer, iMsgBufCount, format, argList);
        }

        if (hr == S_OK && pMsgBuffer) {
            output.assign(pMsgBuffer);
        }

        if (pMsgBuffer) {
            free(pMsgBuffer);
            pMsgBuffer = NULL;
        }

        ret = (hr == S_OK);
    } catch (std::exception& e) {
        (e);
        ret = false;
    }

    return ret;
}

std::string StringPrintf(const char* format, ...) noexcept {
    std::string output;
    try {
        va_list args;
        va_start(args, format);

        StringPrintfV(format, args, output);
        va_end(args);
    } catch (std::exception& e) {
        (e);
        output.clear();
    }

    return output;
}

std::string ToHexString(void* value) {
    size_t address = reinterpret_cast<size_t>(value);
    char buffer[200] = {0};
    static bool is_32bit = (sizeof(address) == 4);
    static bool is_64bit = (sizeof(address) == 8);

    if (is_32bit)
        StringCchPrintfA(buffer, 200, "0x%Ix", address);
    else if (is_64bit)
        StringCchPrintfA(buffer, 200, "0x%Ix", address);

    return buffer;
}

template <class T>
class FlagDescriptor {
   public:
    struct FLAGS {
        T value;
        const char* name;
    };

    static std::string ToBitString(T value, const FLAGS* flags) {
        std::string desc;

        size_t index = 0;
        while (flags[index].name) {
            const T& flag = flags[index].value;
            const char* name = flags[index].name;

            //if flag is set
            if ((value & flag) == flag) {
                if (!desc.empty())
                    desc.append("|");
                desc.append(name);
            }

            //next flag
            index++;
        }
        return desc;
    }

    static std::string ToValueString(T value, const FLAGS* flags) {
        std::string desc;

        size_t index = 0;
        while (flags[index].name) {
            const T& flag = flags[index].value;
            const char* name = flags[index].name;

            //if flag is set
            if (value == flag) {
                if (!desc.empty())
                    desc.append(",");
                desc.append(name);
            }

            //next flag
            index++;
        }
        return desc;
    }
};

std::string GetQueryContextMenuFlags(UINT flags) {
    static const FlagDescriptor<UINT>::FLAGS descriptors[] = {
        {CMF_NORMAL, "CMF_NORMAL"},
        {CMF_DEFAULTONLY, "CMF_DEFAULTONLY"},
        {CMF_VERBSONLY, "CMF_VERBSONLY"},
        {CMF_EXPLORE, "CMF_EXPLORE"},
        {CMF_NOVERBS, "CMF_NOVERBS"},
        {CMF_CANRENAME, "CMF_CANRENAME"},
        {CMF_NODEFAULT, "CMF_NODEFAULT"},
        {CMF_ITEMMENU, "CMF_ITEMMENU"},
        {CMF_EXTENDEDVERBS, "CMF_EXTENDEDVERBS"},
        {CMF_DISABLEDVERBS, "CMF_DISABLEDVERBS"},
        {CMF_ASYNCVERBSTATE, "CMF_ASYNCVERBSTATE"},
        {CMF_OPTIMIZEFORINVOKE, "CMF_OPTIMIZEFORINVOKE"},
        {CMF_SYNCCASCADEMENU, "CMF_SYNCCASCADEMENU"},
        {CMF_DONOTPICKDEFAULT, "CMF_DONOTPICKDEFAULT"},
        {NULL, NULL},
    };
    std::string flags_str = FlagDescriptor<UINT>::ToBitString(flags, descriptors);
    return flags_str;
}

std::string GetGetCommandStringFlags(UINT flags) {
    //build function descriptor
    static const FlagDescriptor<UINT>::FLAGS descriptors[] = {
        {GCS_VERBA, "GCS_VERBA"},
        {GCS_HELPTEXTA, "GCS_HELPTEXTA"},
        {GCS_VALIDATEA, "GCS_VALIDATEA"},
        {GCS_VERBW, "GCS_VERBW"},
        {GCS_HELPTEXTW, "GCS_HELPTEXTW"},
        {GCS_VALIDATEW, "GCS_VALIDATEW"},
        {GCS_VERBICONW, "GCS_VERBICONW"},
        {GCS_UNICODE, "GCS_UNICODE"},
        {NULL, NULL},
    };
    std::string flags_str = FlagDescriptor<UINT>::ToBitString(flags, descriptors);
    return flags_str;
}

std::string GetMenuItemDetails(HMENU hMenu, UINT pos) {
    MENUITEMINFOW info = {0};
    info.cbSize = sizeof(MENUITEMINFOW);
    info.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_ID | MIIM_STRING | MIIM_SUBMENU;
    BOOL wInfoSuccess = GetMenuItemInfoW(hMenu, pos, TRUE, &info);
    if (!wInfoSuccess)
        return "";

    const UINT& id = info.wID;  //GetMenuItemID(hMenu, pos);

    bool isSeparator = ((info.fType & MFT_SEPARATOR) != 0);
    bool isDisabled = ((info.fState & MFS_DISABLED) != 0);
    bool isChecked = ((info.fState & MFS_CHECKED) != 0);

    //compute display name
    static const int BUFFER_SIZE = 1024;
    char titleA[BUFFER_SIZE] = {0};
    char tmp[BUFFER_SIZE] = {0};
    wchar_t tmpW[BUFFER_SIZE] = {0};
    if (isSeparator) {
        StringCchCopyA(titleA, BUFFER_SIZE, "------------------------");
    }
    //try with ansi text
    else if (GetMenuStringW(hMenu, id, tmpW, BUFFER_SIZE, 0)) {
        //Can't log unicode characters, convert to ansi.
        std::string atext = StringEncode::UnicodeToAnsi(tmpW);
        StringCchPrintfA(titleA, BUFFER_SIZE, "%s", atext.c_str());
    }

    //build full menu description string
    std::string descriptionA;
    descriptionA.append(titleA);
    descriptionA.append(" (");
    StringCchPrintfA(tmp, BUFFER_SIZE, "pos=%lu, id=%lu", pos, id);
    descriptionA.append(tmp);
    if (isChecked)
        descriptionA.append(", checked");
    if (isDisabled && !isSeparator)
        descriptionA.append(", disabled");
    descriptionA.append(")");

    return descriptionA;
}

std::string GetMenuTree(HMENU hMenu, int indent) {
    std::string output;

    int numItems = GetMenuItemCount(hMenu);

    for (int i = 0; i < numItems; i++) {
        std::string details = GetMenuItemDetails(hMenu, i);

        //Detect if this menu is a parent menu
        HMENU hSubMenu = NULL;
        {
            MENUITEMINFOW info = {0};
            info.cbSize = sizeof(MENUITEMINFOW);
            info.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_ID | MIIM_STRING | MIIM_SUBMENU;
            BOOL wInfoSuccess = GetMenuItemInfoW(hMenu, i, TRUE, &info);
            if (wInfoSuccess) {
                if (info.hSubMenu) {
                    hSubMenu = info.hSubMenu;
                }
            }
        }

        if (hSubMenu == NULL) {
            //no child
            output += std::string(indent, ' ');
            output += details + "\n";
        }
        else {
            //output this menu and open a bracket
            output += std::string(indent, ' ');
            output += details + " {\n";

            //query for submenu description.
            std::string sub_desc = GetMenuTree(hSubMenu, indent + 4);
            output += sub_desc;

            //close the opened bracket
            output += std::string(indent, ' ');
            output += "}\n";
        }
    }

    return output;
}

std::string GetProcessContextDesc() {
    std::string desc;
    desc.reserve(32);
    desc += "pid=";
    desc += std::to_string(GetCurrentProcessId());
    desc += " tid=";
    desc += std::to_string((uint32_t)GetCurrentThreadId());
    return desc;
}

std::wstring CLSIDToString(CLSID c) {
    wchar_t szCLSID[MAX_PATH];
    StringFromGUID2(c, szCLSID, ARRAYSIZE(szCLSID));
    return szCLSID;
}

}  // namespace esx