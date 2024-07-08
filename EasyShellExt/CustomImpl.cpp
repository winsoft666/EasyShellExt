#include "pch.h"
#include "CustomImpl.h"
#include "LoggerHelper.h"

namespace esx {

enum Commands {
    CMD_A = 0,
    CMD_B,
    CMD_C
};

HRESULT CreateMenus(HMENU hMenu, UINT firstCommandId, UINT firstMenuIndex, USHORT& menuCount) {
    HRESULT hr = E_FAIL;

    // CMD_A
    {
        MENUITEMINFOW menuinfo = {0};
        menuinfo.cbSize = sizeof(MENUITEMINFOW);
        menuinfo.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
        menuinfo.wID = firstCommandId + CMD_A;
        menuinfo.fType = MFT_STRING;
        menuinfo.dwTypeData = (LPWSTR)L"Command A(&A)";
        menuinfo.fState = MFS_ENABLED;
        menuinfo.hbmpItem = NULL;

        if (!InsertMenuItemW(hMenu, firstMenuIndex + 0, TRUE, &menuinfo)) {
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // CMD_B
    {
        MENUITEMINFOW menuinfo = {0};
        menuinfo.cbSize = sizeof(MENUITEMINFOW);
        menuinfo.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
        menuinfo.wID = firstCommandId + CMD_B;
        menuinfo.fType = MFT_STRING;
        menuinfo.dwTypeData = (LPWSTR)L"Command B(&B)";
        menuinfo.fState = MFS_ENABLED;
        menuinfo.hbmpItem = NULL;

        if (!InsertMenuItemW(hMenu, firstMenuIndex + 1, TRUE, &menuinfo)) {
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // CMD_C
    {
        MENUITEMINFOW menuinfo = {0};
        menuinfo.cbSize = sizeof(MENUITEMINFOW);
        menuinfo.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
        menuinfo.wID = firstCommandId + CMD_C;
        menuinfo.fType = MFT_STRING;
        menuinfo.dwTypeData = (LPWSTR)L"Command C(&C)";
        menuinfo.fState = MFS_ENABLED;
        menuinfo.hbmpItem = NULL;

        if (!InsertMenuItemW(hMenu, firstMenuIndex + 2, TRUE, &menuinfo)) {
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

    menuCount = 3;

    hr = S_OK;

    return hr;
}

bool GetCommandHelpText(UINT targetCommandOffset, std::wstring& text) {
    return false;
}

bool GetCommandVerb(UINT targetCommandOffset, std::wstring& verb) {
    return false;
}

HRESULT InvokeMenuCommand(UINT targetCommandOffset, const SelectionContext& selectionCtx) {
    HRESULT hr = E_FAIL;
    if (targetCommandOffset == (UINT)CMD_A) {
    }
    else if (targetCommandOffset == (UINT)CMD_B) {
    }
    else if (targetCommandOffset == (UINT)CMD_C) {
    }
    return hr;
}
}  // namespace esx