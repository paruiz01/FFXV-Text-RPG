// Helena Ruiz Ramírez and Pablo Andrés Ruiz Ramírez
// December 20 of 2022
// Basics of computer programming for game developers
//
// Assignment 3: A Menace Sleeps in Keforgda
// Function: A dungeon crawler based on text prompts that relies on 
//           random encounters until you reach the objective.
//           Inspired by the quest "Menace Beneath Lucis" in FFXV.

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#pragma region Player Variables
struct Player {
    // Stats
    int health;
    int atk; // start at 10
    int def; // start at 10
    int effect; // 0 none, 1 poisoned, 2 petrified, 3 broken

    // Ally
    int allyChoice; // 1 Ignis (follow attack), 2 Gladio (block attacks), 3 Prompto (lower defense)
    char allyName[50];

    // Items
    int weaponEquip; // adds attack
    int shieldEquip; // adds defense
    int etherEquip; // allows magic attack
};
static struct Player noctis;
#pragma endregion

#pragma region Enemy Variables
struct Enemy {
    char name[50];
    int health;
    int atk;
    int def;
    int effect;
};
static struct Enemy floorEnemy;

// Floor 1 & 2
const int flanhealth = 80;
const int flanAtk = 10;
const int flanDef = 0;
// Floor 3 & 4
const int mindhealth = 130;
const int mindAtk = 15;
const int mindDef = 5;
// Floor 5
const int malborohealth = 180;
const int malboroAtk = 25;
const int malboroDef = 10;
// Secret Floor 6
const int sirTonberryhealth= 240;
const int sirTonberryAtk = 30;
const int sirTonberryDef = 15;
#pragma endregion

#pragma region Game Variables
int gameChoice;
static int roomToEnter = 0; // 1 = left, 2 = middle, 3 = right
static int floorsCleared = 0;
static int checkInput; // for checking number only inputs

/* RandomRoomRules = amount of room types to spawn on random floors
// Enemy  Weapon   Shield
  { 2,      0,      0 }, // Floor 1 two enemies, one empty
  { 1,      1,      1 }, // Floor 2 one room of each
  { 1,      1,      1 }, // Floor 3 one room of each
  { 2,      0,      0 }, // Floor 4 two enemies, one empty
  { 1,      1,      1 }, // Floor 5 one room of each
  { 1,      1,      1 }, // Floor 6 one room of each
  { 3,      0,      0 }, // Floor 7 all enemies
  { 3,      0,      0 }, // Floor 8 all enemies
*/

static int dungeonLayout[8][3] = {
// 0 = empty, 1 = shield, 2 = weapon, 3 = ether
// 4 = flan, 5 = mindflayer, 6 = malboro, 7 = sir tonberry
// first 4 floors are occupied randomly. floors 5 and 6 are mandatory fights
// Left   Middle   Right
  { 0,      0,      0 }, // Floor 1
  { 0,      0,      0 }, // Floor 2
  { 0,      0,      0 }, // Floor 3
  { 0,      0,      0 }, // Floor 4
  { 0,      0,      0 }, // Floor 5
  { 0,      0,      0 }, // Floor 6
  { 6,      6,      6 }, // Floor 7
  { 7,      7,      7 }  // (Secret) Floor 8
};

static char dungeonContent[8][3][50] = {
    {"","",""},
    {"","",""},
    {"","",""},
    {"","",""},
    {"","",""},
    {"","",""},
    {"Malboro","Malboro","Malboro"},
    {"Sir Tonberry","Sir Tonberry","Sir Tonberry"}
};
#pragma endregion

#pragma region Game Functions
void saveToFile() {
    FILE * fp = fopen("gameData.txt", "w+");
    if (fp == NULL) {
        printf("\nCould not open the file 'gameData.txt'\n");
        exit (1);
    }

    struct Player noctisInput;
    fwrite(&noctis, sizeof(struct Player), 1, fp);
    fwrite(&dungeonLayout, sizeof(dungeonLayout), 1, fp);
    fwrite(&dungeonContent, sizeof(dungeonContent), 1, fp);
    fwrite(&floorsCleared, sizeof(int), 1, fp);
    
    if(fwrite == 0) {
        printf("\nCould not write to the file 'gameData.txt'\n");
    }

    fclose (fp);
}

void readFromFile() {
    FILE * fp = fopen("gameData.txt", "r+");
    if (fp == NULL) {
        return;
    }
    
    struct Player playerInput;
    fread(&playerInput, sizeof(struct Player), 1, fp);
    noctis = playerInput;

    int layoutInput[8][3];
    fread(&layoutInput, sizeof(layoutInput), 1, fp);
    for (int i=0; i<8; i++) {
        for (int j=0; j<3; j++) {
            dungeonLayout[i][j] = layoutInput[i][j];
        }
    }

    char contentInput[8][3][50];
    fread(&contentInput, sizeof(contentInput), 1, fp);
    for (int i=0; i<8; i++) {
        for (int j=0; j<3; j++) {
            strncpy(dungeonContent[i][j], contentInput[i][j], sizeof(dungeonContent[i][j]));
        }
    }

    int floorsInput;
    fread(&floorsInput, sizeof(int), 1, fp);
    floorsCleared = floorsInput;

    fclose (fp);
}

