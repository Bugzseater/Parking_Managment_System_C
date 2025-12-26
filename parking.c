#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_RECORDS 1000

#define MOTORCYCLE 0
#define THREEWHEELER 1
#define CAR 2
#define VAN 3
#define BUS 4

#define DISABLED 0
#define VIP 1
#define STAFF 2
#define REGISTERED 3
#define GUEST 4

int maxSlots[5] = {100,75,100,50,30};

int slotLimit[5][5] = {
    {15,10,15,25,35},
    {11,7,11,19,27},
    {15,10,15,25,35},
    {8,5,8,12,17},
    {5,3,5,7,10}
};

int usedSlots[5][5] = {0};

char vNames[5][15] = {"Motorcycle","ThreeWheeler","Car","Van","Bus"};
char cNames[5][15] = {"Disabled","VIP","Staff","Registered","Guest"};
char vMap[5] = {'M','T','C','V','B'};
char cMap[5] = {'D','V','S','R','G'};

typedef struct {
    char number[20];
    int vType;
    int cType;
    int allocType;
    time_t entry;
    time_t exit;
    int active;
} Vehicle;

Vehicle db[MAX_RECORDS];
int dbCount = 0;

/* ---------------- FILE HANDLING ---------------- */

void saveData(){
    FILE *f = fopen("parking_data.txt","w");
    fprintf(f,"%d\n",dbCount);
    for(int i=0;i<dbCount;i++){
        Vehicle v = db[i];
        fprintf(f,"%s %d %d %d %ld %ld %d\n",
            v.number,v.vType,v.cType,v.allocType,
            v.entry,v.exit,v.active);
    }
    fclose(f);
}

void loadData(){
    FILE *f = fopen("parking_data.txt","r");
    if(!f) return;

    fscanf(f,"%d",&dbCount);
    for(int i=0;i<dbCount;i++){
        Vehicle v;
        fscanf(f,"%s %d %d %d %ld %ld %d",
            v.number,&v.vType,&v.cType,&v.allocType,
            &v.entry,&v.exit,&v.active);
        db[i]=v;
        if(v.active)
            usedSlots[v.vType][v.allocType]++;
    }
    fclose(f);
}

/* ---------------- SLOT ALLOCATION ---------------- */

int allocateSlot(int vt,int ct){
    for(int p=ct;p<5;p++){
        if(p==DISABLED){
            int free=slotLimit[vt][0]-usedSlots[vt][0];
            if(free<=slotLimit[vt][0]*0.6) continue;
        }
        if(p==VIP){
            int free=slotLimit[vt][1]-usedSlots[vt][1];
            if(free<=slotLimit[vt][1]*0.5) continue;
        }
        if(usedSlots[vt][p]<slotLimit[vt][p]){
            usedSlots[vt][p]++;
            return p;
        }
    }
    return -1;
}

/* ---------------- CHARGE ---------------- */

float calcCharge(int vt,int ct,int h,int m){
    int first[]={20,30,40,50,80};
    int extra[]={30,40,50,60,100};
    float disc[]={0.6,0.5,0.8,0.3,0};

    float total=first[vt];
    if(h>1||m>0) total+=(h-1+(m>0))*extra[vt];
    return total*(1-disc[ct]);
}

/* ---------------- CORE FUNCTIONS ---------------- */

void enterVehicle(){
    Vehicle v;
    char vt,ct;

    printf("\nVehicle Number: ");
    scanf("%s",v.number);

    printf("Vehicle Type (M T C V B): ");
    scanf(" %c",&vt);

    printf("Customer Type (D V S R G): ");
    scanf(" %c",&ct);

    for(int i=0;i<5;i++){
        if(vMap[i]==vt) v.vType=i;
        if(cMap[i]==ct) v.cType=i;
    }

    int slot=allocateSlot(v.vType,v.cType);
    if(slot==-1){
        printf("No Slot Available!\n");
        return;
    }

    v.allocType=slot;
    v.entry=time(NULL);
    v.active=1;

    db[dbCount++]=v;
    printf("Vehicle Parked Successfully.\n");
}

void exitVehicle(){
    char num[20];
    printf("\nEnter Vehicle Number: ");
    scanf("%s",num);

    for(int i=0;i<dbCount;i++){
        if(db[i].active && strcmp(db[i].number,num)==0){
            db[i].exit=time(NULL);
            db[i].active=0;
            usedSlots[db[i].vType][db[i].allocType]--;

            double sec=difftime(db[i].exit,db[i].entry);
            int h=sec/3600;
            int m=(sec-h*3600)/60;

            float pay=calcCharge(db[i].vType,db[i].cType,h,m);

            printf("\n--------- PARKING BILL ---------\n");
            printf("Vehicle: %s\n",db[i].number);
            printf("Vehicle Type: %s\n",vNames[db[i].vType]);
            printf("Customer Type: %s\n",cNames[db[i].cType]);
            printf("Duration: %d h %d m\n",h,m);
            printf("Total Payable: Rs %.2f\n",pay);
            printf("--------------------------------\n");
            return;
        }
    }
    printf("Vehicle Not Found!\n");
}

void viewSpace(){
    for(int v=0;v<5;v++){
        printf("\n%s: { ",vNames[v]);
        for(int c=0;c<5;c++){
            for(int i=0;i<usedSlots[v][c];i++) printf("%c",cMap[c]);
            for(int i=0;i<slotLimit[v][c]-usedSlots[v][c];i++) printf("F");
            if(c<4) printf(" | ");
        }
        printf(" }");
    }
    printf("\n");
}

/* ---------------- MAIN ---------------- */

int main(){
    loadData();
    int ch;

    do{
        printf("\n\n1.Check Availability\n2.Enter Vehicle\n3.Exit Vehicle\n4.View Parking Space\n5.Exit\nChoice: ");
        scanf("%d",&ch);

        switch(ch){
            case 1: viewSpace(); break;
            case 2: enterVehicle(); break;
            case 3: exitVehicle(); break;
            case 4: viewSpace(); break;
            case 5: saveData(); break;
        }

    }while(ch!=5);

    return 0;
}
