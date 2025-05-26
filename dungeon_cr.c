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
void print_room_contents(Room* room);
bool handle_monster_encounter(Dungeon* dungeon);
void handle_item_pickup(Dungeon* dungeon);
void print_player_status(Player* player);
void print_doors(Room* room);
void move_player(Dungeon* dungeon);
void free_dungeon(Dungeon* dungeon);
int get_user_input(const char* prompt, int min, int max);
Monster* create_monster(MonsterType type);
Item* create_item();
void print_move(int room_id);
void print_attack_sequence(int pattern);
void print_hp_loss(const char* name, int damage, int current_hp, int max_hp);

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
            item->value = 5 + rand() % 6;
            item->name = "Kleine Health Potion";
            break;
        case HEALTH_POTION_MEDIUM:
            item->value = 10 + rand() % 11;
            item->name = "Medium Health Potion";
            break;
        case HEALTH_POTION_LARGE:
            item->value = 20 + rand() % 16;
            item->name = "Grote Health Potion";
            break;
        case POWER_GLOVE:
            item->value = 3 + rand() % 4;
            item->name = "Power Glove";
            break;
        case MAGIC_AMULET:
            item->value = 1 + rand() % 10;
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

    for (int i = 0; i < num_rooms; i++) {
        int max_doors = 1 + rand() % 4;
        dungeon->rooms[i] = create_room(i, max_doors);
    }

    dungeon->entrance = dungeon->rooms[0];
    
    dungeon->player.current_room_id = 0;
    dungeon->player.hp = 100;
    dungeon->player.max_hp = 100;
    dungeon->player.damage = 10 + rand() % 10;
    dungeon->player.has_treasure = false;

    for (int i = 1; i < num_rooms; i++) {
        int parent_idx = rand() % i;
        connect_rooms(dungeon->rooms[parent_idx], dungeon->rooms[i]);
    }

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
    for (int i = 1; i < dungeon->num_rooms; i++) {
        dungeon->rooms[i]->content.type = EMPTY;
        dungeon->rooms[i]->cleared = false;
    }

    int treasure_room = 1 + rand() % (dungeon->num_rooms - 1);
    dungeon->rooms[treasure_room]->content.type = TREASURE;
    dungeon->rooms[treasure_room]->cleared = false;

    int monster_room;
    do {
        monster_room = 1 + rand() % (dungeon->num_rooms - 1);
    } while (monster_room == treasure_room);
    
    dungeon->rooms[monster_room]->content.type = MONSTER;
    dungeon->rooms[monster_room]->content.content.monster = create_monster(rand() % MAX_MONSTER_TYPES);
    dungeon->rooms[monster_room]->cleared = false;

    for (int i = 1; i < dungeon->num_rooms; i++) {
        if (i == treasure_room || i == monster_room) continue;

        int rand_val = rand() % 100;
        if (rand_val < 40) {
            dungeon->rooms[i]->content.type = MONSTER;
            dungeon->rooms[i]->content.content.monster = create_monster(rand() % MAX_MONSTER_TYPES);
        } 
        else if (rand_val < 75) {
            dungeon->rooms[i]->content.type = ITEM;
            dungeon->rooms[i]->content.content.item = create_item();
        }
    }
}

void print_move(int room_id) {
    printf("De held gaat naar kamer %d\n", room_id);
}

void print_room_contents(Room* room) {
    switch(room->content.type) {
        case MONSTER:
            if (!room->cleared) {
                printf("Er is een %s in de kamer\n", room->content.content.monster->name);
            } else {
                printf("Het lijk van een %s ligt op de grond\n", room->content.content.monster->name);
            }
            break;
        case ITEM:
            if (!room->cleared) {
                printf("Er ligt een %s op de grond\n", room->content.content.item->name);
            } else {
                printf("De kamer is leeg\n");
            }
            break;
        case TREASURE:
            if (!room->cleared) {
                printf("De schat ligt hier!\n");
            } else {
                printf("De lege schatkist staat hier\n");
            }
            break;
        default:
            printf("De kamer is leeg\n");
    }
}

void print_doors(Room* room) {
    printf("De kamer heeft deuren naar: ");
    for (int i = 0; i < room->num_doors; i++) {
        printf("%d", room->doors[i]->id);
        if (i < room->num_doors - 1) {
            printf(", ");
        }
    }
    printf("\n");
}

