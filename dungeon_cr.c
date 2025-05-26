#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

// Enum voor verschillende inhoud types
typedef enum {
    EMPTY,
    MONSTER,
    ITEM,
    TREASURE
} ContentType;

// Enum voor monster types
typedef enum {
    GOBLIN,
    SKELETON,
    MAX_MONSTER_TYPES
} MonsterType;

// Enum voor item types
typedef enum {
    HEALTH_POTION_SMALL,
    HEALTH_POTION_MEDIUM,
    HEALTH_POTION_LARGE,
    POWER_GLOVE,
    MAGIC_AMULET,
    MAX_ITEM_TYPES
} ItemType;

// Struct voor een monster
typedef struct {
    MonsterType type;
    int hp;
    int damage;
    char* name;
} Monster;

// Struct voor een item
typedef struct {
    ItemType type;
    int value;
    char* name;
} Item;

// Struct voor kamerinhoud
typedef struct {
    ContentType type;
    union {
        Monster* monster;
        Item* item;
    } content;
} RoomContent;

// Struct voor een kamer
typedef struct Room {
    int id;
    struct Room** doors;
    int num_doors;
    int max_doors;
    RoomContent content;
    bool visited;
    bool cleared;
} Room;

// Struct voor de speler
typedef struct {
    int current_room_id;
    int hp;
    int max_hp;
    int damage;
    bool has_treasure;
} Player;

// Struct voor de dungeon
typedef struct {
    Room* entrance;
    Room** rooms;
    int num_rooms;
    Player player;
} Dungeon;

// Functieprototypes
Room* create_room(int id, int max_doors);
void connect_rooms(Room* room1, Room* room2);
bool is_already_connected(Room* room1, Room* room2);
Dungeon* generate_dungeon(int num_rooms);
void populate_rooms(Dungeon* dungeon);
void print_room_description(Room* room);
bool handle_monster_encounter(Dungeon* dungeon);
void handle_item_pickup(Dungeon* dungeon);
void print_player_status(Player* player);
void print_doors(Room* room);
void move_player(Dungeon* dungeon);
void free_dungeon(Dungeon* dungeon);
int get_user_input(const char* prompt, int min, int max);
Monster* create_monster(MonsterType type);
Item* create_item(ItemType type);

// Implementatie
Room* create_room(int id, int max_doors) {
    Room* room = (Room*)malloc(sizeof(Room));
    room->id = id;
    room->num_doors = 0;
    room->max_doors = max_doors;
    room->doors = (Room**)malloc(max_doors * sizeof(Room*));
    room->content.type = EMPTY;
    room->visited = false;
    room->cleared = false;
    return room;
}

Monster* create_monster(MonsterType type) {
    Monster* monster = (Monster*)malloc(sizeof(Monster));
    monster->type = type;
    
    switch(type) {
        case GOBLIN:
            monster->hp = 30 + rand() % 20;
            monster->damage = 5 + rand() % 5;
            monster->name = "Goblin";
            break;
        case SKELETON:
            monster->hp = 20 + rand() % 15;
            monster->damage = 8 + rand() % 7;
            monster->name = "Skeleton";
            break;
        default:
            monster->hp = 25 + rand() % 20;
            monster->damage = 6 + rand() % 6;
            monster->name = "Monster";
    }
    return monster;
}

Item* create_item() {
    Item* item = (Item*)malloc(sizeof(Item));
    item->type = rand() % MAX_ITEM_TYPES;
    
    switch(item->type) {
        case HEALTH_POTION_SMALL:
            item->value = 5 + rand() % 6;  // 5-10 HP
            item->name = "Kleine Health Potion";
            break;
        case HEALTH_POTION_MEDIUM:
            item->value = 10 + rand() % 11; // 10-20 HP
            item->name = "Medium Health Potion";
            break;
        case HEALTH_POTION_LARGE:
            item->value = 20 + rand() % 16; // 20-35 HP
            item->name = "Grote Health Potion";
            break;
        case POWER_GLOVE:
            item->value = 3 + rand() % 4;   // +3-6 damage
            item->name = "Power Glove";
            break;
        case MAGIC_AMULET:
            item->value = 1 + rand() % 10;  // +1-10 max HP
            item->name = "Magisch Amulet";
            break;
    }
    return item;
}

void connect_rooms(Room* room1, Room* room2) {
    if (room1->num_doors < room1->max_doors && 
        room2->num_doors < room2->max_doors) {
        room1->doors[room1->num_doors++] = room2;
        room2->doors[room2->num_doors++] = room1;
    }
}

