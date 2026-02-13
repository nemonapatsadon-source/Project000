#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <sstream>

using namespace std;

// --- 1. ข้อมูลพื้นฐาน (เพิ่มพืชหรือไอเทมที่นี่) ---
const int PLANT_TYPES = 4;
string plantNames[] = {"Carrot", "Corn", "Pumpkin"};
int plantCosts[]    = {20, 50, 100};   
int plantSells[]    = {40, 120, 300};  
int baseSuccess[]   = {80, 60, 40}; 

const int ITEM_TYPES = 5;
string itemNames[] = {"Fertilizer", "Sprinkler", "CCTV", "Scarecrow", "Lottery"};
int itemCosts[]    = {50, 150, 200, 150, 50};

// --- โครงสร้างข้อมูล ---
struct Row {
    bool isPlanted = false;
    string plantName = "";
    int plantTypeIndex = 0; 
    int age = 0; 
    bool isWatered = false;
    bool isRotten = false;
    bool hasFertilizer = false; 
    int sprinklerDays = 0;      
};

struct Quest {
    string targetPlant = "Carrot";
    int targetAge = 1;
    int rewardMoney = 150;
    bool isActive = false;
};

// --- ตัวแปรหลัก ---
string playerName = "Farmer";
int D = 1, M = 100, G = 0, AP = 5;
const int MAX_AP = 5, RENT_AMOUNT = 500, RENT_CYCLE = 7;
int debt = 0, rentStrikes = 0, expansionCost = 5;
bool hasCCTV = false, hasScarecrow = false, hasLottery = false;

vector<Row> rows(3); 
Quest currentQuest;

// --- ฟังก์ชันช่วยเหลือ ---
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void waitEnter() {
    cout << "\n> Press Enter to continue...";
    cin.ignore(1000, '\n'); cin.get();    
}

// --- ระบบ Save/Load (ใช้ stringstream ช่วยจัดการรหัส) ---
void stopGame() {
    stringstream ss;
    ss << playerName << "_" << D << "_" << M << "_" << G << "_" << debt << "_" << rentStrikes << "_" << rows.size();
    ss << "_" << currentQuest.targetPlant << "." << currentQuest.targetAge << "." << currentQuest.rewardMoney;
    ss << "_" << hasCCTV << "." << hasScarecrow << "." << hasLottery;
    for(const auto& r : rows) {
        ss << "_" << (r.isPlanted ? r.plantTypeIndex : -1) << "." << r.age << "." 
           << r.isWatered << "." << r.isRotten << "." << r.hasFertilizer << "." << r.sprinklerDays;
    }
    clearScreen();
    cout << "________________________________________________________________\n";
    cout << " [ GAME STOPPED ]\n Code: " << ss.str() << "\n";
    cout << "________________________________________________________________\n";
    exit(0);
}

void loadGame(string code) {
    try {
        replace(code.begin(), code.end(), '_', ' ');
        stringstream ss(code); int rowCount;
        ss >> playerName >> D >> M >> G >> debt >> rentStrikes >> rowCount;
        
        string qD; ss >> qD; replace(qD.begin(), qD.end(), '.', ' ');
        stringstream qss(qD); qss >> currentQuest.targetPlant >> currentQuest.targetAge >> currentQuest.rewardMoney;
        
        string iD; ss >> iD; replace(iD.begin(), iD.end(), '.', ' ');
        stringstream iss(iD); iss >> hasCCTV >> hasScarecrow >> hasLottery;
        
        rows.clear();
        for(int i = 0; i < rowCount; i++) {
            string rD; ss >> rD; replace(rD.begin(), rD.end(), '.', ' ');
            stringstream rss(rD); Row r; int pIdx;
            rss >> pIdx >> r.age >> r.isWatered >> r.isRotten >> r.hasFertilizer >> r.sprinklerDays;
            if(pIdx != -1) { r.isPlanted = true; r.plantTypeIndex = pIdx; r.plantName = plantNames[pIdx]; }
            rows.push_back(r);
        }
        expansionCost = 5 + ((rows.size() - 3) * 5);
        cout << "\n[Success] Welcome back, " << playerName << "!"; waitEnter();
    } catch (...) { 
        cout << "\n[Error] Invalid Code! Starting new game."; 
        waitEnter(); 
    }
}

