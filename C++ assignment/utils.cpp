#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cctype>
#include <cstdlib>
#include <limits>
#include "utils.h"
using namespace std;

/* =========================================================================
   UTILS.CPP  (Team-shared responsibility: User Interface + validation)
   ========================================================================= */

// -------------------------------------------------------------------------
// Screen / UI helpers
// -------------------------------------------------------------------------
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pauseScreen() {
    cout << "\nPress ENTER to continue...";
    cin.get();
}

void exportToFile(const string &prefix, const string &content) {
    string filename = prefix + "_" + getTodayDate() + ".txt";
    for (char &c : filename) if (c == '/') c = '-';
    string path = getProgramDir() + filename;
    ofstream f(path);
    if (!f) { cout << "\nFailed to create file.\n"; return; }
    f << content;
    f.close();
    cout << "\nExported to: " << path << "\n";
}

void printDivider(char ch, int length) {
    cout << string(length, ch) << "\n";
}

void printHeader(const string &title) {
    printDivider('=');
    int pad = (65 - (int)title.size()) / 2;
    if (pad < 0) pad = 0;
    cout << string(pad, ' ') << title << "\n";
    printDivider('=');
}

// -------------------------------------------------------------------------
// Validated numeric input
// cin >> leaves the trailing newline in the buffer, so every numeric read
// is followed by cin.ignore(...) to clear it - this keeps getline() calls
// elsewhere in the program from being skipped.
// -------------------------------------------------------------------------
int getValidatedInt(const string &prompt, int minVal, int maxVal) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a whole number.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (value < minVal || value > maxVal) {
            cout << "Please enter a value between " << minVal << " and " << maxVal << ".\n";
            continue;
        }
        return value;
    }
}

double getValidatedDouble(const string &prompt, double minVal, double maxVal) {
    double value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (value < minVal || value > maxVal) {
            cout << "Please enter a value between " << fixed << setprecision(2)
                 << minVal << " and " << maxVal << ".\n";
            continue;
        }
        return value;
    }
}

// -------------------------------------------------------------------------
// Validated string-based input
// -------------------------------------------------------------------------
string getValidatedName(const string &prompt, bool allowCancel) {
    string name;
    while (true) {
        cout << prompt;
        getline(cin, name);
        if (allowCancel && name == "0") return "";
        if (isValidName(name)) return name;
        cout << "Invalid name. Use letters and spaces only (no digits/symbols).\n";
    }
}

string getValidatedPhone(const string &prompt, bool allowCancel) {
    string phone;
    while (true) {
        cout << prompt;
        getline(cin, phone);
        if (allowCancel && phone == "0") return "";
        if (isValidPhone(phone)) return phone;
        cout << "Invalid phone number. Use 9-12 digits only, no spaces or symbols.\n";
    }
}

string getValidatedEmail(const string &prompt, bool allowCancel) {
    string email;
    while (true) {
        cout << prompt;
        getline(cin, email);
        if (allowCancel && email == "0") return "";
        if (isValidEmail(email)) return email;
        cout << "Invalid email. It must contain '@' and a '.' (e.g. name@mail.com).\n";
    }
}

char getValidatedGender(const string &prompt) {
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        if (input.size() == 1) {
            char g = toupper((unsigned char)input[0]);
            if (g == 'M' || g == 'F' || g == 'O') return g;
        }
        cout << "Invalid input. Enter M (Male), F (Female) or O (Other).\n";
    }
}

char getYesNo(const string &prompt) {
    string input;
    while (true) {
        cout << prompt << " (Y/N): ";
        getline(cin, input);
        if (input.size() == 1) {
            char c = toupper((unsigned char)input[0]);
            if (c == 'Y' || c == 'N') return c;
        }
        cout << "Please enter Y or N.\n";
    }
}

// -------------------------------------------------------------------------
// Date validation & helpers  (format DD/MM/YYYY)
// -------------------------------------------------------------------------
bool isValidDate(const string &date) {
    if (date.size() != 10 || date[2] != '/' || date[5] != '/') return false;
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (!isdigit((unsigned char)date[i])) return false;
    }

    int day   = stoi(date.substr(0, 2));
    int month = stoi(date.substr(3, 2));
    int year  = stoi(date.substr(6, 4));

    if (month < 1 || month > 12) return false;
    if (year < 2026 || year > 2030) return false;

    int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (isLeap) daysInMonth[1] = 29;

    if (day < 1 || day > daysInMonth[month - 1]) return false;
    return true;
}

long dateToComparable(const string &date) {
    int day   = stoi(date.substr(0, 2));
    int month = stoi(date.substr(3, 2));
    int year  = stoi(date.substr(6, 4));
    return year * 10000L + month * 100 + day;
}

string getTodayDate() {
    time_t t = time(nullptr);
    tm *now = localtime(&t);
    ostringstream oss;
    oss << setw(2) << setfill('0') << now->tm_mday << "/"
        << setw(2) << setfill('0') << (now->tm_mon + 1) << "/"
        << (now->tm_year + 1900);
    return oss.str();
}

string getValidatedDate(const string &prompt, bool mustNotBePast) {
    string date;
    while (true) {
        cout << prompt << "(DD/MM/YYYY): ";
        getline(cin, date);
        if (!isValidDate(date)) {
            cout << "Invalid date. Please use format DD/MM/YYYY with a real calendar date.\n";
            continue;
        }
        if (mustNotBePast && dateToComparable(date) < dateToComparable(getTodayDate())) {
            cout << "Date cannot be in the past. Please enter today or a future date.\n";
            continue;
        }
        return date;
    }
}

// -------------------------------------------------------------------------
// Raw validation checks
// -------------------------------------------------------------------------
bool isValidName(const string &name) {
    if (name.empty()) return false;
    bool hasLetter = false;
    for (char c : name) {
        if (isalpha((unsigned char)c)) hasLetter = true;
        else if (c != ' ' && c != '\'' && c != '-') return false;
    }
    return hasLetter;
}

bool isValidPhone(const string &phone) {
    if (phone.size() < 9 || phone.size() > 12) return false;
    for (char c : phone) {
        if (!isdigit((unsigned char)c)) return false;
    }
    return true;
}

bool isValidEmail(const string &email) {
    size_t atPos = email.find('@');
    size_t dotPos = email.find_last_of('.');
    if (atPos == string::npos || dotPos == string::npos) return false;
    if (atPos == 0 || dotPos < atPos || dotPos == email.size() - 1) return false;
    return true;
}

// -------------------------------------------------------------------------
// File path helpers
// -------------------------------------------------------------------------
static string g_programDir;

void setProgramDir(const string &argv0) {
    g_programDir = argv0;
    size_t pos = g_programDir.find_last_of("\\/");
    if (pos != string::npos)
        g_programDir = g_programDir.substr(0, pos + 1);
    else
        g_programDir = "";
}

string getProgramDir() {
    return g_programDir;
}
