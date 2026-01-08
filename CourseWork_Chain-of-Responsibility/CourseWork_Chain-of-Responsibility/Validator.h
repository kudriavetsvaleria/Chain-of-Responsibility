#pragma once
#include "Document.h"
#include <memory>
#include <vector>
#include <string>
#include <iostream>

class Validator {
protected:
    std::shared_ptr<Validator> next;
public:
    virtual ~Validator() = default;

    void setNext(std::shared_ptr<Validator> nextValidator) {
        next = nextValidator;
    }

    // Returns true if valid so far, but we want to collect ALL errors.
    // So we usually return void or bool, but append to errors vector.
    virtual void validate(const Document& doc, std::vector<std::string>& errors) {
        if (next) {
            next->validate(doc, errors);
        }
    }
};

class FormatValidator : public Validator {
public:
    void validate(const Document& doc, std::vector<std::string>& errors) override {
        if (doc.format != "txt" && doc.format != "pdf") {
            errors.push_back("- Формат");
        }
        Validator::validate(doc, errors);
    }
};

class ContentValidator : public Validator {
public:
    void validate(const Document& doc, std::vector<std::string>& errors) override {
        if (doc.content.empty()) {
            errors.push_back("- Вміст");
        }
        Validator::validate(doc, errors);
    }
};

class SignatureValidator : public Validator {
public:
    void validate(const Document& doc, std::vector<std::string>& errors) override {
        if (!doc.isSigned) {
            errors.push_back("- Підпис");
        }
        Validator::validate(doc, errors);
    }
};