bool is_already_connected(Room* room1, Room* room2) {
    for (int i = 0; i < room1->num_doors; i++) {
        if (room1->doors[i] == room2) {
            return true;
        }
    }
    return false;
}
Dungeon* generate_dungeon(int num_rooms) {
    if (num_rooms <= 0) return NULL;

    Dungeon* dungeon = (Dungeon*)malloc(sizeof(Dungeon));
    dungeon->num_rooms = num_rooms;
    dungeon->rooms = (Room**)malloc(num_rooms * sizeof(Room*));

    // Maak kamers met 1-4 deuren
    for (int i = 0; i < num_rooms; i++) {
        int max_doors = 1 + rand() % 4;
        dungeon->rooms[i] = create_room(i, max_doors);
    }

    dungeon->entrance = dungeon->rooms[0];
    
    // Initialiseer speler
    dungeon->player.current_room_id = 0;
    dungeon->player.hp = 100;
    dungeon->player.max_hp = 100;
    dungeon->player.damage = 10 + rand() % 10;
    dungeon->player.has_treasure = false;

    // Verbind kamers in een boomstructuur
    for (int i = 1; i < num_rooms; i++) {
        int parent_idx = rand() % i;
        connect_rooms(dungeon->rooms[parent_idx], dungeon->rooms[i]);
    }

    // Voeg extra verbindingen toe
    for (int i = 0; i < num_rooms; i++) {
        Room* current = dungeon->rooms[i];
        int remaining_doors = current->max_doors - current->num_doors;

        for (int j = 0; j < remaining_doors; j++) {
            int target_idx;
            int attempts = 0;
            do {
                target_idx = rand() % num_rooms;
                attempts++;
            } while ((target_idx == i || is_already_connected(current, dungeon->rooms[target_idx])) && attempts < 100);

            if (attempts < 100) {
                connect_rooms(current, dungeon->rooms[target_idx]);
            }
        }
    }

    return dungeon;
}

void populate_rooms(Dungeon* dungeon) {
    // Eerst alle kamers resetten (behalve startkamer)
    for (int i = 1; i < dungeon->num_rooms; i++) {
        dungeon->rooms[i]->content.type = EMPTY;
        dungeon->rooms[i]->cleared = false;
    }

    // Plaats de schat in een willekeurige kamer (niet startkamer)
    int treasure_room = 1 + rand() % (dungeon->num_rooms - 1);
    dungeon->rooms[treasure_room]->content.type = TREASURE;
    dungeon->rooms[treasure_room]->cleared = false;

    // Plaats minstens 1 monster in een andere kamer
    int monster_room;
    do {
        monster_room = 1 + rand() % (dungeon->num_rooms - 1);
    } while (monster_room == treasure_room);
    
    dungeon->rooms[monster_room]->content.type = MONSTER;
    dungeon->rooms[monster_room]->content.content.monster = create_monster(rand() % MAX_MONSTER_TYPES);
    dungeon->rooms[monster_room]->cleared = false;

    // Vul de rest van de kamers random in
    for (int i = 1; i < dungeon->num_rooms; i++) {
        if (i == treasure_room || i == monster_room) continue;

        int rand_val = rand() % 100;
        if (rand_val < 40) {  // 40% kans op monster
            dungeon->rooms[i]->content.type = MONSTER;
            dungeon->rooms[i]->content.content.monster = create_monster(rand() % MAX_MONSTER_TYPES);
        } 
        else if (rand_val < 75) {  // 35% kans op item
            dungeon->rooms[i]->content.type = ITEM;
            dungeon->rooms[i]->content.content.item = create_item();
        }
        // 25% kans blijft leeg
    }
}

void print_room_description(Room* room) {
    printf("\n=== Kamer %d ===\n", room->id);
    
    if (!room->visited) {
        printf("Je betreedt deze kamer voor het eerst.\n");
        room->visited = true;
    }

    switch(room->content.type) {
        case MONSTER:
            if (!room->cleared) {
                printf("Er staat een %s klaar om aan te vallen! (HP: %d, Damage: %d)\n", 
                       room->content.content.monster->name,
                       room->content.content.monster->hp,
                       room->content.content.monster->damage);
            } else {
                printf("Het dode lichaam van een %s ligt hier.\n",
                       room->content.content.monster->name);
            }
            break;
        case ITEM:
            if (!room->cleared) {
                printf("Je ziet een %s op de grond.\n",
                       room->content.content.item->name);
            } else {
                printf("Deze kamer is leeg.\n");
            }
            break;
        case TREASURE:
            if (!room->cleared) {
                printf("De schat ligt hier glinsterend in het midden van de kamer!\n");
            } else {
                printf("De schatkist staat hier open en leeg.\n");
            }
            break;
        default:
            printf("Deze kamer is leeg.\n");
    }
}