void createDungeon() {
    int enemyRoom, enemiesSpawned, weaponRoom, weaponSpawned, 
        shieldRoom, shieldSpawned, etherRoom, etherSpawned;
    int roomsOccupied[3] = {0,0,0};
    enemiesSpawned = weaponSpawned = shieldSpawned = etherSpawned = 0;
    for (int j=floorsCleared; j<6; j++) {
        enemiesSpawned = 0; // resets enemies per each floor
        for (int i=0; i<3; i++) {
            roomsOccupied[i] = 0;
        }
        // if populating floors 1 and 4...
        if (j == 0 || j == 3) {
            while (enemiesSpawned < 2) { // keep trying to spawn 2 enemies
                do {
                    enemyRoom = rand() % 3; // if room is not free, keeps trying
                } while (roomsOccupied[enemyRoom] == 1);
                if (j==0) {
                    dungeonLayout[j][enemyRoom] = 4; // spawns a flan
                    strcpy(dungeonContent[j][enemyRoom], "Flan");
                } else {
                    dungeonLayout[j][enemyRoom] = 5; // spawns a mindflayer
                    strcpy(dungeonContent[j][enemyRoom], "Mindflayer");
                }
                roomsOccupied[enemyRoom] = 1;
                enemiesSpawned++;
            }
        // if populating floors 2, 3, 5, and 6...
        } else {
            enemyRoom = rand() % 3; // since this goes first, all rooms will be free. always spawns
            if (j == 1 || j == 2) {
                dungeonLayout[j][enemyRoom] = 4; // spawns a flan
                strcpy(dungeonContent[j][enemyRoom], "Flan");
            } else {
                dungeonLayout[j][enemyRoom] = 5; // spawns a mindflayer
                strcpy(dungeonContent[j][enemyRoom], "Mindflayer");
            }
            roomsOccupied[enemyRoom] = 1;
            enemiesSpawned++;
            // shields, weapons, and ethers spawn randomly, BUT there can only be 1 of each at the start
            shieldRoom = rand() % 3;
            if ((roomsOccupied[shieldRoom] == 0) && (shieldSpawned == 0)) { // if room is free, and no shields exist, spawns
                dungeonLayout[j][shieldRoom] = 1; // spawns shield
                strcpy(dungeonContent[j][shieldRoom], "Shield");
                shieldSpawned++;
                roomsOccupied[shieldRoom] = 1;
            }
            weaponRoom = rand() % 3;
            if ((roomsOccupied[weaponRoom] == 0) && (weaponSpawned == 0)) { // if room is free, and no weapons exist, spawns
                dungeonLayout[j][weaponRoom] = 2; // spawns a weapon
                strcpy(dungeonContent[j][weaponRoom], "Weapon");
                weaponSpawned++;
                roomsOccupied[weaponRoom] = 1;
            }
            etherRoom = rand() % 3;
            if ((roomsOccupied[etherRoom] == 0) && (etherSpawned == 0)) { // if room is free, and no weapons exist, spawns
                dungeonLayout[j][etherRoom] = 3; // spawns a weapon
                strcpy(dungeonContent[j][etherRoom], "Ether");
                etherSpawned++;
                roomsOccupied[etherRoom] = 1;
            }
            // if there are still 2 rooms free, spawn a monster
            int ctrRooms = 0;
            for (int k=0; k<3; k++) {
                if (roomsOccupied[k] == 0) ctrRooms++;
            }
            if (ctrRooms == 2) {
                while (enemiesSpawned < 2) {
                    do {
                        enemyRoom = rand() % 3; // if room is not free, keeps trying
                    } while (roomsOccupied[enemyRoom] == 1);
                    if (j == 1 || j == 2) {
                        dungeonLayout[j][enemyRoom] = 4; // spawns a flan
                        strcpy(dungeonContent[j][enemyRoom], "Flan");
                    } else {
                        dungeonLayout[j][enemyRoom] = 5; // spawns a mindflayer
                        strcpy(dungeonContent[j][enemyRoom], "Mindflayer");
                    }
                    enemiesSpawned++;
                }
            }
        }
    }
}
#pragma endregion

#pragma region Other Functions
// int input check
char c;
int clean_stdin()
{
    while (getchar()!='\n');
    return 1;
}

