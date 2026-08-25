#include <iostream>
#include <fstream>
#include <sstream>
#include "common.h"
#include "utils.h"
#include "persistence.h"
using namespace std;

/* =========================================================================
   PERSISTENCE.CPP  --  File I/O for data persistence
   Team: Ting Wei, Jeremy, Michael, Jia Zhi
   ---------------------------------------------------------------------
   Pipe-delimited text files stored in the program directory.
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
