#include <iostream>
#include <string>
#include <set>
#include <memory>
#include <fstream>
#include <iomanip>
#include <vector>

using namespace std;

void logResult(const string& message);

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

struct DocumentComparator {
	bool operator()(const shared_ptr<Document>& a, const shared_ptr<Document>& b) const {
		return a->id < b->id;
	}
};

set<shared_ptr<Document>, DocumentComparator> documents;

bool isValidFormat(const string& format) {
	return format == "txt" || format == "pdf";
}

bool isValidSignedFlag(const string& input) {
	return input == "0" || input == "1";
}

bool getValidatedInt(const string& prompt, int& result, int min = INT_MIN, int max = INT_MAX) {
	cout << prompt;
	string input;
	getline(cin, input);

	try {
		result = stoi(input);
		if (result < min || result > max) throw out_of_range("Out of range");
		return true;
	}
	catch (...) {
		cout << "���������� ��������. ��������� �� ���!\n";
		return false;
	}
}


void findInvalidDocumentsByError(const string& errorType) {
	for (const auto& doc : documents) {
		if (errorType == "empty_content" && doc->content.empty()) {
			cout << "�������� �������� ��� �����. (ID: " << doc->id << ")\n";
		}
		else if (errorType == "not_signed" && !doc->isSigned) {
			cout << "�������� �� ��������� ��������. (ID: " << doc->id << ")\n";
		}
		else if (errorType == "invalid_format" && doc->format != "txt" && doc->format != "pdf") {
			cout << "�������� �������� � �������� ��������: " << doc->format << " (ID: " << doc->id << ")\n";
		}
	}
}



void printAllDocuments() {
	if (documents.empty()) {
		cout << "������ ��������� �������!\n";
		return;
	}
	cout << "������ ���������";
	cout << "\n+----+--------------------------+--------+--------+\n";
	cout << "| ID |          ����           | ϳ���� | ������ |\n";
	cout << "+----+--------------------------+--------+--------+\n";

	for (const auto& doc : documents) {
		cout << "| " << left << setw(2) << doc->id << " | "
			<< left << setw(25) << (doc->content.length() > 25 ? doc->content.substr(0, 20) + "..." : doc->content) << "| "
			<< left << setw(6) << (doc->isSigned ? "Yes" : "No") << " | "
			<< left << setw(6) << doc->format << " |\n";
	}


	cout << "+----+--------------------------+--------+--------+\n";

	string countLine = "| ������ ���������: " + to_string(documents.size());
	int totalLength = 49;
	int padding = totalLength - static_cast<int>(countLine.length());
	cout << countLine << string(padding, ' ') << " |\n";

	cout << "+-------------------------------------------------+\n";

}


void deleteDocumentById(int targetId) {
	bool found = false;
	for (auto it = documents.begin(); it != documents.end(); ++it) {
		if ((*it)->id == targetId) {
			documents.erase(it);
			cout << "�������� � ID " << targetId << " ������ ��������!\n";
			found = true;
			break;
		}
	}
	if (!found) {
		cout << "\n�������� � ID " << targetId << " �� ��������.\n";
	}
}


