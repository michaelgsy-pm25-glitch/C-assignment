#include <iostream>
#include <iomanip>
#include <sstream>
#include "booking.h"
#include "customer.h"
#include "stylist_service.h"
#include "utils.h"
using namespace std;

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

static int chooseTimeSlot() {
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
    cout << "Stylist      : " << stylists[sIdx].name << "\n";
    cout << "Service(s)   : ";
    double totalPrice = 0;
    for (int i = 0; i < b.numServices; i++) {
        int svIdx = findServiceIndex(services, b.serviceIDs[i]);
        cout << services[svIdx].serviceName;
        totalPrice += services[svIdx].price;
        if (i < b.numServices - 1) cout << ", ";
    }
    cout << "\nDate & Time  : " << b.date << " " << b.timeSlot << "\n";
    cout << fixed << setprecision(2);
    cout << "Estimated Bill: RM" << totalPrice << " (before tax/discount)\n";
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
        out << left << setw(16) << stylists[stylistRow[r]].name;
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
        cout << "3. Cancel Appointment\n";
        cout << "4. Update Appointment\n";
        cout << "5. View Today's Schedule\n";
        cout << "0. Back to Main Menu\n";
        choice = getValidatedInt("Enter your choice: ", 0, 5);

        switch (choice) {
            case 1: bookAppointment(bookings, nextBookingID, customers, stylists, services); break;
            case 2: viewSearchAppointment(bookings, customers); break;
            case 3: cancelAppointment(bookings); break;
            case 4: updateAppointment(bookings, stylists, services); break;
            case 5: viewTodaySchedule(bookings, stylists, customers); break;
            case 0: break;
        }
    } while (choice != 0);
}