void print_attack_sequence(int pattern) {
    printf("Aanval volgorde: ");
    for (int i = 3; i >= 0; i--) {
        printf("%d", (pattern >> i) & 1);
    }
    printf(" (0 = monster valt aan, 1 = speler valt aan)\n");
}

void print_hp_loss(const char* name, int damage, int current_hp, int max_hp) {
    printf("%s verliest %d hp (%d/%d)\n", name, damage, current_hp, max_hp);
}

bool handle_monster_encounter(Dungeon* dungeon) {
    Room* current = dungeon->rooms[dungeon->player.current_room_id];
    Monster* monster = current->content.content.monster;

    printf("\n=== Gevecht met %s ===\n", monster->name);
    printf("Jouw HP: %d/%d, Damage: %d\n", dungeon->player.hp, dungeon->player.max_hp, dungeon->player.damage);
    printf("%s HP: %d, Damage: %d\n\n", monster->name, monster->hp, monster->damage);

    while (dungeon->player.hp > 0 && monster->hp > 0) {
        // Genereer random getal 0-15 en converteer naar 4-bit binaire vorm
        int attack_pattern = rand() % 16;
        print_attack_sequence(attack_pattern);

        // Loop door elke bit (van hoog naar laag)
        for (int i = 3; i >= 0; i--) {
            if (dungeon->player.hp <= 0 || monster->hp <= 0) break;

            if ((attack_pattern >> i) & 1) {
                // Bit is 1: speler valt aan
                monster->hp -= dungeon->player.damage;
                printf("Jij valt de %s aan voor %d schade!\n", 
                       monster->name, dungeon->player.damage);
                print_hp_loss(monster->name, dungeon->player.damage, 
                             monster->hp > 0 ? monster->hp : 0, 
                             monster->hp + dungeon->player.damage);
                
                if (monster->hp <= 0) {
                    printf("%s is verslagen!\n", monster->name);
                    current->cleared = true;
                    return true;
                }
            } else {
                // Bit is 0: monster valt aan
                dungeon->player.hp -= monster->damage;
                printf("%s valt jou aan voor %d schade!\n", 
                       monster->name, monster->damage);
                print_hp_loss("Jij", monster->damage, 
                             dungeon->player.hp > 0 ? dungeon->player.hp : 0, 
                             dungeon->player.max_hp);
                
                if (dungeon->player.hp <= 0) {
                    printf("Je bent verslagen... Game Over!\n");
                    return false;
                }
            }
        }

        if (dungeon->player.hp > 0 && monster->hp > 0) {
            printf("\n=== Status na ronde ===\n");
            printf("Jouw HP: %d/%d\n", dungeon->player.hp, dungeon->player.max_hp);
            printf("%s HP: %d\n\n", monster->name, monster->hp);
            printf("Druk op enter om door te gaan...");
            getchar(); getchar();
        }
    }
    return true;
}

void handle_item_pickup(Dungeon* dungeon) {
    Room* current = dungeon->rooms[dungeon->player.current_room_id];
    
    if (current->content.type != ITEM || current->cleared) {
        printf("Er is hier niets om op te rapen!\n");
        return;
    }

    Item* item = current->content.content.item;
    
    switch(item->type) {
        case HEALTH_POTION_SMALL:
        case HEALTH_POTION_MEDIUM:
        case HEALTH_POTION_LARGE:
            dungeon->player.hp += item->value;
            if (dungeon->player.hp > dungeon->player.max_hp) {
                dungeon->player.hp = dungeon->player.max_hp;
            }
            printf("Je gebruikt de %s en herstelt %d HP. Jouw HP: %d/%d\n",
                   item->name, item->value, dungeon->player.hp, dungeon->player.max_hp);
            break;
            
        case POWER_GLOVE:
            dungeon->player.damage += item->value;
            printf("Je trekt de %s aan en krijgt +%d damage. Jouw damage: %d\n",
                   item->name, item->value, dungeon->player.damage);
            break;
            
        case MAGIC_AMULET:
            dungeon->player.max_hp += item->value;
            dungeon->player.hp += item->value;
            printf("Je draagt nu het %s en krijgt +%d max HP. Jouw HP: %d/%d\n",
                   item->name, item->value, dungeon->player.hp, dungeon->player.max_hp);
            break;
    }
    
    current->cleared = true;
    free(item);
    current->content.type = EMPTY;
}

