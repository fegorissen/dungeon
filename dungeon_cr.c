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
    SKELETON
} MonsterType;

// Enum voor item types
typedef enum {
    HEALTH_POTION,
    POWER_GLOVE
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
