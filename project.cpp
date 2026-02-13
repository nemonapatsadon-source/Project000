#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <sstream>

using namespace std;

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
int D = 1; 
int M = 100; 
int G = 0;      // <--- เริ่มต้นเป็น 0 ตามที่แจ้งครับ
int AP = 5;
const int MAX_AP = 5;
const int RENT_AMOUNT = 500; 
const int RENT_CYCLE = 7;
int debt = 0; 
int rentStrikes = 0; 
int expansionCost = 5;

// ไอเทมพิเศษ
bool hasCCTV = false;
bool hasScarecrow = false;
bool hasLottery = false;

string plantNames[] = {"Carrot", "Corn", "Pumpkin"};
int plantCosts[] = {20, 50, 100};   
int plantSells[] = {40, 120, 300};  
int baseSuccess[] = {80, 60, 40}; 

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

void generateQuest() {
    int pIdx = rand() % 3;
    currentQuest.targetPlant = plantNames[pIdx];
    currentQuest.targetAge = (rand() % 3) + 1;
    currentQuest.rewardMoney = (pIdx * 60) + 150;
    currentQuest.isActive = true;
}

// --- ระบบ Save/Load ---
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
    cout << "________________________________________________________________\n [ GAME STOPPED ]\n Code: " << ss.str() << "\n________________________________________________________________\n";
    exit(0);
}

void loadGame(string code) {
    try {
        replace(code.begin(), code.end(), '_', ' ');
        stringstream ss(code); int rowCount;
        ss >> playerName >> D >> M >> G >> debt >> rentStrikes >> rowCount;
        string qD; ss >> qD; replace(qD.begin(), qD.end(), '.', ' ');
        stringstream qss(qD); qss >> currentQuest.targetPlant >> currentQuest.targetAge >> currentQuest.rewardMoney;
        currentQuest.isActive = true;
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
        cout << "\n[Success] Welcome back!"; waitEnter();
    } catch (...) { generateQuest(); }
}

// --- เหตุการณ์สุ่ม ---
void processNightEvents() {
    clearScreen();
    cout << "________________________________________________________________" << endl;
    cout << " [ NIGHT REPORT - DAY " << D << " ]" << endl;
    cout << "________________________________________________________________" << endl;
    
    int event = rand() % 100;
    if (event < 15) {
        cout << " > Luck: It rained! All plants are watered (+20% Success tomorrow)." << endl;
        for(auto &r : rows) if(r.isPlanted) r.isWatered = true;
    } else if (event < 25) {
        if (hasScarecrow) cout << " > Defense: Wild animals came, but the Scarecrow scared them off!" << endl;
        else {
            int target = rand() % rows.size();
            if (rows[target].isPlanted) { rows[target].isPlanted = false; cout << " > Danger: Animals ate your crop in Row " << target+1 << "!" << endl; }
            else cout << " > Info: Animals visited but there was nothing to eat." << endl;
        }
    } else if (event < 35) {
        if (hasCCTV) cout << " > Defense: A thief spotted your CCTV and fled!" << endl;
        else {
            int loss = (rand() % 151) + 50; M = max(0, M - loss);
            cout << " > Danger: A thief stole $" << loss << " from your wallet!" << endl;
        }
    } else cout << " > Peace: A quiet night at the farm." << endl;

    if (hasLottery) {
        if (rand() % 10 == 0) { M += 1000; cout << " > !!! WINNER: You won the Lottery jackpot $1000! !!!" << endl; }
        else cout << " > Lottery: Better luck next time!" << endl;
        hasLottery = false;
    }
    cout << "________________________________________________________________" << endl;
}

