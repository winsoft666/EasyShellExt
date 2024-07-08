#include "pch.h"
#include "ContextMenu.h"
#include <shellapi.h>
#include "StringEncode.h"
#include "Utils.h"
#include "LoggerHelper.h"
#include "CustomImpl.h"

OBJECT_ENTRY_AUTO(CLSID_EasyShellExt, ContextMenu)

ContextMenu::ContextMenu() {
    ESX_LOG(INFO) << __FUNCTION__ << "(), new instance " << esx::ToHexString(this);

#if SA_QUERYINTERFACE_IMPL == 0
    // reference counter must be initialized to 0 even if we are actually creating an instance. 
    // A reference to this instance will be added when the instance will be queried by explorer.exe.
    refCount_ = 0;
#elif SA_QUERYINTERFACE_IMPL == 1
    refCount_ = 1;
#endif

    isBackGround_ = false;
    previousMenu_ = 0;
}

ContextMenu::~ContextMenu() {
}

HRESULT STDMETHODCALLTYPE ContextMenu::QueryContextMenu(HMENU hMenu, UINT menuIndex, UINT idCmdFirst, UINT idCmdLast, UINT uFlags) {
    std::string flags_str = esx::GetQueryContextMenuFlags(uFlags);
    std::string flags_hex = esx::StringPrintf("0x%08x", uFlags);

    ESX_LOG(INFO) << __FUNCTION__ << "(), hMenu=" << esx::StringPrintf("0x%08x", hMenu).c_str()
                  << ",count=" << GetMenuItemCount(hMenu)
                  << ", menuIndex=" << menuIndex
                  << ", idCmdFirst=" << idCmdFirst
                  << ", idCmdLast=" << idCmdLast
                  << ", flags=" << flags_hex << "=(" << flags_str << ")"
                  << " this=" << esx::ToHexString(this);

    //https://docs.microsoft.com/en-us/windows/desktop/shell/how-to-implement-the-icontextmenu-interface

    //From this point, it is safe to use class members without other threads interference
    CriticalSectionGuard cs_guard(&cs_);

    //Note on flags...
    //Right-click on a file or directory with Windows Explorer on the right area:  flags=0x00020494=132244(dec)=(CMF_NORMAL|CMF_EXPLORE|CMF_CANRENAME|CMF_ITEMMENU|CMF_ASYNCVERBSTATE)
    //Right-click on the empty area      with Windows Explorer on the right area:  flags=0x00020424=132132(dec)=(CMF_NORMAL|CMF_EXPLORE|CMF_NODEFAULT|CMF_ASYNCVERBSTATE)
    //Right-click on a directory         with Windows Explorer on the left area:   flags=0x00000414=001044(dec)=(CMF_NORMAL|CMF_EXPLORE|CMF_CANRENAME|CMF_ASYNCVERBSTATE)
    //Right-click on a drive             with Windows Explorer on the left area:   flags=0x00000414=001044(dec)=(CMF_NORMAL|CMF_EXPLORE|CMF_CANRENAME|CMF_ASYNCVERBSTATE)
    //Right-click on the empty area      on the Desktop:                           flags=0x00020420=132128(dec)=(CMF_NORMAL|CMF_NODEFAULT|CMF_ASYNCVERBSTATE)
    //Right-click on a directory         on the Desktop:                           flags=0x00020490=132240(dec)=(CMF_NORMAL|CMF_CANRENAME|CMF_ITEMMENU|CMF_ASYNCVERBSTATE)

    //Filter out queries that have nothing selected.
    //This can happend if user is copy & pasting files (using CTRL+C and CTRL+V)
    //and if the shell extension is registered as a DragDropHandlers.
    if (selectionCtx_.GetElements().size() == 0) {
        //Don't know what to do with this
        ESX_LOG(INFO) << __FUNCTION__ << "(), skipped, nothing is selected.";
        return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);  //nothing inserted
    }

    // Filter out queries that are called twice for the same directory.
    if (hMenu == previousMenu_) {
        //Issue #6 - Right-click on a directory with Windows Explorer in the left panel shows the menus twice.
        //Issue #31 - Error in logs for CContextMenu::GetCommandString().
        //Using a static variable is a poor method for solving the issue but it is a "good enough" strategy.
        ESX_LOG(INFO) << __FUNCTION__ << "(), skipped, QueryContextMenu() called twice and menu is already populated once.";
        return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);  //nothing inserted
    }

    //Remember current menu to prevent issues calling twice QueryContextMenu()
    previousMenu_ = hMenu;

    //Log what is selected by the user
    const esx::StringList& elements = selectionCtx_.GetElements();
    size_t numSelectedTotal = elements.size();
    int numFiles = selectionCtx_.GetNumFiles();
    int numDirectories = selectionCtx_.GetNumDirectories();
    ESX_LOG(INFO) << __FUNCTION__ << "(), SelectionContext have " << numSelectedTotal << " element(s): " << numFiles << " files and " << numDirectories << " directories.";

    USHORT menuCount = 0;
    HRESULT hr = esx::CreateMenus(hMenu, idCmdFirst, menuIndex, menuCount);
    if (FAILED(hr)) {
        ESX_LOG(ERROR) << __FUNCTION__ << "(), Create menus failed.";
        return hr;
    }
    assert(menuCount > 0);

    //debug the constructed menu tree
