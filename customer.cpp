#include <iostream>
#include <iomanip>
#include <sstream>
#include <cctype>
#include "common.h"
#include "utils.h"
#include "customer.h"
using namespace std;

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
    cout << "1. Search by Customer ID\n2. Search by Name\n0. Back\n";
    int mode = getValidatedInt("Choice: ", 0, 2);
    if (mode == 0) return;

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
        cout << "Enter name or keyword (or Enter to cancel): ";
        getline(cin, keyword);
        if (keyword.empty()) { cout << "\nCancelled.\n"; pauseScreen(); return; }
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
