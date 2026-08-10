#ifndef BOOKING_H
#define BOOKING_H

#include <vector>
#include "common.h"
using namespace std;

/* =========================================================================
   BOOKING.H  --  MODULE 3: Booking Management   (Author: Michael)
   ========================================================================= */

// Module menu (called from main.cpp)
void bookingManagementMenu(vector<Booking> &bookings, int &nextBookingID,
                            const vector<Customer> &customers,
                            const vector<Stylist> &stylists,
                            const vector<Service> &services);

void bookAppointment(vector<Booking> &bookings, int &nextBookingID,
                      const vector<Customer> &customers,
                      const vector<Stylist> &stylists,
                      const vector<Service> &services);
void viewSearchAppointment(const vector<Booking> &bookings, const vector<Customer> &customers);
void viewBookingDetail(const vector<Booking> &bookings, const vector<Customer> &customers,
                       const vector<Stylist> &stylists, const vector<Service> &services);
void cancelAppointment(vector<Booking> &bookings);
void updateAppointment(vector<Booking> &bookings, const vector<Stylist> &stylists, const vector<Service> &services);
void viewTodaySchedule(const vector<Booking> &bookings, const vector<Stylist> &stylists, const vector<Customer> &customers);

// Shared helpers - called by Billing (Module 4)
int  findBookingIndex(const vector<Booking> &bookings, int bookingID);
bool isTimeSlotTaken(const vector<Booking> &bookings, int stylistID, const string &date,
                      const string &timeSlot, int excludeBookingID = -1);

#endif
