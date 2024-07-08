#ifndef SHELL_EXT_CONTEXT_MENU_H_
#define SHELL_EXT_CONTEXT_MENU_H_
#pragma once

#include <shlobj.h>
#include <shlguid.h>
#include <comdef.h>
#include <atlconv.h>   // for ATL string conversion macros
#include "resource.h"  // main symbols

// Generated files
#include "shellext_h.h"
#include "shellext_i.c"

#include "CriticalSection.h"
#include "SelectionContext.h"

class ATL_NO_VTABLE ContextMenu : public CComObjectRootEx<CComSingleThreadModel>,
                                  public CComCoClass<ContextMenu, &CLSID_EasyShellExt>,
                                  public IDispatchImpl<IEasyShellExt, &IID_IEasyShellExt, &LIBID_EASYSHELLEXTENSIONLib>,
                                  public IShellExtInit,
                                  public IContextMenu {
   public:
    ContextMenu();
    ~ContextMenu();

    DECLARE_REGISTRY_RESOURCEID(IDR_EASYSHELLEXT)

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(ContextMenu)
    COM_INTERFACE_ENTRY(IEasyShellExt)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IShellExtInit)
    COM_INTERFACE_ENTRY(IContextMenu)
    END_COM_MAP()

    //IContextMenu interface
    HRESULT STDMETHODCALLTYPE QueryContextMenu(HMENU hMenu, UINT menuIndex, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    HRESULT STDMETHODCALLTYPE InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi);
    HRESULT STDMETHODCALLTYPE GetCommandString(UINT_PTR idCmd, UINT uType, UINT* pReserved, CHAR* pszName, UINT cchMax);

    //IShellExtInit interface
    HRESULT STDMETHODCALLTYPE Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hKeyID);

   private:
    CriticalSection cs_;
    ULONG refCount_;
    bool isBackGround_;
    HMENU previousMenu_;
    esx::SelectionContext selectionCtx_;
};
#endif  // !SHELL_EXT_CONTEXT_MENU_H_
