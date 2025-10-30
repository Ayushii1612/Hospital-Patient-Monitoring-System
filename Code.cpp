#include <iostream>
#include <queue>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <ctime>
#include <sstream>
#include <chrono>
#include <fstream>

using namespace std;
using namespace chrono;

// ==================== ENUMERATIONS ====================
enum class Priority {
    CRITICAL = 1,
    HIGH = 2,
    MEDIUM = 3,
    LOW = 4
};

enum class VitalSign {
    HEART_RATE = 1,
    BLOOD_PRESSURE = 2,
    OXYGEN_SATURATION = 3,
    TEMPERATURE = 4,
    RESPIRATORY_RATE = 5
};

enum class Gender {
    MALE,
    FEMALE,
    OTHER
};

// ==================== UTILITY FUNCTIONS ====================
class Utils {
public:
    static string getCurrentDateTime() {
        auto now = system_clock::now();
        time_t now_time = system_clock::to_time_t(now);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now_time));
        return string(buffer);
    }
    
    static string getCurrentDate() {
        auto now = system_clock::now();
        time_t now_time = system_clock::to_time_t(now);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", localtime(&now_time));
        return string(buffer);
    }
    
    static void printBox(const string& text, int width = 70) {
        cout << "+" << string(width - 2, '=') << "+" << endl;
        int padding = (width - 2 - text.length()) / 2;
        cout << "|" << string(padding, ' ') << text 
             << string(width - 2 - padding - text.length(), ' ') << "|" << endl;
        cout << "+" << string(width - 2, '=') << "+" << endl;
    }
    
    static void printSeparator(char ch = '=', int width = 70) {
        cout << string(width, ch) << endl;
    }
    
    static string colorText(const string& text, Priority priority) {
        // Return text without colors for Windows compatibility
        return text;
    }
};

// ==================== VITAL READING STRUCTURE ====================
struct VitalReading {
    VitalSign type;
    double value;
    string timestamp;
    Priority riskLevel;
    
    VitalReading(VitalSign t, double v, string ts, Priority r)
        : type(t), value(v), timestamp(ts), riskLevel(r) {}
};

// ==================== PATIENT STRUCTURE ====================
struct Patient {
    int id;
    string name;
    int age;
    Gender gender;
    string admissionDate;
    string diagnosis;
    string ward;
    map<VitalSign, double> currentVitals;
    vector<VitalReading> vitalHistory;
    Priority riskLevel;
    bool isActive;
    
    Patient(int i, string n, int a, Gender g, string diag, string w) 
        : id(i), name(n), age(a), gender(g), diagnosis(diag), ward(w),
          riskLevel(Priority::LOW), isActive(true) {
        admissionDate = Utils::getCurrentDate();
    }
    
    void addVitalReading(VitalSign type, double value, Priority risk) {
        currentVitals[type] = value;
        vitalHistory.push_back(VitalReading(type, value, Utils::getCurrentDateTime(), risk));
        
        // Update overall risk level to highest priority among all vitals
        if (static_cast<int>(risk) < static_cast<int>(riskLevel)) {
            riskLevel = risk;
        }
    }
};

// ==================== ALERT STRUCTURE ====================
struct Alert {
    int alertId;
    int patientId;
    string patientName;
    Priority priority;
    string message;
    VitalSign vital;
    double value;
    string timestamp;
    bool isProcessed;
    
    Alert(int aid, int pid, string pname, Priority p, string msg, VitalSign v, double val)
        : alertId(aid), patientId(pid), patientName(pname), priority(p), 
          message(msg), vital(v), value(val), isProcessed(false) {
        timestamp = Utils::getCurrentDateTime();
    }
};

// ==================== ALERT COMPARATOR ====================
struct AlertComparator {
    bool operator()(const shared_ptr<Alert>& a, const shared_ptr<Alert>& b) {
        if (a->priority != b->priority) {
            return a->priority > b->priority; // Lower enum value = higher priority
        }
        return a->alertId > b->alertId; // Earlier alerts first
    }
};

// ==================== HOSPITAL SYSTEM CLASS ====================
class HospitalSystem {
private:
    map<int, shared_ptr<Patient>> patients;
    priority_queue<shared_ptr<Alert>, vector<shared_ptr<Alert>>, AlertComparator> alertQueue;
    vector<shared_ptr<Alert>> processedAlerts;
    int alertCount;
    int nextPatientId;
    
public:
    HospitalSystem() : alertCount(0), nextPatientId(1001) {}
    