void handle_treasure(Dungeon* dungeon) {
    Room* current = dungeon->rooms[dungeon->player.current_room_id];
    
    if (current->content.type == TREASURE && !current->cleared) {
        printf("Je hebt de schat gevonden! Gefeliciteerd, je wint!\n");
        dungeon->player.has_treasure = true;
        current->cleared = true;
    } else {
        printf("Er is hier geen schat.\n");
    }
}

void print_player_status(Player* player) {
    printf("\n=== Speler Status ===\n");
    printf("HP: %d/%d\n", player->hp, player->max_hp);
    printf("Damage: %d\n", player->damage);
    printf("Huidige kamer: %d\n", player->current_room_id);
    if (player->has_treasure) {
        printf("Je draagt de schat!\n");
    }
}

void move_player(Dungeon* dungeon) {
    Room* current = dungeon->rooms[dungeon->player.current_room_id];
    print_doors(current);
    
    printf("Kies een deur: ");
    int target = get_user_input("", 0, dungeon->num_rooms - 1);
    
    for (int i = 0; i < current->num_doors; i++) {
        if (current->doors[i]->id == target) {
            dungeon->player.current_room_id = target;
            print_move(target);
            
            Room* new_room = dungeon->rooms[target];
            print_room_contents(new_room);
            
            if (new_room->content.type == MONSTER && !new_room->cleared) {
                if (!handle_monster_encounter(dungeon)) {
                    return;
                }
            }
            return;
        }
    }
    printf("Er is geen deur naar die kamer!\n");
}

int get_user_input(const char* prompt, int min, int max) {
    int input;
    char ch;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &input) != 1) {
            while ((ch = getchar()) != '\n' && ch != EOF);
            printf("Voer een nummer in tussen %d en %d.\n", min, max);
            continue;
        }
        while ((ch = getchar()) != '\n' && ch != EOF);
        
        if (input >= min && input <= max) {
            return input;
        }
        printf("Ongeldige keuze. Kies tussen %d en %d.\n", min, max);
    }
}

void free_dungeon(Dungeon* dungeon) {
    for (int i = 0; i < dungeon->num_rooms; i++) {
        switch(dungeon->rooms[i]->content.type) {
            case MONSTER:
                free(dungeon->rooms[i]->content.content.monster);
                break;
            case ITEM:
                free(dungeon->rooms[i]->content.content.item);
                break;
            default:
                break;
        }
        free(dungeon->rooms[i]->doors);
        free(dungeon->rooms[i]);
    }
    free(dungeon->rooms);
    free(dungeon);
}

int main() {
    srand(time(NULL));
    
    printf("=== Dungeon Adventure ===\n");
    int num_rooms = get_user_input("Hoeveel kamers moeten er in de dungeon zijn? (min 3): ", 3, 20);
    
    Dungeon* dungeon = generate_dungeon(num_rooms);
    populate_rooms(dungeon);
    
    printf("\nDe held start in kamer 0\n");
    print_room_contents(dungeon->entrance);
    print_doors(dungeon->entrance);
    
    bool playing = true;
    while (playing && dungeon->player.hp > 0 && !dungeon->player.has_treasure) {
        printf("\n1. Verplaatsen\n");
        printf("2. Ruim kamer op\n");
        printf("3. Status bekijken\n");
        printf("4. Schat pakken\n");
        printf("5. Stoppen\n");
        
        int choice = get_user_input("Wat wil je doen? ", 1, 5);
        
        switch(choice) {
            case 1:
                move_player(dungeon);
                break;
            case 2:
                if (dungeon->rooms[dungeon->player.current_room_id]->content.type == MONSTER) {
                    if (!handle_monster_encounter(dungeon)) {
                        playing = false;
                    }
                } else if (dungeon->rooms[dungeon->player.current_room_id]->content.type == ITEM) {
                    handle_item_pickup(dungeon);
                } else {
                    printf("Er is hier niets om op te ruimen.\n");
                }
                break;
            case 3:
                print_player_status(&dungeon->player);
                break;
            case 4:
                handle_treasure(dungeon);
                break;
            case 5:
                playing = false;
                break;
        }
    }
    
    if (dungeon->player.has_treasure) {
        printf("\n*** Gefeliciteerd! Je hebt de schat gevonden en gewonnen! ***\n");
    } else if (dungeon->player.hp <= 0) {
        printf("\n*** Helaas, je hebt het niet overleefd... ***\n");
    }
    
    free_dungeon(dungeon);
    printf("Bedankt voor het spelen!\n");
    return 0;
}
