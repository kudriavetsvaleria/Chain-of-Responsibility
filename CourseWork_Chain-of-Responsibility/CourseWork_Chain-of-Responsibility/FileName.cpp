#include <iostream>
#include <string>
#include <set>
#include <memory>
#include <fstream>
#include <iomanip>
#include <vector>
#include <conio.h> 
#include <algorithm>


using namespace std;

void logResult(const string& message);
bool getValidatedInt(const string& prompt, int& result, int min = INT_MIN, int max = INT_MAX);


class Document {
public:
	int id;
	string content;
	bool isSigned;
	string format;
	static int nextId;
	Document(string c, bool s, string f) : content(c), isSigned(s), format(f) {
		id = nextId++;
	}
};

int Document::nextId = 1;

struct DocumentComparator {
	bool operator()(const shared_ptr<Document>& a, const shared_ptr<Document>& b) const {
		return a->id < b->id;
	}
};

class Validator {
protected:
	shared_ptr<Validator> next;
public:
	void setNext(shared_ptr<Validator> nextValidator) {
		next = nextValidator;
	}
	virtual bool validate(Document& doc) {
		if (next) {
			return next->validate(doc);
		}
		return true;
	}
};

class FormatValidator : public Validator {
public:
	bool validate(Document& doc) override {
		if (doc.format != "txt" && doc.format != "pdf") {
			cout << "Invalid format.\n";
			logResult("Invalid format for document.");
			return false;
		}
		return Validator::validate(doc);
	}
};

class ContentValidator : public Validator {
public:
	bool validate(Document& doc) override {
		if (doc.content.empty()) {
			cout << "Empty content.\n";
			logResult("Document has empty content.");
			return false;
		}
		return Validator::validate(doc);
	}
};

class SignatureValidator : public Validator {
public:
	bool validate(Document& doc) override {
		if (!doc.isSigned) {
			cout << "Document is not signed.\n";
			logResult("Document is not signed.");
			return false;
		}
		return Validator::validate(doc);
	}
};

class DocumentStorage {
private:
	set<shared_ptr<Document>, DocumentComparator> documents;

public:
	void printErrorTable(const vector<shared_ptr<Document>>& docs, const string& header) {
		cout << header << "\n";
		cout << "+-----+-------------------------+--------+--------+-------------------------------+\n";
		cout << "| ID  | Content                 | Підпис | Формат | Проблеми                      |\n";
		cout << "+-----+-------------------------+--------+--------+-------------------------------+\n";

		for (const auto& doc : docs) {
			string errors;
			if (doc->content.empty()) errors += "- Вміст; ";
			if (!doc->isSigned) errors += "- Підпис; ";
			if (doc->format != "txt" && doc->format != "pdf") errors += "- Формат; ";

			cout << "| " << left << setw(3) << doc->id << " | "
				<< left << setw(24) << (doc->content.length() > 22 ? doc->content.substr(0, 19) + "..." : doc->content) << "| "
				<< left << setw(7) << (doc->isSigned ? "Так" : "Ні") << "| "
				<< left << setw(7) << doc->format << "| "
				<< left << setw(30) << errors << "|\n";
		}

		cout << "+-----+-------------------------+--------+--------+-------------------------------+\n";
	}

	void showErrorFilterMenu() {
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

	void addDocumentManually() {
		string content, formatInput;
		bool signedFlag = false;

		cout << "Введіть вміст документа. Введіть `::end` на окремому рядку, щоб завершити:\n";

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

	void deleteDocumentById(int targetId) {
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

	void editDocumentById() {
		int editId;
		while (!getValidatedInt("Введіть ID документа, який хочете редагувати: ", editId, 1)) {}

		for (auto& doc : documents) {
			if (doc->id == editId) {
				// Обробка контенту для виводу в таблицю
				string displayContent = doc->content;
				replace(displayContent.begin(), displayContent.end(), '\n', ' ');
				if (displayContent.length() > 22)
					displayContent = displayContent.substr(0, 19) + "...";

				// Вивід інформації про документ
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
					cin.ignore();
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
					cin.ignore();
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

	void printAllDocuments() {
		if (documents.empty()) {
			cout << "Список документів порожній!\n";
			return;
		}

		cout << "Список документів";
		cout << "\n+----+--------------------------+--------+--------+\n";
		cout << "| ID |          Зміст           | Підпис | Формат |\n";
		cout << "+----+--------------------------+--------+--------+\n";

		for (const auto& doc : documents) {
			// Копія контенту для відображення
			string displayContent = doc->content;
			// Заміна переносів рядків на пробіли
			replace(displayContent.begin(), displayContent.end(), '\n', ' ');
			// Усічення довгих рядків з трикрапкою
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
		cout << countLine << string(padding, ' ') << " |\n";

		cout << "+-------------------------------------------------+\n";
	}

	void verifyAllDocuments() {
		cout << "Перевірка документів";
		cout << "\n+-----+-------------------------+--------+--------+--------------------------------+\n";
		cout << "| ID  | Content                 | Підпис | Формат | Статус перевірки               |\n";
		cout << "+-----+-------------------------+--------+--------+--------------------------------+\n";

		for (const auto& doc : documents) {
			bool valid = true;
			string status;

			if (doc->format != "txt" && doc->format != "pdf") {
				status += "- Формат; ";
				valid = false;
			}
			if (doc->content.empty()) {
				status += "- Вміст; ";
				valid = false;
			}
			if (!doc->isSigned) {
				status += "- Підпис; ";
				valid = false;
			}
			if (valid) {
				status = "+ Успішно перевірено";
			}

			// 🔧 Обробка багаторядкового контенту
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

	void clearAllDocuments() {
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

	void saveDocumentsToFile(const string& filename = "documents.txt") {
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

	void findInvalidDocumentsByError(const string& errorType) {
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


	void loadDocumentsFromFile(const string& filename = "documents.txt") {
		ifstream in(filename);
		if (!in.is_open()) {
			cerr << "Файл документів не знайдено.\n";
			return;
		}

		string line;
		string content, format;
		bool isSigned = false;
		int id = 0;
		int maxId = 0;

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



	void handleErrorSearch(int option) {
		vector<shared_ptr<Document>> result;

		for (const auto& doc : documents) {
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


};

string readFromFile(const string& filename) {
	ifstream file(filename);
	if (!file.is_open()) {
		cerr << "Error opening file: " << filename << endl;
		return "";
	}
	string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
	file.close();
	return content;
}

void logResult(const string& message) {
	ofstream logFile("log.txt", ios::app);
	logFile << message << endl;
	logFile.close();
}



bool isValidFormat(const string& format) {
	return format == "txt" || format == "pdf";
}

bool isValidSignedFlag(const string& input) {
	return input == "0" || input == "1";
}

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
	DocumentStorage DocSystem;
	auto formatValidator = make_shared<FormatValidator>();
	auto contentValidator = make_shared<ContentValidator>();
	auto signatureValidator = make_shared<SignatureValidator>();

	formatValidator->setNext(contentValidator);
	contentValidator->setNext(signatureValidator);
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
			while (!getValidatedInt("Введіть ID документа для видалення: ", delId, 1)) {}
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



