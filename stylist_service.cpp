#include <iostream>
#include <iomanip>
#include <sstream>
#include "stylist_service.h"
#include "utils.h"
using namespace std;

/* =========================================================================
   STYLIST_SERVICE.CPP  --  MODULE 2: Service & Stylist Management
   (Author: Jeremy)
   ========================================================================= */

// ---------------------- Shared lookup helpers ----------------------
int findStylistIndex(const vector<Stylist> &stylists, int stylistID) {
    for (int i = 0; i < (int)stylists.size(); i++)
        if (stylists[i].stylistID == stylistID && stylists[i].isActive) return i;
    return -1;
}

int findServiceIndex(const vector<Service> &services, int serviceID) {
    for (int i = 0; i < (int)services.size(); i++)
        if (services[i].serviceID == serviceID && services[i].isActive) return i;
    return -1;
}

string getServiceName(const vector<Service> &services, int serviceID) {
    int idx = findServiceIndex(services, serviceID);
    return (idx == -1) ? "Unknown Service" : services[idx].serviceName;
}

double getServicePrice(const vector<Service> &services, int serviceID) {
    int idx = findServiceIndex(services, serviceID);
    return (idx == -1) ? 0.0 : services[idx].price;
}

bool canStylistDoService(const Stylist &stylist, const string &serviceCategory) {
    if (serviceCategory == "Hair")
        return stylist.specialization == "Hair Styling" || stylist.specialization == "Colouring";
    if (serviceCategory == "Nail")
        return stylist.specialization == "Nail Care";
    if (serviceCategory == "Spa")
        return stylist.specialization == "Spa & Massage";
    if (serviceCategory == "Makeup")
        return stylist.specialization == "Makeup";
    return true;
}

// ============================ STYLIST FUNCTIONS ============================
static string chooseSpecialization() {
    cout << "Specialization:\n  1. Hair Styling\n  2. Colouring\n  3. Nail Care\n  4. Spa & Massage\n  5. Makeup\n";
    int c = getValidatedInt("Choice: ", 1, 5);
    switch (c) {
        case 1: return "Hair Styling";
        case 2: return "Colouring";
        case 3: return "Nail Care";
        case 4: return "Spa & Massage";
        default: return "Makeup";
    }
}

static bool isDuplicateStylistPhone(const vector<Stylist> &stylists, const string &phone) {
    for (const Stylist &s : stylists) {
        if (s.isActive && s.phone == phone) return true;
    }
    return false;
}

void addStylist(vector<Stylist> &stylists, int &nextStylistID) {
    clearScreen();
    printHeader("ADD NEW STYLIST");

    Stylist s;
    s.stylistID = nextStylistID;
    s.name = getValidatedName("Stylist Name (or 0 to cancel): ", true);
    if (s.name.empty()) { cout << "\nCancelled.\n"; pauseScreen(); return; }

    string phone;
    bool duplicate;
    do {
        phone = getValidatedPhone("Phone Number (or 0 to cancel): ", true);
        if (phone.empty()) { cout << "\nCancelled.\n"; pauseScreen(); return; }
        duplicate = isDuplicateStylistPhone(stylists, phone);
        if (duplicate) cout << "A stylist with this phone number is already registered.\n";
    } while (duplicate);
    s.phone = phone;

    s.specialization = chooseSpecialization();
    s.isAvailable = true;
    s.isActive = true;

    stylists.push_back(s);
    nextStylistID++;
    cout << "\nStylist added successfully! Assigned Stylist ID: " << s.stylistID << "\n";
    pauseScreen();
}

