#include "DocumentStorage.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

// Helper function declarations (can be moved to a utils file later, but keeping local for now)
extern bool getValidatedInt(const string& prompt, int& result, int min, int max);
extern void logResult(const string& message);

DocumentStorage::DocumentStorage() {}

void DocumentStorage::setValidatorChain(shared_ptr<Validator> chain) {
    validatorChain = chain;
}

void DocumentStorage::printErrorTable(const vector<shared_ptr<Document>>& docs, const string& header) {
    cout << header << "\n";
    cout << "+-----+-------------------------+--------+--------+-------------------------------+\n";
    cout << "| ID  | Content                 | Підпис | Формат | Проблеми                      |\n";
    cout << "+-----+-------------------------+--------+--------+-------------------------------+\n";

    for (const auto& doc : docs) {
        string errorStr;
        // Use the chain to get errors just for display construction in this filtered view?
        // The original code passed in docs that were ALREADY filtered.
        // Re-validating here to get the error strings is consistent.
        
        vector<string> errors;
        if (validatorChain) {
            validatorChain->validate(*doc, errors);
        } else {
             // Fallback if no chain (shouldn't happen with correct usage)
             if (doc->content.empty()) errors.push_back("- Вміст");
             if (!doc->isSigned) errors.push_back("- Підпис");
             if (doc->format != "txt" && doc->format != "pdf") errors.push_back("- Формат");
        }

        for (const auto& err : errors) {
            errorStr += err + "; ";
        }

        cout << "| " << left << setw(3) << doc->id << " | "
            << left << setw(24) << (doc->content.length() > 22 ? doc->content.substr(0, 19) + "..." : doc->content) << "| "
            << left << setw(7) << (doc->isSigned ? "Так" : "Ні") << "| "
            << left << setw(7) << doc->format << "| "
            << left << setw(30) << errorStr << "|\n";
    }

    cout << "+-----+-------------------------+--------+--------+-------------------------------+\n";
}

void DocumentStorage::showErrorFilterMenu() {
    cout << "+-------------------------------------------------+\n";
    cout << "|       Оберіть тип помилки для фільтрації:       |\n";
    cout << "+-------------------------------------------------+\n";
    cout << "| 1 | Документи без вмісту                        |\n";
    cout << "| 2 | Документи без підпису                       |\n";
    cout << "| 3 | Документи з недійсним форматом              |\n";
    cout << "| 4 | Усі документи з будь-якими помилками        |\n";
    cout << "| 0 | Повернутись до головного меню               |\n";
    cout << "+-------------------------------------------------+\n";
}

void DocumentStorage::addDocumentManually() {
    string content, formatInput;
    bool signedFlag = false;

    cout << "Введіть вміст документа. Введіть `::end` на окремому рядку, щоб завершити:\n";

    // Clear input buffer if needed (simple approximation)
    // cin.ignore(); // can be risky depending on previous input

    while (true) {
        string line;
        getline(cin, line);
        if (line == "::end") break;
        content += line + "\n";
    }

    string flagInput;
    while (true) {
        cout << "Документ підписано? (1 – так, 0 – ні): ";
        getline(cin, flagInput);

        if (flagInput == "1" || flagInput == "0") {
            signedFlag = (flagInput == "1");
            break;
        }
        else {
            cout << "Некоректне значення. Введіть 1 або 0\n";
        }
    }

    cout << "Введіть формат документа (txt/pdf): ";
    getline(cin, formatInput);

    auto doc = make_shared<Document>(content, signedFlag, formatInput);
    documents.insert(doc);

    cout << "Документ успішно додано!\n";
    logResult("Додано новий документ вручну (ID: " + to_string(doc->id) + ")");
}

