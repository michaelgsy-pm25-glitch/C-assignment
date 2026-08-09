#ifndef STYLIST_SERVICE_H
#define STYLIST_SERVICE_H

#include <vector>
#include "common.h"
using namespace std;

/* =========================================================================
   STYLIST_SERVICE.H  --  MODULE 2: Service & Stylist Management
   (Author: Jeremy)
   ========================================================================= */

// Module menu (called from main.cpp)
void serviceStylistMenu(vector<Stylist> &stylists, vector<Service> &services,
                         int &nextStylistID, int &nextServiceID);

// Stylist functions
void addStylist(vector<Stylist> &stylists, int &nextStylistID);
void updateStylist(vector<Stylist> &stylists);
void deleteStylist(vector<Stylist> &stylists);
void viewStylists(const vector<Stylist> &stylists);

// Service functions
void addService(vector<Service> &services, int &nextServiceID);
void updateService(vector<Service> &services);
void deleteService(vector<Service> &services);
void viewServices(const vector<Service> &services);

// Shared helpers - called by Booking (Module 3) and Billing (Module 4)
int    findStylistIndex(const vector<Stylist> &stylists, int stylistID);
int    findServiceIndex(const vector<Service> &services, int serviceID);
string getServiceName(const vector<Service> &services, int serviceID);
double getServicePrice(const vector<Service> &services, int serviceID);

#endif
