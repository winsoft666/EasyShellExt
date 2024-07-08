#ifndef SHELL_EXT_SELECTION_CONTEXT_H
#define SHELL_EXT_SELECTION_CONTEXT_H
#pragma once

#include <string>
#include <vector>

namespace esx {

typedef std::vector<std::string> StringList;

class SelectionContext {
   public:
    SelectionContext();
    SelectionContext(const SelectionContext& c);
    virtual ~SelectionContext();

    const SelectionContext& operator=(const SelectionContext& c);

    const StringList& GetElements() const;

    void SetElements(const StringList& elements);

    void Clear();

    int GetNumFiles() const;

    int GetNumDirectories() const;

   private:
    StringList elements_;
    int numFiles_;
    int numDirectories_;
};
}  // namespace esx
#endif  //SHELL_EXT_SELECTION_CONTEXT_H