    // ==================== INPUT HANDLING ====================
    void clearInput() {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    
    int getValidatedInt(const string& prompt, int min, int max) {
        int value;
        while (true) {
            cout << prompt;
            if (cin >> value && value >= min && value <= max) {
                return value;
            }
            cout << "[X] Invalid input! Please enter a number between " << min << " and " << max << "." << endl;
            clearInput();
        }
    }
    
    double getValidatedDouble(const string& prompt, double min, double max) {
        double value;
        while (true) {
            cout << prompt;
            if (cin >> value && value >= min && value <= max) {
                return value;
            }
            cout << "[X] Invalid input! Please enter a number between " << min << " and " << max << "." << endl;
            clearInput();
        }
    }
    
    // ==================== DISPLAY MENU ====================
    void displayMenu() {
        cout << "\n";
        Utils::printSeparator('=', 75);
        cout << "|" << string(15, ' ') << "*** HOSPITAL PATIENT MONITORING SYSTEM ***" 
             << string(15, ' ') << "|" << endl;
        Utils::printSeparator('=', 75);
        
        cout << "\n+-------------------------------------------------------------------------+" << endl;
        cout << "|  [*] PATIENT MANAGEMENT                                                 |" << endl;
        cout << "|     1. Register New Patient                                             |" << endl;
        cout << "|     2. View All Patients                                                |" << endl;
        cout << "|     3. Search Patient by ID                                             |" << endl;
        cout << "|     4. View Patient Complete Profile                                    |" << endl;
        cout << "|     5. Discharge Patient                                                |" << endl;
        cout << "+-------------------------------------------------------------------------+" << endl;
        cout << "|  [+] VITAL SIGNS MONITORING                                             |" << endl;
        cout << "|     6. Record Vital Signs                                               |" << endl;
        cout << "|     7. View Vital Signs History                                         |" << endl;
        cout << "|     8. Bulk Vital Entry (Multiple Vitals)                               |" << endl;
        cout << "+-------------------------------------------------------------------------+" << endl;
        cout << "|  [!] ALERT MANAGEMENT                                                   |" << endl;
        cout << "|     9. View Active Alerts Queue                                         |" << endl;
        cout << "|    10. Process Next Critical Alert                                      |" << endl;
        cout << "|    11. Process All Alerts                                               |" << endl;
        cout << "|    12. View Processed Alerts History                                    |" << endl;
        cout << "+-------------------------------------------------------------------------+" << endl;
        cout << "|  [#] REPORTS & ANALYTICS                                                |" << endl;
        cout << "|    13. System Dashboard                                                 |" << endl;
        cout << "|    14. Critical Patients Report                                         |" << endl;
        cout << "|    15. Ward-wise Summary                                                |" << endl;
        cout << "|    16. Export System Report                                             |" << endl;
        cout << "+-------------------------------------------------------------------------+" << endl;
        cout << "|    17. [EXIT] Exit System                                               |" << endl;
        cout << "+-------------------------------------------------------------------------+" << endl;
        
        cout << "\n[TIME] Current Time: " << Utils::getCurrentDateTime() << endl;
        cout << "[INFO] Active Patients: " << getActivePatientCount() << " | ";
        cout << "Pending Alerts: " << alertQueue.size() << endl;
        Utils::printSeparator('-', 75);
    }
    
    // ==================== PATIENT MANAGEMENT ====================
    void registerPatient() {
        cout << "\n";
        Utils::printBox("PATIENT REGISTRATION", 75);
        
        int id = nextPatientId++;
        cout << "\n[OK] Patient ID Auto-Generated: " << id << endl;
        
        clearInput();
        
        string name;
        cout << "\nEnter Patient Details:" << endl;
        cout << "Full Name: ";
        getline(cin, name);
        
        int age = getValidatedInt("Age (0-120): ", 0, 120);
        
        cout << "\nGender:" << endl;
        cout << "  1. Male" << endl;
        cout << "  2. Female" << endl;
        cout << "  3. Other" << endl;
        int genderChoice = getValidatedInt("Select (1-3): ", 1, 3);
        Gender gender = static_cast<Gender>(genderChoice - 1);
        
        clearInput();
        
        string diagnosis;
        cout << "Initial Diagnosis/Complaint: ";
        getline(cin, diagnosis);
        
        cout << "\nWard Assignment:" << endl;
        cout << "  1. Emergency" << endl;
        cout << "  2. ICU" << endl;
        cout << "  3. General Ward" << endl;
        cout << "  4. Private Room" << endl;
        cout << "  5. Pediatrics" << endl;
        int wardChoice = getValidatedInt("Select Ward (1-5): ", 1, 5);
        
        string ward;
        switch (wardChoice) {
            case 1: ward = "Emergency"; break;
            case 2: ward = "ICU"; break;
            case 3: ward = "General Ward"; break;
            case 4: ward = "Private Room"; break;
            case 5: ward = "Pediatrics"; break;
        }
        
        auto patient = make_shared<Patient>(id, name, age, gender, diagnosis, ward);
        patients[id] = patient;
        
        cout << "\n";
        Utils::printSeparator('-', 75);
        cout << "[SUCCESS] PATIENT REGISTERED SUCCESSFULLY!" << endl;
        Utils::printSeparator('-', 75);
        cout << "| Patient ID    : " << id << endl;
        cout << "| Name          : " << name << endl;
        cout << "| Age/Gender    : " << age << " years / " << genderToString(gender) << endl;
        cout << "| Diagnosis     : " << diagnosis << endl;
        cout << "| Ward          : " << ward << endl;
        cout << "| Admission Date: " << patient->admissionDate << endl;
        Utils::printSeparator('-', 75);
    }
    
    void viewAllPatients() {
        cout << "\n";
        Utils::printBox("ACTIVE PATIENTS LIST", 75);
        
        if (patients.empty()) {
            cout << "\n[INFO] No patients registered in the system." << endl;
            return;
        }
        
        int activeCount = 0;
        cout << "\n" << left << setw(6) << "ID" 
             << setw(25) << "Name" 
             << setw(6) << "Age" 
             << setw(15) << "Ward"
             << setw(15) << "Risk Level" << endl;
        Utils::printSeparator('-', 75);
        
        for (const auto& pair : patients) {
            auto p = pair.second;
            if (p->isActive) {
                activeCount++;
                cout << left << setw(6) << p->id
                     << setw(25) << (p->name.length() > 24 ? p->name.substr(0, 21) + "..." : p->name)
                     << setw(6) << p->age
                     << setw(15) << p->ward
                     << priorityToString(p->riskLevel) << endl;
            }
        }
        
        cout << "\n[STATS] Total Active Patients: " << activeCount << endl;
    }
    
    void searchPatient() {
        cout << "\n";
        Utils::printBox("SEARCH PATIENT", 75);
        
        int id = getValidatedInt("\nEnter Patient ID: ", 1000, 99999);
        
        if (patients.find(id) == patients.end()) {
            cout << "\n[ERROR] Patient with ID " << id << " not found!" << endl;
            return;
        }
        
        auto patient = patients[id];
        
        if (!patient->isActive) {
            cout << "\n[WARNING] Patient has been discharged." << endl;
        }
        
        displayPatientSummary(patient);
    }
    
    void viewPatientProfile() {
        cout << "\n";
        Utils::printBox("COMPLETE PATIENT PROFILE", 75);
        
        int id = getValidatedInt("\nEnter Patient ID: ", 1000, 99999);
        
        if (patients.find(id) == patients.end()) {
            cout << "\n[ERROR] Patient not found!" << endl;
            return;
        }
        
        auto patient = patients[id];
        
        cout << "\n";
        Utils::printSeparator('=', 75);
        cout << "PATIENT INFORMATION" << endl;
        Utils::printSeparator('=', 75);
        cout << "| ID             : " << patient->id << endl;
        cout << "| Name           : " << patient->name << endl;
        cout << "| Age            : " << patient->age << " years" << endl;
        cout << "| Gender         : " << genderToString(patient->gender) << endl;
        cout << "| Ward           : " << patient->ward << endl;
        cout << "| Diagnosis      : " << patient->diagnosis << endl;
        cout << "| Admission Date : " << patient->admissionDate << endl;
        cout << "| Status         : " << (patient->isActive ? "Active" : "Discharged") << endl;
        cout << "| Risk Level     : " << priorityToString(patient->riskLevel) << endl;
        
        Utils::printSeparator('-', 75);
        cout << "CURRENT VITAL SIGNS" << endl;
        Utils::printSeparator('-', 75);
        
        if (patient->currentVitals.empty()) {
            cout << "| No vital signs recorded yet." << endl;
        } else {
            for (const auto& vital : patient->currentVitals) {
                Priority risk = assessRisk(vital.first, vital.second);
                cout << "| " << left << setw(25) << vitalSignToString(vital.first) 
                     << ": " << setw(8) << vital.second << " " << setw(10) << getUnit(vital.first)
                     << " [" << priorityToString(risk) << "]" << endl;
            }
        }
        
        if (!patient->vitalHistory.empty()) {
            Utils::printSeparator('-', 75);
            cout << "RECENT VITAL READINGS (Last 5)" << endl;
            Utils::printSeparator('-', 75);
            
            int count = 0;
            for (auto it = patient->vitalHistory.rbegin(); 
                 it != patient->vitalHistory.rend() && count < 5; ++it, ++count) {
                cout << "| " << it->timestamp << " | " 
                     << vitalSignToString(it->type) << ": " << it->value 
                     << " [" << priorityToString(it->riskLevel) << "]" << endl;
            }
        }
        Utils::printSeparator('=', 75);
    }
    
    void dischargePatient() {
        cout << "\n";
        Utils::printBox("DISCHARGE PATIENT", 75);
        
        int id = getValidatedInt("\nEnter Patient ID: ", 1000, 99999);
        
        if (patients.find(id) == patients.end()) {
            cout << "\n[ERROR] Patient not found!" << endl;
            return;
        }
        
        auto patient = patients[id];
        
        if (!patient->isActive) {
            cout << "\n[WARNING] Patient is already discharged!" << endl;
            return;
        }
        
        cout << "\n[INFO] Patient: " << patient->name << " (ID: " << id << ")" << endl;
        cout << "Are you sure you want to discharge this patient? (Y/N): ";
        
        char confirm;
        cin >> confirm;
        
        if (confirm == 'Y' || confirm == 'y') {
            patient->isActive = false;
            cout << "\n[SUCCESS] Patient discharged successfully on " << Utils::getCurrentDateTime() << endl;
        } else {
            cout << "\n[CANCELLED] Discharge cancelled." << endl;
        }
    }
    
    // ==================== VITAL SIGNS MONITORING ====================
    void recordVitalSigns() {
        cout << "\n";
        Utils::printBox("RECORD VITAL SIGNS", 75);
        
        if (getActivePatientCount() == 0) {
            cout << "\n[INFO] No active patients. Please register patients first." << endl;
            return;
        }
        
        int id = getValidatedInt("\nEnter Patient ID: ", 1000, 99999);
        
        if (patients.find(id) == patients.end() || !patients[id]->isActive) {
            cout << "\n[ERROR] Active patient not found!" << endl;
            return;
        }
        
        auto patient = patients[id];
        
        cout << "\n[PATIENT] Recording vitals for: " << patient->name << " (ID: " << id << ")" << endl;
        cout << "\nSelect Vital Sign:" << endl;
        cout << "  1. Heart Rate (bpm)" << endl;
        cout << "  2. Blood Pressure (systolic mmHg)" << endl;
        cout << "  3. Oxygen Saturation (%)" << endl;
        cout << "  4. Temperature (C)" << endl;
        cout << "  5. Respiratory Rate (breaths/min)" << endl;
        
        int vitalChoice = getValidatedInt("Select (1-5): ", 1, 5);
        VitalSign vital = static_cast<VitalSign>(vitalChoice);
        
        double value = getValidatedDouble("\nEnter " + vitalSignToString(vital) + " value: ", 0, 500);
        
        Priority risk = assessRisk(vital, value);
        patient->addVitalReading(vital, value, risk);
        
        cout << "\n";
        Utils::printSeparator('-', 75);
        cout << "[SUCCESS] VITAL SIGN RECORDED!" << endl;
        Utils::printSeparator('-', 75);
        cout << "| Vital Sign : " << vitalSignToString(vital) << endl;
        cout << "| Value      : " << value << " " << getUnit(vital) << endl;
        cout << "| Risk Level : " << priorityToString(risk) << endl;
        cout << "| Time       : " << Utils::getCurrentDateTime() << endl;
        Utils::printSeparator('-', 75);
        
        if (risk != Priority::LOW) {
            generateAlert(patient, vital, value, risk);
        }
    }
    
    void viewVitalHistory() {
        cout << "\n";
        Utils::printBox("VITAL SIGNS HISTORY", 75);
        
        int id = getValidatedInt("\nEnter Patient ID: ", 1000, 99999);
        
        if (patients.find(id) == patients.end()) {
            cout << "\n[ERROR] Patient not found!" << endl;
            return;
        }
        
        auto patient = patients[id];
        
        if (patient->vitalHistory.empty()) {
            cout << "\n[INFO] No vital signs recorded for this patient." << endl;
            return;
        }
        
        cout << "\n[PATIENT] " << patient->name << " (ID: " << id << ")" << endl;
        cout << "\n" << left << setw(22) << "Timestamp" 
             << setw(25) << "Vital Sign"
             << setw(12) << "Value"
             << "Risk Level" << endl;
        Utils::printSeparator('-', 75);
        
        for (auto it = patient->vitalHistory.rbegin(); it != patient->vitalHistory.rend(); ++it) {
            cout << left << setw(22) << it->timestamp
                 << setw(25) << vitalSignToString(it->type)
                 << setw(12) << (to_string(it->value) + " " + getUnit(it->type))
                 << priorityToString(it->riskLevel) << endl;
        }
        
        cout << "\n[STATS] Total Readings: " << patient->vitalHistory.size() << endl;
    }
    
    void bulkVitalEntry() {
        cout << "\n";
        Utils::printBox("BULK VITAL ENTRY", 75);
        
        int id = getValidatedInt("\nEnter Patient ID: ", 1000, 99999);
        
        if (patients.find(id) == patients.end() || !patients[id]->isActive) {
            cout << "\n[ERROR] Active patient not found!" << endl;
            return;
        }
        
        auto patient = patients[id];
        cout << "\n[PATIENT] Recording multiple vitals for: " << patient->name << endl;
        
        cout << "\nEnter the following vital signs:" << endl;
        
        double hr = getValidatedDouble("\n1. Heart Rate (bpm): ", 0, 300);
        Priority riskHR = assessRisk(VitalSign::HEART_RATE, hr);
        patient->addVitalReading(VitalSign::HEART_RATE, hr, riskHR);
        if (riskHR != Priority::LOW) generateAlert(patient, VitalSign::HEART_RATE, hr, riskHR);
        
        double bp = getValidatedDouble("2. Blood Pressure (systolic mmHg): ", 0, 300);
        Priority riskBP = assessRisk(VitalSign::BLOOD_PRESSURE, bp);
        patient->addVitalReading(VitalSign::BLOOD_PRESSURE, bp, riskBP);
        if (riskBP != Priority::LOW) generateAlert(patient, VitalSign::BLOOD_PRESSURE, bp, riskBP);
        
        double spo2 = getValidatedDouble("3. Oxygen Saturation (%): ", 0, 100);
        Priority riskSPO2 = assessRisk(VitalSign::OXYGEN_SATURATION, spo2);
        patient->addVitalReading(VitalSign::OXYGEN_SATURATION, spo2, riskSPO2);
        if (riskSPO2 != Priority::LOW) generateAlert(patient, VitalSign::OXYGEN_SATURATION, spo2, riskSPO2);
        
        double temp = getValidatedDouble("4. Temperature (C): ", 30, 45);
        Priority riskTemp = assessRisk(VitalSign::TEMPERATURE, temp);
        patient->addVitalReading(VitalSign::TEMPERATURE, temp, riskTemp);
        if (riskTemp != Priority::LOW) generateAlert(patient, VitalSign::TEMPERATURE, temp, riskTemp);
        
        double rr = getValidatedDouble("5. Respiratory Rate (breaths/min): ", 0, 100);
        Priority riskRR = assessRisk(VitalSign::RESPIRATORY_RATE, rr);
        patient->addVitalReading(VitalSign::RESPIRATORY_RATE, rr, riskRR);
        if (riskRR != Priority::LOW) generateAlert(patient, VitalSign::RESPIRATORY_RATE, rr, riskRR);
        
        cout << "\n[SUCCESS] All vital signs recorded successfully!" << endl;
        cout << "[INFO] Overall Patient Risk Level: " << priorityToString(patient->riskLevel) << endl;
    }
    
    // ==================== ALERT MANAGEMENT ====================
    void viewAlertsQueue() {
        cout << "\n";
        Utils::printBox("ACTIVE ALERTS QUEUE", 75);
        
        if (alertQueue.empty()) {
            cout << "\n[OK] No pending alerts. All patients stable!" << endl;
            return;
        }
        
        priority_queue<shared_ptr<Alert>, vector<shared_ptr<Alert>>, AlertComparator> tempQueue = alertQueue;
        
        cout << "\n[ALERT] Total Pending Alerts: " << alertQueue.size() << endl;
        Utils::printSeparator('-', 75);
        
        int count = 1;
        while (!tempQueue.empty()) {
            auto alert = tempQueue.top();
            tempQueue.pop();
            
            cout << "\n" << count++ << ". ";
            cout << "[" << priorityToString(alert->priority) << "]";
            cout << " Alert ID: " << alert->alertId << endl;
            cout << "   Patient: " << alert->patientName << " (ID: " << alert->patientId << ")" << endl;
            cout << "   Issue: " << alert->message << " (Value: " << alert->value << " " << getUnit(alert->vital) << ")" << endl;
            cout << "   Time: " << alert->timestamp << endl;
        }
        Utils::printSeparator('-', 75);
    }
    
    void processNextAlert() {
        cout << "\n";
        Utils::printBox("PROCESS CRITICAL ALERT", 75);
        
        if (alertQueue.empty()) {
            cout << "\n[OK] No pending alerts to process." << endl;
            return;
        }
        
        auto alert = alertQueue.top();
        alertQueue.pop();
        alert->isProcessed = true;
        processedAlerts.push_back(alert);
        
        cout << "\n[!] PROCESSING ALERT #" << alert->alertId << endl;
        Utils::printSeparator('=', 75);
        cout << "Priority   : " << priorityToString(alert->priority) << endl;
        cout << "Patient    : " << alert->patientName << " (ID: " << alert->patientId << ")" << endl;
        cout << "Issue      : " << alert->message << endl;
        cout << "Value      : " << alert->value << " " << getUnit(alert->vital) << endl;
        cout << "Alert Time : " << alert->timestamp << endl;
        Utils::printSeparator('=', 75);
        
        displayAction(alert->priority);
        
        cout << "\n[SUCCESS] Alert processed and logged!" << endl;
        cout << "[INFO] Remaining alerts: " << alertQueue.size() << endl;
    }
    
    void processAllAlerts() {
        cout << "\n";
        Utils::printBox("PROCESS ALL ALERTS", 75);
        
        if (alertQueue.empty()) {
            cout << "\n[OK] No pending alerts." << endl;
            return;
        }
        
        int count = 0;
        cout << "\n[!] Processing all pending alerts...\n" << endl;
        
        while (!alertQueue.empty()) {
            auto alert = alertQueue.top();
            alertQueue.pop();
            alert->isProcessed = true;
            processedAlerts.push_back(alert);
            count++;
            
            cout << count << ". ";
            cout << "[" << priorityToString(alert->priority) << "]";
            cout << " " << alert->patientName << ": " << alert->message;
            cout << " (" << alert->value << " " << getUnit(alert->vital) << ")";
            
            if (alert->priority == Priority::CRITICAL) {
                cout << " >>> [!] IMMEDIATE ACTION REQUIRED!";
            }
            cout << endl;
        }
        
        cout << "\n[SUCCESS] All " << count << " alerts processed successfully!" << endl;
    }
    
    void viewProcessedAlerts() {
        cout << "\n";
        Utils::printBox("PROCESSED ALERTS HISTORY", 75);
        
        if (processedAlerts.empty()) {
            cout << "\n[INFO] No alerts have been processed yet." << endl;
            return;
        }
        
        cout << "\n[STATS] Total Processed Alerts: " << processedAlerts.size() << endl;
        Utils::printSeparator('-', 75);
        
        for (size_t i = 0; i < min(processedAlerts.size(), size_t(20)); ++i) {
            auto alert = processedAlerts[processedAlerts.size() - 1 - i];
            cout << "\n" << (i + 1) << ". ";
            cout << "[" << priorityToString(alert->priority) << "]";
            cout << " Alert #" << alert->alertId << endl;
            cout << "   Patient: " << alert->patientName << " | " << alert->message << endl;
            cout << "   Processed: " << alert->timestamp << endl;
        }
        
        if (processedAlerts.size() > 20) {
            cout << "\n... and " << (processedAlerts.size() - 20) << " more alerts" << endl;
        }
    }
    
    // ==================== REPORTS & ANALYTICS ====================
    void systemDashboard() {
        cout << "\n";
        Utils::printBox("SYSTEM DASHBOARD", 75);
        
        int totalPatients = patients.size();
        int activePatients = getActivePatientCount();
        int discharged = totalPatients - activePatients;
        
        int critical = 0, high = 0, medium = 0, low = 0;
        for (const auto& pair : patients) {
            if (pair.second->isActive) {
                switch (pair.second->riskLevel) {
                    case Priority::CRITICAL: critical++; break;
                    case Priority::HIGH: high++; break;
                    case Priority::MEDIUM: medium++; break;
                    case Priority::LOW: low++; break;
                }
            }
        }
        
        cout << "\n+-------------------------------------------------------------------------+" << endl;
        cout << "| [#] PATIENT STATISTICS                                                  |" << endl;
        cout << "+-------------------------------------------------------------------------+" << endl;
        cout << "|   Total Registered    : " << setw(3) << totalPatients << " patients" << string(44, ' ') << "|" << endl;
        cout << "|   Currently Active    : " << setw(3) << activePatients << " patients" << string(44, ' ') << "|" << endl;
        cout << "|   Discharged          : " << setw(3) << discharged << " patients" << string(44, ' ') << "|" << endl;
        cout << "+-------------------------------------------------------------------------+" << endl;
        
        cout << "\n+-------------------------------------------------------------------------+" << endl;
        cout << "| [!] RISK LEVEL DISTRIBUTION                                             |" << endl;
        cout << "+-------------------------------------------------------------------------+" << endl;
        cout << "|   [CRITICAL]          : " << setw(3) << critical << " patients" << string(44, ' ') << "|" << endl;
        cout << "|   [HIGH]              : " << setw(3) << high << " patients" << string(44, ' ') << "|" << endl;
        cout << "|   [MEDIUM]            : " << setw(3) << medium << " patients" << string(44, ' ') << "|" << endl;
        cout << "|   [LOW]               : " << setw(3) << low << " patients" << string(44, ' ') << "|" << endl;
        cout << "+-------------------------------------------------------------------------+" << endl;
        
        cout << "\n+-------------------------------------------------------------------------+" << endl;
        cout << "| [*] ALERT STATISTICS                                                    |" << endl;
        cout << "+-------------------------------------------------------------------------+" << endl;
        cout << "|   Total Alerts Generated  : " << setw(4) << alertCount << string(44, ' ') << "|" << endl;
        cout << "|   Pending Alerts          : " << setw(4) << alertQueue.size() << string(44, ' ') << "|" << endl;
        cout << "|   Processed Alerts        : " << setw(4) << processedAlerts.size() << string(44, ' ') << "|" << endl;
        cout << "+-------------------------------------------------------------------------+" << endl;
        
        cout << "\n[TIME] Report Generated: " << Utils::getCurrentDateTime() << endl;
    }
    
    void criticalPatientsReport() {
        cout << "\n";
        Utils::printBox("CRITICAL PATIENTS REPORT", 75);
        
        vector<shared_ptr<Patient>> criticalPatients;
        for (const auto& pair : patients) {
            if (pair.second->isActive && 
                (pair.second->riskLevel == Priority::CRITICAL || 
                 pair.second->riskLevel == Priority::HIGH)) {
                criticalPatients.push_back(pair.second);
            }
        }
        
        if (criticalPatients.empty()) {
            cout << "\n[OK] No critical or high-risk patients!" << endl;
            return;
        }
        
        cout << "\n[ALERT] Critical/High-Risk Patients: " << criticalPatients.size() << endl;
        Utils::printSeparator('-', 75);
        
        for (const auto& p : criticalPatients) {
            cout << "\n[PATIENT] " << p->name << " (ID: " << p->id << ")" << endl;
            cout << "   Ward: " << p->ward << " | Age: " << p->age << " | Gender: " << genderToString(p->gender) << endl;
            cout << "   Risk Level: " << priorityToString(p->riskLevel) << endl;
            cout << "   Diagnosis: " << p->diagnosis << endl;
            
            if (!p->currentVitals.empty()) {
                cout << "   Current Vitals:" << endl;
                for (const auto& vital : p->currentVitals) {
                    Priority risk = assessRisk(vital.first, vital.second);
                    if (risk == Priority::CRITICAL || risk == Priority::HIGH) {
                        cout << "     - " << vitalSignToString(vital.first) << ": " 
                             << vital.second << " " << getUnit(vital.first)
                             << " [" << priorityToString(risk) << "]" << endl;
                    }
                }
            }
        }
        
        cout << "\n" << string(75, '-') << endl;
        cout << "[WARNING] RECOMMENDATION: Immediate medical attention required!" << endl;
    }
    
    void wardWiseSummary() {
        cout << "\n";
        Utils::printBox("WARD-WISE SUMMARY", 75);
        
        map<string, vector<shared_ptr<Patient>>> wardPatients;
        
        for (const auto& pair : patients) {
            if (pair.second->isActive) {
                wardPatients[pair.second->ward].push_back(pair.second);
            }
        }
        
        if (wardPatients.empty()) {
            cout << "\n[INFO] No active patients in any ward." << endl;
            return;
        }
        
        for (const auto& ward : wardPatients) {
            cout << "\n[WARD] " << ward.first << endl;
            Utils::printSeparator('-', 75);
            cout << "Total Patients: " << ward.second.size() << endl;
            
            int criticalCount = 0, highCount = 0, mediumCount = 0, lowCount = 0;
            for (const auto& p : ward.second) {
                switch (p->riskLevel) {
                    case Priority::CRITICAL: criticalCount++; break;
                    case Priority::HIGH: highCount++; break;
                    case Priority::MEDIUM: mediumCount++; break;
                    case Priority::LOW: lowCount++; break;
                }
            }
            
            cout << "Risk Distribution: ";
            if (criticalCount > 0) cout << "Critical(" << criticalCount << ") ";
            if (highCount > 0) cout << "High(" << highCount << ") ";
            if (mediumCount > 0) cout << "Medium(" << mediumCount << ") ";
            if (lowCount > 0) cout << "Low(" << lowCount << ")";
            cout << endl;
            
            cout << "\nPatients:" << endl;
            for (const auto& p : ward.second) {
                cout << "  * " << p->name << " (ID: " << p->id << ") - " 
                     << priorityToString(p->riskLevel) << endl;
            }
        }
        cout << "\n" << string(75, '-') << endl;
    }
    
    void exportSystemReport() {
        cout << "\n";
        Utils::printBox("EXPORT SYSTEM REPORT", 75);
        
        string filename = "hospital_report_" + Utils::getCurrentDate() + ".txt";
        ofstream reportFile(filename);
        
        if (!reportFile.is_open()) {
            cout << "\n[ERROR] Unable to create report file!" << endl;
            return;
        }
        
        reportFile << "========================================" << endl;
        reportFile << "HOSPITAL PATIENT MONITORING SYSTEM" << endl;
        reportFile << "COMPREHENSIVE SYSTEM REPORT" << endl;
        reportFile << "========================================" << endl;
        reportFile << "Generated: " << Utils::getCurrentDateTime() << endl;
        reportFile << "========================================" << endl;
        
        reportFile << "\n--- PATIENT STATISTICS ---" << endl;
        reportFile << "Total Patients: " << patients.size() << endl;
        reportFile << "Active Patients: " << getActivePatientCount() << endl;
        
        reportFile << "\n--- ACTIVE PATIENTS LIST ---" << endl;
        for (const auto& pair : patients) {
            if (pair.second->isActive) {
                auto p = pair.second;
                reportFile << "\nID: " << p->id << " | Name: " << p->name << endl;
                reportFile << "Age: " << p->age << " | Gender: " << genderToString(p->gender) << endl;
                reportFile << "Ward: " << p->ward << " | Diagnosis: " << p->diagnosis << endl;
                reportFile << "Risk Level: " << priorityToString(p->riskLevel) << endl;
                
                if (!p->currentVitals.empty()) {
                    reportFile << "Current Vitals:" << endl;
                    for (const auto& vital : p->currentVitals) {
                        reportFile << "  - " << vitalSignToString(vital.first) 
                                  << ": " << vital.second << " " << getUnit(vital.first) << endl;
                    }
                }
            }
        }
        
        reportFile << "\n--- ALERT STATISTICS ---" << endl;
        reportFile << "Total Alerts: " << alertCount << endl;
        reportFile << "Pending: " << alertQueue.size() << endl;
        reportFile << "Processed: " << processedAlerts.size() << endl;
        
        reportFile << "\n========================================" << endl;
        reportFile << "End of Report" << endl;
        reportFile << "========================================" << endl;
        
        reportFile.close();
        
        cout << "\n[SUCCESS] Report exported successfully!" << endl;
        cout << "[FILE] " << filename << endl;
        cout << "[INFO] Report saved in current directory." << endl;
    }
    
    // ==================== HELPER FUNCTIONS ====================
    void displayPatientSummary(shared_ptr<Patient> patient) {
        cout << "\n" << string(75, '-') << endl;
        cout << "[PATIENT] " << patient->name << " (ID: " << patient->id << ")" << endl;
        cout << string(75, '-') << endl;
        cout << "Age: " << patient->age << " | Gender: " << genderToString(patient->gender) << endl;
        cout << "Ward: " << patient->ward << " | Status: " << (patient->isActive ? "Active" : "Discharged") << endl;
        cout << "Risk Level: " << priorityToString(patient->riskLevel) << endl;
        cout << string(75, '-') << endl;
    }
    
    void generateAlert(shared_ptr<Patient> patient, VitalSign vital, double value, Priority risk) {
        string message = vitalSignToString(vital) + " is " + getRiskDescription(vital, value);
        
        auto alert = make_shared<Alert>(
            ++alertCount,
            patient->id, 
            patient->name, 
            risk, 
            message, 
            vital, 
            value
        );
        
        alertQueue.push(alert);
        
        cout << "\n[WARNING] ALERT #" << alertCount << " GENERATED!" << endl;
        cout << "Priority: " << priorityToString(risk) << endl;
        cout << "Message: " << message << endl;
    }
    
    int getActivePatientCount() const {
        int count = 0;
        for (const auto& pair : patients) {
            if (pair.second->isActive) count++;
        }
        return count;
    }
    
    string genderToString(Gender g) {
        switch (g) {
            case Gender::MALE: return "Male";
            case Gender::FEMALE: return "Female";
            case Gender::OTHER: return "Other";
            default: return "Unknown";
        }
    }
    
    string priorityToString(Priority p) {
        switch (p) {
            case Priority::CRITICAL: return "CRITICAL";
            case Priority::HIGH: return "HIGH";
            case Priority::MEDIUM: return "MEDIUM";
            case Priority::LOW: return "LOW";
            default: return "UNKNOWN";
        }
    }
    
    string vitalSignToString(VitalSign v) {
        switch (v) {
            case VitalSign::HEART_RATE: return "Heart Rate";
            case VitalSign::BLOOD_PRESSURE: return "Blood Pressure";
            case VitalSign::OXYGEN_SATURATION: return "Oxygen Saturation";
            case VitalSign::TEMPERATURE: return "Temperature";
            case VitalSign::RESPIRATORY_RATE: return "Respiratory Rate";
            default: return "Unknown";
        }
    }
    
    string getUnit(VitalSign v) {
        switch (v) {
            case VitalSign::HEART_RATE: return "bpm";
            case VitalSign::BLOOD_PRESSURE: return "mmHg";
            case VitalSign::OXYGEN_SATURATION: return "%";
            case VitalSign::TEMPERATURE: return "C";
            case VitalSign::RESPIRATORY_RATE: return "breaths/min";
            default: return "";
        }
    }
    
    Priority assessRisk(VitalSign vital, double value) {
        switch (vital) {
            case VitalSign::HEART_RATE:
                if (value < 30 || value > 180) return Priority::CRITICAL;
                if (value < 50 || value > 120) return Priority::HIGH;
                if (value < 60 || value > 100) return Priority::MEDIUM;
                return Priority::LOW;
                
            case VitalSign::BLOOD_PRESSURE:
                if (value < 60 || value > 200) return Priority::CRITICAL;
                if (value < 80 || value > 160) return Priority::HIGH;
                if (value < 90 || value > 140) return Priority::MEDIUM;
                return Priority::LOW;
                
            case VitalSign::OXYGEN_SATURATION:
                if (value < 85) return Priority::CRITICAL;
                if (value < 92) return Priority::HIGH;
                if (value < 95) return Priority::MEDIUM;
                return Priority::LOW;
                
            case VitalSign::TEMPERATURE:
                if (value < 35.0 || value > 39.5) return Priority::CRITICAL;
                if (value < 35.5 || value > 38.5) return Priority::HIGH;
                if (value < 36.1 || value > 37.5) return Priority::MEDIUM;
                return Priority::LOW;
                
            case VitalSign::RESPIRATORY_RATE:
                if (value < 8 || value > 35) return Priority::CRITICAL;
                if (value < 10 || value > 25) return Priority::HIGH;
                if (value < 12 || value > 20) return Priority::MEDIUM;
                return Priority::LOW;
                
            default:
                return Priority::LOW;
        }
    }
    
    string getRiskDescription(VitalSign vital, double value) {
        Priority risk = assessRisk(vital, value);
        
        if (risk == Priority::CRITICAL) return "CRITICALLY ABNORMAL";
        if (risk == Priority::HIGH) return "ABNORMAL - HIGH RISK";
        if (risk == Priority::MEDIUM) return "SLIGHTLY ABNORMAL";
        return "NORMAL";
    }
    
    void displayAction(Priority priority) {
        cout << "\n+-------------------------------------------------------------------------+" << endl;
        cout << "| [*] RECOMMENDED ACTION                                                  |" << endl;
        cout << "+-------------------------------------------------------------------------+" << endl;
        
        switch (priority) {
            case Priority::CRITICAL:
                cout << "| [!] EMERGENCY! Immediate medical intervention required!                |" << endl;
                cout << "| >> Alert senior doctor and prepare emergency equipment                  |" << endl;
                cout << "| >> Response time: IMMEDIATE (< 30 seconds)                              |" << endl;
                cout << "| >> Prepare for possible ICU transfer                                    |" << endl;
                break;
            case Priority::HIGH:
                cout << "| [!] HIGH PRIORITY! Nurse must respond within 1 minute                   |" << endl;
                cout << "| >> Assess patient condition immediately                                 |" << endl;
                cout << "| >> Prepare for possible medical intervention                            |" << endl;
                cout << "| >> Notify attending physician                                           |" << endl;
                break;
            case Priority::MEDIUM:
                cout << "| [+] MEDIUM PRIORITY: Monitor patient closely                            |" << endl;
                cout << "| >> Check patient within 5 minutes                                       |" << endl;
                cout << "| >> Record observations and reassess vitals                              |" << endl;
                break;
            case Priority::LOW:
                cout << "| [OK] LOW PRIORITY: Routine monitoring                                   |" << endl;
                cout << "| >> Check during next scheduled rounds                                   |" << endl;
                cout << "| >> Continue regular monitoring protocol                                 |" << endl;
                break;
        }
        
        cout << "+-------------------------------------------------------------------------+" << endl;
    }
};

// ==================== MAIN FUNCTION ====================
int main() {
    HospitalSystem hospital;
    
    cout << "\n";
    cout << "+=========================================================================+" << endl;
    cout << "|                                                                         |" << endl;
    cout << "|            *** HOSPITAL PATIENT MONITORING SYSTEM v2.0 ***              |" << endl;
    cout << "|                                                                         |" << endl;
    cout << "|              Advanced Medical Alert Management System                   |" << endl;
    cout << "|                   Real-time Vital Signs Tracking                        |" << endl;
    cout << "|                                                                         |" << endl;
    cout << "+=========================================================================+" << endl;
    
    cout << "\n" << string(75, '-') << endl;
    cout << "  Developed for Healthcare Excellence" << endl;
    cout << "  Features: Patient Management | Vital Monitoring | Alert System" << endl;
    cout << string(75, '-') << endl;
    
    cout << "\nPress Enter to continue...";
    cin.get();
    
    while (true) {
        hospital.displayMenu();
        
        int choice;
        cin >> choice;
        
        if (cin.fail()) {
            hospital.clearInput();
            cout << "\n[ERROR] Invalid input! Please enter a number." << endl;
            cout << "Press Enter to continue...";
            cin.get();
            continue;
        }
        
        switch (choice) {
            case 1:
                hospital.registerPatient();
                break;
            case 2:
                hospital.viewAllPatients();
                break;
            case 3:
                hospital.searchPatient();
                break;
            case 4:
                hospital.viewPatientProfile();
                break;
            case 5:
                hospital.dischargePatient();
                break;
            case 6:
                hospital.recordVitalSigns();
                break;
            case 7:
                hospital.viewVitalHistory();
                break;
            case 8:
                hospital.bulkVitalEntry();
                break;
            case 9:
                hospital.viewAlertsQueue();
                break;
            case 10:
                hospital.processNextAlert();
                break;
            case 11:
                hospital.processAllAlerts();
                break;
            case 12:
                hospital.viewProcessedAlerts();
                break;
            case 13:
                hospital.systemDashboard();
                break;
            case 14:
                hospital.criticalPatientsReport();
                break;
            case 15:
                hospital.wardWiseSummary();
                break;
            case 16:
                hospital.exportSystemReport();
                break;
            case 17:
                cout << "\n";
                Utils::printSeparator('=', 75);
                cout << "  Thank you for using Hospital Monitoring System!" << endl;
                cout << "  Patient safety is our priority. Stay safe!" << endl;
                Utils::printSeparator('=', 75);
                cout << endl;
                return 0;
            default:
                cout << "\n[ERROR] Invalid choice! Please enter a number between 1-17." << endl;
        }
        
        cout << "\n\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }
    
    return 0;
}