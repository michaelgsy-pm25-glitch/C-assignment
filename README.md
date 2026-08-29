# Bliss Hair & Beauty Salon Management System
AMCS2123 Systems & Programming Concepts — Team Assignment (June 2026)

## Team & Module Ownership

| # | Functional Area | Member | Files |
|---|---|---|---|
| 1 | Customer Management + Loyalty Points (extra) | **Ting Wei** | `customer.h` / `customer.cpp` |
| 2 | Service & Stylist Management | **Jeremy** | `stylist_service.h` / `stylist_service.cpp` |
| 3 | Booking Management | **Michael** | `booking.h` / `booking.cpp` |
| 4 | Billing / Payment & Reporting | **Jia Zhi** | `billing.h` / `billing.cpp` |
| Team | Architecture, main menu, shared data, UI helpers, integration | All 4 | `common.h`, `utils.h`/`.cpp`, `main.cpp` |

Every module is a self-contained `.h`/`.cpp` pair. `common.h` holds the five
shared structs (`Customer`, `Stylist`, `Service`, `Booking`, `Invoice`) and
constants that all modules use to talk to each other; `main.cpp` owns the
single master copy of every data list and passes it **by reference** into
each module's menu function — that's how the whole system stays integrated.

## How to Compile

**Command line (g++), from inside the project folder:**
```
g++ -std=c++11 -Wall -o SalonSystem main.cpp customer.cpp stylist_service.cpp booking.cpp billing.cpp utils.cpp persistence.cpp
```
Then run `./SalonSystem` (Mac/Linux/WSL) or `SalonSystem.exe` (Windows).

**Dev-C++ / Code::Blocks:** create a new **empty** C++ project, then
"Add to Project" every `.h` and `.cpp` file here. Don't let the IDE
generate its own blank `main.cpp` — use the one provided. Build & run as
usual.

This was compiled and test-run end-to-end (booking → invoice → payment →
receipt → report, plus customer/stylist CRUD, appointment cancel/update,
and invalid-input handling) with zero warnings under `-Wall`.

## Features

**Compulsory**
- Customer Management — add, search (ID or name), update, delete, list
- Stylist & Service Management — add/update/delete/view for both
- Booking Management — book (up to 3 services per appointment), view/search,
  cancel, update, plus a **Today's Schedule** grid (stylist × time-slot)
- Billing & Reporting — generate invoice, record payment, print receipt,
  generate report (daily/overall sales + service popularity ranking)

**Extra features (business functions, not technical/UI features)**
1. **Loyalty Points Program** — customers earn 1 point per RM1 spent;
   redeemable at 10 points = RM1 discount, applied instantly on a bill.
2. **Membership Tier System** — Bronze/Silver/Gold is calculated
   automatically from lifetime points earned (100 / 300 thresholds) and
   applies an automatic 5%/10% discount on every invoice — no manual entry.
3. **File Persistence** — all data (customers, stylists, services, bookings,
   invoices, ID counters) is saved to text files on exit and automatically
   loaded on next startup. Use menu option 5 to save at any time.
