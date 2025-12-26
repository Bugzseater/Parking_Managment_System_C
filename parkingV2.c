/*
 * CSC 506 1.0 - Car Park Management System
 * COMPLETE VERSION with PROPER PERSISTENCE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_MOTORCYCLE 100
#define MAX_THREE_WHEELER 75
#define MAX_CAR 100
#define MAX_VAN 50
#define MAX_BUS 30

#define DISABLED_PERCENT 15
#define VIP_PERCENT 10
#define STAFF_PERCENT 15
#define REGISTERED_PERCENT 25
#define GUEST_PERCENT 35

#define FIRST_HOUR_RATE_M 20
#define FIRST_HOUR_RATE_T 30
#define FIRST_HOUR_RATE_C 40
#define FIRST_HOUR_RATE_V 50
#define FIRST_HOUR_RATE_B 80

#define ADDITIONAL_HOUR_RATE_M 30
#define ADDITIONAL_HOUR_RATE_T 40
#define ADDITIONAL_HOUR_RATE_C 50
#define ADDITIONAL_HOUR_RATE_V 60
#define ADDITIONAL_HOUR_RATE_B 100

#define DISCOUNT_DISABLED 60
#define DISCOUNT_VIP 50
#define DISCOUNT_STAFF 80
#define DISCOUNT_REGISTERED 30
#define DISCOUNT_GUEST 0

#define MAX_RECORDS 1000
#define DATA_FILE "parking_data.dat"  // Binary file for proper persistence
#define TEXT_FILE "parking_report.txt" // Text file for reports

// =============== STRUCTURES ===============
typedef struct {
    int slot_id;
    char vehicle_type;
    char reserved_for;
    int is_occupied;
    char vehicle_number[20];
    char customer_type;
    time_t arrival_time;
} ParkingSlot;

typedef struct {
    char vehicle_number[20];
    char vehicle_type;
    char customer_type;
    time_t entry_time;
    time_t exit_time;
    float base_charge;
    float discount;
    float payable;
} ParkingRecord;

// =============== GLOBAL VARIABLES ===============
ParkingSlot *parking_slots[5];
int max_slots[5] = {MAX_MOTORCYCLE, MAX_THREE_WHEELER, MAX_CAR, MAX_VAN, MAX_BUS};

ParkingRecord records[MAX_RECORDS];
int record_count = 0;
int total_vehicles_served = 0;
float total_revenue = 0;

// =============== UTILITY FUNCTIONS ===============
int get_vehicle_index(char t){
    t = toupper(t);
    if(t == 'M') return 0;
    if(t == 'T') return 1;
    if(t == 'C') return 2;
    if(t == 'V') return 3;
    if(t == 'B') return 4;
    return -1;
}

char* vehicle_name(char t){
    switch(toupper(t)){
        case 'M': return "Motorcycle";
        case 'T': return "Three Wheeler";
        case 'C': return "Car";
        case 'V': return "Van";
        case 'B': return "Bus";
        default: return "Unknown";
    }
}

char* customer_name(char t){
    switch(toupper(t)){
        case 'D': return "Disabled";
        case 'V': return "VIP";
        case 'S': return "Staff";
        case 'R': return "Registered";
        case 'G': return "Guest";
        default: return "Unknown";
    }
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// =============== INITIALIZATION ===============
void initialize_system(){
    for(int i = 0; i < 5; i++){
        parking_slots[i] = malloc(sizeof(ParkingSlot) * max_slots[i]);

        int disabled_slots = max_slots[i] * DISABLED_PERCENT / 100;
        int vip_slots = max_slots[i] * VIP_PERCENT / 100;
        int staff_slots = max_slots[i] * STAFF_PERCENT / 100;
        int registered_slots = max_slots[i] * REGISTERED_PERCENT / 100;
        int guest_slots = max_slots[i] - (disabled_slots + vip_slots + staff_slots + registered_slots);

        int slot_index = 0;
        char vehicle_type = "MTCVB"[i];

        // Assign disabled slots
        for(int j = 0; j < disabled_slots; j++, slot_index++) {
            parking_slots[i][slot_index].slot_id = slot_index + 1;
            parking_slots[i][slot_index].vehicle_type = vehicle_type;
            parking_slots[i][slot_index].reserved_for = 'D';
            parking_slots[i][slot_index].is_occupied = 0;
            parking_slots[i][slot_index].vehicle_number[0] = '\0';
            parking_slots[i][slot_index].customer_type = '\0';
            parking_slots[i][slot_index].arrival_time = 0;
        }

        // Assign VIP slots
        for(int j = 0; j < vip_slots; j++, slot_index++) {
            parking_slots[i][slot_index].slot_id = slot_index + 1;
            parking_slots[i][slot_index].vehicle_type = vehicle_type;
            parking_slots[i][slot_index].reserved_for = 'V';
            parking_slots[i][slot_index].is_occupied = 0;
            parking_slots[i][slot_index].vehicle_number[0] = '\0';
            parking_slots[i][slot_index].customer_type = '\0';
            parking_slots[i][slot_index].arrival_time = 0;
        }

        // Assign staff slots
        for(int j = 0; j < staff_slots; j++, slot_index++) {
            parking_slots[i][slot_index].slot_id = slot_index + 1;
            parking_slots[i][slot_index].vehicle_type = vehicle_type;
            parking_slots[i][slot_index].reserved_for = 'S';
            parking_slots[i][slot_index].is_occupied = 0;
            parking_slots[i][slot_index].vehicle_number[0] = '\0';
            parking_slots[i][slot_index].customer_type = '\0';
            parking_slots[i][slot_index].arrival_time = 0;
        }

        // Assign registered slots
        for(int j = 0; j < registered_slots; j++, slot_index++) {
            parking_slots[i][slot_index].slot_id = slot_index + 1;
            parking_slots[i][slot_index].vehicle_type = vehicle_type;
            parking_slots[i][slot_index].reserved_for = 'R';
            parking_slots[i][slot_index].is_occupied = 0;
            parking_slots[i][slot_index].vehicle_number[0] = '\0';
            parking_slots[i][slot_index].customer_type = '\0';
            parking_slots[i][slot_index].arrival_time = 0;
        }

        // Assign guest slots
        for(int j = 0; j < guest_slots; j++, slot_index++) {
            parking_slots[i][slot_index].slot_id = slot_index + 1;
            parking_slots[i][slot_index].vehicle_type = vehicle_type;
            parking_slots[i][slot_index].reserved_for = 'G';
            parking_slots[i][slot_index].is_occupied = 0;
            parking_slots[i][slot_index].vehicle_number[0] = '\0';
            parking_slots[i][slot_index].customer_type = '\0';
            parking_slots[i][slot_index].arrival_time = 0;
        }
    }
}

// =============== FILE STORAGE (PROPER PERSISTENCE) ===============
void save_data(){
    // Save binary data for proper persistence
    FILE *binary_file = fopen(DATA_FILE, "wb");
    if(!binary_file) {
        printf("Error: Cannot create data file!\n");
        return;
    }
    
    // Save statistics
    fwrite(&total_vehicles_served, sizeof(int), 1, binary_file);
    fwrite(&total_revenue, sizeof(float), 1, binary_file);
    fwrite(&record_count, sizeof(int), 1, binary_file);
    
    // Save records
    if(record_count > 0) {
        fwrite(records, sizeof(ParkingRecord), record_count, binary_file);
    }
    
    // Save parking slots (CURRENT STATE)
    for(int v = 0; v < 5; v++){
        fwrite(parking_slots[v], sizeof(ParkingSlot), max_slots[v], binary_file);
    }
    
    fclose(binary_file);
    
    // Also save text report
    FILE *text_file = fopen(TEXT_FILE, "w");
    if(text_file) {
        fprintf(text_file, "========== CAR PARK REPORT ==========\n");
        fprintf(text_file, "Generated: %s", ctime(&(time_t){time(NULL)}));
        fprintf(text_file, "Total Vehicles Served: %d\n", total_vehicles_served);
        fprintf(text_file, "Total Revenue: Rs %.2f\n\n", total_revenue);
        
        fprintf(text_file, "========== CURRENT PARKING STATUS ==========\n");
        for(int v = 0; v < 5; v++){
            fprintf(text_file, "\n%s:\n", vehicle_name("MTCVB"[v]));
            int occupied = 0;
            for(int i = 0; i < max_slots[v]; i++){
                if(parking_slots[v][i].is_occupied){
                    occupied++;
                    char time_str[20];
                    struct tm *tm_info = localtime(&parking_slots[v][i].arrival_time);
                    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", tm_info);
                    
                    fprintf(text_file, "  Slot %03d: %s (%s, Arrived: %s)\n",
                            parking_slots[v][i].slot_id,
                            parking_slots[v][i].vehicle_number,
                            customer_name(parking_slots[v][i].customer_type),
                            time_str);
                }
            }
            fprintf(text_file, "  Occupied: %d, Free: %d\n", occupied, max_slots[v] - occupied);
        }
        
        fprintf(text_file, "\n========== RECENT TRANSACTIONS ==========\n");
        int start = (record_count > 10) ? record_count - 10 : 0;
        for(int i = start; i < record_count; i++){
            char entry_str[20], exit_str[20];
            struct tm *entry_tm = localtime(&records[i].entry_time);
            struct tm *exit_tm = localtime(&records[i].exit_time);
            strftime(entry_str, sizeof(entry_str), "%Y-%m-%d %H:%M", entry_tm);
            strftime(exit_str, sizeof(exit_str), "%Y-%m-%d %H:%M", exit_tm);
            
            fprintf(text_file, "%s - %s: Rs %.2f\n", 
                    records[i].vehicle_number, entry_str, records[i].payable);
        }
        
        fclose(text_file);
        printf("Data saved to '%s' and report to '%s'\n", DATA_FILE, TEXT_FILE);
    } else {
        printf("Data saved to '%s'\n", DATA_FILE);
    }
}

void load_data(){
    // First initialize the system
    initialize_system();
    
    FILE *file = fopen(DATA_FILE, "rb");
    if(!file) {
        printf("No previous data found. Starting fresh system.\n");
        return;
    }
    
    printf("Loading previous data...\n");
    
    // Load statistics
    fread(&total_vehicles_served, sizeof(int), 1, file);
    fread(&total_revenue, sizeof(float), 1, file);
    fread(&record_count, sizeof(int), 1, file);
    
    // Load records
    if(record_count > 0 && record_count <= MAX_RECORDS) {
        fread(records, sizeof(ParkingRecord), record_count, file);
    } else if(record_count > MAX_RECORDS) {
        printf("Warning: Too many records, loading only %d\n", MAX_RECORDS);
        fread(records, sizeof(ParkingRecord), MAX_RECORDS, file);
        record_count = MAX_RECORDS;
    }
    
    // Load parking slots (CURRENT STATE)
    for(int v = 0; v < 5; v++){
        fread(parking_slots[v], sizeof(ParkingSlot), max_slots[v], file);
    }
    
    fclose(file);
    
    // Count currently parked vehicles
    int parked_count = 0;
    for(int v = 0; v < 5; v++){
        for(int i = 0; i < max_slots[v]; i++){
            if(parking_slots[v][i].is_occupied){
                parked_count++;
            }
        }
    }
    
    printf("Loaded: %d records, %d currently parked vehicles, Revenue: Rs %.2f\n", 
           record_count, parked_count, total_revenue);
}

// =============== CHARGES CALCULATION ===============
float calculate_charge(char v_type, time_t entry, time_t exit){
    double hours = difftime(exit, entry) / 3600.0;
    
    int first_hour_rates[] = {FIRST_HOUR_RATE_M, FIRST_HOUR_RATE_T, FIRST_HOUR_RATE_C, 
                              FIRST_HOUR_RATE_V, FIRST_HOUR_RATE_B};
    int additional_hour_rates[] = {ADDITIONAL_HOUR_RATE_M, ADDITIONAL_HOUR_RATE_T, 
                                  ADDITIONAL_HOUR_RATE_C, ADDITIONAL_HOUR_RATE_V, 
                                  ADDITIONAL_HOUR_RATE_B};

    int idx = get_vehicle_index(v_type);
    if(idx == -1) return 0;
    
    if(hours <= 1.0) {
        return first_hour_rates[idx];
    } else {
        int full_hours = (int)hours;
        float remaining = hours - full_hours;
        
        float charge = first_hour_rates[idx];
        charge += (full_hours - 1) * additional_hour_rates[idx];
        
        if(remaining > 0) {
            charge += additional_hour_rates[idx] * remaining;
        }
        
        return charge;
    }
}

// =============== STATISTICS ===============
void show_statistics(){
    printf("\n========== CAR PARK STATISTICS ==========\n");
    
    // Current parking status
    printf("\nCURRENT PARKING STATUS:\n");
    printf("%-20s %-10s %-10s %-10s\n", "Vehicle Type", "Total", "Occupied", "Free");
    printf("----------------------------------------------------\n");
    
    int total_occupied = 0;
    int total_slots = 0;
    
    for(int v = 0; v < 5; v++){
        char vt = "MTCVB"[v];
        int occupied = 0;
        
        for(int i = 0; i < max_slots[v]; i++){
            if(parking_slots[v][i].is_occupied){
                occupied++;
            }
        }
        
        total_occupied += occupied;
        total_slots += max_slots[v];
        
        float utilization = (float)occupied / max_slots[v] * 100;
        
        printf("%-20s %-10d %-10d %-10d (%.1f%%)\n", 
               vehicle_name(vt), max_slots[v], occupied, max_slots[v] - occupied, utilization);
    }
    
    printf("\nOverall: %d/%d slots occupied (%.1f%%)\n", 
           total_occupied, total_slots, (float)total_occupied/total_slots*100);
    
    // Show currently parked vehicles
    if(total_occupied > 0){
        printf("\nCURRENTLY PARKED VEHICLES:\n");
        for(int v = 0; v < 5; v++){
            for(int i = 0; i < max_slots[v]; i++){
                if(parking_slots[v][i].is_occupied){
                    char time_str[20];
                    struct tm *tm_info = localtime(&parking_slots[v][i].arrival_time);
                    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", tm_info);
                    
                    printf("  %s (%s) - Slot %d, Arrived: %s\n",
                           parking_slots[v][i].vehicle_number,
                           vehicle_name(parking_slots[v][i].vehicle_type),
                           parking_slots[v][i].slot_id,
                           time_str);
                }
            }
        }
    }
    
    // Financial statistics
    printf("\nFINANCIAL STATISTICS:\n");
    printf("Total Vehicles Served: %d\n", total_vehicles_served);
    printf("Total Revenue: Rs %.2f\n", total_revenue);
    
    if(total_vehicles_served > 0){
        printf("Average per Vehicle: Rs %.2f\n", total_revenue / total_vehicles_served);
    }
    
    printf("\nPress Enter to continue...");
    clear_input_buffer();
    getchar();
}

// =============== PARKING OPERATIONS ===============
int find_available_slot(char v_type, char c_type){
    int vi = get_vehicle_index(v_type);
    if(vi == -1) return -1;
    
    c_type = toupper(c_type);
    
    // Priority: Disabled > VIP > Staff > Registered > Guest
    // Try exact match first
    for(int i = 0; i < max_slots[vi]; i++){
        if(!parking_slots[vi][i].is_occupied && 
           parking_slots[vi][i].reserved_for == c_type){
            return i;
        }
    }
    
    // Check special rules for disabled slots
    if(c_type != 'D'){
        int free_disabled = 0;
        int total_disabled = 0;
        
        for(int i = 0; i < max_slots[vi]; i++){
            if(parking_slots[vi][i].reserved_for == 'D'){
                total_disabled++;
                if(!parking_slots[vi][i].is_occupied){
                    free_disabled++;
                }
            }
        }
        
        // Can use disabled slots if more than 60% are free
        if(total_disabled > 0 && free_disabled > (0.6 * total_disabled)){
            for(int i = 0; i < max_slots[vi]; i++){
                if(!parking_slots[vi][i].is_occupied && 
                   parking_slots[vi][i].reserved_for == 'D'){
                    return i;
                }
            }
        }
    }
    
    // Check VIP slots for non-VIP customers
    if(c_type != 'V' && c_type != 'D'){
        int free_vip = 0;
        int total_vip = 0;
        
        for(int i = 0; i < max_slots[vi]; i++){
            if(parking_slots[vi][i].reserved_for == 'V'){
                total_vip++;
                if(!parking_slots[vi][i].is_occupied){
                    free_vip++;
                }
            }
        }
        
        // Can use VIP slots if more than 50% are free
        if(total_vip > 0 && free_vip > (0.5 * total_vip)){
            for(int i = 0; i < max_slots[vi]; i++){
                if(!parking_slots[vi][i].is_occupied && 
                   parking_slots[vi][i].reserved_for == 'V'){
                    return i;
                }
            }
        }
    }
    
    // Find any available slot
    for(int i = 0; i < max_slots[vi]; i++){
        if(!parking_slots[vi][i].is_occupied){
            return i;
        }
    }
    
    return -1;
}

void enter_vehicle(){
    char vehicle_num[20];
    char v_type, c_type;
    
    printf("\n=== ENTER VEHICLE ===\n");
    
    printf("Vehicle Number: ");
    scanf("%s", vehicle_num);
    clear_input_buffer();
    
    printf("Vehicle Type (M/T/C/V/B): ");
    scanf("%c", &v_type);
    clear_input_buffer();
    
    printf("Customer Type (D/V/S/R/G): ");
    scanf("%c", &c_type);
    clear_input_buffer();
    
    v_type = toupper(v_type);
    c_type = toupper(c_type);
    
    int vi = get_vehicle_index(v_type);
    if(vi == -1){
        printf("Invalid vehicle type!\n");
        return;
    }
    
    if(c_type != 'D' && c_type != 'V' && c_type != 'S' && c_type != 'R' && c_type != 'G'){
        printf("Invalid customer type!\n");
        return;
    }
    
    int slot_index = find_available_slot(v_type, c_type);
    if(slot_index == -1){
        printf("No parking slots available for %s!\n", vehicle_name(v_type));
        return;
    }
    
    // Park the vehicle
    parking_slots[vi][slot_index].is_occupied = 1;
    strcpy(parking_slots[vi][slot_index].vehicle_number, vehicle_num);
    parking_slots[vi][slot_index].customer_type = c_type;
    parking_slots[vi][slot_index].arrival_time = time(NULL);
    
    printf("\n✓ VEHICLE PARKED SUCCESSFULLY!\n");
    printf("  Slot ID: %d\n", parking_slots[vi][slot_index].slot_id);
    printf("  Vehicle: %s (%s)\n", vehicle_num, vehicle_name(v_type));
    printf("  Customer: %s\n", customer_name(c_type));
    
    char time_str[20];
    struct tm *timeinfo = localtime(&parking_slots[vi][slot_index].arrival_time);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", timeinfo);
    printf("  Arrival Time: %s\n", time_str);
    
    // Save immediately to persist the change
    save_data();
    
    printf("\nPress Enter to continue...");
    getchar();
}

void exit_vehicle(){
    char vehicle_num[20];
    
    printf("\n=== EXIT VEHICLE ===\n");
    printf("Enter Vehicle Number: ");
    scanf("%s", vehicle_num);
    clear_input_buffer();
    
    // Search for vehicle
    int found = 0;
    int v_found = -1, slot_found = -1;
    
    for(int v = 0; v < 5; v++){
        for(int i = 0; i < max_slots[v]; i++){
            if(parking_slots[v][i].is_occupied &&
               strcmp(parking_slots[v][i].vehicle_number, vehicle_num) == 0){
                found = 1;
                v_found = v;
                slot_found = i;
                break;
            }
        }
        if(found) break;
    }
    
    if(!found){
        printf("Vehicle '%s' not found in parking lot!\n", vehicle_num);
        return;
    }
    
    // Calculate charges
    time_t exit_time = time(NULL);
    char v_type = parking_slots[v_found][slot_found].vehicle_type;
    char c_type = parking_slots[v_found][slot_found].customer_type;
    
    float base_charge = calculate_charge(v_type, 
                                         parking_slots[v_found][slot_found].arrival_time, 
                                         exit_time);
    
    // Calculate discount
    float discount_percentage = 0;
    switch(c_type){
        case 'D': discount_percentage = DISCOUNT_DISABLED; break;
        case 'V': discount_percentage = DISCOUNT_VIP; break;
        case 'S': discount_percentage = DISCOUNT_STAFF; break;
        case 'R': discount_percentage = DISCOUNT_REGISTERED; break;
        case 'G': discount_percentage = DISCOUNT_GUEST; break;
    }
    
    float discount = base_charge * discount_percentage / 100.0;
    float payable = base_charge - discount;
    
    // Create parking record
    if(record_count < MAX_RECORDS){
        ParkingRecord r;
        strcpy(r.vehicle_number, vehicle_num);
        r.vehicle_type = v_type;
        r.customer_type = c_type;
        r.entry_time = parking_slots[v_found][slot_found].arrival_time;
        r.exit_time = exit_time;
        r.base_charge = base_charge;
        r.discount = discount;
        r.payable = payable;
        
        records[record_count++] = r;
    }
    
    // Free the slot
    parking_slots[v_found][slot_found].is_occupied = 0;
    parking_slots[v_found][slot_found].vehicle_number[0] = '\0';
    parking_slots[v_found][slot_found].customer_type = '\0';
    
    // Update statistics
    total_vehicles_served++;
    total_revenue += payable;
    
    // Print bill
    printf("\n========== PARKING BILL ==========\n");
    printf("Vehicle Number : %s\n", vehicle_num);
    printf("Vehicle Type   : %s\n", vehicle_name(v_type));
    printf("Customer Type  : %s\n", customer_name(c_type));
    
    char entry_str[20], exit_str[20];
    struct tm *entry_tm = localtime(&parking_slots[v_found][slot_found].arrival_time);
    struct tm *exit_tm = localtime(&exit_time);
    strftime(entry_str, sizeof(entry_str), "%Y-%m-%d %H:%M", entry_tm);
    strftime(exit_str, sizeof(exit_str), "%Y-%m-%d %H:%M", exit_tm);
    
    printf("Entry Time     : %s\n", entry_str);
    printf("Exit Time      : %s\n", exit_str);
    
    double duration = difftime(exit_time, parking_slots[v_found][slot_found].arrival_time);
    int hours = (int)duration / 3600;
    int minutes = ((int)duration % 3600) / 60;
    printf("Duration       : %d hours %d minutes\n", hours, minutes);
    
    printf("----------------------------------\n");
    printf("Base Charge    : Rs %8.2f\n", base_charge);
    printf("Discount       : Rs %8.2f\n", discount);
    printf("----------------------------------\n");
    printf("Total Payable  : Rs %8.2f\n", payable);
    printf("==================================\n");
    
    // Save immediately to persist the change
    save_data();
    
    printf("\nPress Enter to continue...");
    getchar();
}

void view_parking_space(){
    printf("\n========== PARKING SPACE VISUALIZATION ==========\n");
    printf("Legend: D=Disabled, V=VIP, S=Staff, R=Registered, G=Guest, F=Free\n\n");
    
    for(int v = 0; v < 5; v++){
        char vt = "MTCVB"[v];
        printf("%-15s: ", vehicle_name(vt));
        
        for(int i = 0; i < max_slots[v]; i++){
            if(parking_slots[v][i].is_occupied){
                printf("%c", parking_slots[v][i].customer_type);
            } else {
                printf("F");
            }
            
            // Add separator every 10 slots for readability
            if((i + 1) % 10 == 0 && i != max_slots[v] - 1){
                printf(" | ");
            }
        }
        printf("\n");
    }
    
    // Show example from assignment
    printf("\nExample from assignment (Bus slots):\n");
    printf("Bus: { DDFF | VVF | SFFF | RRRFFFF | GGGGGGGFFFF }\n");
    
    printf("\nPress Enter to continue...");
    clear_input_buffer();
    getchar();
}

// =============== MAIN MENU ===============
void display_menu(){
    system("cls");  // Clear screen (Windows)
    printf("\n========== CAR PARK MANAGEMENT SYSTEM ==========\n");
    printf("Current Time: %s", ctime(&(time_t){time(NULL)}));
    printf("================================================\n");
    printf("(1) Enter Vehicle\n");
    printf("(2) Exit Vehicle\n");
    printf("(3) View Parking Space\n");
    printf("(4) View Statistics\n");
    printf("(5) Save Data & Exit\n");
    printf("================================================\n");
    printf("Choice: ");
}

// =============== MAIN FUNCTION ===============
int main(){
    printf("Initializing Car Park Management System...\n");
    load_data();  // This initializes and loads data
    
    int choice;
    
    do{
        display_menu();
        if(scanf("%d", &choice) != 1){
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();
        
        switch(choice){
            case 1:
                enter_vehicle();
                break;
            case 2:
                exit_vehicle();
                break;
            case 3:
                view_parking_space();
                break;
            case 4:
                show_statistics();
                break;
            case 5:
                save_data();
                printf("\nThank you for using Car Park Management System!\n");
                printf("Goodbye!\n");
                break;
            default:
                printf("Invalid choice! Please enter 1-5.\n");
                printf("Press Enter to continue...");
                getchar();
        }
        
    } while(choice != 5);
    
    // Free allocated memory
    for(int i = 0; i < 5; i++){
        free(parking_slots[i]);
    }
    
    return 0;
}