// Color Text
void blue () { // Noctis
  printf("\033[0;34m");
}
void red () { // Ignis
  printf("\033[0;31m");
}
void green () { // Gladio
  printf("\033[0;32m");
}
void yellow () { // Prompto
  printf("\033[0;33m");
}
void reset () { // Default Color
  printf("\033[0m");
}
#pragma endregion

#pragma region Function Definitions
void runGame();
void handleEffects();
int exploreFloor();
void printMap();
void chooseRoom();
void emptyRoom(); // may find shield, weapon, or ether
int handleMonsterEncounter();
void playerChoice();
void addPlayerHealth();
void daemonAttack(int);
void ignisEffect(int);
int gladioEffect(int);
void promptoEffect();
void ringOfLucii(int);
#pragma endregion

int main() {
    srand(time(NULL));

    readFromFile();

    system("cls");

    printf("\t\n== WELCOME TO THE TEXT-BASED RPG INSPIRED BY FINAL FANTASY XV! ==\n");
    if (floorsCleared > 0) {
        printf("\nAn ongoing campaing was found, would you like to continue or start over?\n");
        do {
            printf("1) Continue, 2) Start Over: ");
            checkInput = scanf("%d", &gameChoice);
            if (gameChoice == 0) c = getchar();
            clean_stdin();
        } while(gameChoice!=1 && gameChoice!=2);
        if (gameChoice == 2) floorsCleared = 0;
    }

    createDungeon();

    if (floorsCleared == 0) {
        noctis.health = 360;
        noctis.atk = 10;
        noctis.def = 0;
        noctis.effect = 0;
        noctis.weaponEquip = 0;
        noctis.shieldEquip = 0;
        noctis.etherEquip = 0;
        printf("\n== A MENACE SLEEPS IN KEFORGDA ==\n");
        printf("\nPrince Noctis and his friends spent the night camping out in the wild."
        "\nWhen they woke up, they noticed that their chocobos were stolen!"
        "\nThey followed a trail of their footsteps that led to the entrance of Keforgda Dungeon."
        "\nThey heard a faint \"Kweeeh!\" call out from the depths..."
        "\nNoctis decides to bring one ally with him while the others watch over the camp."
        "\n\nWho will he choose?\n");
        do {
            printf("1) Ignis, 2) Gladio, 3) Prompto: ");
            checkInput = scanf("%d", &noctis.allyChoice);
            if (checkInput == 0) c = getchar();
            clean_stdin();
        } while(noctis.allyChoice<1 || noctis.allyChoice>6);
        switch (noctis.allyChoice) {
            case 1:
                printf("\nNoctis chose Ignis! The Hand of the King will help with tactical attacks.\n");
                red();
                printf("\n\tIgnis: Let nothing stand in your way.");
                blue();
                printf("\n\tNoctis: You've got my back?");
                red();
                printf("\n\tIgnis: Always.\n\n");
                reset();
                strcpy(noctis.allyName, "Ignis");
                break;
            case 2:
                printf("\nNoctis chose Gladio! The Shield of the King will protect his majesty from enemies.\n");
                green(); 
                printf("\n\tGladio: I got this. You just sit back and watch how it's done.");
                blue();
                printf("\n\tNoctis: I bet I could show you a thing or two.");
                green(); 
                printf("\n\tGladio: Well, then. By all means.\n\n");
                reset();
                strcpy(noctis.allyName, "Gladio");
                break;
            case 3:
                printf("\nNoctis chose Prompto! The firearms master will break through any enemy defense.\n");
                yellow();
                printf("\n\tPrompto: It's a good chance to earn some experience points.");
                blue();
                printf("\n\tNoctis: Is this a game to you?");
                yellow();
                printf("\n\tPrompto: Sort of. Just trying to level up here, dude!\n\n");
                reset();
                strcpy(noctis.allyName, "Prompto");
                break;
            default:
                break;
        }
        system("pause");
    }
    
    runGame();

    return 0;
}

void runGame() {
    int isGameOver = 0;
    // While the game is not over, execute loop
    while (isGameOver == 0) {
        printf("===============================================================================================\n");
        isGameOver = exploreFloor();
    }

    if (isGameOver == 1) {
        printf("\n== GAME OVER ==");
        printf("\nNoctis took a critical hit and fell unconscious."
                "\n%s took him to safety, but the chocobos were never seen again...\n",
                noctis.allyName);
    }
    else if (isGameOver == 2) {
        printf("\n== CHOCOBOS RESCUED ==");
        printf("\nFinally, the party has reached the end of the dungeon."
            "\nThe cries from the Chocobos were stronger than ever, they are close."
            "\nNoctis shines his light to the corner of the cave and sees them!\n"
            "\nThey rescued their Chocobos and talked about their adventures on their way out."
            "\nOnce they met up with the other 2 that had stayed behind, they set up camp and relaxed by the fire.\n"
            "\nHowever, Noctis couldn't help but wonder... who was the perpetrator?\n"
            , noctis.allyName);
    }
    else if (isGameOver == 3) {
        printf("\n== CLOSING GAME ==\n\n");
    }
    else {
        printf("\n== CHOCOBOS RESCUED AND THIEF DEFEATED ==");
        printf("\nAt last, Noctis and %s defeated the true villain. No Chocobo would be terrorized again.\n"
        "\nThey rescued their Chocobos and talked about their adventures on their way out."
        "\nOnce they met up with the other 2 that had stayed behind, they set up camp and relaxed by the fire.\n"
        "\nThey were ready to face together other mishaps in their journey.\n", noctis.allyName);
    }

    printf("===============================================================================================\n");
    system("pause");
}

