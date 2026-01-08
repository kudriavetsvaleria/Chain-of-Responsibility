#pragma once
#include <set>
#include <vector>
#include <memory>
#include <string>
#include "Document.h"
#include "Validator.h"

class DocumentStorage {
private:
    std::set<std::shared_ptr<Document>, DocumentComparator> documents;
    std::shared_ptr<Validator> validatorChain;

public:
    DocumentStorage();
    void setValidatorChain(std::shared_ptr<Validator> chain);

    void addDocumentManually();
    void deleteDocumentById(int targetId);
    void editDocumentById();
    void printAllDocuments();
    void verifyAllDocuments(); // Uses the chain now!
    void clearAllDocuments();
    void saveDocumentsToFile(const std::string& filename = "documents.txt");
    void loadDocumentsFromFile(const std::string& filename = "documents.txt");
    
    // Filtering
    void showErrorFilterMenu();
    void handleErrorSearch(int option);
    void findInvalidDocumentsByError(const std::string& errorType);
    
    // Helpers
    void printErrorTable(const std::vector<std::shared_ptr<Document>>& docs, const std::string& header);
};
