#ifndef SHELL_EXT_CUSTOM_IMPL_H_
#define SHELL_EXT_CUSTOM_IMPL_H_
#pragma once
#include <string>
#include "SelectionContext.h"

// Modify these value!
#define COMPANY_NAME L"EasyShellExt-Company"
#define SHELL_EXT_NAME L"EasyShellExt-Name"

namespace esx {
HRESULT CreateMenus(HMENU hMenu, UINT firstCommandId, UINT firstMenuIndex, USHORT& menuCount);
bool GetCommandHelpText(UINT targetCommandOffset, std::wstring& text);
bool GetCommandVerb(UINT targetCommandOffset, std::wstring& verb);
HRESULT InvokeMenuCommand(UINT targetCommandOffset, const SelectionContext& selectionCtx);
}  // namespace esx

#endif  // !SHELL_EXT_CUSTOM_IMPL_H_