int exploreFloor() {
    if (floorsCleared == 7) { // secret boss unlocked
        if (noctis.health>=100 && noctis.atk>=30 && noctis.def>=10) { // secret boss
            printf("Finally, the party has reached the end of the dungeon."
            "\nThe cries from the Chocobos stronger than ever, they are close."
            "\nNoctis shines his light to the corner of the cave and sees them!"
            "\n\nSuddenly, %s shouts to watch out, a monster appeared behind him!"
            "\nThis was him, he was the Chocobo thief!\n"
            , noctis.allyName);
            return handleMonsterEncounter();
        } else { // game won! without secret boss
            return 2;
        }
    } else if (floorsCleared == 8) { // game won with secret boss
        return 4;
    }

    printMap(); // prints map as player moves through floors
    chooseRoom(); // player chooses which room to enter
    
    switch (dungeonLayout[floorsCleared][roomToEnter]) {
        case 1:
            if (noctis.shieldEquip == 0)
            {
                printf("\nNoctis found the Ziedrich Shield!\n");
                noctis.shieldEquip = 1;
            } else {
                emptyRoom();
            }
            break;
        case 2:
            if (noctis.weaponEquip == 0)
            {
                printf("\nNoctis found the Soul Saber!\n");
                noctis.weaponEquip = 1;
            } else {
                emptyRoom();
            }
            break;
        case 3:
            if (noctis.etherEquip == 0) {
                printf("\nNoctis picked up an ether!\n");
                noctis.etherEquip = 1;
            } else {
                emptyRoom();
            }
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            return handleMonsterEncounter();
        default:
            emptyRoom();
            break;
    }

    printf("\nNoctis and %s descend further into the darkness...\n", noctis.allyName);
    floorsCleared++;

    printf("\n===============================================================================================");
    saveToFile();
    printf("\nYour progress has been saved. Do you wish to exit? \n");
    do {
        printf("1) Continue, 2) Exit: ");
        checkInput = scanf("%d", &gameChoice);
        if (gameChoice == 0) c = getchar();
        clean_stdin();
    } while(gameChoice!=1 && gameChoice!=2);
    if (gameChoice == 2) return 3;

    return 0;
}

void printMap() {
    printf("\n\n");
    printf("          %-19s%-19s%-19s\n", "Left Tunnel", "Middle Tunnel", "Right Tunnel");
    for (int j = 0; j <= floorsCleared && j < 8; j++) {
        printf("         ===========================================================\n");
        if ( j < floorsCleared) {
            printf("Floor %d: ||  %-15s||  %-15s||  %-15s||\n", j+1,
                    dungeonContent[j][0], dungeonContent[j][1], dungeonContent[j][2]);
        } else {
            printf("Floor %d: ||  %-15s||  %-15s||  %-15s||\n", j+1, "?", "?", "?");
        }
        
    }
    printf("         ===========================================================\n");
    printf("\n\n");
    return;
}

void chooseRoom() {
    char c;
    printf("Which tunnel will Noctis and %s enter?\n", noctis.allyName);
    do {
        printf("1) Left, 2) Middle, 3) Right: ");
        checkInput = scanf(" %d", &roomToEnter);
        if (checkInput == 0) roomToEnter = 4;
        clean_stdin();
    } while (roomToEnter<1 || roomToEnter>3);
    roomToEnter--;
}

