/*
 * CSC 506 1.0 Computer Programming Laboratory
 * Final Assessment - Car Park Management System
 * Complete implementation in single file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

// =============== CONSTANTS AND MACROS ===============
#define MAX_VEHICLE_NUMBER 20
#define MAX_VEHICLE_TYPES 5
#define MAX_CUSTOMER_TYPES 5
#define DATE_TIME_FORMAT "%Y-%m-%d %H:%M"

// Capacity for each vehicle type
#define MAX_MOTORCYCLE 100
#define MAX_THREE_WHEELER 75
#define MAX_CAR 100
#define MAX_VAN 50
#define MAX_BUS 30

// Customer distribution percentages
#define DISABLED_PERCENT 15
#define VIP_PERCENT 10
#define STAFF_PERCENT 15
#define REGISTERED_PERCENT 25
#define GUEST_PERCENT 35

// Hourly rates
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

// Discount percentages
#define DISCOUNT_DISABLED 60
#define DISCOUNT_VIP 50
#define DISCOUNT_STAFF 80
#define DISCOUNT_REGISTERED 30
#define DISCOUNT_GUEST 0

// =============== ENUMERATIONS ===============
typedef enum {
    MOTORCYCLE,
    THREE_WHEELER,
    CAR,
    VAN,
    BUS
} VehicleType;

typedef enum {
    DISABLED,
    VIP,
    STAFF,
    REGISTERED,
    GUEST
} CustomerType;

typedef enum {
    FREE,
    OCCUPIED
} SlotStatus;

// =============== STRUCTURES ===============
typedef struct {
    int slot_id;
    VehicleType vehicle_type;
    CustomerType reserved_for;
    SlotStatus status;
    char vehicle_number[MAX_VEHICLE_NUMBER];
    CustomerType current_customer;
    time_t arrival_time;
} ParkingSlot;

typedef struct {
    char vehicle_number[MAX_VEHICLE_NUMBER];
    VehicleType vehicle_type;
    CustomerType customer_type;
    time_t entry_time;
    time_t exit_time;
    float charge;
    float discount;
    float payable;
} ParkingRecord;

typedef struct {
    int total_slots[MAX_VEHICLE_TYPES];
    int occupied_slots[MAX_VEHICLE_TYPES];
    int free_slots[MAX_VEHICLE_TYPES];
    int slots_by_customer[MAX_VEHICLE_TYPES][MAX_CUSTOMER_TYPES];
    float total_revenue;
    int total_vehicles;
} ParkingStats;

// =============== GLOBAL VARIABLES ===============
ParkingSlot **parking_slots = NULL;
ParkingStats stats = {0};
ParkingRecord *records = NULL;
int record_count = 0;
int max_slots[MAX_VEHICLE_TYPES] = {MAX_MOTORCYCLE, MAX_THREE_WHEELER, MAX_CAR, MAX_VAN, MAX_BUS};

// =============== FUNCTION PROTOTYPES ===============
// Core functions
void initialize_system();
void load_from_file();
void save_to_file();
void display_menu();
void check_availability();
void enter_vehicle();
void exit_vehicle();
void view_parking_space();
void view_statistics();
void print_bill(ParkingRecord record);
int find_available_slot(VehicleType v_type, CustomerType c_type);
float calculate_charge(VehicleType v_type, CustomerType c_type, time_t entry, time_t exit);
void update_statistics();
void visualize_parking_space(VehicleType v_type);

// Utility functions
char* vehicle_type_to_str(VehicleType type);
char* customer_type_to_str(CustomerType type);
VehicleType char_to_vehicle_type(char c);
CustomerType char_to_customer_type(char c);
void clear_input_buffer();
void display_header(const char* title);

// =============== UTILITY FUNCTIONS ===============
char* vehicle_type_to_str(VehicleType type) {
    switch(type) {
        case MOTORCYCLE: return "Motorcycle";
        case THREE_WHEELER: return "Three Wheeler";
        case CAR: return "Car";
        case VAN: return "Van";
        case BUS: return "Bus";
        default: return "Unknown";
    }
}

char* customer_type_to_str(CustomerType type) {
    switch(type) {
        case DISABLED: return "Disabled";
        case VIP: return "VIP";
        case STAFF: return "Staff";
        case REGISTERED: return "Registered";
        case GUEST: return "Guest";
        default: return "Unknown";
    }
}

VehicleType char_to_vehicle_type(char c) {
    switch(toupper(c)) {
        case 'M': return MOTORCYCLE;
        case 'T': return THREE_WHEELER;
        case 'C': return CAR;
        case 'V': return VAN;
        case 'B': return BUS;
        default: return -1;
    }
}

CustomerType char_to_customer_type(char c) {
    switch(toupper(c)) {
        case 'D': return DISABLED;
        case 'V': return VIP;
        case 'S': return STAFF;
        case 'R': return REGISTERED;
        case 'G': return GUEST;
        default: return -1;
    }
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void display_header(const char* title) {
    printf("\n========================================\n");
    printf("    %s\n", title);
    printf("========================================\n");
}

// =============== CORE FUNCTIONS ===============
void initialize_system() {
    // Allocate memory for parking slots
    parking_slots = (ParkingSlot**)malloc(MAX_VEHICLE_TYPES * sizeof(ParkingSlot*));
    if (!parking_slots) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    
    for(int vt = 0; vt < MAX_VEHICLE_TYPES; vt++) {
        parking_slots[vt] = (ParkingSlot*)malloc(max_slots[vt] * sizeof(ParkingSlot));
        if (!parking_slots[vt]) {
            printf("Memory allocation failed!\n");
            exit(1);
        }
        
        // Calculate slot distribution for customer types
        int disabled_slots = (max_slots[vt] * DISABLED_PERCENT) / 100;
        int vip_slots = (max_slots[vt] * VIP_PERCENT) / 100;
        int staff_slots = (max_slots[vt] * STAFF_PERCENT) / 100;
        int registered_slots = (max_slots[vt] * REGISTERED_PERCENT) / 100;
        int guest_slots = max_slots[vt] - (disabled_slots + vip_slots + staff_slots + registered_slots);
        
        int slot_index = 0;
        
        // Assign disabled slots
        for(int i = 0; i < disabled_slots; i++) {
            parking_slots[vt][slot_index].slot_id = slot_index + 1;
            parking_slots[vt][slot_index].vehicle_type = vt;
            parking_slots[vt][slot_index].reserved_for = DISABLED;
            parking_slots[vt][slot_index].status = FREE;
            parking_slots[vt][slot_index].vehicle_number[0] = '\0';
            slot_index++;
        }
        
        // Assign VIP slots
        for(int i = 0; i < vip_slots; i++) {
            parking_slots[vt][slot_index].slot_id = slot_index + 1;
            parking_slots[vt][slot_index].vehicle_type = vt;
            parking_slots[vt][slot_index].reserved_for = VIP;
            parking_slots[vt][slot_index].status = FREE;
            parking_slots[vt][slot_index].vehicle_number[0] = '\0';
            slot_index++;
        }
        
        // Assign staff slots
        for(int i = 0; i < staff_slots; i++) {
            parking_slots[vt][slot_index].slot_id = slot_index + 1;
            parking_slots[vt][slot_index].vehicle_type = vt;
            parking_slots[vt][slot_index].reserved_for = STAFF;
            parking_slots[vt][slot_index].status = FREE;
            parking_slots[vt][slot_index].vehicle_number[0] = '\0';
            slot_index++;
        }
        
        // Assign registered slots
        for(int i = 0; i < registered_slots; i++) {
            parking_slots[vt][slot_index].slot_id = slot_index + 1;
            parking_slots[vt][slot_index].vehicle_type = vt;
            parking_slots[vt][slot_index].reserved_for = REGISTERED;
            parking_slots[vt][slot_index].status = FREE;
            parking_slots[vt][slot_index].vehicle_number[0] = '\0';
            slot_index++;
        }
        
        // Assign guest slots
        for(int i = 0; i < guest_slots; i++) {
            parking_slots[vt][slot_index].slot_id = slot_index + 1;
            parking_slots[vt][slot_index].vehicle_type = vt;
            parking_slots[vt][slot_index].reserved_for = GUEST;
            parking_slots[vt][slot_index].status = FREE;
            parking_slots[vt][slot_index].vehicle_number[0] = '\0';
            slot_index++;
        }
    }
    
    // Initialize statistics
    for(int vt = 0; vt < MAX_VEHICLE_TYPES; vt++) {
        stats.total_slots[vt] = max_slots[vt];
        stats.free_slots[vt] = max_slots[vt];
        stats.occupied_slots[vt] = 0;
        for(int ct = 0; ct < MAX_CUSTOMER_TYPES; ct++) {
            stats.slots_by_customer[vt][ct] = 0;
        }
    }
    stats.total_revenue = 0.0;
    stats.total_vehicles = 0;
}

int find_available_slot(VehicleType v_type, CustomerType c_type) {
    int total_slots = max_slots[v_type];
    
    // Priority order: Disabled > VIP > Staff > Registered > Guest
    // Check priority rules
    
    // First, check if there's a slot specifically reserved for this customer type
    for(int i = 0; i < total_slots; i++) {
        if(parking_slots[v_type][i].reserved_for == c_type && 
           parking_slots[v_type][i].status == FREE) {
            return i;
        }
    }
    
    // If customer is not disabled, check if we can use disabled slots
    if(c_type != DISABLED) {
        int free_disabled_slots = 0;
        int total_disabled_slots = 0;
        
        for(int i = 0; i < total_slots; i++) {
            if(parking_slots[v_type][i].reserved_for == DISABLED) {
                total_disabled_slots++;
                if(parking_slots[v_type][i].status == FREE) {
                    free_disabled_slots++;
                }
            }
        }
        
        // Can use disabled slots only if more than 60% are free
        if(free_disabled_slots > (0.6 * total_disabled_slots)) {
            for(int i = 0; i < total_slots; i++) {
                if(parking_slots[v_type][i].reserved_for == DISABLED && 
                   parking_slots[v_type][i].status == FREE) {
                    return i;
                }
            }
        }
    }
    
    // If customer is not VIP and not disabled, check if we can use VIP slots
    if(c_type != VIP && c_type != DISABLED) {
        int free_vip_slots = 0;
        int total_vip_slots = 0;
        
        for(int i = 0; i < total_slots; i++) {
            if(parking_slots[v_type][i].reserved_for == VIP) {
                total_vip_slots++;
                if(parking_slots[v_type][i].status == FREE) {
                    free_vip_slots++;
                }
            }
        }
        
        // Can use VIP slots only if more than 50% are free
        if(free_vip_slots > (0.5 * total_vip_slots)) {
            for(int i = 0; i < total_slots; i++) {
                if(parking_slots[v_type][i].reserved_for == VIP && 
                   parking_slots[v_type][i].status == FREE) {
                    return i;
                }
            }
        }
    }
    
    // Check lower priority slots based on priority order
    // For non-VIP customers, they can use Staff slots if available
    if(c_type != VIP && c_type != DISABLED) {
        for(int i = 0; i < total_slots; i++) {
            if(parking_slots[v_type][i].reserved_for == STAFF && 
               parking_slots[v_type][i].status == FREE) {
                return i;
            }
        }
    }
    
    // For non-Registered customers (Guests), they can use Registered slots
    if(c_type == GUEST) {
        for(int i = 0; i < total_slots; i++) {
            if(parking_slots[v_type][i].reserved_for == REGISTERED && 
               parking_slots[v_type][i].status == FREE) {
                return i;
            }
        }
    }
    
    // Check Guest slots (lowest priority)
    for(int i = 0; i < total_slots; i++) {
        if(parking_slots[v_type][i].reserved_for == GUEST && 
           parking_slots[v_type][i].status == FREE) {
            return i;
        }
    }
    
    return -1; // No slot available
}

float calculate_charge(VehicleType v_type, CustomerType c_type, time_t entry, time_t exit) {
    double duration = difftime(exit, entry) / 3600.0; // Convert to hours
    float first_hour_rate, additional_hour_rate;
    
    // Get rates based on vehicle type
    switch(v_type) {
        case MOTORCYCLE:
            first_hour_rate = FIRST_HOUR_RATE_M;
            additional_hour_rate = ADDITIONAL_HOUR_RATE_M;
            break;
        case THREE_WHEELER:
            first_hour_rate = FIRST_HOUR_RATE_T;
            additional_hour_rate = ADDITIONAL_HOUR_RATE_T;
            break;
        case CAR:
            first_hour_rate = FIRST_HOUR_RATE_C;
            additional_hour_rate = ADDITIONAL_HOUR_RATE_C;
            break;
        case VAN:
            first_hour_rate = FIRST_HOUR_RATE_V;
            additional_hour_rate = ADDITIONAL_HOUR_RATE_V;
            break;
        case BUS:
            first_hour_rate = FIRST_HOUR_RATE_B;
            additional_hour_rate = ADDITIONAL_HOUR_RATE_B;
            break;
        default:
            first_hour_rate = 0;
            additional_hour_rate = 0;
    }
    
    float charge;
    if(duration <= 1.0) {
        charge = first_hour_rate;
    } else {
        int full_hours = (int)duration;
        float remaining = duration - full_hours;
        
        charge = first_hour_rate + (full_hours - 1) * additional_hour_rate;
        if(remaining > 0) {
            charge += additional_hour_rate * (remaining / 1.0);
        }
    }
    
    return charge;
}

void enter_vehicle() {
    char vehicle_num[MAX_VEHICLE_NUMBER];
    char v_type_char, c_type_char;
    VehicleType v_type;
    CustomerType c_type;
    
    display_header("ENTER VEHICLE");
    
    printf("Enter Vehicle Number: ");
    fgets(vehicle_num, MAX_VEHICLE_NUMBER, stdin);
    vehicle_num[strcspn(vehicle_num, "\n")] = '\0';
    
    if(strlen(vehicle_num) == 0) {
        printf("Vehicle number cannot be empty!\n");
        return;
    }
    
    printf("Enter Vehicle Type (M-Motorcycle, T-Three Wheeler, C-Car, V-Van, B-Bus): ");
    scanf("%c", &v_type_char);
    clear_input_buffer();
    
    printf("Enter Customer Type (D-Disabled, V-VIP, S-Staff, R-Registered, G-Guest): ");
    scanf("%c", &c_type_char);
    clear_input_buffer();
    
    v_type = char_to_vehicle_type(v_type_char);
    c_type = char_to_customer_type(c_type_char);
    
    if(v_type == -1) {
        printf("Invalid vehicle type! Please enter M, T, C, V, or B.\n");
        return;
    }
    
    if(c_type == -1) {
        printf("Invalid customer type! Please enter D, V, S, R, or G.\n");
        return;
    }
    
    int slot_index = find_available_slot(v_type, c_type);
    
    if(slot_index == -1) {
        printf("\n❌ No parking slots available for %s (Vehicle Type: %s)\n", 
               vehicle_num, vehicle_type_to_str(v_type));
        return;
    }
    
    // Allocate slot
    parking_slots[v_type][slot_index].status = OCCUPIED;
    strcpy(parking_slots[v_type][slot_index].vehicle_number, vehicle_num);
    parking_slots[v_type][slot_index].current_customer = c_type;
    parking_slots[v_type][slot_index].arrival_time = time(NULL);
    
    // Update statistics
    stats.occupied_slots[v_type]++;
    stats.free_slots[v_type]--;
    stats.slots_by_customer[v_type][c_type]++;
    
    printf("\n✅ Vehicle parked successfully!\n");
    printf("   Slot ID: %d\n", parking_slots[v_type][slot_index].slot_id);
    printf("   Vehicle Type: %s\n", vehicle_type_to_str(v_type));
    printf("   Customer Type: %s\n", customer_type_to_str(c_type));
    
    char time_str[20];
    struct tm *timeinfo = localtime(&parking_slots[v_type][slot_index].arrival_time);
    strftime(time_str, sizeof(time_str), DATE_TIME_FORMAT, timeinfo);
    printf("   Arrival Time: %s\n", time_str);
}

void exit_vehicle() {
    char vehicle_num[MAX_VEHICLE_NUMBER];
    int found = 0;
    VehicleType v_type_found;
    int slot_index_found;
    
    display_header("EXIT VEHICLE");
    
    printf("Enter Vehicle Number to exit: ");
    fgets(vehicle_num, MAX_VEHICLE_NUMBER, stdin);
    vehicle_num[strcspn(vehicle_num, "\n")] = '\0';
    
    if(strlen(vehicle_num) == 0) {
        printf("Vehicle number cannot be empty!\n");
        return;
    }
    
    // Search for vehicle
    for(int vt = 0; vt < MAX_VEHICLE_TYPES; vt++) {
        for(int i = 0; i < max_slots[vt]; i++) {
            if(parking_slots[vt][i].status == OCCUPIED && 
               strcmp(parking_slots[vt][i].vehicle_number, vehicle_num) == 0) {
                found = 1;
                v_type_found = vt;
                slot_index_found = i;
                break;
            }
        }
        if(found) break;
    }
    
    if(!found) {
        printf("❌ Vehicle '%s' not found in the parking lot!\n", vehicle_num);
        return;
    }
    
    // Calculate parking duration and charge
    time_t exit_time = time(NULL);
    time_t entry_time = parking_slots[v_type_found][slot_index_found].arrival_time;
    CustomerType c_type = parking_slots[v_type_found][slot_index_found].current_customer;
    
    float charge = calculate_charge(v_type_found, c_type, entry_time, exit_time);
    float discount_percentage;
    
    // Get discount percentage
    switch(c_type) {
        case DISABLED: discount_percentage = DISCOUNT_DISABLED; break;
        case VIP: discount_percentage = DISCOUNT_VIP; break;
        case STAFF: discount_percentage = DISCOUNT_STAFF; break;
        case REGISTERED: discount_percentage = DISCOUNT_REGISTERED; break;
        case GUEST: discount_percentage = DISCOUNT_GUEST; break;
        default: discount_percentage = 0;
    }
    
    float discount = charge * (discount_percentage / 100.0);
    float payable = charge - discount;
    
    // Create parking record
    record_count++;
    ParkingRecord *temp = realloc(records, record_count * sizeof(ParkingRecord));
    if(temp == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }
    records = temp;
    
    strcpy(records[record_count-1].vehicle_number, vehicle_num);
    records[record_count-1].vehicle_type = v_type_found;
    records[record_count-1].customer_type = c_type;
    records[record_count-1].entry_time = entry_time;
    records[record_count-1].exit_time = exit_time;
    records[record_count-1].charge = charge;
    records[record_count-1].discount = discount;
    records[record_count-1].payable = payable;
    
    // Free the slot
    parking_slots[v_type_found][slot_index_found].status = FREE;
    parking_slots[v_type_found][slot_index_found].vehicle_number[0] = '\0';
    
    // Update statistics
    stats.occupied_slots[v_type_found]--;
    stats.free_slots[v_type_found]++;
    stats.slots_by_customer[v_type_found][c_type]--;
    stats.total_revenue += payable;
    stats.total_vehicles++;
    
    // Print bill
    print_bill(records[record_count-1]);
}

void print_bill(ParkingRecord record) {
    struct tm *entry_tm = localtime(&record.entry_time);
    struct tm *exit_tm = localtime(&record.exit_time);
    
    char entry_str[20], exit_str[20];
    strftime(entry_str, sizeof(entry_str), DATE_TIME_FORMAT, entry_tm);
    strftime(exit_str, sizeof(exit_str), DATE_TIME_FORMAT, exit_tm);
    
    double duration = difftime(record.exit_time, record.entry_time);
    int hours = (int)duration / 3600;
    int minutes = ((int)duration % 3600) / 60;
    
    printf("\n========================================\n");
    printf("           PARKING BILL\n");
    printf("========================================\n");
    printf("Vehicle Number    : %s\n", record.vehicle_number);
    printf("Vehicle Type      : %s\n", vehicle_type_to_str(record.vehicle_type));
    printf("Customer Type     : %s\n", customer_type_to_str(record.customer_type));
    printf("Entered Date & Time : %s\n", entry_str);
    printf("Exit Date & Time    : %s\n", exit_str);
    printf("Parking Duration    : %d hours %d minutes\n", hours, minutes);
    printf("----------------------------------------\n");
    printf("Total Charge      : Rs %8.2f\n", record.charge);
    printf("Discount          : Rs %8.2f\n", record.discount);
    printf("----------------------------------------\n");
    printf("Total Payable     : Rs %8.2f\n", record.payable);
    printf("========================================\n\n");
}

void check_availability() {
    display_header("CHECK AVAILABILITY");
    
    printf("Available Parking Slots:\n");
    printf("========================\n");
    
    for(int vt = 0; vt < MAX_VEHICLE_TYPES; vt++) {
        printf("%-15s: %3d slots available out of %3d\n", 
               vehicle_type_to_str(vt), 
               stats.free_slots[vt], 
               stats.total_slots[vt]);
    }
    
    printf("\nDetailed Availability by Customer Type:\n");
    printf("--------------------------------------\n");
    for(int vt = 0; vt < MAX_VEHICLE_TYPES; vt++) {
        printf("\n%s:\n", vehicle_type_to_str(vt));
        for(int ct = 0; ct < MAX_CUSTOMER_TYPES; ct++) {
            int reserved_count = 0;
            int free_count = 0;
            
            for(int i = 0; i < max_slots[vt]; i++) {
                if(parking_slots[vt][i].reserved_for == ct) {
                    reserved_count++;
                    if(parking_slots[vt][i].status == FREE) {
                        free_count++;
                    }
                }
            }
            
            printf("  %-12s: %2d free / %2d reserved\n", 
                   customer_type_to_str(ct), free_count, reserved_count);
        }
    }
}

void visualize_parking_space(VehicleType v_type) {
    printf("{ ");
    
    for(int i = 0; i < max_slots[v_type]; i++) {
        char symbol;
        
        if(parking_slots[v_type][i].status == OCCUPIED) {
            // Show customer type for occupied slots
            switch(parking_slots[v_type][i].current_customer) {
                case DISABLED: symbol = 'D'; break;
                case VIP: symbol = 'V'; break;
                case STAFF: symbol = 'S'; break;
                case REGISTERED: symbol = 'R'; break;
                case GUEST: symbol = 'G'; break;
                default: symbol = 'X';
            }
        } else {
            // Show 'F' for free slots
            symbol = 'F';
        }
        
        printf("%c", symbol);
        
        // Add space after every few slots for readability
        if((i + 1) % 10 == 0 && i != max_slots[v_type] - 1) {
            printf(" | ");
        }
    }
    
    printf(" }");
}

void view_parking_space() {
    display_header("VIEW PARKING SPACE");
    
    printf("Visual Representation:\n");
    printf("D = Disabled, V = VIP, S = Staff, R = Registered, G = Guest, F = Free\n");
    printf("========================================\n\n");
    
    for(int vt = 0; vt < MAX_VEHICLE_TYPES; vt++) {
        printf("%-15s: ", vehicle_type_to_str(vt));
        visualize_parking_space(vt);
        printf("\n");
    }
    
    // Show example from assignment
    printf("\nExample from assignment (Bus slots):\n");
    printf("Bus: { DDFF | VVF | SFFF | RRRFFFF | GGGGGGGFFFF }\n");
}

void view_statistics() {
    display_header("VIEW STATISTICS");
    
    printf("Overall Statistics:\n");
    printf("-------------------\n");
    printf("Total Vehicles Served : %d\n", stats.total_vehicles);
    printf("Total Revenue         : Rs %.2f\n", stats.total_revenue);
    
    printf("\nSlot Utilization:\n");
    printf("----------------\n");
    for(int vt = 0; vt < MAX_VEHICLE_TYPES; vt++) {
        float utilization = stats.total_slots[vt] > 0 ? 
            (float)stats.occupied_slots[vt] / stats.total_slots[vt] * 100 : 0;
        printf("%-15s: %3d/%3d slots (%.1f%% occupied)\n", 
               vehicle_type_to_str(vt),
               stats.occupied_slots[vt],
               stats.total_slots[vt],
               utilization);
    }
    
    printf("\nCurrent Customer Distribution:\n");
    printf("-----------------------------\n");
    for(int ct = 0; ct < MAX_CUSTOMER_TYPES; ct++) {
        int total = 0;
        for(int vt = 0; vt < MAX_VEHICLE_TYPES; vt++) {
            total += stats.slots_by_customer[vt][ct];
        }
        printf("%-12s: %d vehicles currently parked\n", customer_type_to_str(ct), total);
    }
    
    printf("\nHourly Rates:\n");
    printf("-------------\n");
    printf("Motorcycle   : Rs %d (1st hour) + Rs %d (additional)\n", 
           FIRST_HOUR_RATE_M, ADDITIONAL_HOUR_RATE_M);
    printf("Three Wheeler: Rs %d (1st hour) + Rs %d (additional)\n", 
           FIRST_HOUR_RATE_T, ADDITIONAL_HOUR_RATE_T);
    printf("Car          : Rs %d (1st hour) + Rs %d (additional)\n", 
           FIRST_HOUR_RATE_C, ADDITIONAL_HOUR_RATE_C);
    printf("Van          : Rs %d (1st hour) + Rs %d (additional)\n", 
           FIRST_HOUR_RATE_V, ADDITIONAL_HOUR_RATE_V);
    printf("Bus          : Rs %d (1st hour) + Rs %d (additional)\n", 
           FIRST_HOUR_RATE_B, ADDITIONAL_HOUR_RATE_B);
    
    printf("\nDiscount Rates:\n");
    printf("---------------\n");
    printf("Disabled   : %d%%\n", DISCOUNT_DISABLED);
    printf("VIP        : %d%%\n", DISCOUNT_VIP);
    printf("Staff      : %d%%\n", DISCOUNT_STAFF);
    printf("Registered : %d%%\n", DISCOUNT_REGISTERED);
    printf("Guest      : %d%%\n", DISCOUNT_GUEST);
}

void save_to_file() {
    FILE *file = fopen("parking_data.dat", "wb");
    if(!file) {
        printf("❌ Error: Cannot create data file!\n");
        return;
    }
    
    // Save parking slots
    for(int vt = 0; vt < MAX_VEHICLE_TYPES; vt++) {
        fwrite(parking_slots[vt], sizeof(ParkingSlot), max_slots[vt], file);
    }
    
    // Save statistics
    fwrite(&stats, sizeof(ParkingStats), 1, file);
    
    // Save record count
    fwrite(&record_count, sizeof(int), 1, file);
    
    // Save records
    if(record_count > 0) {
        fwrite(records, sizeof(ParkingRecord), record_count, file);
    }
    
    fclose(file);
    printf("✅ Data saved successfully to 'parking_data.dat'!\n");
}

void load_from_file() {
    FILE *file = fopen("parking_data.dat", "rb");
    if(!file) {
        printf("ℹ️  No previous data found. Starting fresh system.\n");
        return;
    }
    
    // Load parking slots
    for(int vt = 0; vt < MAX_VEHICLE_TYPES; vt++) {
        size_t read_count = fread(parking_slots[vt], sizeof(ParkingSlot), max_slots[vt], file);
        if(read_count != max_slots[vt]) {
            printf("⚠️  Warning: Partial data loaded for %s\n", vehicle_type_to_str(vt));
        }
    }
    
    // Load statistics
    fread(&stats, sizeof(ParkingStats), 1, file);
    
    // Load record count
    fread(&record_count, sizeof(int), 1, file);
    
    // Load records
    if(record_count > 0) {
        records = malloc(record_count * sizeof(ParkingRecord));
        if(records == NULL) {
            printf("❌ Error: Memory allocation failed!\n");
            fclose(file);
            return;
        }
        fread(records, sizeof(ParkingRecord), record_count, file);
    }
    
    fclose(file);
    printf("✅ Previous data loaded successfully!\n");
}

void display_menu() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
    
    printf("\n========================================\n");
    printf("    CAR PARK MANAGEMENT SYSTEM\n");
    printf("    CSC 506 - Final Assessment\n");
    printf("========================================\n");
    printf("(1) Check Availability\n");
    printf("(2) Enter Vehicle\n");
    printf("(3) Exit Vehicle\n");
    printf("(4) View Parking Space\n");
    printf("(5) View Statistics\n");
    printf("(6) Save and Exit\n");
    printf("========================================\n");
}

// =============== MAIN FUNCTION ===============
int main() {
    printf("Initializing Car Park Management System...\n");
    initialize_system();
    load_from_file();
    
    int choice;
    
    do {
        display_menu();
        printf("Enter your choice (1-6): ");
        
        if(scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a number.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();
        
        switch(choice) {
            case 1:
                check_availability();
                break;
            case 2:
                enter_vehicle();
                break;
            case 3:
                exit_vehicle();
                break;
            case 4:
                view_parking_space();
                break;
            case 5:
                view_statistics();
                break;
            case 6:
                save_to_file();
                printf("\nThank you for using Car Park Management System!\n");
                printf("Goodbye!\n");
                break;
            default:
                printf("❌ Invalid choice! Please enter 1-6.\n");
        }
        
        if(choice != 6) {
            printf("\nPress Enter to continue...");
            clear_input_buffer();
        }
        
    } while(choice != 6);
    
    // Free allocated memory
    if(parking_slots) {
        for(int i = 0; i < MAX_VEHICLE_TYPES; i++) {
            if(parking_slots[i]) {
                free(parking_slots[i]);
            }
        }
        free(parking_slots);
    }
    
    if(records) {
        free(records);
    }
    
    return 0;
}