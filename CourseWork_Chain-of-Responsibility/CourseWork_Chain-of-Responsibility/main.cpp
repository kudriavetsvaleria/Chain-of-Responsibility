#include <iostream>
#include <string>
#include <limits>
#include <fstream>
#include "DocumentStorage.h"
#include <Windows.h>

using namespace std;

// Helper functions (moved from original monolithic file or duplicated if simple)
bool getValidatedInt(const string& prompt, int& result, int min, int max) {
    cout << prompt;
    string input;
    getline(cin, input);

    try {
        result = stoi(input);
        if (result < min || result > max) throw out_of_range("Out of range");
        return true;
    }
    catch (...) {
        cout << "Некоректне значення. Спробуйте ще раз!\n";
        return false;
    }
}

void logResult(const string& message) {
    ofstream logFile("log.txt", ios::app);
    logFile << message << endl;
    logFile.close();
}

void showMenu() {
    cout << "+-------------------------------------------------+" << endl;
    cout << "|        Меню системи перевірки документів        |" << endl;
    cout << "+-------------------------------------------------+" << endl;
    cout << "| 1 |  Додати документ вручну                     |" << endl;
    cout << "| 2 |  Редагувати документ за ID                  |" << endl;
    cout << "| 3 |  Перевірити всі документи                   |" << endl;
    cout << "| 4 |  Очистити всі документи                     |" << endl;
    cout << "| 5 |  Пошук документів з помилкою                |" << endl;
    cout << "| 6 |  Переглянути всі документи                  |" << endl;
    cout << "| 7 |  Зберегти документи у файл                  |" << endl;
    cout << "| 8 |  Видалити документ за ID                    |" << endl;
    cout << "| 9 |  Завантажити документи з файлу              |" << endl;
    cout << "| 0 |  Вийти                                      |" << endl;
    cout << "+-------------------------------------------------+" << endl;
}

int getValidatedMenuChoice(const string& prompt, int minOption, int maxOption) {
    int choice;
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);

        try {
            choice = stoi(input);
            if (choice >= minOption && choice <= maxOption) {
                return choice;
            }
            else {
                cout << "Неправильний пункт меню. Вибір має бути між " << minOption << " і " << maxOption << ".\n";
            }
        }
        catch (...) {
            cout << "Некоректний пункт меню. Спробуйте ще раз.\n";
        }
    }
}

int main() {
    // Set console encoding for Ukrainian characters
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    DocumentStorage DocSystem;
    
    // Construct Chain of Responsibility
    auto formatValidator = make_shared<FormatValidator>();
    auto contentValidator = make_shared<ContentValidator>();
    auto signatureValidator = make_shared<SignatureValidator>();

    // Link the chain: Format -> Content -> Signature
    formatValidator->setNext(contentValidator);
    contentValidator->setNext(signatureValidator);

    // Inject chain into storage
    DocSystem.setValidatorChain(formatValidator);

    showMenu();
    int choice;
    do {
        choice = getValidatedMenuChoice("Оберіть опцію: ", 0, 9);

        switch (choice) {
        case 1:
            system("cls");
            showMenu();
            DocSystem.addDocumentManually();
            break;
        case 2:
            system("cls");
            showMenu();
            DocSystem.editDocumentById();
            break;
        case 3:
            system("cls");
            showMenu();
            DocSystem.verifyAllDocuments();
            break;
        case 4:
            system("cls");
            showMenu();
            DocSystem.clearAllDocuments();
            break;
        case 5: {
            int errorOption;
            DocSystem.showErrorFilterMenu();
            do {
                errorOption = getValidatedMenuChoice("Ваш вибір: ", 0, 4);
                if (errorOption >= 1 && errorOption <= 4) {
                    system("cls");
                    showMenu();
                    DocSystem.showErrorFilterMenu();
                    DocSystem.handleErrorSearch(errorOption);
                }
            } while (errorOption != 0);
            system("cls");
            showMenu();
            break;
        }
        case 6:
            system("cls");
            showMenu();
            DocSystem.printAllDocuments();
            break;
        case 7:
            system("cls");
            showMenu();
            DocSystem.saveDocumentsToFile();
            cout << "Документи збережено!" << endl;
            break;
        case 8: {
            system("cls");
            showMenu();
            int delId;
            while (!getValidatedInt("Введіть ID документа для видалення: ", delId, 1, INT_MAX)) {}
            DocSystem.deleteDocumentById(delId);
            break;
        }
        case 9:
            system("cls");
            showMenu();
            DocSystem.loadDocumentsFromFile();
            cout << "Документи завантажено!" << endl;
            break;
        case 0:
            cout << "Вихід з програми...\n";
            break;
        default:
            cout << "Невірний вибір! Спробуйте ще раз.\n";
        }
    } while (choice != 0);

    return 0;
}