#if (defined _DEBUG) || (defined DEBUG)
    std::string menuTree = esx::GetMenuTree(hMenu, 2);
    ESX_LOG(INFO) << __FUNCTION__ << "(), Menu tree:\n" << menuTree.c_str();
#endif

    hr = MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, menuCount);
    return hr;
}

HRESULT STDMETHODCALLTYPE ContextMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici) {
    //define the type of structure pointed by pici
    const char* structName = "UNKNOWN";
    if (pici->cbSize == sizeof(CMINVOKECOMMANDINFO))
        structName = "CMINVOKECOMMANDINFO";
    else if (pici->cbSize == sizeof(CMINVOKECOMMANDINFOEX))
        structName = "CMINVOKECOMMANDINFOEX";

    //define how we should interpret pici->lpVerb
    std::string verb;
    if (IS_INTRESOURCE(pici->lpVerb)) {
#pragma warning(push)
#pragma warning(disable : 4302)
#pragma warning(disable : 4311)
        verb = std::to_string(reinterpret_cast<int>(pici->lpVerb));
#pragma warning(pop)
    }
    else {
        verb = pici->lpVerb;
    }

    ESX_LOG(INFO) << __FUNCTION__ << "(), pici->cbSize=" << structName
                  << ", pici->fMask=" << pici->fMask
                  << ", pici->lpVerb=" << verb << " this=" << esx::ToHexString(this);

    //validate
    if (!IS_INTRESOURCE(pici->lpVerb))
        return E_INVALIDARG;  //don't know what to do with pici->lpVerb

    UINT targetCommandOffset = LOWORD(pici->lpVerb);  // matches the command_id offset (command id of the selected menu - command id of the first menu)

    //From this point, it is safe to use class members without other threads interference
    CriticalSectionGuard cs_guard(&cs_);
    return esx::InvokeMenuCommand(targetCommandOffset, selectionCtx_);
}