// --- การแสดงผล UI ---
void drawInterface() {
    stringstream ss;
    int dRent = RENT_CYCLE - (D % RENT_CYCLE);
    if (D % RENT_CYCLE == 0) dRent = 0;

    ss << "________________________________________________________________\n";
    ss << " [ PLAYER: " << left << setw(12) << playerName << " ]  DAY: " << setw(4) << D << " | MONEY: " << setw(6) << M << "\n";
    ss << " [ STATUS ]  GEMS: " << left << setw(4) << G << " | DEBT: $" << setw(5) << debt << " | STRIKES: " << rentStrikes << "/3\n";
    ss << " [ GEAR ]    CCTV: " << (hasCCTV?"ON":"OFF") << " | Scarecrow: " << (hasScarecrow?"ON":"OFF") << " | Lottery: " << (hasLottery?"Owned":"-") << "\n";
    
    if (dRent == 0) ss << " !!! ALERT: RENT DUE TODAY: $" << RENT_AMOUNT + debt << "\n";
    else ss << " [ INFO ]    Rent collection in: " << dRent << " days\n";
    ss << "________________________________________________________________\n\n";

    for (int i = 0; i < (int)rows.size(); i++) {
        string vis = rows[i].isPlanted ? (rows[i].isRotten ? "[X] [X] [X]" : "{@} {@} {@}") : "[ ] [ ] [ ]";
        string info = rows[i].isPlanted ? rows[i].plantName : "Empty";
        if (rows[i].isRotten) info = "!!! ROTTEN !!!";
        else {
            if (rows[i].isWatered) info += " (Wet)";
            if (rows[i].hasFertilizer) info += " [Fert]";
            if (rows[i].sprinklerDays > 0) info += " [Spr:" + to_string(rows[i].sprinklerDays) + "]";
        }
        ss << "  " << i+1 << ")  " << vis << "    Plant: " << left << setw(20) << info << " | Age: " << (rows[i].isPlanted ? to_string(rows[i].age) : "-") << "\n";
        ss << "      ------------------------------------------------------\n";
    }

    ss << "\n [ QUEST ] Need: " << currentQuest.targetPlant << " (Age:" << currentQuest.targetAge << ") Reward: $" << currentQuest.rewardMoney << "\n";
    ss << "________________________________________________________________\n\n";
    ss << " [ ACTIONS ]  AP: " << AP << "/" << MAX_AP << " | Expand: " << expansionCost << " Gems\n";
    
    string menus[] = {"Plant", "Harvest", "Water", "Shop", "Index", "Quest", "End Day", "Stop"};
    for(int i=0; i<8; i++) {
        ss << "  " << i+1 << ") " << left << setw(11) << menus[i];
        if((i+1)%2==0) ss << "\n";
    }
    ss << "________________________________________________________________\n Command: ";

    clearScreen();
    cout << ss.str();
}

// --- ฟังก์ชันหลักอื่นๆ ---
void generateQuest() {
    int pIdx = rand() % PLANT_TYPES;
    currentQuest.targetPlant = plantNames[pIdx];
    currentQuest.targetAge = (rand() % 3) + 1;
    currentQuest.rewardMoney = (pIdx * 60) + 150;
    currentQuest.isActive = true;
}

void planting() {
    if (AP <= 0) return;
    int r; cout << "Row: "; cin >> r; r--;
    if (r >= 0 && r < (int)rows.size() && !rows[r].isPlanted) {
        cout << "Select Seed: ";
        for(int i=0; i<PLANT_TYPES; i++) cout << i+1 << "." << plantNames[i] << "($" << plantCosts[i] << ") ";
        int p; cin >> p; p--;
        if (p >= 0 && p < PLANT_TYPES && M >= plantCosts[p]) {
            M -= plantCosts[p]; rows[r].isPlanted = true; rows[r].plantName = plantNames[p];
            rows[r].plantTypeIndex = p; rows[r].age = 0; rows[r].isRotten = false; AP--;
        }
    }
}

