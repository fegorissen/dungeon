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