HRESULT STDMETHODCALLTYPE ContextMenu::GetCommandString(UINT_PTR idCmd, UINT uType, UINT* pReserved, CHAR* pszName, UINT cchMax) {
    std::string typeStr = esx::GetGetCommandStringFlags(uType);
    std::string typeHex = esx::StringPrintf("0x%08x", uType);

    // only show this log in verbose mode
    ESX_LOG(INFO) << __FUNCTION__ << "(), idCmd=" << idCmd
                  << ", cchMax=" << cchMax
                  << " this=" << esx::ToHexString(this)
                  << ", type=" << typeHex << ":" << typeStr;

    UINT targetCommandOffset = (UINT)idCmd;  // matches the command_id offset (command id of the selected menu substracted by command id of the first menu)

    //From this point, it is safe to use class members without other threads interference
    CriticalSectionGuard cs_guard(&cs_);
    std::wstring w;
    std::string ansi;

    if (uType == GCS_HELPTEXTA || uType == GCS_HELPTEXTW) {
        if (!esx::GetCommandHelpText(targetCommandOffset, w)) {
            return S_OK;
        }
    }
    else if (uType == GCS_VERBA || uType == GCS_VERBW) {
        if (!esx::GetCommandVerb(targetCommandOffset, w)) {
            return S_OK;
        }
    }

    //Build up tooltip string
    switch (uType) {
        case GCS_HELPTEXTA: {
            ansi = esx::StringEncode::UnicodeToAnsi(w);
            //ANIS tooltip handling
            lstrcpynA(pszName, ansi.c_str(), cchMax);
            return S_OK;
        } break;
        case GCS_HELPTEXTW: {
            //UNICODE tooltip handling
            lstrcpynW((LPWSTR)pszName, w.c_str(), cchMax);
            return S_OK;
        } break;
        case GCS_VERBA: {
            ansi = esx::StringEncode::UnicodeToAnsi(w);
            //ANIS tooltip handling
            lstrcpynA(pszName, ansi.c_str(), cchMax);
            return S_OK;
        } break;
        case GCS_VERBW: {
            //UNICODE tooltip handling
            lstrcpynW((LPWSTR)pszName, w.c_str(), cchMax);
            return S_OK;
        } break;
        case GCS_VALIDATEA:
        case GCS_VALIDATEW: {
            return S_OK;
        } break;
    }

    ESX_LOG(ERROR) << __FUNCTION__ << "(), unknown flags: " << uType;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE ContextMenu::Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hRegKey) {
    //From this point, it is safe to use class members without other threads interference
    CriticalSectionGuard cs_guard(&cs_);

    selectionCtx_.Clear();
    isBackGround_ = false;

    esx::StringList files;

    // Did we clicked on a folder's background or the desktop directory?
    if (pIDFolder) {
        ESX_LOG(INFO) << __FUNCTION__ << "(), User right-clicked on a background directory.";

        wchar_t szPath[2 * MAX_PATH] = {0};

        if (SHGetPathFromIDListW(pIDFolder, szPath)) {
            if (szPath[0] != '\0') {
                std::string pathA = esx::StringEncode::UnicodeToAnsi(szPath);
                ESX_LOG(INFO) << __FUNCTION__ << "(), Found directory '" << pathA << "'.";
                isBackGround_ = true;
                files.push_back(pathA);
            }
            else {
                ESX_LOG(WARNING) << __FUNCTION__ << "(), found empty path in pIDFolder.";
                return E_INVALIDARG;
            }
        }
        else {
            ESX_LOG(ERROR) << __FUNCTION__ << "(), SHGetPathFromIDList() has failed.";
            return E_INVALIDARG;
        }
    }

    // User clicked on one or more file or directory
    else if (pDataObj) {
        ESX_LOG(INFO) << __FUNCTION__ << "(), User right-clicked on selected files/directories.";

        FORMATETC fmt = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        STGMEDIUM stg = {TYMED_HGLOBAL};
        HDROP hDropInfo;

        // The selected files are expected to be in HDROP format.
        if (FAILED(pDataObj->GetData(&fmt, &stg))) {
            ESX_LOG(WARNING) << __FUNCTION__ << "(), selected files are not in HDROP format.";
            return E_INVALIDARG;
        }

        // Get a locked pointer to the files data.
        hDropInfo = (HDROP)GlobalLock(stg.hGlobal);
        if (NULL == hDropInfo) {
            ReleaseStgMedium(&stg);
            ESX_LOG(ERROR) << __FUNCTION__ << "(), failed to get lock on selected files.";
            return E_INVALIDARG;
        }

        UINT numFiles = DragQueryFileW(hDropInfo, 0xFFFFFFFF, NULL, 0);
        ESX_LOG(INFO) << __FUNCTION__ << "(), User right-clicked on " << numFiles << " files/directories.";

        // For each files
        for (UINT i = 0; i < numFiles; i++) {
            UINT length = DragQueryFileW(hDropInfo, i, NULL, 0);

            // Allocate a temporary buffer
            std::wstring path(length, '\0');
            if (path.size() != length)
                continue;
            size_t num_characters = size_t(length) + 1;

            // Copy the element into the temporary buffer
            DragQueryFileW(hDropInfo, i, (wchar_t*)path.data(), (UINT)num_characters);

            //add the new file
            std::string pathA = esx::StringEncode::UnicodeToAnsi(path);
            ESX_LOG(INFO) << __FUNCTION__ << "(), Found file/directory #" << esx::StringPrintf("%03d", i) << ": '" << pathA << "'.";
            files.push_back(pathA);
        }

        GlobalUnlock(stg.hGlobal);
        ReleaseStgMedium(&stg);
    }

    //update the selection context
    selectionCtx_.SetElements(files);

    return S_OK;
}