void saveDocumentsToFile(const string& filename = "documents.txt") {
	ofstream out(filename);
	if (!out.is_open()) {
		cerr << "�� ������� ������� ���� ��� ������.\n";
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




void loadDocumentsFromFile(const string& filename = "documents.txt") {
	ifstream in(filename);
	if (!in.is_open()) {
		cerr << "���� ��������� �� ��������.\n";
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

void clearAllDocuments() {
	char confirm;
	cout << "�����! �� �������, �� ������ �������� �� ���������? (y/n): ";
	cin >> confirm;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');

	if (confirm == 'y' || confirm == 'Y') {
		documents.clear();
		cout << "�� ��������� ������ ��������.\n";
		logResult("���������� ������� �� ���������.");
	}
	else {
		cout << "�������� ���������.\n";
	}
}

void editDocumentById() {
	int editId;
	while (!getValidatedInt("������ ID ���������, ���� ������ ����������: ", editId, 1)) {}

	for (auto& doc : documents) {
		if (doc->id == editId) {
			cout << "\n+----+--------------------------+--------+--------+\n";
			cout << "| ID | ����                    | ϳ���� | ������ |\n";
			cout << "+----+--------------------------+--------+--------+\n";
			cout << "| " << left << setw(2) << doc->id << " | "
				<< left << setw(24) << (doc->content.length() > 22 ? doc->content.substr(0, 19) + "..." : doc->content) << " | "
				<< left << setw(6) << (doc->isSigned ? "���" : "ͳ") << " | "
				<< left << setw(6) << doc->format << " |\n";
			cout << "+----+--------------------------------------------+\n";
			cout << "|            ������ ����� �����������            |\n";
			cout << "+----+--------------------------------------------+\n";
			cout << "| 1  | ������ ���� ���������                    |\n";
			cout << "| 2  | ������ ������ (txt/pdf)                   |\n";
			cout << "| 3  | ������ ������ ������                     |\n";
			cout << "+----+--------------------------------------------+\n";

			int choice;
			while (!getValidatedInt("��� ����: ", choice, 1, 3)) {}

			switch (choice) {
			case 1: {
				cout << "����� ����: ";
				cin.ignore();
				getline(cin, doc->content);
				break;
			}
			case 2: {
				cout << "����� ������ (txt/pdf): ";
				cin.ignore();
				getline(cin, doc->format);
				break;
			}
			case 3: {
				int flag;
				while (!getValidatedInt("����� ������ (1 - ��������, 0 - �� ��������): ", flag, 0, 1)) {}
				doc->isSigned = (flag == 1);
				break;
			}
			}

			cout << "�������� ��������!\n";
			logResult("�������� � ID " + to_string(editId) + " ������������.");
			return;
		}
	}

	cout << "�������� � ����� ID �� ��������\n";
}


void verifyAllDocuments() {
	cout << "�������� ���������";
	cout << "\n+-----+-------------------------+--------+--------+--------------------------------+\n";
	cout << "| ID  | Content                 | ϳ���� | ������ | ������ ��������               |\n";
	cout << "+-----+-------------------------+--------+--------+--------------------------------+\n";

	for (const auto& doc : documents) {
		bool valid = true;
		string status;

		if (doc->format != "txt" && doc->format != "pdf") {
			status += "- ������; ";
			valid = false;
		}
		if (doc->content.empty()) {
			status += "- ����; ";
			valid = false;
		}
		if (!doc->isSigned) {
			status += "- ϳ����; ";
			valid = false;
		}

		if (valid) {
			status = "+ ������ ���������";
		}

		cout << "| " << left << setw(3) << doc->id << " | "
			<< left << setw(24) << (doc->content.length() > 22 ? doc->content.substr(0, 19) + "..." : doc->content) << "| "
			<< left << setw(7) << (doc->isSigned ? "���" : "ͳ") << "| "
			<< left << setw(7) << doc->format << "| "
			<< left << setw(31) << status << "|\n";
	}

	cout << "+-----+-------------------------+--------+--------+--------------------------------+\n";
}

void printErrorTable(const vector<shared_ptr<Document>>& docs, const string& header) {
	cout << header << "\n";
	cout << "+-----+-------------------------+--------+--------+-------------------------------+\n";
	cout << "| ID  | Content                 | ϳ���� | ������ | ��������                      |\n";
	cout << "+-----+-------------------------+--------+--------+-------------------------------+\n";

	for (const auto& doc : docs) {
		string errors;
		if (doc->content.empty()) errors += "- ����; ";
		if (!doc->isSigned) errors += "- ϳ����; ";
		if (doc->format != "txt" && doc->format != "pdf") errors += "- ������; ";

		cout << "| " << left << setw(3) << doc->id << " | "
			<< left << setw(24) << (doc->content.length() > 22 ? doc->content.substr(0, 19) + "..." : doc->content) << "| "
			<< left << setw(7) << (doc->isSigned ? "���" : "ͳ") << "| "
			<< left << setw(7) << doc->format << "| "
			<< left << setw(30) << errors << "|\n";
	}

	cout << "+-----+-------------------------+--------+--------+-------------------------------+\n";
}

void addDocumentManually() {
	string content, formatInput;
	bool signedFlag = false;

	cout << "������ ���� ���������: ";
	getline(cin, content);

	string flagInput;
	while (true) {
		cout << "�������� ��������? (1 � ���, 0 � �): ";
		getline(cin, flagInput);

		if (flagInput == "1" || flagInput == "0") {
			signedFlag = (flagInput == "1");
			break;
		}
		else {
			cout << "���������� ��������. ������ 1 ��� 0\n";
		}
	}

	cout << "������ ������ ��������� (txt/pdf): ";
	getline(cin, formatInput);

	auto doc = make_shared<Document>(content, signedFlag, formatInput);
	documents.insert(doc);

	cout << "�������� ������ ������!\n";
	logResult("������ ����� �������� ������ (ID: " + to_string(doc->id) + ")");
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
			cout << "������� ���� �������!\n";
			return;
		}
	}

	if (result.empty()) {
		cout << "��������� �� �������� ������� �� ��������\n";
	}
	else {
		string header = "��������� � ���������: ";
		if (option == 1) header += "����";
		else if (option == 2) header += "ϳ����";
		else if (option == 3) header += "������";
		else header += "��";
		printErrorTable(result, header);
	}
}


void showErrorFilterMenu() {
	cout << "+-------------------------------------------------+\n";
	cout << "|       ������ ��� ������� ��� ����������:       |\n";
	cout << "+-------------------------------------------------+\n";
	cout << "| 1 | ��������� ��� �����                        |\n";
	cout << "| 2 | ��������� ��� ������                       |\n";
	cout << "| 3 | ��������� � �������� ��������              |\n";
	cout << "| 4 | �� ��������� � ����-����� ���������        |\n";
	cout << "| 0 | ����������� �� ��������� ����               |\n";
	cout << "+-------------------------------------------------+\n";
}


void showMenu() {
	cout << "+-------------------------------------------------+" << endl;
	cout << "|        ���� ������� �������� ���������        |" << endl;
	cout << "+-------------------------------------------------+" << endl;
	cout << "| 1 |  ������ �������� ������                     |" << endl;
	cout << "| 2 |  ���������� �������� �� ID                  |" << endl;
	cout << "| 3 |  ��������� �� ���������                   |" << endl;
	cout << "| 4 |  �������� �� ���������                     |" << endl;
	cout << "| 5 |  ����� ��������� � ��������                |" << endl;
	cout << "| 6 |  ����������� �� ���������                  |" << endl;
	cout << "| 7 |  �������� ��������� � ����                  |" << endl;
	cout << "| 8 |  �������� �������� �� ID                    |" << endl;
	cout << "| 9 |  ����������� ��������� � �����              |" << endl;
	cout << "| 0 |  �����                                      |" << endl;
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
				cout << "������������ ����� ����. ���� �� ���� �� " << minOption << " � " << maxOption << ".\n";
			}
		}
		catch (...) {
			cout << "����������� ����� ����. ��������� �� ���.\n";
		}
	}
}