void openShop() {
    clearScreen();
    stringstream ss;
    ss << "--- SHOP --- (Money: $" << M << " | Gems: " << G << ")\n";
    for(int i=0; i<ITEM_TYPES; i++) {
        ss << " " << i+1 << ") " << left << setw(12) << itemNames[i] << "($" << itemCosts[i] << ")  ";
        if((i+1)%2==0) ss << "\n";
    }
    ss << "\n 6) Expand Farm (" << expansionCost << " Gems)\n 0) Exit\n Choice: ";
    cout << ss.str();
    int sc; cin >> sc;
    if(sc >= 1 && sc <= 2) {
        int r; cout << "Row: "; cin >> r; r--;
        if(r >= 0 && r < (int)rows.size() && M >= itemCosts[sc-1]) {
            M -= itemCosts[sc-1];
            if(sc == 1) rows[r].hasFertilizer = true; else rows[r].sprinklerDays = 3;
        }
    } else if (sc == 3 && M >= itemCosts[2]) { M -= itemCosts[2]; hasCCTV = true; }
    else if (sc == 4 && M >= itemCosts[3]) { M -= itemCosts[3]; hasScarecrow = true; }
    else if (sc == 5 && M >= itemCosts[4]) { M -= itemCosts[4]; hasLottery = true; }
    else if (sc == 6 && G >= expansionCost) { G -= expansionCost; rows.push_back(Row()); expansionCost += 5; }
}

void harvesting() {
    if (AP <= 0) return;
    int r; cout << "Row: "; cin >> r; r--;
    if (r >= 0 && r < (int)rows.size() && rows[r].isPlanted) {
        int ch = baseSuccess[rows[r].plantTypeIndex] + (rows[r].isWatered ? 20 : 0) + (rows[r].hasFertilizer ? 15 : 0);
        if (rows[r].age < 2) ch -= 40;
        if (!rows[r].isRotten && (rand() % 100) < ch) {
            int s = plantSells[rows[r].plantTypeIndex] + (rows[r].age * 5); M += s;
            cout << "[Success] Sold for $" << s;
        } else cout << "[Fail] Crop lost!";
        rows[r].isPlanted = false; rows[r].hasFertilizer = false; AP--; waitEnter();
    }
}

int main() {
    srand(time(0));
    clearScreen();
    cout << "========================================" << endl;
    cout << "       WELCOME TO THE MINI FARM         " << endl;
    cout << "========================================" << endl;
    cout << " Have you played before? (y/n): "; char ch; cin >> ch;
    
    if (ch == 'y' || ch == 'Y') {
        cout << " Enter Save Code: "; string code; cin >> code;
        loadGame(code);
    } else {
        cout << " Enter Farmer Name: "; cin >> playerName;
        generateQuest();
    }

    while (true) {
        drawInterface();
        int c; if (!(cin >> c)) { cin.clear(); cin.ignore(1000, '\n'); continue; }
        switch (c) {
            case 1: planting(); break;
            case 2: harvesting(); break;
            case 3: { int r; cout << "Row: "; cin >> r; r--; if(r>=0 && r<(int)rows.size()){rows[r].isWatered=true; AP--;} break; }
            case 4: openShop(); break;
            case 5: { clearScreen(); cout << "Check Rules in Code Index Section\n"; waitEnter(); break; }
            case 6: {
                bool found = false;
                for(auto &r : rows) if(r.isPlanted && !r.isRotten && r.plantName == currentQuest.targetPlant && r.age == currentQuest.targetAge){
                    M += currentQuest.rewardMoney; G += 2; r.isPlanted = false; 
                    cout << "\n[Success] Quest Done! Gem +2"; generateQuest(); found = true; waitEnter(); break;
                } 
                if(!found) { cout << "\n[!] No matching crop."; waitEnter(); }
                break;
            }
            case 7: {
                D++; AP = MAX_AP; M += 50;
                for(auto &r : rows) if(r.isPlanted) { r.age++; r.isWatered = false; if(r.sprinklerDays > 0) { r.isWatered = true; r.sprinklerDays--; } if(rand()%100 < 10) r.isRotten = true; }
                cout << "\n> Next Day..."; waitEnter(); break;
            }
            case 8: stopGame(); break;
        }
    }
    return 0;
}