void emptyRoom() {
    int roomSearched = 0;
    int itemGrabbed = 0;
    int itemChance = rand() % 10;

    printf("\nNoctis and %s entered a dark room...", noctis.allyName);

    if (noctis.allyChoice == 1) {
        if (itemChance >= 5) { // found sword
            roomSearched = 1;
            if (noctis.weaponEquip == 0) {
                printf("\nAnd they found a sturdy Rune Saber!\n");
                noctis.weaponEquip = 1;
                itemGrabbed = 1;
            }
        }
        else if (itemChance >= 3 && itemChance < 5) { // found shield
            roomSearched = 1;
            if (noctis.shieldEquip == 0) {
                printf("\nAnd they found a forgotten Black Prince Shield!\n");
                noctis.shieldEquip = 1;
                itemGrabbed = 1;
            }
        }
        else if (itemChance >= 1 && itemChance < 3) { // found ether
            roomSearched = 1;
            if (noctis.etherEquip == 0) {
                printf("\nAnd they picked up an ether!\n");
                noctis.etherEquip = 1;
                itemGrabbed = 1;
            }
        }
    }
    else if (noctis.allyChoice == 2) {
        if (itemChance >= 5) {
            roomSearched = 1;
            if (noctis.shieldEquip == 0) {
                printf("\nAnd they found a sturdy Hero's Shield!\n");
                noctis.shieldEquip = 1;
                itemGrabbed = 1;
            }
        }
        else if (itemChance >= 3 && itemChance < 5) {
            roomSearched = 1;
            if (noctis.weaponEquip == 0) {
                printf("\nAnd they found a forgotten Flame Tongue!\n");
                noctis.weaponEquip = 1;
                itemGrabbed = 1;
            }
        }
        else if (itemChance >= 1 && itemChance < 3) {
            roomSearched = 1;
            if (noctis.etherEquip == 0) {
                printf("\nAnd they picked up an ether!\n");
                noctis.etherEquip = 1;
                itemGrabbed = 1;
            }
        }
    }
    else if (noctis.allyChoice == 3) {
        if (itemChance >= 5) {
            roomSearched = 1;
            if (noctis.etherEquip == 0) {
                printf("\nAnd they picked up a pack of ethers!\n");
                noctis.etherEquip = 1;
                itemGrabbed = 1;
            }
        }
        else if (itemChance >= 3 && itemChance < 5) {
            roomSearched = 1;
            if (noctis.weaponEquip == 0) {
                printf("\nAnd they found a forgotten Flame Tongue!\n");
                noctis.weaponEquip = 1;
                itemGrabbed = 1;
            }
        }
        else if (itemChance >= 1 && itemChance < 3) {
            roomSearched = 1;
            if (noctis.shieldEquip == 0) {
                printf("\nAnd they found a forgotten Black Prince Shield!\n");
                noctis.shieldEquip = 1;
                itemGrabbed = 1;
            }
        }
    }

    if (roomSearched == 1) {
        if (itemGrabbed == 0) printf("\n... but they already had better equipment!\n");
    } else {
        printf("\n... but they found nothing. The silence was deafening and disappointing.\n");
    }

}