void updateStylist(vector<Stylist> &stylists) {
    clearScreen();
    printHeader("UPDATE STYLIST");
    cout << left << setw(6) << "ID" << setw(20) << "Name" << setw(18) << "Specialization" << "\n";
    printDivider('-', 45);
    for (const Stylist &s : stylists)
        if (s.isActive) cout << left << setw(6) << s.stylistID << setw(20) << s.name << setw(18) << s.specialization << "\n";
    printDivider('-', 45);
    int id = getValidatedInt("\nEnter Stylist ID to update (or 0 to cancel): ", 0, 999999);
    if (id == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int idx = findStylistIndex(stylists, id);
    if (idx == -1) { cout << "\nStylist not found.\n"; pauseScreen(); return; }

    int choice;
    do {
        cout << "\nCurrent -> Name: " << stylists[idx].name << " | Phone: " << stylists[idx].phone
             << " | Specialization: " << stylists[idx].specialization
             << " | Available: " << (stylists[idx].isAvailable ? "Yes" : "No") << "\n";
        cout << "1. Update Name\n2. Update Phone\n3. Update Specialization\n4. Toggle Availability\n0. Done\n";
        choice = getValidatedInt("Choice: ", 0, 4);

        switch (choice) {
            case 1: stylists[idx].name = getValidatedName("New Name: "); break;
            case 2: stylists[idx].phone = getValidatedPhone("New Phone: "); break;
            case 3: stylists[idx].specialization = chooseSpecialization(); break;
            case 4: stylists[idx].isAvailable = !stylists[idx].isAvailable; break;
            case 0: break;
        }
    } while (choice != 0);

    cout << "\nStylist record updated successfully.\n";
    pauseScreen();
}

void deleteStylist(vector<Stylist> &stylists) {
    clearScreen();
    printHeader("DELETE STYLIST");
    cout << left << setw(6) << "ID" << setw(20) << "Name" << setw(18) << "Specialization" << "\n";
    printDivider('-', 45);
    for (const Stylist &s : stylists)
        if (s.isActive) cout << left << setw(6) << s.stylistID << setw(20) << s.name << setw(18) << s.specialization << "\n";
    printDivider('-', 45);
    int id = getValidatedInt("\nEnter Stylist ID to delete (or 0 to cancel): ", 0, 999999);
    if (id == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int idx = findStylistIndex(stylists, id);
    if (idx == -1) { cout << "\nStylist not found.\n"; pauseScreen(); return; }

    cout << "\nStylist: " << stylists[idx].name << " (ID " << stylists[idx].stylistID << ")\n";
    char confirm = getYesNo("Are you sure you want to delete this stylist?");
    if (confirm == 'Y') {
        stylists[idx].isActive = false;
        cout << "\nStylist deleted successfully.\n";
    } else {
        cout << "\nDeletion cancelled.\n";
    }
    pauseScreen();
}

void viewStylists(const vector<Stylist> &stylists) {
    clearScreen();
    printHeader("ALL STYLISTS");
    ostringstream out;
    int count = 0;
    out << left << setw(6) << "ID" << setw(20) << "Name" << setw(14) << "Phone"
        << setw(18) << "Specialization" << "Available\n";
    out << string(70, '-') << "\n";
    for (const Stylist &s : stylists) {
        if (!s.isActive) continue;
        out << left << setw(6) << s.stylistID << setw(20) << s.name << setw(14) << s.phone
            << setw(18) << s.specialization << (s.isAvailable ? "Yes" : "No") << "\n";
        count++;
    }
    if (count == 0) out << "(No stylists on file yet.)\n";
    out << string(70, '-') << "\n";
    out << "Total active stylists: " << count << "\n";

    cout << out.str();
    if (count > 0 && getYesNo("\nExport stylist list to file?") == 'Y')
        exportToFile("stylist_list", out.str());
    pauseScreen();
}

// ============================ SERVICE FUNCTIONS ============================
static string chooseCategory() {
    cout << "Category:\n  1. Hair\n  2. Nail\n  3. Spa\n  4. Makeup\n";
    int c = getValidatedInt("Choice: ", 1, 4);
    switch (c) {
        case 1: return "Hair";
        case 2: return "Nail";
        case 3: return "Spa";
        default: return "Makeup";
    }
}

static bool isDuplicateServiceName(const vector<Service> &services, const string &name) {
    for (const Service &s : services) {
        if (s.isActive && s.serviceName == name) return true;
    }
    return false;
}

void addService(vector<Service> &services, int &nextServiceID) {
    clearScreen();
    printHeader("ADD NEW SERVICE");

    Service sv;
    sv.serviceID = nextServiceID;

    string name;
    bool duplicate;
    do {
        name = getValidatedName("Service Name (or 0 to cancel): ", true);
        if (name.empty()) { cout << "\nCancelled.\n"; pauseScreen(); return; }
        duplicate = isDuplicateServiceName(services, name);
        if (duplicate) cout << "A service with this name already exists.\n";
    } while (duplicate);
    sv.serviceName = name;

    sv.category = chooseCategory();
    sv.price    = getValidatedDouble("Price (RM, or 0 to cancel): ", 0.0, 10000.0);
    if (sv.price == 0.0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    sv.duration = getValidatedInt("Duration (minutes, or 0 to cancel): ", 0, 480);
    if (sv.duration == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    sv.isActive = true;

    services.push_back(sv);
    nextServiceID++;
    cout << "\nService added successfully! Assigned Service ID: " << sv.serviceID << "\n";
    pauseScreen();
}

void updateService(vector<Service> &services) {
    clearScreen();
    printHeader("UPDATE SERVICE");
    cout << fixed << setprecision(2);
    cout << left << setw(6) << "ID" << setw(22) << "Service Name" << setw(8) << "Price" << "\n";
    printDivider('-', 40);
    for (const Service &sv : services)
        if (sv.isActive) cout << left << setw(6) << sv.serviceID << setw(22) << sv.serviceName << "RM" << sv.price << "\n";
    printDivider('-', 40);
    int id = getValidatedInt("\nEnter Service ID to update (or 0 to cancel): ", 0, 999999);
    if (id == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int idx = findServiceIndex(services, id);
    if (idx == -1) { cout << "\nService not found.\n"; pauseScreen(); return; }

    int choice;
    do {
        cout << fixed << setprecision(2);
        cout << "\nCurrent -> Name: " << services[idx].serviceName << " | Category: " << services[idx].category
             << " | Price: RM" << services[idx].price << " | Duration: " << services[idx].duration << " min\n";
        cout << "1. Update Name\n2. Update Category\n3. Update Price\n4. Update Duration\n0. Done\n";
        choice = getValidatedInt("Choice: ", 0, 4);

        switch (choice) {
            case 1: services[idx].serviceName = getValidatedName("New Name: "); break;
            case 2: services[idx].category = chooseCategory(); break;
            case 3: services[idx].price = getValidatedDouble("New Price (RM): ", 1.0, 10000.0); break;
            case 4: services[idx].duration = getValidatedInt("New Duration (minutes): ", 5, 480); break;
            case 0: break;
        }
    } while (choice != 0);

    cout << "\nService record updated successfully.\n";
    pauseScreen();
}

void deleteService(vector<Service> &services) {
    clearScreen();
    printHeader("DELETE SERVICE");
    cout << fixed << setprecision(2);
    cout << left << setw(6) << "ID" << setw(22) << "Service Name" << setw(8) << "Price" << "\n";
    printDivider('-', 40);
    for (const Service &sv : services)
        if (sv.isActive) cout << left << setw(6) << sv.serviceID << setw(22) << sv.serviceName << "RM" << sv.price << "\n";
    printDivider('-', 40);
    int id = getValidatedInt("\nEnter Service ID to delete (or 0 to cancel): ", 0, 999999);
    if (id == 0) { cout << "\nCancelled.\n"; pauseScreen(); return; }
    int idx = findServiceIndex(services, id);
    if (idx == -1) { cout << "\nService not found.\n"; pauseScreen(); return; }

    cout << "\nService: " << services[idx].serviceName << " (ID " << services[idx].serviceID << ")\n";
    char confirm = getYesNo("Are you sure you want to delete this service?");
    if (confirm == 'Y') {
        services[idx].isActive = false;
        cout << "\nService deleted successfully.\n";
    } else {
        cout << "\nDeletion cancelled.\n";
    }
    pauseScreen();
}

void viewServices(const vector<Service> &services) {
    clearScreen();
    printHeader("ALL SERVICES");
    ostringstream out;
    int count = 0;
    out << fixed << setprecision(2);
    out << left << setw(6) << "ID" << setw(22) << "Service Name" << setw(10) << "Category"
        << setw(10) << "Price" << "Duration\n";
    out << string(65, '-') << "\n";
    for (const Service &sv : services) {
        if (!sv.isActive) continue;
        out << left << setw(6) << sv.serviceID << setw(22) << sv.serviceName << setw(10) << sv.category
            << "RM" << setw(8) << sv.price << sv.duration << " min\n";
        count++;
    }
    if (count == 0) out << "(No services on file yet.)\n";
    out << string(65, '-') << "\n";
    out << "Total active services: " << count << "\n";

    cout << out.str();
    if (count > 0 && getYesNo("\nExport service list to file?") == 'Y')
        exportToFile("service_list", out.str());
    pauseScreen();
}

// -------------------------------------------------------------------------
// serviceStylistMenu - Module 2's entry point, called from main.cpp.
// -------------------------------------------------------------------------
void serviceStylistMenu(vector<Stylist> &stylists, vector<Service> &services,
                         int &nextStylistID, int &nextServiceID) {
    int choice;
    do {
        clearScreen();
        printHeader("SERVICE & STYLIST MANAGEMENT");
        cout << "1. Stylist Management\n";
        cout << "2. Service Management\n";
        cout << "0. Back to Main Menu\n";
        choice = getValidatedInt("Enter your choice: ", 0, 2);

        if (choice == 1) {
            int styChoice;
            do {
                clearScreen();
                printHeader("STYLIST MANAGEMENT");
                cout << "1. Add Stylist\n";
                cout << "2. Update Stylist\n";
                cout << "3. Delete Stylist\n";
                cout << "4. View Stylists\n";
                cout << "0. Back\n";
                styChoice = getValidatedInt("Enter your choice: ", 0, 4);
                switch (styChoice) {
                    case 1: addStylist(stylists, nextStylistID); break;
                    case 2: updateStylist(stylists); break;
                    case 3: deleteStylist(stylists); break;
                    case 4: viewStylists(stylists); break;
                    case 0: break;
                }
            } while (styChoice != 0);
        } else if (choice == 2) {
            int svcChoice;
            do {
                clearScreen();
                printHeader("SERVICE MANAGEMENT");
                cout << "1. Add Service\n";
                cout << "2. Update Service\n";
                cout << "3. Delete Service\n";
                cout << "4. View Services\n";
                cout << "0. Back\n";
                svcChoice = getValidatedInt("Enter your choice: ", 0, 4);
                switch (svcChoice) {
                    case 1: addService(services, nextServiceID); break;
                    case 2: updateService(services); break;
                    case 3: deleteService(services); break;
                    case 4: viewServices(services); break;
                    case 0: break;
                }
            } while (svcChoice != 0);
        }
    } while (choice != 0);
}
