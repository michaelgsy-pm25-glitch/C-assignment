#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include "common.h"
#include "utils.h"
#include "customer.h"
#include "stylist_service.h"
#include "booking.h"
#include "billing.h"
using namespace std;

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