int handleMonsterEncounter() {
    if (floorsCleared <= 2) {
        strcpy(floorEnemy.name, "Flan");
        floorEnemy.health = flanhealth;
        floorEnemy.atk = flanAtk;
        floorEnemy.def = flanDef;
        floorEnemy.effect = 0;
        printf("\nA horrible and squidgy blue substance with yellow eyes forms before you. %d Attack | %d Defense\n", floorEnemy.atk, floorEnemy.def);
        switch (noctis.allyChoice) {
            case 1:
                red();
                printf("\n\tIgnis: Who will fall first: us or them?");
                blue();
                printf("\n\tNoctis: I'm takin em down!\n");
                reset();
                break;
            case 2:
                blue();
                printf("\n\tNoctis: I'm going all out.");
                green();
                printf("\n\tGladio: Just don't get too carried away.");
                blue();
                printf("\n\tNoctis: You're one to talk.\n");
                reset();
                break;
            case 3:
                blue();
                printf("\n\tNoctis: Wanna bash some heads in?");
                yellow();
                printf("\n\tPrompto: Oooh! Head-bashing!\n");
                reset();
                break;
            default:
                break;
        }                      
    } else if (floorsCleared > 2 && floorsCleared <= 5) {
        strcpy(floorEnemy.name, "Mindflayer");
        floorEnemy.health = mindhealth;
        floorEnemy.atk = mindAtk;
        floorEnemy.def = mindDef;
        printf("\nA forgotten, slimy horror lies ahead. Its face twisted as a mask that mocks nature, with countless tentacles under a robe. %d Attack | %d Defense\n", floorEnemy.atk, floorEnemy.def);
        switch (noctis.allyChoice) {
            case 1:
                red();
                printf("\n\tIgnis: A fiend fit to be eaten.");
                blue();
                printf("\n\tCookin' up a new recipe?");
                red();
                printf("\n\tIgnis: You'll find out come supper.\n");
                reset();
                break;
            case 2:
                green();
                printf("\n\tGladio: UGH. Ugliest things I've ever seen.");
                blue();
                printf("\n\tNoctis: I don't wanna go anywhere near it.\n");
                reset();
                break;
            case 3:
                yellow();
                printf("\n\tPrompto: Whatever that is, it's disgusting.");
                blue();
                printf("\n\tNoctis: Quit your crying.\n");
                reset();
                break;
            default:
                break;
        }        
    } else if (floorsCleared == 6) {
        strcpy(floorEnemy.name, "Malboro");
        floorEnemy.health = malborohealth;
        floorEnemy.atk = malboroAtk;
        floorEnemy.def = malboroDef;
        printf("\nA huge plant-like monster jumps out of nowhere. Its giant extremities send shivers down your spine. It also smells putridly... %d Attack | %d Defense\n", floorEnemy.atk, floorEnemy.def );
        switch (noctis.allyChoice) {
            case 1:
                blue();
                printf("\n\tNoctis: This could be trouble.");
                red();
                printf("\n\tIgnis: Mustn't let ourselves be outflanked.\n");
                reset();
                break;
            case 2:
                green();
                printf("\n\tGladio: The bigger they are, the harder they fall.");
                blue();
                printf("\n\tNoctis: You should know.\n");
                reset();
                break;
            case 3:
                blue();
                printf("\n\tNoctis: What the hell's this?");
                yellow();
                printf("\n\tPrompto: Something we don't want to mess with.\n");
                reset();
                break;
            default:
                break;
        }   
    } else {
        strcpy(floorEnemy.name, "SirTonberry");
        floorEnemy.health = sirTonberryhealth;
        floorEnemy.atk = sirTonberryAtk;
        floorEnemy.def = sirTonberryDef;
        printf("\nA small, orange, reptile-looking figure carrying a toy knife appears. It has its hood up appearing ready to strike at any moment. %d Attack | %d Defense\n", floorEnemy.atk, floorEnemy.def);
        switch (noctis.allyChoice) {
            case 1:
                red();
                printf("\n\tIgnis: A kitchen knife. Wonder if it's a culinary battle he wants.");
                blue();
                printf("\n\tNoctis: I somehow doubt that.\n");
                reset();
                break;
            case 2:
                blue();
                printf("\n\tNoctis: This is gonna take a while.");
                green();
                printf("\n\tGladio: What, you worried?");
                blue();
                printf("\n\tNoctis: You wish.\n");
                reset();
                break;
            case 3:
                blue();
                printf("\n\tPrompto: He's got a knife! Maybe it's for cooking?");
                yellow();
                printf("\n\tNoctis: Don't bet on it.\n");
                reset();
                break;
            default:
                break;
        }   
    }

    // battle turn cycle
    while (floorEnemy.health > 0) {
        printf("\n------------------------------\n");
        if (noctis.health <= 0) {
            return 1; // noctis killed, game over
        }
        if (floorEnemy.health == 1) {
        printf("\nThe %s has %d health left! Now is your chance!", floorEnemy.name, floorEnemy.health);
        } else {
            printf("\n%s: %d health || %d defense || %d attack", floorEnemy.name, floorEnemy.health, floorEnemy.def, floorEnemy.atk);
        }
        printf("\nNoctis: %d health || %d defense || %d attack\n", noctis.health, noctis.def, noctis.atk);
        playerChoice();
    }

    floorsCleared++;
    printf("\nThe %s was defeated! Noctis and %s descend further into the darkness...\n", floorEnemy.name, noctis.allyName);

    switch (noctis.allyChoice) {
        case 1:
            red();
            printf("\n\tIgnis: None the worse for wear.");
            blue();
            printf("\n\tNoctis: Nothing to it.\n");
            reset();
            break;
        case 2:
            blue();
            printf("\n\tNoctis: I even amaze myself sometimes.");
            green(); 
            printf("\n\tGladio: Your ego is what amazes me.\n");
            reset();
            break;
        case 3:
            yellow();
            printf("\n\tPrompto: Was that perfect or what?");
            blue();
            printf("\n\tNoctis: I'll give you that.\n");
            reset();
            break;
        default:
            break;
    }

    printf("\n------------------------------\n");
    printf("\nCURRENT STATS:"
            "\n  Health: %d"
            "\n  Attack: %d (+5)"
            "\n  Defense: %d (+3)", noctis.health, noctis.atk, noctis.def);
    if (noctis.etherEquip) printf("\n  With the ether, Noctis can use magic.");
    if (noctis.weaponEquip) printf("\n  The Durandal is currently equipped.");
    if (noctis.shieldEquip) printf("\n  The Aegis Shield is currently equipped.");
    printf("\n\n------------------------------\n");
    noctis.atk += 5;
    noctis.def += 3;

    printf("\n===============================================================================================");
    saveToFile();
    printf("\nYour progress has been saved. Do you wish to exit? \n");
    do {
        printf("1) Continue, 2) Exit: ");
        checkInput = scanf("%d", &gameChoice);
        if (gameChoice == 0) c = getchar();
        clean_stdin();
    } while(gameChoice<1 || gameChoice>2);
    if (gameChoice == 2) return 3;

    return 0; // enemy killed, next floor
}

