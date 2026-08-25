#include <iostream>
#include <vector>
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
