#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <vector>
#include "common.h"
using namespace std;

/* =========================================================================
   PERSISTENCE.H  --  File I/O for data persistence
   Team: Ting Wei, Jeremy, Michael, Jia Zhi
   ---------------------------------------------------------------------
   Saves all program data to text files so it persists between runs.
   Loads saved data on startup; falls back to sample data if no files exist.
   ========================================================================= */

bool saveAllData(const vector<Customer> &customers,
                 const vector<Stylist>  &stylists,
                 const vector<Service>  &services,
                 const vector<Booking>  &bookings,
                 const vector<Invoice>  &invoices,
                 int nextCustomerID, int nextStylistID, int nextServiceID,
                 int nextBookingID, int nextInvoiceID);

bool loadAllData(vector<Customer> &customers,
                 vector<Stylist>  &stylists,
                 vector<Service>  &services,
                 vector<Booking>  &bookings,
                 vector<Invoice>  &invoices,
                 int &nextCustomerID, int &nextStylistID, int &nextServiceID,
                 int &nextBookingID, int &nextInvoiceID);

#endif