void playerChoice() {
    int totalDamage;
    int shieldDeflect = 0;
    int attackChance;
    int choice;

    if (noctis.allyChoice == 3) promptoEffect();

    printf("\nNoctis thinks carefully about his next move..."
            "\n  1. Normal Attack");
    if (noctis.etherEquip) printf("\n  2. Magic Effect");
    if (noctis.weaponEquip) printf("\n  3. Strong Attack");
    if (noctis.shieldEquip) printf("\n  4. Deflect");
    printf("\n");
    do {
        checkInput = scanf(" %d", &choice);
        if (checkInput == 0) choice = 5;
        clean_stdin();
    } while ((choice < 1 || choice > 4) || (!noctis.etherEquip && choice == 2) || (!noctis.weaponEquip && choice == 3) || (!noctis.shieldEquip && choice == 4));

    switch (choice) {
        case 1:
            attackChance = rand() % 10;
            if (attackChance < 8) {
                printf("\nNoctis attacks the %s successfully!", floorEnemy.name);
                if (floorEnemy.def > 0) {
                    totalDamage = noctis.atk - floorEnemy.def;
                } else {
                    totalDamage = noctis.atk;
                }
                printf("\nNoctis dealt %d damage with his Engine Blade.", totalDamage, floorEnemy.name);
                floorEnemy.health -= totalDamage;
                ringOfLucii(totalDamage);
                if (noctis.allyChoice == 1) ignisEffect(totalDamage);
            } else {
                printf("\nNoctis swings into the air and misses his attack.\n");
                switch (noctis.allyChoice) {
                    case 1:
                        red();
                        printf("\n\tIgnis: *chuckles* Ahem, excuse me.");
                        blue();
                        printf("\n\tNoctis: Goddammit.\n");
                        reset();
                        break;
                    case 2:
                        green();
                        printf("\n\tGladio: The hell you doing?");
                        blue();
                        printf("\n\tNoctis: Come on. Save it.\n");
                        reset();
                        break;
                    case 3:
                        yellow();
                        printf("\n\tPrompto: Uh, Noct? Did you just-");
                        blue();
                        printf("\n\tNoctis: You saw nothing!");
                        yellow();
                        printf("\n\tPrompto: Right. Nothing.\n");
                        reset();
                        break;
                    default:
                        break;
                }
            }
            break;
        case 2:
            printf("\nNoctis secures the attack with a magic attack!");
            totalDamage = noctis.atk * 0.4;
            if (totalDamage == 0) totalDamage = 2;
            printf("\nThe %s took %d damage from Blizzara.", floorEnemy.name, totalDamage);
            floorEnemy.health -= totalDamage;
            ringOfLucii(totalDamage);
            if (noctis.allyChoice == 1) ignisEffect(totalDamage);
            break;
        case 3:
            attackChance = rand() % 10;
            if (attackChance < 6) {
                printf("\nNoctis dealt a strong attack successfully!", floorEnemy.name);
                if (floorEnemy.def > 0) {
                    totalDamage = noctis.atk - floorEnemy.def;
                } else {
                    totalDamage = noctis.atk;
                }
                totalDamage += 10;
                printf("\nThe %s trembles as it takes %d damage.", floorEnemy.name, totalDamage);
                floorEnemy.health -= totalDamage;
                ringOfLucii(totalDamage);
                if (noctis.allyChoice == 1) ignisEffect(totalDamage);
            } else {
                printf("\nNoctis tripped from brute force and missed his attack.\n");
                switch (noctis.allyChoice) {
                    case 1:
                        red();
                        printf("\n\tIgnis: *chuckles* Ahem, excuse me.");
                        blue();
                        printf("\n\tNoctis: Goddammit.\n");
                        reset();
                        break;
                    case 2:
                        green();
                        printf("\n\tGladio: The hell you doing?");
                        blue();
                        printf("\n\tNoctis: Come on. Save it.\n");
                        reset();
                        break;
                    case 3:
                        yellow();
                        printf("\n\tPrompto: Uh, Noct? Did you just-");
                        blue();
                        printf("\n\tNoctis: You saw nothing!");
                        yellow();
                        printf("\n\tPrompto: Right. Nothing.\n");
                        reset();
                        break;
                    default:
                        break;
                }
            }
            break;
        case 4:
            printf("\nNoctis chose to brace himself and deflect the %s's attack.", floorEnemy.name);
            attackChance = rand() % 10;
            if (attackChance < 4) {
                printf("\nNoctis parries the %s successfully!", floorEnemy.name);
                shieldDeflect = -1;
            } else {
                printf("\nNoctis' shield protected him, but did not deflect the attack!\n");
                shieldDeflect = 1;
            }
        default:
            break;
    }
    daemonAttack(shieldDeflect);
    addPlayerHealth();
}

