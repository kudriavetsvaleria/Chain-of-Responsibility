#include "Document.h"

int Document::nextId = 1;

Document::Document(std::string c, bool s, std::string f) 
    : content(c), isSigned(s), format(f) {
    id = nextId++;
}
