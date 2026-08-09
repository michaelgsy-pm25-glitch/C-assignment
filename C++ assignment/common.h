#ifndef COMMON_H
#define COMMON_H

#include <string>
using namespace std;

/* =========================================================================
   COMMON.H
   TEAM-SHARED FILE (Shared Data Management)
   ---------------------------------------------------------------------
   Holds the global constants and the five record structures (Customer,
   Stylist, Service, Booking, Invoice) that every module needs. Every
   .cpp file in this project includes this header so all four modules
   "speak the same language" when passing records between each other.
   ========================================================================= */

// ---------------------- Business constants ----------------------
const double SST_RATE             = 0.06; // 6% Sales & Service Tax (Malaysia)
const int    POINTS_PER_RM        = 1;    // 1 loyalty point earned per RM1 paid
const int    POINTS_PER_RM_REDEEM = 10;   // 10 points = RM1 discount when redeemed

// Membership tier thresholds - EXTRA FEATURE (based on lifetime points earned)
const int    SILVER_TIER_POINTS = 100;
const int    GOLD_TIER_POINTS   = 300;
const double SILVER_DISCOUNT    = 0.05;   // 5% automatic discount
const double GOLD_DISCOUNT      = 0.10;   // 10% automatic discount

const int MAX_SERVICES_PER_BOOKING = 3;   // a booking can bundle up to 3 services

const int NUM_TIME_SLOTS = 8;
const string TIME_SLOTS[NUM_TIME_SLOTS] = {
    "09:00", "10:00", "11:00", "12:00",
    "14:00", "15:00", "16:00", "17:00"
};

// ---------------------- Records (structures) ----------------------
struct Customer {
    int    customerID;
    string name;
    string phone;
    string email;
    char   gender;         // 'M', 'F' or 'O'
    int    loyaltyPoints;  // current redeemable balance
    int    lifetimePoints; // total ever earned - drives membership tier
    bool   isActive;       // false = "deleted" (soft delete keeps history intact)
};

struct Stylist {
    int    stylistID;
    string name;
    string phone;
    string specialization; // e.g. "Hair Styling", "Colouring", "Nail Care"
    bool   isAvailable;    // currently accepting bookings
    bool   isActive;       // false = "deleted"
};

struct Service {
    int    serviceID;
    string serviceName;
    string category;       // e.g. "Hair", "Nail", "Spa", "Makeup"
    double price;
    int    duration;       // minutes
    bool   isActive;
};

struct Booking {
    int    bookingID;
    int    customerID;
    int    stylistID;
    int    serviceIDs[MAX_SERVICES_PER_BOOKING]; // fixed-size array of service IDs
    int    numServices;                          // how many of the slots above are used
    string date;      // DD/MM/YYYY
    string timeSlot;  // must match an entry in TIME_SLOTS
    string status;    // "Confirmed", "Completed", "Cancelled"
};

struct Invoice {
    int    invoiceID;
    int    bookingID;
    int    customerID;
    double subtotal;
    double tierDiscount;     // from membership tier (extra feature)
    double pointsDiscount;   // from redeemed loyalty points (extra feature)
    double taxAmount;
    double totalAmount;
    double amountPaid;
    double changeAmount;
    string paymentMethod;    // "Cash", "Card", "E-Wallet"
    string paymentStatus;    // "Pending", "Paid"
    int    pointsRedeemed;
    int    pointsEarned;
    string invoiceDate;
};

#endif
