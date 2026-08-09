#ifndef UTILS_H
#define UTILS_H

#include <string>
using namespace std;

/* =========================================================================
   UTILS.H
   TEAM-SHARED FILE (User Interface + shared validation helpers)
   ---------------------------------------------------------------------
   Generic, reusable functions used by every module so that input
   validation and on-screen formatting stay consistent across the whole
   system instead of every member reinventing their own version.
   ========================================================================= */

// ---- Screen / UI helpers ----
void clearScreen();
void pauseScreen();
void printDivider(char ch = '-', int length = 65);
void printHeader(const string &title);
void exportToFile(const string &prefix, const string &content);

// ---- Validated input (loops until the user gives acceptable input) ----
int    getValidatedInt(const string &prompt, int minVal, int maxVal);
double getValidatedDouble(const string &prompt, double minVal, double maxVal);
string getValidatedName(const string &prompt, bool allowCancel = false);
string getValidatedPhone(const string &prompt, bool allowCancel = false);
string getValidatedEmail(const string &prompt, bool allowCancel = false);
char   getValidatedGender(const string &prompt);
string getValidatedDate(const string &prompt, bool mustNotBePast);
char   getYesNo(const string &prompt);

// ---- Raw validation checks (reusable building blocks) ----
bool isValidName(const string &name);
bool isValidPhone(const string &phone);
bool isValidEmail(const string &email);
bool isValidDate(const string &date);

// ---- Date helpers ----
long   dateToComparable(const string &date); // only call after isValidDate() passes
string getTodayDate();

// ---- File path helpers ----
void   setProgramDir(const string &argv0);
string getProgramDir();

#endif