// --- UI ---
void drawInterface() {
    clearScreen();
    int dRent = RENT_CYCLE - (D % RENT_CYCLE); if (D % RENT_CYCLE == 0) dRent = 0;
    cout << "________________________________________________________________" << endl;
    cout << " [ PLAYER: " << left << setw(12) << playerName << " ]  DAY: " << left << setw(4) << D << " | MONEY: " << left << setw(6) << M << endl;
    cout << " [ STATUS ]  GEMS: " << left << setw(4) << G << " | DEBT: $" << left << setw(5) << debt << " | STRIKES: " << rentStrikes << "/3" << endl;
    cout << " [ GEAR ]    CCTV: " << (hasCCTV?"ON":"OFF") << " | Scarecrow: " << (hasScarecrow?"ON":"OFF") << " | Lottery: " << (hasLottery?"Owned":"-") << endl;
    if (dRent == 0) cout << " !!! ALERT: RENT DUE TODAY: $" << RENT_AMOUNT + debt << endl;
    else cout << " [ INFO ]    Rent collection in: " << dRent << " days" << endl;
    cout << "________________________________________________________________" << endl << endl;

    for (int i = 0; i < (int)rows.size(); i++) {
        string vis = rows[i].isPlanted ? (rows[i].isRotten ? "[X] [X] [X]" : "{@} {@} {@}") : "[ ] [ ] [ ]";
        string st = rows[i].isPlanted ? rows[i].plantName : "Empty";
        if (rows[i].isRotten) st = "!!! ROTTEN !!!";
        else {
            if (rows[i].isWatered) st += " (Wet)";
            if (rows[i].hasFertilizer) st += " [Fert]";
            if (rows[i].sprinklerDays > 0) st += " [Spr:" + to_string(rows[i].sprinklerDays) + "]";
        }
        cout << "  " << i+1 << ")  " << vis << "    Plant: " << left << setw(20) << st << " | Age: " << (rows[i].isPlanted ? to_string(rows[i].age) : "-") << endl;
        cout << "      ------------------------------------------------------" << endl;
    }
    cout << "\n [ QUEST ] Need: " << currentQuest.targetPlant << " (Age:" << currentQuest.targetAge << ") Reward: $" << currentQuest.rewardMoney << endl;
    cout << "________________________________________________________________" << endl << endl;
    cout << " [ ACTIONS ]  AP: " << AP << "/" << MAX_AP << " | Expand: " << expansionCost << " Gems" << endl;
    cout << "  1) Plant   - Grow seeds ($)     2) Harvest - Collect crop" << endl;
    cout << "  3) Water   - Hydrate row        4) Shop    - Buy upgrades" << endl;
    cout << "  5) Index   - Rules & Prices     6) Quest   - Deliver order" << endl;
    cout << "  7) End Day - Sleep & Events     8) Stop    - Save & Exit" << endl;
    cout << "________________________________________________________________\n Command: ";
}

void showIndex() {
    clearScreen();
    cout << "________________________________________________________________" << endl;
    cout << " [ FARMER'S HANDBOOK ]" << endl;
    cout << "________________________________________________________________" << endl;
    cout << "  PLANTS INFO:" << endl;
    for(int i=0; i<3; i++) {
        cout << "  - " << left << setw(8) << plantNames[i] << " Cost: $" << setw(4) << plantCosts[i] 
             << " | Base Sell: $" << setw(4) << plantSells[i] << " | Success: " << baseSuccess[i] << "%" << endl;
    }
    cout << "\n  SUCCESS BONUSES:" << endl;
    cout << "  - Watering: +20% | Fertilizer: +15% | Young Crop: -40%" << endl;
    cout << "\n  HOW TO GET GEMS:" << endl;
    cout << "  - Complete Quests (Option 6) to receive 2 Gems per quest." << endl;
    cout << "________________________________________________________________" << endl;
    waitEnter();
}