int main() {
	auto formatValidator = make_shared<FormatValidator>();
	auto contentValidator = make_shared<ContentValidator>();
	auto signatureValidator = make_shared<SignatureValidator>();

	formatValidator->setNext(contentValidator);
	contentValidator->setNext(signatureValidator);
	showMenu();
	int choice;
	do {
		choice = getValidatedMenuChoice("������ �����: ", 0, 9);

		switch (choice) {
		case 1:
			system("cls");
			showMenu();
			addDocumentManually();
			break;
		case 2:
			system("cls");
			showMenu();
			editDocumentById();
			break;
		case 3:
			system("cls");
			showMenu();
			verifyAllDocuments();
			break;
		case 4:
			system("cls");
			showMenu();
			clearAllDocuments();
			break;
		case 5: {
			int errorOption;
			showErrorFilterMenu();
			do {

				errorOption = getValidatedMenuChoice("��� ����: ", 0, 4);

				if (errorOption >= 1 && errorOption <= 4) {
					system("cls");
					showMenu();
					showErrorFilterMenu();
					handleErrorSearch(errorOption);

				}
			} while (errorOption != 0);
			system("cls");
			showMenu();
			break;
		}
		case 6:
			system("cls");
			showMenu();
			printAllDocuments();
			break;
		case 7:
			system("cls");
			showMenu();
			saveDocumentsToFile();
			cout << "��������� ���������!" << endl;
			break;
		case 8: {
			system("cls");
			showMenu();
			int delId;
			while (!getValidatedInt("������ ID ��������� ��� ���������: ", delId, 1)) {}
			deleteDocumentById(delId);
			break;
		}
		case 9:
			system("cls");
			showMenu();
			loadDocumentsFromFile();
			cout << "��������� �����������!" << endl;
			break;
		case 0:
			cout << "����� � ��������...\n";
			break;
		default:
			cout << "������� ����! ��������� �� ���.\n";
		}


	} while (choice != 0);

	return 0;
}


