#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cctype>
#include <cstdlib>
#include <limits>
#include <algorithm>
#include "common.h"
#include "utils.h"
#include "customer.h"
#include "stylist_service.h"
#include "booking.h"
#include "billing.h"
#include "persistence.h"
using namespace std;

/* =========================================================================
   MAIN.CPP
   TEAM-SHARED FILE (System Architecture, Main Menu & Navigation,
   System Integration, Shared Data Management)
   Team: Ting Wei, Jeremy, Michael, Jia Zhi
   ---------------------------------------------------------------------
   This file owns the single "master copy" of every data list (customers,
   stylists, services, bookings, invoices) and every ID counter, then
   PASSES THEM BY REFERENCE into each module's menu function. That is how
   all four modules read and update the same shared data.
   ========================================================================= */

// Forward declarations
void displayIntroScreen();
void loadSampleData(vector<Customer> &customers, vector<Stylist> &stylists, vector<Service> &services,
                     int &nextCustomerID, int &nextStylistID, int &nextServiceID);

// =========================================================================
// IMPLEMENTATIONS FROM utils.cpp
// =========================================================================

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
string g_programDir;

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

string truncate(const string &s, int maxLen) {
    if ((int)s.size() <= maxLen) return s;
    return s.substr(0, maxLen - 2) + "..";
}

// =========================================================================
// IMPLEMENTATIONS FROM customer.cpp
// =========================================================================

/* =========================================================================
   CUSTOMER.CPP  --  MODULE 1: Customer Management   (Author: Ting Wei)
   Functions: Add / Search / Update / Delete / List customers, plus the
   Loyalty Points & Membership Tier extra feature.
   ========================================================================= */

// -------------------------------------------------------------------------
// findCustomerIndex - linear search by ID. Returns the vector index, or -1
// if not found/inactive. This is the SHARED lookup that Booking (Module 3)
// and Billing (Module 4) call to confirm a customer exists before creating
// a booking or an invoice - this is the cross-module data sharing the
// spec asks for.
// -------------------------------------------------------------------------
int findCustomerIndex(const vector<Customer> &customers, int customerID) {
    for (int i = 0; i < (int)customers.size(); i++) {
        if (customers[i].customerID == customerID && customers[i].isActive) {
            return i;
        }
    }
    return -1;
}

void displayCustomerBrief(const Customer &c) {
    cout << left << setw(6) << c.customerID
         << setw(20) << c.name
         << setw(14) << c.phone
         << c.email << "\n";
}

bool isDuplicatePhone(const vector<Customer> &customers, const string &phone) {
    for (const Customer &c : customers) {
        if (c.isActive && c.phone == phone) return true;
    }
    return false;
}

// -------------------------------------------------------------------------
// addCustomer - vector passed by REFERENCE so the new record persists in
// the caller; nextCustomerID passed by REFERENCE so the ID counter keeps
// incrementing correctly across every call from the menu loop.
// -------------------------------------------------------------------------
void addCustomer(vector<Customer> &customers, int &nextCustomerID) {
    clearScreen();
    printHeader("ADD NEW CUSTOMER");

    Customer c;
    c.customerID = nextCustomerID;
    c.name = getValidatedName("Customer Name (or 0 to cancel): ", true);
    if (c.name.empty()) { cout << "\nCancelled.\n"; pauseScreen(); return; }

    string phone;
    bool duplicate;
    do {
        phone = getValidatedPhone("Phone Number (or 0 to cancel): ", true);
        if (phone.empty()) { cout << "\nCancelled.\n"; pauseScreen(); return; }
        duplicate = isDuplicatePhone(customers, phone);
        if (duplicate) cout << "A customer with this phone number is already registered.\n";
    } while (duplicate);
    c.phone = phone;

    c.email = getValidatedEmail("Email Address (or 0 to cancel): ", true);
    if (c.email.empty()) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    c.gender         = getValidatedGender("Gender (M/F/O): ");
    c.loyaltyPoints  = 0;
    c.lifetimePoints = 0;
    c.isActive       = true;

    customers.push_back(c);
    nextCustomerID++;

    cout << "\nCustomer added successfully! Assigned Customer ID: " << c.customerID << "\n";
    pauseScreen();
}