void endDay() {
    if (D % RENT_CYCLE == 0) {
        int total = RENT_AMOUNT + debt;
        if (M >= total) { M -= total; debt = 0; rentStrikes = 0; }
        else { debt = total - M; M = 0; rentStrikes++; if (rentStrikes >= 3) { cout << "GAME OVER!"; exit(0); } }
    }
    processNightEvents();
    D++; AP = MAX_AP; M += 50;
    for(auto &r : rows) {
        if (r.isPlanted) {
            r.age++; r.isWatered = false;
            if (r.sprinklerDays > 0) { r.isWatered = true; r.sprinklerDays--; }
            if (rand() % 100 < 10) r.isRotten = true;
        } else if (r.sprinklerDays > 0) r.sprinklerDays--;
    }
    waitEnter();
}

void planting() {
    if (AP <= 0) return;
    int r; cout << "Row: "; cin >> r; r--;
    if (r >= 0 && r < (int)rows.size() && !rows[r].isPlanted) {
        cout << "1.Carrot($20) 2.Corn($50) 3.Pumpkin($100): ";
        int p; cin >> p; p--;
        if (p >= 0 && p <= 2 && M >= plantCosts[p]) {
            M -= plantCosts[p]; rows[r].isPlanted = true; rows[r].plantName = plantNames[p];
            rows[r].plantTypeIndex = p; rows[r].age = 0; rows[r].isRotten = false; AP--;
        }
    }
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

void openShop() {
    clearScreen();
    cout << "--- SHOP --- (Money: $" << M << " | Gems: " << G << ")" << endl;
    cout << " 1) Fertilizer($50)  2) Sprinkler($150)  3) Expand(" << expansionCost << "G)" << endl;
    cout << " 4) CCTV($200)       5) Scarecrow($150)  6) Lottery($50)" << endl;
    cout << " 0) Exit" << endl;
    int sc; cin >> sc;
    if(sc==1){ int r; cout<<"Row: "; cin>>r; if(M>=50){M-=50; rows[r-1].hasFertilizer=true;} }
    else if(sc==2){ int r; cout<<"Row: "; cin>>r; if(M>=150){M-=150; rows[r-1].sprinklerDays=3;} }
    else if(sc==3){ if(G >= expansionCost){G -= expansionCost; rows.push_back(Row()); expansionCost += 5;} }
    else if(sc==4){ if(M>=200){M-=200; hasCCTV=true;} }
    else if(sc==5){ if(M>=150){M-=150; hasScarecrow=true;} }
    else if(sc==6){ if(M>=50){M-=50; hasLottery=true;} }
}

int main() {
    srand(time(0)); clearScreen();
    cout << "========================================" << endl;
    cout << "       WELCOME TO THE MINI FARM         " << endl;
    cout << "========================================" << endl;
    cout << " Have you played before? (y/n): "; char ch; cin >> ch;
    if (ch == 'y' || ch == 'Y') { cout << "Enter Save Code: "; string code; cin >> code; loadGame(code); }
    else { cout << "Enter Farmer Name: "; cin >> playerName; generateQuest(); }

    while (true) {
        drawInterface();
        int c; if (!(cin >> c)) { cin.clear(); cin.ignore(1000, '\n'); continue; }
        switch (c) {
            case 1: planting(); break;
            case 2: harvesting(); break;
            case 3: { int r; cout << "Row: "; cin >> r; r--; if(r>=0 && r<(int)rows.size()){rows[r].isWatered=true; AP--;} break; }
            case 4: openShop(); break;
            case 5: showIndex(); break;
            case 6: {
                bool found = false;
                for(auto &r : rows) if(r.isPlanted && !r.isRotten && r.plantName == currentQuest.targetPlant && r.age == currentQuest.targetAge){
                    M += currentQuest.rewardMoney; G += 2; r.isPlanted = false; 
                    cout << "\n[Success] Quest Done! Gem +2"; generateQuest(); found = true; waitEnter(); break;
                } 
                if(!found) { cout << "\n[!] No matching crop."; waitEnter(); }
                break;
            }
            case 7: endDay(); break;
            case 8: stopGame(); break;
        }
    }
    return 0;
}