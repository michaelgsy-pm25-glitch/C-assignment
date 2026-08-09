#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <vector>
#include "common.h"
using namespace std;

/* =========================================================================
   CUSTOMER.H   --  MODULE 1: Customer Management   (Author: Ting Wei)
   ========================================================================= */

// Module menu (called from main.cpp)
void customerManagementMenu(vector<Customer> &customers, int &nextCustomerID);

// Core module functions
void addCustomer(vector<Customer> &customers, int &nextCustomerID);
void searchCustomer(const vector<Customer> &customers);
void updateCustomer(vector<Customer> &customers);
void deleteCustomer(vector<Customer> &customers);
void listAllCustomers(const vector<Customer> &customers);

// Extra Feature: Loyalty Points & Membership Tier
void   loyaltyPointsMenu(vector<Customer> &customers);
string getMembershipTier(int lifetimePoints);
double getTierDiscountRate(const string &tier);

// Shared helpers - called by Booking (Module 3) and Billing (Module 4)
// to validate customers and manage points during their own workflows.
int  findCustomerIndex(const vector<Customer> &customers, int customerID);
void addLoyaltyPoints(vector<Customer> &customers, int customerID, int pointsToAdd);
bool redeemLoyaltyPoints(vector<Customer> &customers, int customerID, int pointsToRedeem);

#endif