// -------------------------------------------------------------------------
// searchCustomer - read-only, so it takes a const reference. Supports
// search by exact ID or partial (case-insensitive) name match.
// -------------------------------------------------------------------------
void searchCustomer(const vector<Customer> &customers) {
    clearScreen();
    printHeader("SEARCH CUSTOMER");
    cout << "1. Search by Customer ID\n2. Search by Name\n";
    int mode = getValidatedInt("Choice: ", 1, 2);

    bool found = false;

    if (mode == 1) {
        int id = getValidatedInt("Enter Customer ID (or 0 to cancel): ", 0, 999999);
        if (id == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
        int idx = findCustomerIndex(customers, id);
        if (idx != -1) {
            found = true;
            cout << "\n" << left << setw(6) << "ID" << setw(20) << "Name" << setw(14) << "Phone" << "Email" << "\n";
            printDivider('-', 60);
            displayCustomerBrief(customers[idx]);
            cout << "Gender: " << customers[idx].gender
                 << " | Loyalty Points: " << customers[idx].loyaltyPoints
                 << " | Tier: " << getMembershipTier(customers[idx].lifetimePoints) << "\n";
        }
    } else {
        string keyword;
        cout << "Enter name (or part of name): ";
        getline(cin, keyword);
        string keyLower = keyword;
        for (char &ch : keyLower) ch = tolower((unsigned char)ch);

        cout << "\n" << left << setw(6) << "ID" << setw(20) << "Name" << setw(14) << "Phone" << "Email" << "\n";
        printDivider('-', 60);

        for (const Customer &c : customers) {
            if (!c.isActive) continue;
            string nameLower = c.name;
            for (char &ch : nameLower) ch = tolower((unsigned char)ch);
            if (nameLower.find(keyLower) != string::npos) {
                displayCustomerBrief(c);
                found = true;
            }
        }
    }

    if (!found) cout << "\nNo matching customer found.\n";
    pauseScreen();
}

// -------------------------------------------------------------------------
// updateCustomer - find the record, then let staff pick which field(s) to
// change through a repeating sub-menu (nested selection + repetition).
// -------------------------------------------------------------------------
void updateCustomer(vector<Customer> &customers) {
    clearScreen();
    printHeader("UPDATE CUSTOMER");
    cout << left << setw(6) << "ID" << setw(20) << "Name" << setw(14) << "Phone" << "\n";
    printDivider('-', 40);
    for (const Customer &c : customers)
        if (c.isActive) cout << left << setw(6) << c.customerID << setw(20) << c.name << setw(14) << c.phone << "\n";
    printDivider('-', 40);
    int id = getValidatedInt("\nEnter Customer ID to update (or 0 to cancel): ", 0, 999999);
    if (id == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int idx = findCustomerIndex(customers, id);

    if (idx == -1) {
        cout << "\nCustomer not found.\n";
        pauseScreen();
        return;
    }

    int choice;
    do {
        cout << "\nCurrent -> Name: " << customers[idx].name
             << " | Phone: " << customers[idx].phone
             << " | Email: " << customers[idx].email << "\n";
        cout << "1. Update Name\n2. Update Phone\n3. Update Email\n4. Update Gender\n0. Done\n";
        choice = getValidatedInt("Choice: ", 0, 4);

        switch (choice) {
            case 1: customers[idx].name = getValidatedName("New Name: "); break;
            case 2: customers[idx].phone = getValidatedPhone("New Phone: "); break;
            case 3: customers[idx].email = getValidatedEmail("New Email: "); break;
            case 4: customers[idx].gender = getValidatedGender("New Gender (M/F/O): "); break;
            case 0: cout << "No further changes.\n"; break;
        }
    } while (choice != 0);

    cout << "\nCustomer record updated successfully.\n";
    pauseScreen();
}

// -------------------------------------------------------------------------
// deleteCustomer - SOFT delete (isActive = false). We deliberately do NOT
// erase the record from the vector: past Bookings/Invoices still reference
// this customerID, so keeping the row (just hidden from active lists)
// preserves that history instead of corrupting it.
// -------------------------------------------------------------------------
void deleteCustomer(vector<Customer> &customers) {
    clearScreen();
    printHeader("DELETE CUSTOMER");
    cout << left << setw(6) << "ID" << setw(20) << "Name" << setw(14) << "Phone" << "\n";
    printDivider('-', 40);
    for (const Customer &c : customers)
        if (c.isActive) cout << left << setw(6) << c.customerID << setw(20) << c.name << setw(14) << c.phone << "\n";
    printDivider('-', 40);
    int id = getValidatedInt("\nEnter Customer ID to delete (or 0 to cancel): ", 0, 999999);
    if (id == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int idx = findCustomerIndex(customers, id);

    if (idx == -1) {
        cout << "\nCustomer not found.\n";
        pauseScreen();
        return;
    }

    cout << "\nCustomer: " << customers[idx].name << " (ID " << customers[idx].customerID << ")\n";
    char confirm = getYesNo("Are you sure you want to delete this customer?");
    if (confirm == 'Y') {
        customers[idx].isActive = false;
        cout << "\nCustomer deleted successfully.\n";
    } else {
        cout << "\nDeletion cancelled.\n";
    }
    pauseScreen();
}

void listAllCustomers(const vector<Customer> &customers) {
    clearScreen();
    printHeader("ALL CUSTOMERS");

    ostringstream out;
    int count = 0;
    out << left << setw(6) << "ID" << setw(20) << "Name" << setw(14) << "Phone"
        << setw(24) << "Email" << setw(8) << "Points" << "Tier" << "\n";
    out << string(85, '-') << "\n";

    for (const Customer &c : customers) {
        if (!c.isActive) continue;
        out << left << setw(6) << c.customerID << setw(20) << c.name << setw(14) << c.phone
            << setw(24) << c.email << setw(8) << c.loyaltyPoints
            << getMembershipTier(c.lifetimePoints) << "\n";
        count++;
    }

    if (count == 0) out << "(No customers on file yet.)\n";
    out << string(85, '-') << "\n";
    out << "Total active customers: " << count << "\n";

    cout << out.str();
    if (count > 0 && getYesNo("\nExport customer list to file?") == 'Y')
        exportToFile("customer_list", out.str());
    pauseScreen();
}

// =========================================================================
// EXTRA FEATURE 1: Membership Tier
// Business rule (not a technical/UI feature): the more a customer spends
// over time (tracked via lifetimePoints), the higher their tier, which
// unlocks an automatic discount applied during billing - see
// billing.cpp -> generateInvoice().
// =========================================================================
string getMembershipTier(int lifetimePoints) {
    if (lifetimePoints >= GOLD_TIER_POINTS)   return "Gold";
    if (lifetimePoints >= SILVER_TIER_POINTS) return "Silver";
    return "Bronze";
}

double getTierDiscountRate(const string &tier) {
    if (tier == "Gold")   return GOLD_DISCOUNT;
    if (tier == "Silver") return SILVER_DISCOUNT;
    return 0.0;
}

// =========================================================================
// EXTRA FEATURE 2: Loyalty Points earning + redemption.
// addLoyaltyPoints() is called by billing.cpp whenever a payment is
// recorded. redeemLoyaltyPoints() is called by billing.cpp when a customer
// wants to use points for an instant discount on the current bill.
// =========================================================================
void addLoyaltyPoints(vector<Customer> &customers, int customerID, int pointsToAdd) {
    int idx = findCustomerIndex(customers, customerID);
    if (idx == -1) return;
    customers[idx].loyaltyPoints  += pointsToAdd;
    customers[idx].lifetimePoints += pointsToAdd;
}

bool redeemLoyaltyPoints(vector<Customer> &customers, int customerID, int pointsToRedeem) {
    int idx = findCustomerIndex(customers, customerID);
    if (idx == -1) return false;
    if (pointsToRedeem <= 0 || pointsToRedeem > customers[idx].loyaltyPoints) return false;
    customers[idx].loyaltyPoints -= pointsToRedeem;
    return true;
}

void loyaltyPointsMenu(vector<Customer> &customers) {
    clearScreen();
    printHeader("LOYALTY POINTS & MEMBERSHIP TIER");
    cout << left << setw(6) << "ID" << setw(20) << "Name" << setw(8) << "Points" << "Tier" << "\n";
    printDivider('-', 50);
    for (const Customer &c : customers)
        if (c.isActive) cout << left << setw(6) << c.customerID << setw(20) << c.name << setw(8) << c.loyaltyPoints << getMembershipTier(c.lifetimePoints) << "\n";
    printDivider('-', 50);
    int id = getValidatedInt("\nEnter your Customer ID (or 0 to cancel): ", 0, 999999);
    if (id == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int idx = findCustomerIndex(customers, id);

    if (idx == -1) {
        cout << "\nCustomer not found.\n";
        pauseScreen();
        return;
    }

    string tier = getMembershipTier(customers[idx].lifetimePoints);
    cout << "\nCustomer: " << customers[idx].name << "\n";
    cout << "Current redeemable points: " << customers[idx].loyaltyPoints << "\n";
    cout << "Lifetime points earned   : " << customers[idx].lifetimePoints << "\n";
    cout << "Membership Tier          : " << tier
         << " (" << (int)(getTierDiscountRate(tier) * 100) << "% automatic discount on every bill)\n";

    cout << "\n1. Redeem Points for a Discount Voucher\n0. Back\n";
    int choice = getValidatedInt("Choice: ", 0, 1);

    if (choice == 1) {
        if (customers[idx].loyaltyPoints < POINTS_PER_RM_REDEEM) {
            cout << "\nYou need at least " << POINTS_PER_RM_REDEEM << " points to redeem. Keep earning!\n";
        } else {
            int maxRedeemable = customers[idx].loyaltyPoints;
            int pts = getValidatedInt("Points to redeem (up to " + to_string(maxRedeemable) + "): ",
                                       0, maxRedeemable);
            pts -= (pts % POINTS_PER_RM_REDEEM); // round down to nearest valid multiple of 10
            if (pts < POINTS_PER_RM_REDEEM) {
                cout << "\nNot enough points selected to redeem (minimum " << POINTS_PER_RM_REDEEM << ").\n";
            } else {
                redeemLoyaltyPoints(customers, id, pts);
                double voucherValue = (double)pts / POINTS_PER_RM_REDEEM;
                cout << fixed << setprecision(2);
                cout << "\nRedeemed " << pts << " points for a RM" << voucherValue
                     << " voucher. Mention this at billing to apply it.\n";
            }
        }
    }
    pauseScreen();
}

// -------------------------------------------------------------------------
// customerManagementMenu - Module 1's entry point, called from main.cpp.
// -------------------------------------------------------------------------
void customerManagementMenu(vector<Customer> &customers, int &nextCustomerID) {
    int choice;
    do {
        clearScreen();
        printHeader("CUSTOMER MANAGEMENT");
        cout << "1. Add New Customer\n";
        cout << "2. Search Customer\n";
        cout << "3. Update Customer\n";
        cout << "4. Delete Customer\n";
        cout << "5. List All Customers\n";
        cout << "6. Loyalty Points & Membership Tier\n";
        cout << "0. Back to Main Menu\n";
        choice = getValidatedInt("Enter your choice: ", 0, 6);

        switch (choice) {
            case 1: addCustomer(customers, nextCustomerID); break;
            case 2: searchCustomer(customers); break;
            case 3: updateCustomer(customers); break;
            case 4: deleteCustomer(customers); break;
            case 5: listAllCustomers(customers); break;
            case 6: loyaltyPointsMenu(customers); break;
            case 0: break;
        }
    } while (choice != 0);
}

// =========================================================================
// IMPLEMENTATIONS FROM stylist_service.cpp
// =========================================================================

/* =========================================================================
   STYLIST_SERVICE.CPP  --  MODULE 2: Service & Stylist Management
   (Author: Jeremy)
   ========================================================================= */

// ---------------------- Shared lookup helpers ----------------------
int findStylistIndex(const vector<Stylist> &stylists, int stylistID) {
    for (int i = 0; i < (int)stylists.size(); i++)
        if (stylists[i].stylistID == stylistID && stylists[i].isActive) return i;
    return -1;
}

int findServiceIndex(const vector<Service> &services, int serviceID) {
    for (int i = 0; i < (int)services.size(); i++)
        if (services[i].serviceID == serviceID && services[i].isActive) return i;
    return -1;
}

string getServiceName(const vector<Service> &services, int serviceID) {
    int idx = findServiceIndex(services, serviceID);
    return (idx == -1) ? "Unknown Service" : services[idx].serviceName;
}

double getServicePrice(const vector<Service> &services, int serviceID) {
    int idx = findServiceIndex(services, serviceID);
    return (idx == -1) ? 0.0 : services[idx].price;
}

bool canStylistDoService(const Stylist &stylist, const string &serviceCategory) {
    if (serviceCategory == "Hair")
        return stylist.specialization == "Hair Styling" || stylist.specialization == "Colouring";
    if (serviceCategory == "Nail")
        return stylist.specialization == "Nail Care";
    if (serviceCategory == "Spa")
        return stylist.specialization == "Spa & Massage";
    if (serviceCategory == "Makeup")
        return stylist.specialization == "Makeup";
    return true;
}

// ============================ STYLIST FUNCTIONS ============================
string chooseSpecialization() {
    cout << "Specialization:\n  1. Hair Styling\n  2. Colouring\n  3. Nail Care\n  4. Spa & Massage\n  5. Makeup\n";
    int c = getValidatedInt("Choice: ", 1, 5);
    switch (c) {
        case 1: return "Hair Styling";
        case 2: return "Colouring";
        case 3: return "Nail Care";
        case 4: return "Spa & Massage";
        default: return "Makeup";
    }
}

bool isDuplicateStylistPhone(const vector<Stylist> &stylists, const string &phone) {
    for (const Stylist &s : stylists) {
        if (s.isActive && s.phone == phone) return true;
    }
    return false;
}

void addStylist(vector<Stylist> &stylists, int &nextStylistID) {
    clearScreen();
    printHeader("ADD NEW STYLIST");

    Stylist s;
    s.stylistID = nextStylistID;
    s.name = getValidatedName("Stylist Name (or 0 to cancel): ", true);
    if (s.name.empty()) { cout << "\nCancelled.\n"; pauseScreen(); return; }

    string phone;
    bool duplicate;
    do {
        phone = getValidatedPhone("Phone Number (or 0 to cancel): ", true);
        if (phone.empty()) { cout << "\nCancelled.\n"; pauseScreen(); return; }
        duplicate = isDuplicateStylistPhone(stylists, phone);
        if (duplicate) cout << "A stylist with this phone number is already registered.\n";
    } while (duplicate);
    s.phone = phone;

    s.specialization = chooseSpecialization();
    s.isAvailable = true;
    s.isActive = true;

    stylists.push_back(s);
    nextStylistID++;
    cout << "\nStylist added successfully! Assigned Stylist ID: " << s.stylistID << "\n";
    pauseScreen();
}

void updateStylist(vector<Stylist> &stylists) {
    clearScreen();
    printHeader("UPDATE STYLIST");
    cout << left << setw(6) << "ID" << setw(20) << "Name" << setw(18) << "Specialization" << "\n";
    printDivider('-', 45);
    for (const Stylist &s : stylists)
        if (s.isActive) cout << left << setw(6) << s.stylistID << setw(20) << s.name << setw(18) << s.specialization << "\n";
    printDivider('-', 45);
    int id = getValidatedInt("\nEnter Stylist ID to update (or 0 to cancel): ", 0, 999999);
    if (id == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int idx = findStylistIndex(stylists, id);
    if (idx == -1) { cout << "\nStylist not found.\n"; pauseScreen(); return; }

    int choice;
    do {
        cout << "\nCurrent -> Name: " << stylists[idx].name << " | Phone: " << stylists[idx].phone
             << " | Specialization: " << stylists[idx].specialization
             << " | Available: " << (stylists[idx].isAvailable ? "Yes" : "No") << "\n";
        cout << "1. Update Name\n2. Update Phone\n3. Update Specialization\n4. Toggle Availability\n0. Done\n";
        choice = getValidatedInt("Choice: ", 0, 4);

        switch (choice) {
            case 1: stylists[idx].name = getValidatedName("New Name: "); break;
            case 2: stylists[idx].phone = getValidatedPhone("New Phone: "); break;
            case 3: stylists[idx].specialization = chooseSpecialization(); break;
            case 4: stylists[idx].isAvailable = !stylists[idx].isAvailable; break;
            case 0: break;
        }
    } while (choice != 0);

    cout << "\nStylist record updated successfully.\n";
    pauseScreen();
}

void deleteStylist(vector<Stylist> &stylists) {
    clearScreen();
    printHeader("DELETE STYLIST");
    cout << left << setw(6) << "ID" << setw(20) << "Name" << setw(18) << "Specialization" << "\n";
    printDivider('-', 45);
    for (const Stylist &s : stylists)
        if (s.isActive) cout << left << setw(6) << s.stylistID << setw(20) << s.name << setw(18) << s.specialization << "\n";
    printDivider('-', 45);
    int id = getValidatedInt("\nEnter Stylist ID to delete (or 0 to cancel): ", 0, 999999);
    if (id == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int idx = findStylistIndex(stylists, id);
    if (idx == -1) { cout << "\nStylist not found.\n"; pauseScreen(); return; }

    cout << "\nStylist: " << stylists[idx].name << " (ID " << stylists[idx].stylistID << ")\n";
    char confirm = getYesNo("Are you sure you want to delete this stylist?");
    if (confirm == 'Y') {
        stylists[idx].isActive = false;
        cout << "\nStylist deleted successfully.\n";
    } else {
        cout << "\nDeletion cancelled.\n";
    }
    pauseScreen();
}

void viewStylists(const vector<Stylist> &stylists) {
    clearScreen();
    printHeader("ALL STYLISTS");
    ostringstream out;
    int count = 0;
    out << left << setw(6) << "ID" << setw(20) << "Name" << setw(14) << "Phone"
        << setw(18) << "Specialization" << "Available\n";
    out << string(70, '-') << "\n";
    for (const Stylist &s : stylists) {
        if (!s.isActive) continue;
        out << left << setw(6) << s.stylistID << setw(20) << s.name << setw(14) << s.phone
            << setw(18) << s.specialization << (s.isAvailable ? "Yes" : "No") << "\n";
        count++;
    }
    if (count == 0) out << "(No stylists on file yet.)\n";
    out << string(70, '-') << "\n";
    out << "Total active stylists: " << count << "\n";

    cout << out.str();
    if (count > 0 && getYesNo("\nExport stylist list to file?") == 'Y')
        exportToFile("stylist_list", out.str());
    pauseScreen();
}

// ============================ SERVICE FUNCTIONS ============================
string chooseCategory() {
    cout << "Category:\n  1. Hair\n  2. Nail\n  3. Spa\n  4. Makeup\n";
    int c = getValidatedInt("Choice: ", 1, 4);
    switch (c) {
        case 1: return "Hair";
        case 2: return "Nail";
        case 3: return "Spa";
        default: return "Makeup";
    }
}

bool isDuplicateServiceName(const vector<Service> &services, const string &name) {
    for (const Service &s : services) {
        if (s.isActive && s.serviceName == name) return true;
    }
    return false;
}

void addService(vector<Service> &services, int &nextServiceID) {
    clearScreen();
    printHeader("ADD NEW SERVICE");

    Service sv;
    sv.serviceID = nextServiceID;

    string name;
    bool duplicate;
    do {
        name = getValidatedName("Service Name (or 0 to cancel): ", true);
        if (name.empty()) { cout << "\nCancelled.\n"; pauseScreen(); return; }
        duplicate = isDuplicateServiceName(services, name);
        if (duplicate) cout << "A service with this name already exists.\n";
    } while (duplicate);
    sv.serviceName = name;

    sv.category = chooseCategory();
    sv.price    = getValidatedDouble("Price (RM, or 0 to cancel): ", 0.0, 10000.0);
    if (sv.price == 0.0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    sv.duration = getValidatedInt("Duration (minutes, or 0 to cancel): ", 0, 480);
    if (sv.duration == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    sv.isActive = true;

    services.push_back(sv);
    nextServiceID++;
    cout << "\nService added successfully! Assigned Service ID: " << sv.serviceID << "\n";
    pauseScreen();
}

void updateService(vector<Service> &services) {
    clearScreen();
    printHeader("UPDATE SERVICE");
    cout << fixed << setprecision(2);
    cout << left << setw(6) << "ID" << setw(22) << "Service Name" << setw(8) << "Price" << "\n";
    printDivider('-', 40);
    for (const Service &sv : services)
        if (sv.isActive) cout << left << setw(6) << sv.serviceID << setw(22) << sv.serviceName << "RM" << sv.price << "\n";
    printDivider('-', 40);
    int id = getValidatedInt("\nEnter Service ID to update (or 0 to cancel): ", 0, 999999);
    if (id == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int idx = findServiceIndex(services, id);
    if (idx == -1) { cout << "\nService not found.\n"; pauseScreen(); return; }

    int choice;
    do {
        cout << fixed << setprecision(2);
        cout << "\nCurrent -> Name: " << services[idx].serviceName << " | Category: " << services[idx].category
             << " | Price: RM" << services[idx].price << " | Duration: " << services[idx].duration << " min\n";
        cout << "1. Update Name\n2. Update Category\n3. Update Price\n4. Update Duration\n0. Done\n";
        choice = getValidatedInt("Choice: ", 0, 4);

        switch (choice) {
            case 1: services[idx].serviceName = getValidatedName("New Name: "); break;
            case 2: services[idx].category = chooseCategory(); break;
            case 3: services[idx].price = getValidatedDouble("New Price (RM): ", 1.0, 10000.0); break;
            case 4: services[idx].duration = getValidatedInt("New Duration (minutes): ", 5, 480); break;
            case 0: break;
        }
    } while (choice != 0);

    cout << "\nService record updated successfully.\n";
    pauseScreen();
}

void deleteService(vector<Service> &services) {
    clearScreen();
    printHeader("DELETE SERVICE");
    cout << fixed << setprecision(2);
    cout << left << setw(6) << "ID" << setw(22) << "Service Name" << setw(8) << "Price" << "\n";
    printDivider('-', 40);
    for (const Service &sv : services)
        if (sv.isActive) cout << left << setw(6) << sv.serviceID << setw(22) << sv.serviceName << "RM" << sv.price << "\n";
    printDivider('-', 40);
    int id = getValidatedInt("\nEnter Service ID to delete (or 0 to cancel): ", 0, 999999);
    if (id == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int idx = findServiceIndex(services, id);
    if (idx == -1) { cout << "\nService not found.\n"; pauseScreen(); return; }

    cout << "\nService: " << services[idx].serviceName << " (ID " << services[idx].serviceID << ")\n";
    char confirm = getYesNo("Are you sure you want to delete this service?");
    if (confirm == 'Y') {
        services[idx].isActive = false;
        cout << "\nService deleted successfully.\n";
    } else {
        cout << "\nDeletion cancelled.\n";
    }
    pauseScreen();
}

void viewServices(const vector<Service> &services) {
    clearScreen();
    printHeader("ALL SERVICES");
    ostringstream out;
    int count = 0;
    out << fixed << setprecision(2);
    out << left << setw(6) << "ID" << setw(22) << "Service Name" << setw(10) << "Category"
        << setw(10) << "Price" << "Duration\n";
    out << string(65, '-') << "\n";
    for (const Service &sv : services) {
        if (!sv.isActive) continue;
        out << left << setw(6) << sv.serviceID << setw(22) << sv.serviceName << setw(10) << sv.category
            << "RM" << setw(8) << sv.price << sv.duration << " min\n";
        count++;
    }
    if (count == 0) out << "(No services on file yet.)\n";
    out << string(65, '-') << "\n";
    out << "Total active services: " << count << "\n";

    cout << out.str();
    if (count > 0 && getYesNo("\nExport service list to file?") == 'Y')
        exportToFile("service_list", out.str());
    pauseScreen();
}

// -------------------------------------------------------------------------
// serviceStylistMenu - Module 2's entry point, called from main.cpp.
// -------------------------------------------------------------------------
void serviceStylistMenu(vector<Stylist> &stylists, vector<Service> &services,
                         int &nextStylistID, int &nextServiceID) {
    int choice;
    do {
        clearScreen();
        printHeader("SERVICE & STYLIST MANAGEMENT");
        cout << "1. Stylist Management\n";
        cout << "2. Service Management\n";
        cout << "0. Back to Main Menu\n";
        choice = getValidatedInt("Enter your choice: ", 0, 2);

        if (choice == 1) {
            int styChoice;
            do {
                clearScreen();
                printHeader("STYLIST MANAGEMENT");
                cout << "1. Add Stylist\n";
                cout << "2. Update Stylist\n";
                cout << "3. Delete Stylist\n";
                cout << "4. View Stylists\n";
                cout << "0. Back\n";
                styChoice = getValidatedInt("Enter your choice: ", 0, 4);
                switch (styChoice) {
                    case 1: addStylist(stylists, nextStylistID); break;
                    case 2: updateStylist(stylists); break;
                    case 3: deleteStylist(stylists); break;
                    case 4: viewStylists(stylists); break;
                    case 0: break;
                }
            } while (styChoice != 0);
        } else if (choice == 2) {
            int svcChoice;
            do {
                clearScreen();
                printHeader("SERVICE MANAGEMENT");
                cout << "1. Add Service\n";
                cout << "2. Update Service\n";
                cout << "3. Delete Service\n";
                cout << "4. View Services\n";
                cout << "0. Back\n";
                svcChoice = getValidatedInt("Enter your choice: ", 0, 4);
                switch (svcChoice) {
                    case 1: addService(services, nextServiceID); break;
                    case 2: updateService(services); break;
                    case 3: deleteService(services); break;
                    case 4: viewServices(services); break;
                    case 0: break;
                }
            } while (svcChoice != 0);
        }
    } while (choice != 0);
}

// =========================================================================
// IMPLEMENTATIONS FROM booking.cpp
// =========================================================================

/* =========================================================================
   BOOKING.CPP  --  MODULE 3: Booking Management   (Author: Michael)
   ========================================================================= */

const int MAX_SCHEDULE_STYLISTS = 30; // upper bound for the schedule grid below

int findBookingIndex(const vector<Booking> &bookings, int bookingID) {
    for (int i = 0; i < (int)bookings.size(); i++)
        if (bookings[i].bookingID == bookingID) return i;
    return -1;
}

bool isTimeSlotTaken(const vector<Booking> &bookings, int stylistID, const string &date,
                      const string &timeSlot, int excludeBookingID) {
    for (const Booking &b : bookings) {
        if (b.bookingID == excludeBookingID) continue; // ignore itself when updating
        if (b.status == "Cancelled") continue;          // cancelled slots are free again
        if (b.stylistID == stylistID && b.date == date && b.timeSlot == timeSlot) {
            return true;
        }
    }
    return false;
}

int chooseTimeSlot() {
    cout << "Available time slots:\n";
    for (int i = 0; i < NUM_TIME_SLOTS; i++) {
        cout << "  " << (i + 1) << ". " << TIME_SLOTS[i] << "\n";
    }
    int c = getValidatedInt("Choose a time slot: ", 1, NUM_TIME_SLOTS);
    return c - 1; // index into TIME_SLOTS
}

// -------------------------------------------------------------------------
// bookAppointment - validates the customer, stylist and up to 3 services
// against the OTHER modules' data (cross-module data sharing), then checks
// for a scheduling conflict before saving.
// -------------------------------------------------------------------------
void bookAppointment(vector<Booking> &bookings, int &nextBookingID,
                      const vector<Customer> &customers,
                      const vector<Stylist> &stylists,
                      const vector<Service> &services) {
    clearScreen();
    printHeader("BOOK APPOINTMENT");

    // ---- Step 1: Service(s) ----
    cout << "--- Active Services ---\n";
    for (const Service &sv : services) {
        if (sv.isActive) {
            cout << "  " << sv.serviceID << ". " << sv.serviceName
                 << " (RM" << fixed << setprecision(2) << sv.price << ", " << sv.duration << " min)\n";
        }
    }

    Booking b;
    b.numServices = 0;
    char more = 'Y';
    while (more == 'Y' && b.numServices < MAX_SERVICES_PER_BOOKING) {
        int svID = getValidatedInt("Enter Service ID to add (or 0 to finish): ", 0, 999999);
        if (svID == 0) break;
        int svIdx = findServiceIndex(services, svID);
        if (svIdx == -1) {
            cout << "Service not found. Try again.\n";
            continue;
        }
        b.serviceIDs[b.numServices] = svID;
        b.numServices++;
        cout << "Added: " << services[svIdx].serviceName << "\n";

        if (b.numServices < MAX_SERVICES_PER_BOOKING) {
            more = getYesNo("Add another service to this booking?");
        }
    }
    if (b.numServices == 0) {
        cout << "\nNo service selected. Booking cancelled.\n";
        pauseScreen();
        return;
    }

    // ---- Step 2: Customer ----
    cout << "\n--- Customers ---\n";
    cout << left << setw(6) << "ID" << setw(20) << "Name" << setw(14) << "Phone" << "\n";
    printDivider('-', 40);
    for (const Customer &c : customers)
        if (c.isActive) cout << left << setw(6) << c.customerID << setw(20) << c.name << setw(14) << c.phone << "\n";
    printDivider('-', 40);
    int custID = getValidatedInt("\nEnter Customer ID (or 0 to cancel): ", 0, 999999);
    if (custID == 0) { cout << "\nBooking cancelled.\n"; pauseScreen(); return; }
    if (findCustomerIndex(customers, custID) == -1) {
        cout << "\nCustomer not found. Please add the customer first (Customer Management).\n";
        pauseScreen();
        return;
    }

    // ---- Step 3: Stylist ----
    cout << "\n--- Available Stylists ---\n";
    int availCount = 0;
    for (const Stylist &s : stylists) {
        if (s.isActive && s.isAvailable) {
            cout << "  " << s.stylistID << ". " << s.name << " (" << s.specialization << ")\n";
            availCount++;
        }
    }
    if (availCount == 0) {
        cout << "No stylists are currently available. Please try again later.\n";
        pauseScreen();
        return;
    }

    int stylistID = getValidatedInt("Enter Stylist ID (or 0 for auto-assign): ", 0, 999999);
    int sIdx;
    if (stylistID == 0) {
        for (const Stylist &s : stylists) {
            if (s.isActive && s.isAvailable) {
                stylistID = s.stylistID;
                break;
            }
        }
        sIdx = findStylistIndex(stylists, stylistID);
        cout << "\nAuto-assigned: " << stylists[sIdx].name << "\n";
    } else {
        sIdx = findStylistIndex(stylists, stylistID);
        if (sIdx == -1 || !stylists[sIdx].isAvailable) {
            cout << "\nInvalid or unavailable stylist.\n";
            pauseScreen();
            return;
        }
    }

    // ---- Step 4: Date & Time ----
    string date = getValidatedDate("Appointment Date ", true);
    int slotIdx = chooseTimeSlot();
    string timeSlot = TIME_SLOTS[slotIdx];

    if (isTimeSlotTaken(bookings, stylistID, date, timeSlot)) {
        cout << "\nSorry, " << stylists[sIdx].name << " is already booked on " << date
             << " at " << timeSlot << ". Please choose a different slot or stylist.\n";
        pauseScreen();
        return;
    }

    // ---- Step 5: Save booking ----
    b.bookingID  = nextBookingID++;
    b.customerID = custID;
    b.stylistID  = stylistID;
    b.date       = date;
    b.timeSlot   = timeSlot;
    b.status     = "Confirmed";
    bookings.push_back(b);

    // ---- Booking confirmation slip (required output) ----
    cout << "\n";
    printDivider('=');
    cout << "              BOOKING CONFIRMATION\n";
    printDivider('=');
    cout << "Booking ID   : " << b.bookingID << "\n";
    cout << "Customer ID  : " << b.customerID << "\n";
    cout << "Stylist      : " << stylists[sIdx].name << " (" << stylists[sIdx].specialization << ")\n";
    cout << "Service(s)   : ";
    double totalPrice = 0; int totalDuration = 0;
    for (int i = 0; i < b.numServices; i++) {
        int svIdx = findServiceIndex(services, b.serviceIDs[i]);
        cout << services[svIdx].serviceName;
        totalPrice += services[svIdx].price;
        totalDuration += services[svIdx].duration;
        if (i < b.numServices - 1) cout << ", ";
    }
    cout << "\nDate & Time  : " << b.date << " " << b.timeSlot << "\n";
    cout << "Duration     : " << totalDuration << " min";
    if (totalDuration >= 60) cout << " (~" << fixed << setprecision(1) << (totalDuration / 60.0) << " hrs)";
    cout << "\n";
    cout << fixed << setprecision(2);
    cout << "Est. Cost    : RM" << totalPrice << " (before tax/discount)\n";
    cout << "Status       : " << b.status << "\n";
    printDivider('=');
    pauseScreen();
}

void viewSearchAppointment(const vector<Booking> &bookings, const vector<Customer> &customers) {
    clearScreen();
    printHeader("VIEW / SEARCH APPOINTMENTS");
    cout << "1. View All Appointments\n2. Search by Booking ID\n3. Search by Customer ID\n4. Search by Date\n";
    int mode = getValidatedInt("Choice: ", 1, 4);

    int filterID = -1;
    string filterDate;
    if (mode == 2) {
        filterID = getValidatedInt("Enter Booking ID (or 0 to cancel): ", 0, 999999);
        if (filterID == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    } else if (mode == 3) {
        filterID = getValidatedInt("Enter Customer ID (or 0 to cancel): ", 0, 999999);
        if (filterID == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    } else if (mode == 4) filterDate = getValidatedDate("Enter Date ", false);

    int count = 0;
    for (const Booking &b : bookings) {
        bool match = false;
        switch (mode) {
            case 1: match = true; break;
            case 2: match = (b.bookingID == filterID); break;
            case 3: match = (b.customerID == filterID); break;
            case 4: match = (b.date == filterDate); break;
        }
        if (match) {
            if (count == 0) {
                cout << "\n" << left << setw(6) << "BkID" << setw(6) << "CustID" << setw(6) << "StyID"
                     << setw(12) << "Date" << setw(8) << "Time" << "Status\n";
                printDivider('-', 55);
            }
            cout << left << setw(6) << b.bookingID << setw(6) << b.customerID << setw(6) << b.stylistID
                 << setw(12) << b.date << setw(8) << b.timeSlot << b.status << "\n";
            count++;
        }
    }
    if (count == 0) cout << "\nNo matching appointment(s) found.\n";
    else printDivider('-', 55);
    cout << "Total appointment(s): " << count << "\n";
    pauseScreen();
    (void)customers; // reserved for a future "show customer name" enhancement
}

void viewBookingDetail(const vector<Booking> &bookings, const vector<Customer> &customers,
                        const vector<Stylist> &stylists, const vector<Service> &services) {
    clearScreen();
    printHeader("BOOKING DETAIL");
    int id = getValidatedInt("Enter Booking ID (or 0 to cancel): ", 0, 999999);
    if (id == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int idx = findBookingIndex(bookings, id);
    if (idx == -1) { cout << "\nBooking not found.\n"; pauseScreen(); return; }

    const Booking &b = bookings[idx];
    int cIdx = findCustomerIndex(customers, b.customerID);
    int sIdx = findStylistIndex(stylists, b.stylistID);

    cout << fixed << setprecision(2);
    cout << "\n";
    printDivider('=', 45);
    cout << "               BOOKING DETAILS\n";
    printDivider('=', 45);
    cout << left << setw(18) << "Booking ID:" << b.bookingID << "\n";
    cout << left << setw(18) << "Status:" << b.status << "\n";
    cout << left << setw(18) << "Customer:"
         << (cIdx != -1 ? customers[cIdx].name : "ID " + to_string(b.customerID)) << "\n";
    if (cIdx != -1) cout << left << setw(18) << "  Phone:" << customers[cIdx].phone << "\n";
    cout << left << setw(18) << "Stylist:"
         << (sIdx != -1 ? stylists[sIdx].name : "ID " + to_string(b.stylistID)) << "\n";
    if (sIdx != -1) cout << left << setw(18) << "  Specialization:" << stylists[sIdx].specialization << "\n";
    cout << left << setw(18) << "Date & Time:" << b.date << " " << b.timeSlot << "\n";
    printDivider('-', 45);
    cout << "Services:\n";
    double totalPrice = 0; int totalDuration = 0;
    for (int i = 0; i < b.numServices; i++) {
        int svIdx = findServiceIndex(services, b.serviceIDs[i]);
        string name = (svIdx != -1) ? services[svIdx].serviceName : "Unknown";
        double pr = (svIdx != -1) ? services[svIdx].price : 0;
        int dur = (svIdx != -1) ? services[svIdx].duration : 0;
        cout << "  " << (i + 1) << ". " << left << setw(26) << name
             << "RM" << setw(8) << pr << dur << " min\n";
        totalPrice += pr;
        totalDuration += dur;
    }
    printDivider('-', 45);
    cout << left << setw(26) << "Estimated Cost:" << "RM" << totalPrice << "\n";
    cout << left << setw(26) << "Total Duration:" << totalDuration << " min (~"
         << fixed << setprecision(1) << (totalDuration / 60.0) << " hrs)\n";
    printDivider('=', 45);
    pauseScreen();
}

void cancelAppointment(vector<Booking> &bookings) {
    clearScreen();
    printHeader("CANCEL APPOINTMENT");
    cout << left << setw(6) << "BkID" << setw(6) << "Cust" << setw(6) << "Sty" << setw(12) << "Date" << setw(8) << "Time" << "Status\n";
    printDivider('-', 50);
    for (const Booking &b : bookings)
        if (b.status == "Confirmed") cout << left << setw(6) << b.bookingID << setw(6) << b.customerID << setw(6) << b.stylistID << setw(12) << b.date << setw(8) << b.timeSlot << b.status << "\n";
    printDivider('-', 50);
    int id = getValidatedInt("\nEnter Booking ID to cancel (or 0 to cancel): ", 0, 999999);
    if (id == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int idx = findBookingIndex(bookings, id);

    if (idx == -1) { cout << "\nBooking not found.\n"; pauseScreen(); return; }
    if (bookings[idx].status != "Confirmed") {
        cout << "\nThis booking is already " << bookings[idx].status << " and cannot be cancelled.\n";
        pauseScreen();
        return;
    }

    cout << "\nBooking " << id << " on " << bookings[idx].date << " " << bookings[idx].timeSlot << "\n";
    char confirm = getYesNo("Are you sure you want to cancel this appointment?");
    if (confirm == 'Y') {
        bookings[idx].status = "Cancelled";
        cout << "\nAppointment cancelled. The time slot is now free for other customers.\n";
    } else {
        cout << "\nNo changes made.\n";
    }
    pauseScreen();
}

void updateAppointment(vector<Booking> &bookings, const vector<Stylist> &stylists, const vector<Service> &services) {
    clearScreen();
    printHeader("UPDATE APPOINTMENT");
    cout << left << setw(6) << "BkID" << setw(6) << "Cust" << setw(6) << "Sty" << setw(12) << "Date" << setw(8) << "Time" << "Status\n";
    printDivider('-', 50);
    for (const Booking &b : bookings)
        if (b.status == "Confirmed") cout << left << setw(6) << b.bookingID << setw(6) << b.customerID << setw(6) << b.stylistID << setw(12) << b.date << setw(8) << b.timeSlot << b.status << "\n";
    printDivider('-', 50);
    int id = getValidatedInt("\nEnter Booking ID to update (or 0 to cancel): ", 0, 999999);
    if (id == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int idx = findBookingIndex(bookings, id);

    if (idx == -1) { cout << "\nBooking not found.\n"; pauseScreen(); return; }
    if (bookings[idx].status != "Confirmed") {
        cout << "\nOnly Confirmed bookings can be updated (this one is " << bookings[idx].status << ").\n";
        pauseScreen();
        return;
    }

    int choice;
    do {
        cout << "\nCurrent -> Date: " << bookings[idx].date << " | Time: " << bookings[idx].timeSlot
             << " | Stylist ID: " << bookings[idx].stylistID << "\n";
        cout << "1. Change Date/Time\n2. Change Stylist\n0. Done\n";
        choice = getValidatedInt("Choice: ", 0, 2);

        if (choice == 1) {
            string newDate = getValidatedDate("New Appointment Date ", true);
            int slotIdx = chooseTimeSlot();
            string newSlot = TIME_SLOTS[slotIdx];
            if (isTimeSlotTaken(bookings, bookings[idx].stylistID, newDate, newSlot, bookings[idx].bookingID)) {
                cout << "\nThat stylist is already booked at that date/time. No changes made.\n";
            } else {
                bookings[idx].date = newDate;
                bookings[idx].timeSlot = newSlot;
                cout << "\nDate/Time updated.\n";
            }
        } else if (choice == 2) {
            cout << "\n--- Available Stylists ---\n";
            for (const Stylist &s : stylists) {
                if (s.isActive && s.isAvailable) cout << "  " << s.stylistID << ". " << s.name << "\n";
            }
            int newStylist = getValidatedInt("New Stylist ID: ", 1, 999999);
            if (findStylistIndex(stylists, newStylist) == -1) {
                cout << "\nInvalid stylist.\n";
            } else if (isTimeSlotTaken(bookings, newStylist, bookings[idx].date, bookings[idx].timeSlot,
                                        bookings[idx].bookingID)) {
                cout << "\nThat stylist is already booked at this date/time. No changes made.\n";
            } else {
                bookings[idx].stylistID = newStylist;
                cout << "\nStylist updated.\n";
            }
        }
    } while (choice != 0);

    (void)services; // kept in the signature for symmetry / future service-change support
    pauseScreen();
}

// -------------------------------------------------------------------------
// viewTodaySchedule - EXTRA demonstration of a 2D ARRAY: rows = active
// stylists, columns = time slots. Each cell shows "Free" or the booked
// customer's name, built fresh each time from the bookings vector.
// -------------------------------------------------------------------------
void viewTodaySchedule(const vector<Booking> &bookings, const vector<Stylist> &stylists, const vector<Customer> &customers) {
    clearScreen();
    printHeader("TODAY'S SCHEDULE");
    string today = getTodayDate();

    string grid[MAX_SCHEDULE_STYLISTS][NUM_TIME_SLOTS];
    int stylistRow[MAX_SCHEDULE_STYLISTS];
    int numRows = 0;

    for (int i = 0; i < (int)stylists.size() && numRows < MAX_SCHEDULE_STYLISTS; i++) {
        if (!stylists[i].isActive) continue;
        stylistRow[numRows] = i;
        for (int t = 0; t < NUM_TIME_SLOTS; t++) grid[numRows][t] = "Free";
        numRows++;
    }

    for (const Booking &b : bookings) {
        if (b.status != "Confirmed" || b.date != today) continue;
        for (int r = 0; r < numRows; r++) {
            if (stylists[stylistRow[r]].stylistID != b.stylistID) continue;
            for (int t = 0; t < NUM_TIME_SLOTS; t++) {
                if (TIME_SLOTS[t] == b.timeSlot) {
                    int cIdx = findCustomerIndex(customers, b.customerID);
                    string cn = (cIdx == -1) ? ("Cust#" + to_string(b.customerID)) : customers[cIdx].name;
                    grid[r][t] = truncate(cn, 10);
                }
            }
        }
    }

    if (numRows == 0) {
        cout << "No active stylists on file.\n";
        pauseScreen();
        return;
    }

    ostringstream out;
    out << "Date: " << today << "\n\n";
    out << left << setw(16) << "Stylist";
    for (int t = 0; t < NUM_TIME_SLOTS; t++) out << setw(12) << TIME_SLOTS[t];
    out << "\n" << string(16 + 12 * NUM_TIME_SLOTS, '-') << "\n";

    for (int r = 0; r < numRows; r++) {
        out << left << setw(16) << truncate(stylists[stylistRow[r]].name, 14);
        for (int t = 0; t < NUM_TIME_SLOTS; t++) out << setw(12) << grid[r][t];
        out << "\n";
    }
    out << string(16 + 12 * NUM_TIME_SLOTS, '-') << "\n";

    cout << out.str();
    if (getYesNo("\nExport today's schedule to file?") == 'Y')
        exportToFile("schedule", out.str());
    pauseScreen();
}

// -------------------------------------------------------------------------
// bookingManagementMenu - Module 3's entry point, called from main.cpp.
// -------------------------------------------------------------------------
void bookingManagementMenu(vector<Booking> &bookings, int &nextBookingID,
                            const vector<Customer> &customers,
                            const vector<Stylist> &stylists,
                            const vector<Service> &services) {
    int choice;
    do {
        clearScreen();
        printHeader("BOOKING MANAGEMENT");
        cout << "1. Book Appointment\n";
        cout << "2. View / Search Appointments\n";
        cout << "3. Booking Detail\n";
        cout << "4. Cancel Appointment\n";
        cout << "5. Update Appointment\n";
        cout << "6. View Today's Schedule\n";
        cout << "0. Back to Main Menu\n";

        string today = getTodayDate();
        int confirmed = 0, completed = 0, cancelled = 0;
        for (const Booking &b : bookings) {
            if (b.date == today && b.status == "Confirmed") confirmed++;
            if (b.date == today && b.status == "Completed") completed++;
            if (b.date == today && b.status == "Cancelled") cancelled++;
        }
        cout << "\n  Today (" << today << "): "
             << confirmed << " Confirmed  |  " << completed << " Completed  |  "
             << cancelled << " Cancelled\n";

        choice = getValidatedInt("Enter your choice: ", 0, 6);

        switch (choice) {
            case 1: bookAppointment(bookings, nextBookingID, customers, stylists, services); break;
            case 2: viewSearchAppointment(bookings, customers); break;
            case 3: viewBookingDetail(bookings, customers, stylists, services); break;
            case 4: cancelAppointment(bookings); break;
            case 5: updateAppointment(bookings, stylists, services); break;
            case 6: viewTodaySchedule(bookings, stylists, customers); break;
            case 0: break;
        }
    } while (choice != 0);
}

// =========================================================================
// IMPLEMENTATIONS FROM billing.cpp
// =========================================================================

/* =========================================================================
   BILLING.CPP  --  MODULE 4: Billing / Payment Processing & Reporting
   (Author: Jia Zhi)
   ========================================================================= */

int findInvoiceIndex(const vector<Invoice> &invoices, int invoiceID) {
    for (int i = 0; i < (int)invoices.size(); i++)
        if (invoices[i].invoiceID == invoiceID) return i;
    return -1;
}

// -------------------------------------------------------------------------
// generateInvoice - pick a Confirmed booking, calculate the subtotal from
// its service list (pulled from Module 2's data), apply the customer's
// membership-tier discount automatically, optionally let them redeem
// loyalty points (both calling into Module 1), then add 6% SST. Stored as
// "Pending" until recordPayment() is used.
// -------------------------------------------------------------------------
void generateInvoice(vector<Invoice> &invoices, int &nextInvoiceID, vector<Booking> &bookings,
                      vector<Customer> &customers, const vector<Stylist> &stylists,
                      const vector<Service> &services) {
    clearScreen();
    printHeader("GENERATE INVOICE");
    cout << left << setw(6) << "BkID" << setw(6) << "Cust" << setw(6) << "Sty" << setw(12) << "Date" << setw(8) << "Time" << "Status\n";
    printDivider('-', 50);
    for (const Booking &b : bookings)
        if (b.status == "Confirmed") cout << left << setw(6) << b.bookingID << setw(6) << b.customerID << setw(6) << b.stylistID << setw(12) << b.date << setw(8) << b.timeSlot << b.status << "\n";
    printDivider('-', 50);
    int bkID = getValidatedInt("\nEnter Booking ID to invoice (or 0 to cancel): ", 0, 999999);
    if (bkID == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int bIdx = findBookingIndex(bookings, bkID);

    if (bIdx == -1) { cout << "\nBooking not found.\n"; pauseScreen(); return; }
    if (bookings[bIdx].status != "Confirmed") {
        cout << "\nThis booking is " << bookings[bIdx].status << " and cannot be invoiced.\n";
        pauseScreen();
        return;
    }

    int cIdx = findCustomerIndex(customers, bookings[bIdx].customerID);
    if (cIdx == -1) { cout << "\nLinked customer record not found.\n"; pauseScreen(); return; }

    // ---- Subtotal from services on the booking ----
    double subtotal = 0.0;
    cout << "\nServices booked:\n";
    for (int i = 0; i < bookings[bIdx].numServices; i++) {
        int svID = bookings[bIdx].serviceIDs[i];
        double price = getServicePrice(services, svID);
        subtotal += price;
        cout << "  - " << getServiceName(services, svID) << " : RM" << fixed << setprecision(2) << price << "\n";
    }

    // ---- EXTRA FEATURE: automatic membership tier discount ----
    string tier = getMembershipTier(customers[cIdx].lifetimePoints);
    double tierRate = getTierDiscountRate(tier);
    double tierDiscount = subtotal * tierRate;
    if (tierRate > 0) {
        cout << "\n" << customers[cIdx].name << " is a " << tier << " member: "
             << (int)(tierRate * 100) << "% automatic discount applied.\n";
    }

    // ---- EXTRA FEATURE: optional loyalty point redemption ----
    double pointsDiscount = 0.0;
    int pointsRedeemed = 0;
    cout << "Customer has " << customers[cIdx].loyaltyPoints << " redeemable loyalty points.\n";
    if (customers[cIdx].loyaltyPoints >= POINTS_PER_RM_REDEEM) {
        char redeem = getYesNo("Redeem loyalty points for extra discount?");
        if (redeem == 'Y') {
            int maxPts = customers[cIdx].loyaltyPoints;
            int pts = getValidatedInt("Points to redeem (max " + to_string(maxPts) + "): ", 0, maxPts);
            pts -= (pts % POINTS_PER_RM_REDEEM);
            if (pts >= POINTS_PER_RM_REDEEM && redeemLoyaltyPoints(customers, customers[cIdx].customerID, pts)) {
                pointsRedeemed = pts;
                pointsDiscount = (double)pts / POINTS_PER_RM_REDEEM;
            }
        }
    }

    double afterDiscount = subtotal - tierDiscount - pointsDiscount;
    if (afterDiscount < 0) afterDiscount = 0;
    double tax = afterDiscount * SST_RATE;
    double total = afterDiscount + tax;

    Invoice inv;
    inv.invoiceID      = nextInvoiceID++;
    inv.bookingID       = bkID;
    inv.customerID      = customers[cIdx].customerID;
    inv.subtotal        = subtotal;
    inv.tierDiscount     = tierDiscount;
    inv.pointsDiscount   = pointsDiscount;
    inv.taxAmount        = tax;
    inv.totalAmount      = total;
    inv.amountPaid       = 0;
    inv.changeAmount     = 0;
    inv.paymentMethod    = "";
    inv.paymentStatus    = "Pending";
    inv.pointsRedeemed   = pointsRedeemed;
    inv.pointsEarned     = 0;
    inv.invoiceDate      = getTodayDate();

    invoices.push_back(inv);

    cout << fixed << setprecision(2);
    printDivider('=');
    cout << "               INVOICE #" << inv.invoiceID << "\n";
    printDivider('=');
    cout << "Subtotal            : RM" << subtotal << "\n";
    cout << "Membership Discount : -RM" << tierDiscount << "\n";
    cout << "Points Discount     : -RM" << pointsDiscount << "\n";
    cout << "SST (6%)            : +RM" << tax << "\n";
    cout << "TOTAL DUE           : RM" << total << "\n";
    cout << "Payment Status      : " << inv.paymentStatus << "\n";
    printDivider('=');
    cout << "Go to 'Record Payment' to complete this transaction.\n";
    pauseScreen();
}

// -------------------------------------------------------------------------
// recordPayment - takes payment for a Pending invoice, marks the linked
// booking Completed, and awards loyalty points (1 point per RM1 paid).
// -------------------------------------------------------------------------
void recordPayment(vector<Invoice> &invoices, vector<Booking> &bookings, vector<Customer> &customers) {
    clearScreen();
    printHeader("RECORD PAYMENT");
    cout << fixed << setprecision(2);
    cout << left << setw(6) << "InvID" << setw(6) << "BkID" << setw(6) << "Cust" << setw(10) << "Total" << "Status\n";
    printDivider('-', 40);
    for (const Invoice &inv : invoices)
        cout << left << setw(6) << inv.invoiceID << setw(6) << inv.bookingID << setw(6) << inv.customerID << "RM" << setw(7) << inv.totalAmount << inv.paymentStatus << "\n";
    printDivider('-', 40);
    int invID = getValidatedInt("\nEnter Invoice ID (or 0 to cancel): ", 0, 999999);
    if (invID == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int idx = findInvoiceIndex(invoices, invID);

    if (idx == -1) { cout << "\nInvoice not found.\n"; pauseScreen(); return; }
    if (invoices[idx].paymentStatus == "Paid") {
        cout << "\nThis invoice has already been paid.\n";
        pauseScreen();
        return;
    }

    cout << fixed << setprecision(2);
    cout << "\nAmount Due: RM" << invoices[idx].totalAmount << "\n";
    cout << "1. Cash\n2. Card\n3. E-Wallet\n";
    int m = getValidatedInt("Payment Method: ", 1, 3);

    string method;
    if (m == 1) method = "Cash";
    else if (m == 2) method = "Card";
    else method = "E-Wallet";

    double paid;
    if (method == "Cash") {
        paid = getValidatedDouble("Amount Received (RM): ", invoices[idx].totalAmount, 100000.0);
    } else {
        paid = invoices[idx].totalAmount; // card/e-wallet is charged the exact amount
        cout << "Charging RM" << paid << " to " << method << "...\n";
    }

    invoices[idx].paymentMethod = method;
    invoices[idx].amountPaid    = paid;
    invoices[idx].changeAmount  = paid - invoices[idx].totalAmount;
    invoices[idx].paymentStatus = "Paid";

    // Mark the linked booking as Completed (data sharing with Module 3)
    int bIdx = findBookingIndex(bookings, invoices[idx].bookingID);
    if (bIdx != -1) bookings[bIdx].status = "Completed";

    // Award loyalty points: 1 point per whole RM paid (data sharing with Module 1)
    int pointsEarned = (int)(invoices[idx].totalAmount) * POINTS_PER_RM;
    invoices[idx].pointsEarned = pointsEarned;
    addLoyaltyPoints(customers, invoices[idx].customerID, pointsEarned);

    cout << "\nPayment recorded successfully!\n";
    cout << "Amount Paid : RM" << paid << "\n";
    if (invoices[idx].changeAmount > 0) cout << "Change       : RM" << invoices[idx].changeAmount << "\n";
    cout << "Loyalty Points Earned: " << pointsEarned << "\n";
    pauseScreen();
}

void printReceipt(const vector<Invoice> &invoices, const vector<Booking> &bookings,
                   const vector<Customer> &customers, const vector<Stylist> &stylists,
                   const vector<Service> &services) {
    clearScreen();
    printHeader("PRINT RECEIPT");
    cout << fixed << setprecision(2);
    cout << left << setw(6) << "InvID" << setw(6) << "BkID" << setw(6) << "Cust" << setw(10) << "Total" << "Status\n";
    printDivider('-', 40);
    for (const Invoice &inv : invoices)
        cout << left << setw(6) << inv.invoiceID << setw(6) << inv.bookingID << setw(6) << inv.customerID << "RM" << setw(7) << inv.totalAmount << inv.paymentStatus << "\n";
    printDivider('-', 40);
    int invID = getValidatedInt("\nEnter Invoice ID (or 0 to cancel): ", 0, 999999);
    if (invID == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int idx = findInvoiceIndex(invoices, invID);

    if (idx == -1) { cout << "\nInvoice not found.\n"; pauseScreen(); return; }
    if (invoices[idx].paymentStatus != "Paid") {
        cout << "\nThis invoice has not been paid yet. Please record payment first.\n";
        pauseScreen();
        return;
    }

    const Invoice &inv = invoices[idx];
    int bIdx = findBookingIndex(bookings, inv.bookingID);
    int cIdx = findCustomerIndex(customers, inv.customerID);

    cout << fixed << setprecision(2);
    printDivider('=', 45);
    cout << "       BLISS HAIR & BEAUTY SALON\n";
    cout << "              OFFICIAL RECEIPT\n";
    printDivider('=', 45);
    cout << "Receipt No. : INV-" << right << setw(4) << setfill('0') << inv.invoiceID << setfill(' ') << "\n";
    cout << "Date        : " << inv.invoiceDate << "\n";
    cout << "Customer    : " << (cIdx != -1 ? customers[cIdx].name : "N/A") << "\n";

    if (bIdx != -1) {
        int sIdx = findStylistIndex(stylists, bookings[bIdx].stylistID);
        cout << "Stylist     : " << (sIdx != -1 ? stylists[sIdx].name : "N/A") << "\n";
        printDivider('-', 45);
        for (int i = 0; i < bookings[bIdx].numServices; i++) {
            int svID = bookings[bIdx].serviceIDs[i];
            cout << left << setw(30) << getServiceName(services, svID)
                 << right << setw(9) << getServicePrice(services, svID) << "\n";
        }
    }

    printDivider('-', 45);
    cout << left << setw(30) << "Subtotal:" << right << setw(9) << inv.subtotal << "\n";
    if (inv.tierDiscount > 0)
        cout << left << setw(30) << "Membership Discount:" << right << setw(9) << -inv.tierDiscount << "\n";
    if (inv.pointsDiscount > 0)
        cout << left << setw(30) << "Points Discount:" << right << setw(9) << -inv.pointsDiscount << "\n";
    cout << left << setw(30) << "SST (6%):" << right << setw(9) << inv.taxAmount << "\n";
    printDivider('-', 45);
    cout << left << setw(30) << "TOTAL:" << right << setw(9) << inv.totalAmount << "\n";
    cout << left << setw(30) << ("Paid via " + inv.paymentMethod + ":") << right << setw(9) << inv.amountPaid << "\n";
    if (inv.changeAmount > 0)
        cout << left << setw(30) << "Change:" << right << setw(9) << inv.changeAmount << "\n";
    printDivider('=', 45);
    if (inv.pointsRedeemed > 0) cout << "Points Redeemed : " << inv.pointsRedeemed << "\n";
    cout << "Points Earned    : " << inv.pointsEarned << "\n";
    cout << "\n          Thank you for visiting us!\n";
    printDivider('=', 45);
    pauseScreen();
}

// -------------------------------------------------------------------------
// generateReport - Daily / Overall sales summaries, plus a Service
// Popularity Ranking that tallies bookings per service into parallel
// arrays and sorts them with a simple descending selection sort.
// -------------------------------------------------------------------------
void generateReport(const vector<Invoice> &invoices, const vector<Booking> &bookings,
                     const vector<Service> &services, const vector<Stylist> &stylists) {
    clearScreen();
    printHeader("GENERATE REPORT");
    cout << "1. Daily Sales Summary (Today)\n";
    cout << "2. Overall Sales Summary (All Time)\n";
    cout << "3. Service Popularity Ranking\n";
    int choice = getValidatedInt("Choice: ", 1, 3);

    ostringstream out;
    cout << fixed << setprecision(2);
    out << fixed << setprecision(2);

    if (choice == 1 || choice == 2) {
        string today = getTodayDate();
        double totalRevenue = 0, totalTax = 0;
        int paidCount = 0;

        for (const Invoice &inv : invoices) {
            if (inv.paymentStatus != "Paid") continue;
            if (choice == 1 && inv.invoiceDate != today) continue;
            totalRevenue += inv.totalAmount;
            totalTax     += inv.taxAmount;
            paidCount++;
        }

        out << string(45, '=') << "\n";
        if (choice == 1) out << "     DAILY SALES SUMMARY - " << today << "\n";
        else out << "        OVERALL SALES SUMMARY\n";
        out << string(45, '=') << "\n";
        out << "Paid Invoices        : " << paidCount << "\n";
        out << "Total SST Collected  : RM" << totalTax << "\n";
        out << "TOTAL REVENUE        : RM" << totalRevenue << "\n";
        out << string(45, '=') << "\n";

        cout << out.str();
        if (getYesNo("\nExport report to file?") == 'Y')
            exportToFile((choice == 1 ? "daily_report" : "overall_report"), out.str());

    } else {
        int n = (int)services.size();
        if (n == 0) { cout << "No services on file.\n"; pauseScreen(); return; }

        vector<int> serviceID(n);
        vector<int> bookingCount(n, 0);
        for (int i = 0; i < n; i++) serviceID[i] = services[i].serviceID;

        for (const Booking &b : bookings) {
            if (b.status == "Cancelled") continue;
            for (int i = 0; i < b.numServices; i++) {
                for (int j = 0; j < n; j++) {
                    if (serviceID[j] == b.serviceIDs[i]) { bookingCount[j]++; break; }
                }
            }
        }

        for (int i = 0; i < n - 1; i++) {
            int maxIdx = i;
            for (int j = i + 1; j < n; j++) {
                if (bookingCount[j] > bookingCount[maxIdx]) maxIdx = j;
            }
            if (maxIdx != i) {
                swap(bookingCount[i], bookingCount[maxIdx]);
                swap(serviceID[i], serviceID[maxIdx]);
            }
        }

        out << string(50, '=') << "\n";
        out << "        SERVICE POPULARITY RANKING\n";
        out << string(50, '=') << "\n";
        out << left << setw(6) << "Rank" << setw(28) << "Service" << "Times Booked\n";
        for (int i = 0; i < n; i++) {
            out << left << setw(6) << (i + 1) << setw(28) << getServiceName(services, serviceID[i])
                << bookingCount[i] << "\n";
        }
        out << string(50, '=') << "\n";

        cout << out.str();
        if (getYesNo("\nExport report to file?") == 'Y')
            exportToFile("service_ranking", out.str());
    }

    (void)stylists; // reserved for a future "revenue per stylist" report
    pauseScreen();
}

// -------------------------------------------------------------------------
// billingReportMenu - Module 4's entry point, called from main.cpp.
// -------------------------------------------------------------------------
void billingReportMenu(vector<Invoice> &invoices, int &nextInvoiceID,
                        vector<Booking> &bookings, vector<Customer> &customers,
                        const vector<Stylist> &stylists, const vector<Service> &services) {
    int choice;
    do {
        clearScreen();
        printHeader("BILLING & REPORTING");
        cout << "1. Generate Invoice\n";
        cout << "2. Record Payment\n";
        cout << "3. Print Receipt\n";
        cout << "4. Generate Report\n";
        cout << "0. Back to Main Menu\n";
        choice = getValidatedInt("Enter your choice: ", 0, 4);

        switch (choice) {
            case 1: generateInvoice(invoices, nextInvoiceID, bookings, customers, stylists, services); break;
            case 2: recordPayment(invoices, bookings, customers); break;
            case 3: printReceipt(invoices, bookings, customers, stylists, services); break;
            case 4: generateReport(invoices, bookings, services, stylists); break;
            case 0: break;
        }
    } while (choice != 0);
}

// =========================================================================
// IMPLEMENTATIONS FROM persistence.cpp
// =========================================================================

/* =========================================================================
   PERSISTENCE.CPP  --  File I/O for data persistence
   Team: Ting Wei, Jeremy, Michael, Jia Zhi
   ---------------------------------------------------------------------
   Pipe-delimited text files stored in a 'data' subfolder.
   Loads on startup; saves on exit and via menu option.
   If no save files exist, the sample data from loadSampleData() is used.
   ========================================================================= */

const string CUST_FILE    = "data_customers.txt";
const string STYL_FILE    = "data_stylists.txt";
const string SERV_FILE    = "data_services.txt";
const string BOOK_FILE    = "data_bookings.txt";
const string INV_FILE     = "data_invoices.txt";
const string IDS_FILE     = "data_ids.txt";

void trim(string &s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) s.pop_back();
}

bool saveAllData(const vector<Customer> &c, const vector<Stylist>  &st,
                 const vector<Service>  &sv, const vector<Booking>  &b,
                 const vector<Invoice>  &inv,
                 int ncID, int nsID, int nsvID, int nbID, int niID) {
    string dir = getProgramDir();
    ofstream f;

    f.open(dir + CUST_FILE); if (!f) return false;
    for (const Customer &r : c)
        f << r.customerID << '|' << r.name << '|' << r.phone << '|' << r.email
          << '|' << r.gender << '|' << r.loyaltyPoints << '|' << r.lifetimePoints
          << '|' << r.isActive << '\n';
    f.close();

    f.open(dir + STYL_FILE); if (!f) return false;
    for (const Stylist &r : st)
        f << r.stylistID << '|' << r.name << '|' << r.phone << '|' << r.specialization
          << '|' << r.isAvailable << '|' << r.isActive << '\n';
    f.close();

    f.open(dir + SERV_FILE); if (!f) return false;
    for (const Service &r : sv)
        f << r.serviceID << '|' << r.serviceName << '|' << r.category
          << '|' << r.price << '|' << r.duration << '|' << r.isActive << '\n';
    f.close();

    f.open(dir + BOOK_FILE); if (!f) return false;
    for (const Booking &r : b) {
        f << r.bookingID << '|' << r.customerID << '|' << r.stylistID << '|';
        for (int i = 0; i < r.numServices; i++) {
            f << r.serviceIDs[i];
            if (i < r.numServices - 1) f << ',';
        }
        f << '|' << r.numServices << '|' << r.date << '|' << r.timeSlot
          << '|' << r.status << '\n';
    }
    f.close();

    f.open(dir + INV_FILE); if (!f) return false;
    for (const Invoice &r : inv)
        f << r.invoiceID << '|' << r.bookingID << '|' << r.customerID
          << '|' << r.subtotal << '|' << r.tierDiscount << '|' << r.pointsDiscount
          << '|' << r.taxAmount << '|' << r.totalAmount << '|' << r.amountPaid
          << '|' << r.changeAmount << '|' << r.paymentMethod << '|' << r.paymentStatus
          << '|' << r.pointsRedeemed << '|' << r.pointsEarned << '|' << r.invoiceDate << '\n';
    f.close();

    f.open(dir + IDS_FILE); if (!f) return false;
    f << ncID << '|' << nsID << '|' << nsvID << '|' << nbID << '|' << niID << '\n';
    f.close();

    cout << "\nAll data saved successfully.\n";
    return true;
}

bool loadAllData(vector<Customer> &c, vector<Stylist>  &st,
                 vector<Service>  &sv, vector<Booking>  &b,
                 vector<Invoice>  &inv,
                 int &ncID, int &nsID, int &nsvID, int &nbID, int &niID) {
    string dir = getProgramDir();
    ifstream f(dir + IDS_FILE);
    if (!f) return false;
    string line;
    if (getline(f, line)) {
        stringstream ss(line);
        string tok;
        getline(ss, tok, '|'); ncID  = stoi(tok);
        getline(ss, tok, '|'); nsID  = stoi(tok);
        getline(ss, tok, '|'); nsvID = stoi(tok);
        getline(ss, tok, '|'); nbID  = stoi(tok);
        getline(ss, tok, '|'); niID  = stoi(tok);
    }
    f.close();

    f.open(dir + CUST_FILE); if (!f) return false;
    while (getline(f, line)) {
        trim(line);
        stringstream ss(line);
        string tok;
        Customer r;
        getline(ss, tok, '|'); r.customerID    = stoi(tok);
        getline(ss, r.name, '|');
        getline(ss, r.phone, '|');
        getline(ss, r.email, '|');
        getline(ss, tok, '|'); r.gender         = tok[0];
        getline(ss, tok, '|'); r.loyaltyPoints  = stoi(tok);
        getline(ss, tok, '|'); r.lifetimePoints = stoi(tok);
        getline(ss, tok, '|'); r.isActive       = (tok == "1");
        c.push_back(r);
    }
    f.close();

    f.open(dir + STYL_FILE); if (!f) return false;
    while (getline(f, line)) {
        trim(line);
        stringstream ss(line);
        string tok;
        Stylist r;
        getline(ss, tok, '|'); r.stylistID      = stoi(tok);
        getline(ss, r.name, '|');
        getline(ss, r.phone, '|');
        getline(ss, r.specialization, '|');
        getline(ss, tok, '|'); r.isAvailable    = (tok == "1");
        getline(ss, tok, '|'); r.isActive       = (tok == "1");
        st.push_back(r);
    }
    f.close();

    f.open(dir + SERV_FILE); if (!f) return false;
    while (getline(f, line)) {
        trim(line);
        stringstream ss(line);
        string tok;
        Service r;
        getline(ss, tok, '|'); r.serviceID      = stoi(tok);
        getline(ss, r.serviceName, '|');
        getline(ss, r.category, '|');
        getline(ss, tok, '|'); r.price          = stod(tok);
        getline(ss, tok, '|'); r.duration       = stoi(tok);
        getline(ss, tok, '|'); r.isActive       = (tok == "1");
        sv.push_back(r);
    }
    f.close();

    f.open(dir + BOOK_FILE); if (!f) return false;
    while (getline(f, line)) {
        trim(line);
        stringstream ss(line);
        string tok;
        Booking r;
        getline(ss, tok, '|'); r.bookingID  = stoi(tok);
        getline(ss, tok, '|'); r.customerID = stoi(tok);
        getline(ss, tok, '|'); r.stylistID  = stoi(tok);
        getline(ss, tok, '|');
        stringstream svids(tok);
        string svid;
        r.numServices = 0;
        while (getline(svids, svid, ',')) {
            if (r.numServices < MAX_SERVICES_PER_BOOKING) {
                r.serviceIDs[r.numServices] = stoi(svid);
                r.numServices++;
            }
        }
        getline(ss, tok, '|');
        getline(ss, r.date, '|');
        getline(ss, r.timeSlot, '|');
        getline(ss, r.status, '|');
        b.push_back(r);
    }
    f.close();

    f.open(dir + INV_FILE); if (!f) return false;
    while (getline(f, line)) {
        trim(line);
        stringstream ss(line);
        string tok;
        Invoice r;
        getline(ss, tok, '|'); r.invoiceID     = stoi(tok);
        getline(ss, tok, '|'); r.bookingID     = stoi(tok);
        getline(ss, tok, '|'); r.customerID    = stoi(tok);
        getline(ss, tok, '|'); r.subtotal      = stod(tok);
        getline(ss, tok, '|'); r.tierDiscount   = stod(tok);
        getline(ss, tok, '|'); r.pointsDiscount = stod(tok);
        getline(ss, tok, '|'); r.taxAmount      = stod(tok);
        getline(ss, tok, '|'); r.totalAmount    = stod(tok);
        getline(ss, tok, '|'); r.amountPaid     = stod(tok);
        getline(ss, tok, '|'); r.changeAmount   = stod(tok);
        getline(ss, r.paymentMethod, '|');
        getline(ss, r.paymentStatus, '|');
        getline(ss, tok, '|'); r.pointsRedeemed = stoi(tok);
        getline(ss, tok, '|'); r.pointsEarned   = stoi(tok);
        getline(ss, r.invoiceDate, '|');
        inv.push_back(r);
    }
    f.close();

    cout << "\nSaved data loaded successfully.\n";
    return true;
}

// =========================================================================
// loadSampleData - preloads a few starter records so the system is not
// empty on first run, making it easy to demo Booking & Billing right away.
// =========================================================================
void loadSampleData(vector<Customer> &customers, vector<Stylist> &stylists, vector<Service> &services,
                     int &nextCustomerID, int &nextStylistID, int &nextServiceID) {
    customers.push_back({1, "Amy Tan", "0123456789", "amy@mail.com", 'F', 0, 0, true});
    customers.push_back({2, "Ben Lee", "0129876543", "ben@mail.com", 'M', 0, 0, true});
    nextCustomerID = 3;

    stylists.push_back({1, "Sarah Lim", "0111112222", "Hair Styling", true, true});
    stylists.push_back({2, "David Wong", "0113334444", "Colouring", true, true});
    stylists.push_back({3, "Nadia Rahman", "0115556666", "Nail Care", true, true});
    nextStylistID = 4;

    services.push_back({1, "Haircut", "Hair", 35.00, 30, true});
    services.push_back({2, "Hair Colouring", "Hair", 120.00, 90, true});
    services.push_back({3, "Manicure", "Nail", 45.00, 40, true});
    services.push_back({4, "Pedicure", "Nail", 50.00, 45, true});
    services.push_back({5, "Facial Spa", "Spa", 90.00, 60, true});
    nextServiceID = 6;
}

// =========================================================================
// displayIntroScreen
// =========================================================================
void displayIntroScreen() {
    printDivider('*', 65);
    cout << "\n";
    cout << "         BLISS HAIR & BEAUTY SALON MANAGEMENT SYSTEM\n";
    cout << "                 AMCS2123 Team Assignment\n";
    cout << "\n";
    printDivider('*', 65);
    cout << "\n  Manage customers, stylists, services, bookings and billing\n";
    cout << "  all in one integrated system.\n\n";
    pauseScreen();
}

// =========================================================================
// main
// =========================================================================
int main(int argc, char *argv[]) {
    if (argc > 0) setProgramDir(argv[0]);

    // ---- Master data lists (shared across all modules) ----
    vector<Customer> customers;
    vector<Stylist>  stylists;
    vector<Service>  services;
    vector<Booking>  bookings;
    vector<Invoice>  invoices;

    // ---- ID counters (shared across all modules) ----
    int nextCustomerID = 1;
    int nextStylistID  = 1;
    int nextServiceID  = 1;
    int nextBookingID  = 1;
    int nextInvoiceID  = 1;

    if (!loadAllData(customers, stylists, services, bookings, invoices,
                      nextCustomerID, nextStylistID, nextServiceID,
                      nextBookingID, nextInvoiceID)) {
        loadSampleData(customers, stylists, services,
                        nextCustomerID, nextStylistID, nextServiceID);
    }
    displayIntroScreen();

    int choice;
    do {
        clearScreen();
        printHeader("MAIN MENU - BLISS HAIR & BEAUTY SALON");
        cout << "1. Customer Management\n";
        cout << "2. Service & Stylist Management\n";
        cout << "3. Booking Management\n";
        cout << "4. Billing & Reporting\n";
        cout << "5. Save All Data to Files\n";
        cout << "0. Exit (auto-saves on exit)\n";
        choice = getValidatedInt("Enter your choice: ", 0, 5);

        switch (choice) {
            case 1: customerManagementMenu(customers, nextCustomerID); break;
            case 2: serviceStylistMenu(stylists, services, nextStylistID, nextServiceID); break;
            case 3: bookingManagementMenu(bookings, nextBookingID, customers, stylists, services); break;
            case 4: billingReportMenu(invoices, nextInvoiceID, bookings, customers, stylists, services); break;
            case 5: saveAllData(customers, stylists, services, bookings, invoices,
                                 nextCustomerID, nextStylistID, nextServiceID,
                                 nextBookingID, nextInvoiceID);
                     pauseScreen();
                     break;
            case 0:
                saveAllData(customers, stylists, services, bookings, invoices,
                            nextCustomerID, nextStylistID, nextServiceID,
                            nextBookingID, nextInvoiceID);
                cout << "\n";
                printDivider('=');
                cout << " Thank you for using Bliss Hair & Beauty Salon Management System.\n";
                cout << "                      See you again soon!\n";
                printDivider('=');
                break;
        }
    } while (choice != 0);

    return 0;
}
