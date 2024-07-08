#include "pch.h"
#include "SelectionContext.h"

namespace esx {
SelectionContext::SelectionContext() :
    numFiles_(0),
    numDirectories_(0) {
}

SelectionContext::SelectionContext(const SelectionContext& c) {
    (*this) = c;
}

SelectionContext::~SelectionContext() {
}

const SelectionContext& SelectionContext::operator=(const SelectionContext& c) {
    if (this != &c) {
        elements_ = c.elements_;
        numFiles_ = c.numFiles_;
        numDirectories_ = c.numDirectories_;
    }
    return (*this);
}

const StringList& SelectionContext::GetElements() const {
    return elements_;
}

void SelectionContext::SetElements(const StringList& elements) {
    elements_ = elements;

    numFiles_ = 0;
    numDirectories_ = 0;

    // Update stats
    for (size_t i = 0; i < elements.size(); i++) {
        const std::string& element = elements[i];
        DWORD attri = GetFileAttributesA(element.c_str());
        if (attri != INVALID_FILE_ATTRIBUTES) {
            if (attri & FILE_ATTRIBUTE_DIRECTORY) {
                numDirectories_++;
            }
            else {
                numFiles_++;
            }
        }
    }
}

void SelectionContext::Clear() {
    elements_.clear();
    numFiles_ = 0;
    numDirectories_ = 0;
}

int SelectionContext::GetNumFiles() const {
    return numFiles_;
}

int SelectionContext::GetNumDirectories() const {
    return numDirectories_;
}
}  // namespace esx