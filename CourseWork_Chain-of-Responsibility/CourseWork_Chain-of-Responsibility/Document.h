#pragma once
#include <string>
#include <memory>

struct Document {
    int id;
    std::string content;
    bool isSigned;
    std::string format;
    static int nextId;

    Document(std::string c, bool s, std::string f);
};

struct DocumentComparator {
    bool operator()(const std::shared_ptr<Document>& a, const std::shared_ptr<Document>& b) const {
        return a->id < b->id;
    }
};