void daemonAttack(int shieldDeflect) {
    if (shieldDeflect == -1) { // shield deflects
        int totalDamage;
        if (floorEnemy.def > 0) {
            totalDamage = floorEnemy.atk - floorEnemy.def;
        } else {
            totalDamage = floorEnemy.atk;
        }
        printf("\nNoctis dealt %d damage.", totalDamage);
        floorEnemy.health -= totalDamage;
        ringOfLucii(totalDamage);
        if (noctis.allyChoice == 1) ignisEffect(totalDamage);
    }

    int damageTaken =  floorEnemy.atk - noctis.def; // shield was not used
    if (shieldDeflect == 1) damageTaken = floorEnemy.atk - (noctis.def + 5); // shield protects
    if (damageTaken > 0)
    {
        printf("\nThe %s hit Noctis for %d damage!\n", floorEnemy.name, damageTaken);
        if (noctis.allyChoice == 2) damageTaken = gladioEffect(damageTaken);
        noctis.health -= damageTaken;
    } else {
        printf("\nThe party's defense is too strong. Noctis took no damage this turn!", floorEnemy.name);
    }
}

void addPlayerHealth() {
    int healChance = rand() % 4; 
    if (healChance == 0 && noctis.health < 300){
        printf("\n%s used a small first aid kit! Noctis recovered +10 health.\n", noctis.allyName);
        noctis.health += 10;

        switch (noctis.allyChoice) {
            case 1:
                red();
                printf("\n\tIgnis: We’re here to support you in times of need.");
                blue();
                printf("\n\tNoctis: What ever would I do without you?\n");
                reset();
                break;
            case 2:
                green();
                printf("\n\tGladio: Looks like I made it.");
                blue();
                printf("\n\tNoctis: Just in time.\n");
                reset();
                break;
            case 3:
                blue();
                printf("\n\tNoctis: Thank you, Prompto");
                yellow();
                printf("\n\tPrompto: Oh! Look who's learned some manners!");
                blue();
                printf("\n\tNoctis: Shut up.\n");
                reset();
                break;
            default:
                break;
        }   
    }
}

void ignisEffect (int noctisDamage) {
    int ignisDamage;
    int extraDamage = rand() % 10;

    if (extraDamage <= 1) {
        ignisDamage = noctisDamage / 2;
        printf("\nIgnis did a follow-up attack of %d damage!\n", ignisDamage);
        floorEnemy.health -= ignisDamage;
        red();
        printf("\n\tIgnis: Magnificent.");
        blue();
        printf("\n\tNoctis: It takes two.\n");
        reset();
    }   
}

int gladioEffect (int damageTaken) {
    int gladioBlock = damageTaken;
    int blockDamage = rand() % 4;

    if (blockDamage == 0) {
        gladioBlock /= 2;
        printf("\n... But Gladio blocked %d damage!\n", gladioBlock);
        green();
        printf("\n\tGladio: Holdin' up okay?");
        blue();
        printf("\n\tNoctis: Got the hot hand.");
        blue();
        green("\n\tGladio: Heh, ya think?\n");
        reset();
    }

    return gladioBlock;
}

void promptoEffect () {
    int promptoEffect = rand() % 10;

    if (promptoEffect == 0) {
        floorEnemy.def /= 2;
        printf("\nPrompto halved the enemy's defense!\n");
        yellow();
        printf("\n\tPrompto: We've got it on its last legs!");
        blue();
        printf("\n\tNoctis: Let's put it out of its misery.");
        yellow();
        printf("\n\tPrompto: Swift and painful!\n");
        reset();
    }
}

void ringOfLucii (int maxDamage) {
    int opDamage = rand() % 300;

    if (opDamage == 0) {
        opDamage = maxDamage * 99999;
        printf("\n\nNoctis summoned the force to use the Ring of Lucii! The enemy has been eradicated by the crystal...\n");
        floorEnemy.health -= opDamage;
        switch (noctis.allyChoice) {
            case 1:
                red();
                printf("\n\tIgnis: Is the storm ready to rage?.");
                blue();
                printf("\n\tNoctis: Just say when.\n");
                reset();
                break;
            case 2:
                green();
                printf("\n\tGladio: You mind explaining what that was?");
                blue();
                printf("\n\tNoctis: That was...more than I expected.\n");
                reset();
                break;
            case 3:
                yellow();
                printf("\n\tPrompto: Why don't we just do that every time?");
                blue();
                printf("\n\tNoctis: Easy for you to say!");
                yellow();
                printf("\n\tPrompto: Yes, yes it is.\n");
                reset();
                break;
            default:
                break;
        }   
    } 
}
       
// References:
// Base Code: Eric Kesa
// Color Text: https://www.theurbanpenguin.com/4184-2/
// Dialogues: https://spelldaggers.wordpress.com/
// Story and Design: https://finalfantasy.fandom.com/wiki/Category:Final_Fantasy_XV