void DocumentStorage::deleteDocumentById(int targetId) {
    bool found = false;
    for (auto it = documents.begin(); it != documents.end(); ++it) {
        if ((*it)->id == targetId) {
            documents.erase(it);
            cout << "Документ з ID " << targetId << " успішно видалено!\n";
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "\nДокумент з ID " << targetId << " не знайдено.\n";
    }
}

void DocumentStorage::editDocumentById() {
    int editId;
    while (!getValidatedInt("Введіть ID документа, який хочете редагувати: ", editId, 1, INT_MAX)) {}

    for (auto& doc : documents) {
        if (doc->id == editId) {
            string displayContent = doc->content;
            replace(displayContent.begin(), displayContent.end(), '\n', ' ');
            if (displayContent.length() > 22)
                displayContent = displayContent.substr(0, 19) + "...";

            cout << "\n+-----+-------------------------+--------+--------+\n";
            cout << "| ID  | Зміст                   | Підпис | Формат |\n";
            cout << "+-----+-------------------------+--------+--------+\n";
            cout << "| " << left << setw(3) << doc->id << " | "
                << setw(25) << displayContent << "| "
                << setw(7) << (doc->isSigned ? "Так" : "Ні") << "| "
                << setw(7) << doc->format << " |\n";
            cout << "+-----+-------------------------+--------+--------+\n";

            cout << "+----+--------------------------------------------+\n";
            cout << "|            Оберіть пункт редагування            |\n";
            cout << "+----+--------------------------------------------+\n";
            cout << "| 1  | Змінити вміст документа                    |\n";
            cout << "| 2  | Змінити формат (txt/pdf)                   |\n";
            cout << "| 3  | Змінити статус підпису                     |\n";
            cout << "+----+--------------------------------------------+\n";

            int choice;
            while (!getValidatedInt("Ваш вибір: ", choice, 1, 3)) {}

            switch (choice) {
            case 1: {
                cout << "Введіть новий вміст документа. Введіть `::end` на окремому рядку, щоб завершити:\n";
                string newContent, line;
                // cin.ignore(); // Carefully used in main logic
                while (true) {
                    getline(cin, line);
                    if (line == "::end") break;
                    newContent += line + "\n";
                }
                doc->content = newContent;
                break;
            }
            case 2: {
                cout << "Новий формат (txt/pdf): ";
                string newFormat;
                // cin.ignore();
                getline(cin, newFormat);
                doc->format = newFormat;
                break;
            }
            case 3: {
                int flag;
                while (!getValidatedInt("Новий статус (1 - підписано, 0 - не підписано): ", flag, 0, 1)) {}
                doc->isSigned = (flag == 1);
                break;
            }
            }

            cout << "Документ оновлено!\n";
            logResult("Документ з ID " + to_string(editId) + " відредаговано.");
            return;
        }
    }

    cout << "Документ з таким ID не знайдено\n";
}

void DocumentStorage::printAllDocuments() {
    if (documents.empty()) {
        cout << "Список документів порожній!\n";
        return;
    }

    cout << "Список документів";
    cout << "\n+----+--------------------------+--------+--------+\n";
    cout << "| ID |          Зміст           | Підпис | Формат |\n";
    cout << "+----+--------------------------+--------+--------+\n";

    for (const auto& doc : documents) {
        string displayContent = doc->content;
        replace(displayContent.begin(), displayContent.end(), '\n', ' ');
        if (displayContent.length() > 25) {
            displayContent = displayContent.substr(0, 20) + "...";
        }

        cout << "| " << left << setw(2) << doc->id << " | "
            << left << setw(25) << displayContent << "| "
            << left << setw(6) << (doc->isSigned ? "Yes" : "No") << " | "
            << left << setw(6) << doc->format << " |\n";
    }

    cout << "+----+--------------------------+--------+--------+\n";

    string countLine = "| Всього документів: " + to_string(documents.size());
    int totalLength = 49;
    int padding = totalLength - static_cast<int>(countLine.length());
    cout << countLine << string(std::max(0, padding), ' ') << " |\n"; // safe max

    cout << "+-------------------------------------------------+\n";
}

void DocumentStorage::verifyAllDocuments() {
    cout << "Перевірка документів";
    cout << "\n+-----+-------------------------+--------+--------+--------------------------------+\n";
    cout << "| ID  | Content                 | Підпис | Формат | Статус перевірки               |\n";
    cout << "+-----+-------------------------+--------+--------+--------------------------------+\n";

    for (const auto& doc : documents) {
        vector<string> errors;
        if (validatorChain) {
            validatorChain->validate(*doc, errors);
        } else {
            // Should prompt error if no chain
            errors.push_back("SYSTEM ERROR: No validator chain");
        }

        string status;
        if (errors.empty()) {
            status = "+ Успішно перевірено";
        } else {
            for (const auto& err : errors) {
                status += err + "; ";
            }
        }

        string displayContent = doc->content;
        replace(displayContent.begin(), displayContent.end(), '\n', ' ');
        if (displayContent.length() > 23) {
            displayContent = displayContent.substr(0, 20) + "...";
        }

        cout << "| " << left << setw(3) << doc->id << " | "
            << left << setw(24) << displayContent << "| "
            << left << setw(7) << (doc->isSigned ? "Так" : "Ні") << "| "
            << left << setw(7) << doc->format << "| "
            << left << setw(31) << status << "|\n";
    }

    cout << "+-----+-------------------------+--------+--------+--------------------------------+\n";
}

void DocumentStorage::clearAllDocuments() {
    char confirm;
    cout << "Увага! Ви впевнені, що хочете видалити всі документи? (y/n): ";
    cin >> confirm;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (confirm == 'y' || confirm == 'Y') {
        documents.clear();
        cout << "Усі документи успішно видалено.\n";
        logResult("Користувач видалив усі документи.");
    }
    else {
        cout << "Очищення скасовано.\n";
    }
}

void DocumentStorage::saveDocumentsToFile(const string& filename) {
    ofstream out(filename);
    if (!out.is_open()) {
        cerr << "Не вдалося відкрити файл для запису.\n";
        return;
    }

    for (const auto& doc : documents) {
        out << "ID: " << doc->id << "\n";
        out << "Content: " << doc->content << "\n";
        out << "Signed: " << (doc->isSigned ? "Yes" : "No") << "\n";
        out << "Format: " << doc->format << "\n";
        out << "---\n";
    }

    out.close();
}

void DocumentStorage::loadDocumentsFromFile(const string& filename) {
    ifstream in(filename);
    if (!in.is_open()) {
        cerr << "Файл документів не знайдено.\n";
        return;
    }

    string line;
    string content, format;
    bool isSigned = false;
    int id = 0; // Initialize id
    int maxId = 0;

    // A simple parsing loop state machine could be better, but sticking to logic close to original
    while (getline(in, line)) {
        if (line.rfind("ID: ", 0) == 0) {
            id = stoi(line.substr(4));
            if (id > maxId) maxId = id;
        }
        else if (line.rfind("Content: ", 0) == 0) {
            content = line.substr(9);
        }
        else if (line.rfind("Signed: ", 0) == 0) {
            isSigned = (line.substr(8) == "Yes");
        }
        else if (line.rfind("Format: ", 0) == 0) {
            format = line.substr(8);
        }
        else if (line == "---") {
            auto doc = make_shared<Document>(content, isSigned, format);
            doc->id = id;
            documents.insert(doc);
        }
    }

    Document::nextId = maxId + 1;
    in.close();
}

void DocumentStorage::handleErrorSearch(int option) {
    vector<shared_ptr<Document>> result;

    for (const auto& doc : documents) {
        // Collect errors using chain or local logic?
        // Since search is by specific type, it's easier to check specific properties manually 
        // OR rely on the chain returning specific error strings. 
        // The original code checked properties directly. 
        // To use the pattern properly, we should ideally ask the chain, but checking property is faster/simpler here.
        // I will keep direct property checks for filtering efficiency as the Validators just append strings which is hard to parse back.
        
        bool invalidContent = doc->content.empty();
        bool invalidSign = !doc->isSigned;
        bool invalidFormat = (doc->format != "txt" && doc->format != "pdf");

        switch (option) {
        case 1:
            if (invalidContent) result.push_back(doc);
            break;
        case 2:
            if (invalidSign) result.push_back(doc);
            break;
        case 3:
            if (invalidFormat) result.push_back(doc);
            break;
        case 4:
            if (invalidContent || invalidSign || invalidFormat) result.push_back(doc);
            break;
        default:
            cout << "Невірний вибір фільтра!\n";
            return;
        }
    }

    if (result.empty()) {
        cout << "Документів за вибраним критерієм не знайдено\n";
    }
    else {
        string header = "Документи з помилками: ";
        if (option == 1) header += "Вміст";
        else if (option == 2) header += "Підпис";
        else if (option == 3) header += "Формат";
        else header += "Всі";
        printErrorTable(result, header);
    }
}

void DocumentStorage::findInvalidDocumentsByError(const string& errorType) {
    // Legacy helper? Or just unused. Keeping for interface compatibility if needed.
    // It was public in original.
     for (const auto& doc : documents) {
        if (errorType == "empty_content" && doc->content.empty()) {
            cout << "Знайдено документ без вмісту. (ID: " << doc->id << ")\n";
        }
        else if (errorType == "not_signed" && !doc->isSigned) {
            cout << "Знайдено не підписаний документ. (ID: " << doc->id << ")\n";
        }
        else if (errorType == "invalid_format" && doc->format != "txt" && doc->format != "pdf") {
            cout << "Знайдено документ з недійсним форматом: " << doc->format << " (ID: " << doc->id << ")\n";
        }
    }
}
