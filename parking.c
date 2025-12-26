#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

typedef struct {
    int id;
    char vehicle_type;
    char reserved_for;
    int occupied;
    char number[20];
    char customer;
    time_t arrival;
} Slot;

Slot *park[5];
int max_slots[5]={100,75,100,50,30};

int total_served=0;
float total_income=0;

int indexOf(char v){ return strchr("MTCVB",v)-"MTCVB"; }

char* nameOf(char v){
    if(v=='M')return"Motorcycle";
    if(v=='T')return"Three Wheeler";
    if(v=='C')return"Car";
    if(v=='V')return"Van";
    return"Bus";
}

void init(){
    static int done=0;
    if(done) return;
    done=1;

    for(int v=0;v<5;v++){
        park[v]=malloc(sizeof(Slot)*max_slots[v]);
        int d=max_slots[v]*15/100,vp=max_slots[v]*10/100,s=max_slots[v]*15/100,r=max_slots[v]*25/100;
        int g=max_slots[v]-(d+vp+s+r),k=0;
        char vt="MTCVB"[v];

        for(int i=0;i<d;i++,k++) park[v][k]=(Slot){k+1,vt,'D',0,"",'D',0};
        for(int i=0;i<vp;i++,k++) park[v][k]=(Slot){k+1,vt,'V',0,"",'V',0};
        for(int i=0;i<s;i++,k++) park[v][k]=(Slot){k+1,vt,'S',0,"",'S',0};
        for(int i=0;i<r;i++,k++) park[v][k]=(Slot){k+1,vt,'R',0,"",'R',0};
        for(int i=0;i<g;i++,k++) park[v][k]=(Slot){k+1,vt,'G',0,"",'G',0};
    }
}

void show_availability(){
    printf("\n===== AVAILABLE SLOTS =====\n");
    for(int v=0;v<5;v++){
        int free=0,occ=0;
        for(int i=0;i<max_slots[v];i++){
            if(park[v][i].occupied) occ++;
            else free++;
        }
        printf("%s: Free %d / %d | Occupied %d\n",
               nameOf("MTCVB"[v]), free, max_slots[v], occ);
    }
}

float rate(char v){
    if(v=='M')return 20;
    if(v=='T')return 30;
    if(v=='C')return 40;
    if(v=='V')return 50;
    return 80;
}

float discount(char c){
    if(c=='D')return 60;
    if(c=='V')return 50;
    if(c=='S')return 80;
    if(c=='R')return 30;
    return 0;
}

void save(){
    FILE *f=fopen("parking.txt","w");
    fprintf(f,"%d %.2f\n",total_served,total_income);
    for(int v=0;v<5;v++)
        for(int i=0;i<max_slots[v];i++)
            fwrite(&park[v][i],sizeof(Slot),1,f);
    fclose(f);
}

void load(){
    init();
    FILE *f=fopen("parking.txt","r");
    if(!f) return;
    fscanf(f,"%d %f",&total_served,&total_income);
    for(int v=0;v<5;v++)
        for(int i=0;i<max_slots[v];i++)
            fread(&park[v][i],sizeof(Slot),1,f);
    fclose(f);
}

void enter(){
    show_availability();

    char num[20],v,c;
    printf("\nEnter Vehicle Number: "); scanf("%s",num);
    printf("Vehicle Type (M/T/C/V/B): "); scanf(" %c",&v);
    printf("Customer Type (D/V/S/R/G): "); scanf(" %c",&c);

    int vi=indexOf(v);
    for(int i=0;i<max_slots[vi];i++){
        if(!park[vi][i].occupied && park[vi][i].reserved_for==c){
            park[vi][i].occupied=1;
            strcpy(park[vi][i].number,num);
            park[vi][i].customer=c;
            park[vi][i].arrival=time(NULL);
            printf("\nVehicle parked successfully at Slot %d\n",park[vi][i].id);
            return;
        }
    }
    printf("\nNo available slot for this category. Try another type.\n");
}

void exitv(){
    char num[20];
    printf("\nEnter Vehicle Number: "); scanf("%s",num);

    for(int v=0;v<5;v++)
        for(int i=0;i<max_slots[v];i++)
            if(park[v][i].occupied && strcmp(park[v][i].number,num)==0){

                time_t now=time(NULL);
                double hrs=difftime(now,park[v][i].arrival)/3600.0;
                float charge=rate(park[v][i].vehicle_type);
                if(hrs>1) charge+=((int)hrs)*rate(park[v][i].vehicle_type);
                float dis=charge*(discount(park[v][i].customer)/100);
                float pay=charge-dis;

                printf("\n======= PARKING BILL =======\n");
                printf("Vehicle Number : %s\n",num);
                printf("Total Charge   : Rs %.2f\n",charge);
                printf("Discount       : Rs %.2f\n",dis);
                printf("Payable        : Rs %.2f\n",pay);
                printf("===========================\n");

                total_income+=pay;
                total_served++;
                park[v][i].occupied=0;
                return;
            }
    printf("\nVehicle not found in parking.\n");
}

void owner_statistics(){
    printf("\n====== OWNER DASHBOARD ======\n");
    show_availability();
    printf("\nTotal Vehicles Served: %d\n",total_served);
    printf("Total Income: Rs %.2f\n",total_income);
}

int main(){
    init();
    load();

    int c;
    do{
        printf("\n1. Enter Vehicle\n2. Exit Vehicle\n3. View Available Slots\n4. Owner Statistics\n5. Save & Exit\nChoice: ");
        scanf("%d",&c);

        if(c==1) enter();
        if(c==2) exitv();
        if(c==3) show_availability();
        if(c==4) owner_statistics();
        if(c==5){
            save();
            printf("\nData saved. System closed safely.\n");
        }
    }while(c!=5);

    return 0;
}
