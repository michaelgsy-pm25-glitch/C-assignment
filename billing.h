#ifndef BILLING_H
#define BILLING_H

#include <vector>
#include "common.h"
using namespace std;

/* =========================================================================
   BILLING.H  --  MODULE 4: Billing / Payment Processing & Reporting
   (Author: Jia Zhi)
   ========================================================================= */

// Module menu (called from main.cpp)
void billingReportMenu(vector<Invoice> &invoices, int &nextInvoiceID,
                        vector<Booking> &bookings, vector<Customer> &customers,
                        const vector<Stylist> &stylists, const vector<Service> &services);

void generateInvoice(vector<Invoice> &invoices, int &nextInvoiceID, vector<Booking> &bookings,
                      vector<Customer> &customers, const vector<Stylist> &stylists,
                      const vector<Service> &services);
void recordPayment(vector<Invoice> &invoices, vector<Booking> &bookings, vector<Customer> &customers);
void printReceipt(const vector<Invoice> &invoices, const vector<Booking> &bookings,
                   const vector<Customer> &customers, const vector<Stylist> &stylists,
                   const vector<Service> &services);
void generateReport(const vector<Invoice> &invoices, const vector<Booking> &bookings,
                     const vector<Service> &services, const vector<Stylist> &stylists);

int findInvoiceIndex(const vector<Invoice> &invoices, int invoiceID);

#endif
