#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

typedef enum { EMPTY, MONSTER, ITEM, TREASURE } ContentType;
typedef enum { GOBLIN, SKELETON, MAX_MONSTER_TYPES } MonsterType;
typedef enum { 
    HEALTH_POTION_SMALL, HEALTH_POTION_MEDIUM, HEALTH_POTION_LARGE, 
    POWER_GLOVE, MAGIC_AMULET, MAX_ITEM_TYPES 
} ItemType;

// Function pointers for monster actions
typedef void (*MonsterAction)(void);

typedef struct {
    MonsterType type;
    int hp, damage;
    const char* name;
    MonsterAction action; // Function pointer for special action
} Monster;

typedef struct {
    ItemType type;
    int value;
    const char* name;
} Item;

typedef struct {
    ContentType type;
    union {
        Monster* monster;
        Item* item;
    } content;
} RoomContent;

// Linked list node for rooms
typedef struct RoomNode {
    struct Room* room;
    struct RoomNode* next;
} RoomNode;

typedef struct Room {
    int id, num_doors, max_doors;
    struct Room** doors;
    RoomContent content;
    bool visited, cleared;
} Room;

typedef struct {
    int current_room_id; // Toegevoegd: houdt bij in welke kamer de speler is
    int hp, max_hp, damage;
    bool has_treasure;
} Player;

typedef struct {
    Room *entrance;
    RoomNode *rooms_list; // Linked list of rooms
    int num_rooms;
    Player player;
} Dungeon;

// Helper macros
#define RAND_RANGE(min, max) ((min) + rand() % ((max) - (min) + 1))
#define CLEAR_INPUT() while (getchar() != '\n')

// Function prototypes
Room* create_room(int id, int max_doors);
Monster* create_monster(MonsterType type);
Item* create_item();
void connect_rooms(Room* a, Room* b);
bool rooms_connected(Room* a, Room* b);
Dungeon* generate_dungeon(int num_rooms);
void populate_rooms(Dungeon* d);
void free_dungeon(Dungeon* d);
bool save_game(Dungeon* d, const char* filename);
Dungeon* load_game(const char* filename);
void game_loop(Dungeon* d);
Room* find_room_by_id(Dungeon* d, int id);
void add_room_to_list(Dungeon* d, Room* room);

// Monster actions
void goblin_special(void) {
    printf("De goblin gooit een steen naar je! (+5 extra schade deze ronde)\n");
}

void skeleton_special(void) {
    printf("Het skelet herrijst tijdelijk met 10 HP!\n");
}

// Game functions
void print_room(Room* r) {
    const char* contents[] = {"De kamer is leeg", "Het lijk van een %s ligt op de grond", 
                             "De lege schatkist staat hier", "De schat ligt hier!"};
    const char* items[] = {"Kleine Health Potion", "Medium Health Potion", "Grote Health Potion", 
                          "Power Glove", "Magisch Amulet"};
    
    if (!r->cleared) {
        if (r->content.type == MONSTER)
            printf("Er is een %s in de kamer\n", r->content.content.monster->name);
        else if (r->content.type == ITEM)
            printf("Er ligt een %s op de grond\n", r->content.content.item->name);
        else if (r->content.type == TREASURE)
            printf("%s\n", contents[3]);
    } else {
        printf("%s\n", contents[r->content.type]);
    }
}

void print_doors(Room* r) {
    printf("De kamer heeft deuren naar: ");
    for (int i = 0; i < r->num_doors; i++) 
        printf("%d%s", r->doors[i]->id, (i < r->num_doors-1) ? ", " : "\n");
}

bool fight(Dungeon* d, Monster* m) {
    printf("\n=== Gevecht met %s ===\nHP: %d/%d, Damage: %d\n", 
           m->name, d->player.hp, d->player.max_hp, d->player.damage);
    printf("%s HP: %d, Damage: %d\n\n", m->name, m->hp, m->damage);

    // 25% chance for special action
    if (rand() % 4 == 0 && m->action) {
        m->action();
        if (m->type == GOBLIN) {
            // Extra schade voor goblin special attack
            d->player.hp -= 5;
            printf("Je verliest 5 extra hp (%d/%d)\n", 
                   d->player.hp > 0 ? d->player.hp : 0, d->player.max_hp);
        } else if (m->type == SKELETON) {
            // Extra HP voor skeleton special
            m->hp += 10;
            printf("Skelet heeft nu %d HP\n", m->hp);
        }
    }

    while (d->player.hp > 0 && m->hp > 0) {
        int pattern = rand() % 16;
        printf("Aanval volgorde: ");
        for (int i = 3; i >= 0; i--) printf("%d", (pattern >> i) & 1);
        printf(" (0 = monster valt aan, 1 = speler valt aan)\n");

        for (int i = 3; i >= 0 && d->player.hp > 0 && m->hp > 0; i--) {
            if ((pattern >> i) & 1) {
                m->hp -= d->player.damage;
                printf("Jij valt de %s aan voor %d schade!\n", m->name, d->player.damage);
                printf("%s verliest %d hp (%d/%d)\n", m->name, d->player.damage, 
                       m->hp > 0 ? m->hp : 0, m->hp + d->player.damage);
                if (m->hp <= 0) return true;
            } else {
                d->player.hp -= m->damage;
                printf("%s valt jou aan voor %d schade!\n", m->name, m->damage);
                printf("Jij verliest %d hp (%d/%d)\n", m->damage, 
                       d->player.hp > 0 ? d->player.hp : 0, d->player.max_hp);
                if (d->player.hp <= 0) return false;
            }
        }

        if (d->player.hp > 0 && m->hp > 0) {
            printf("\n=== Status na ronde ===\nHP: %d/%d\n%s HP: %d\n\n", 
                   d->player.hp, d->player.max_hp, m->name, m->hp);
            printf("Druk op enter om door te gaan...");
            CLEAR_INPUT(); getchar();
        }
    }
    return true;
}

int get_input(const char* prompt, int min, int max) {
    int input;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &input) != 1) {
            CLEAR_INPUT();
            printf("Voer een nummer tussen %d en %d in.\n", min, max);
            continue;
        }
        CLEAR_INPUT();
        if (input >= min && input <= max) return input;
        printf("Ongeldige keuze. Kies tussen %d en %d.\n", min, max);
    }
}

// Main game functions
int main(int argc, char* argv[]) {
    srand(time(NULL));
    
    Dungeon* dungeon = NULL;
    
    if (argc > 1) {
        if (strcmp(argv[1], "-l") == 0 && argc > 2) {
            // Load game mode
            dungeon = load_game(argv[2]);
            if (!dungeon) {
                printf("Kon spel niet laden van %s\n", argv[2]);
                return 1;
            }
            printf("\nSpel geladen, start in kamer %d\n", dungeon->player.current_room_id);
        } else if (strcmp(argv[1], "-n") == 0 && argc > 2) {
            // New game mode
            int rooms = atoi(argv[2]);
            if (rooms < 3 || rooms > 20) {
                printf("Aantal kamers moet tussen 3 en 20 zijn\n");
                return 1;
            }
            dungeon = generate_dungeon(rooms);
            populate_rooms(dungeon);
            printf("\nNieuw spel gestart met %d kamers\nStart in kamer 0\n", rooms);
        } else {
            printf("Usage:\n%s -n <aantal kamers>  - Nieuw spel\n%s -l <bestandsnaam> - Laad spel\n", 
                   argv[0], argv[0]);
            return 1;
        }
    } else {
        // Interactive mode if no arguments
        printf("=== Dungeon Adventure ===\n1. Nieuw spel\n2. Laad spel\n");
        int choice = get_input("Keuze: ", 1, 2);
        
        if (choice == 1) {
            int rooms = get_input("Aantal kamers (3-20): ", 3, 20);
            dungeon = generate_dungeon(rooms);
            populate_rooms(dungeon);
            printf("\nStart in kamer 0\n");
        } else {
            dungeon = load_game("dungeon_save.dat");
            if (!dungeon) {
                printf("Nieuw spel starten...\n");
                int rooms = get_input("Aantal kamers (3-20): ", 3, 20);
                dungeon = generate_dungeon(rooms);
                populate_rooms(dungeon);
                printf("\nStart in kamer 0\n");
            }
        }
    }

    // Show starting room
    Room* current = find_room_by_id(dungeon, dungeon->player.current_room_id);
    print_room(current);
    print_doors(current);
    
    game_loop(dungeon);
    free_dungeon(dungeon);
    return 0;
}

void game_loop(Dungeon* d) {
    while (d->player.hp > 0 && !d->player.has_treasure) {
        Room* current = find_room_by_id(d, d->player.current_room_id);
        printf("\n1. Verplaatsen\n2. Ruim kamer op\n3. Status\n4. Schat\n5. Opslaan\n6. Stoppen\n");
        switch(get_input("Keuze: ", 1, 6)) {
            case 1: {
                print_doors(current);
                int target = get_input("Kies deur: ", 0, d->num_rooms-1);
                
                for (int i = 0; i < current->num_doors; i++) {
                    if (current->doors[i]->id == target) {
                        d->player.current_room_id = target;
                        printf("Naar kamer %d\n", target);
                        Room* new_room = find_room_by_id(d, target);
                        print_room(new_room);
                        if (new_room->content.type == MONSTER && !new_room->cleared && 
                            !fight(d, new_room->content.content.monster)) {
                            printf("Game Over!\n");
                            return;
                        }
                        break;
                    }
                }
                break;
            }
            case 2: {
                if (current->content.type == MONSTER && !current->cleared) {
                    if (!fight(d, current->content.content.monster)) {
                        printf("Game Over!\n");
                        return;
                    }
                    current->cleared = true;
                } else if (current->content.type == ITEM && !current->cleared) {
                    Item* it = current->content.content.item;
                    switch(it->type) {
                        case HEALTH_POTION_SMALL: case HEALTH_POTION_MEDIUM: case HEALTH_POTION_LARGE:
                            d->player.hp = (d->player.hp += it->value) > d->player.max_hp ? d->player.max_hp : d->player.hp;
                            break;
                        case POWER_GLOVE: d->player.damage += it->value; break;
                        case MAGIC_AMULET: 
                            d->player.max_hp += it->value;
                            d->player.hp += it->value;
                            break;
                    }
                    printf("Je gebruikt %s\n", it->name);
                    free(it);
                    current->content.type = EMPTY;
                    current->cleared = true;
                } else {
                    printf("Niets om op te ruimen\n");
                }
                break;
            }
            case 3: {
                printf("\n=== Status ===\nHP: %d/%d\nDamage: %d\nKamer: %d\n%s\n", 
                       d->player.hp, d->player.max_hp, d->player.damage, 
                       d->player.current_room_id, d->player.has_treasure ? "Heeft schat" : "");
                break;
            }
            case 4: {
                if (current->content.type == TREASURE && !current->cleared) {
                    printf("Je wint!\n");
                    d->player.has_treasure = current->cleared = true;
                } else {
                    printf("Geen schat hier\n");
                }
                break;
            }
            case 5: {
                if (save_game(d, "dungeon_save.dat")) 
                    printf("Opgeslagen!\n");
                else 
                    printf("Opslaan mislukt\n");
                break;
            }
            case 6: return;
        }
    }
    printf(d->player.has_treasure ? "\n*** Gewonnen! ***\n" : "\n*** Game Over ***\n");
}

// Dungeon generation and management
Room* create_room(int id, int max_doors) {
    Room* r = malloc(sizeof(Room));
    *r = (Room){id, 0, max_doors, malloc(max_doors * sizeof(Room*)), 
                {EMPTY}, false, false};
    return r;
}

Monster* create_monster(MonsterType type) {
    const char* names[] = {"Goblin", "Skeleton"};
    int hp[] = {RAND_RANGE(30, 50), RAND_RANGE(20, 35)};
    int dmg[] = {RAND_RANGE(5, 10), RAND_RANGE(8, 15)};
    MonsterAction actions[] = {goblin_special, skeleton_special};
    
    Monster* m = malloc(sizeof(Monster));
    *m = (Monster){type, hp[type], dmg[type], names[type], actions[type]};
    return m;
}

Item* create_item() {
    Item* it = malloc(sizeof(Item));
    it->type = rand() % MAX_ITEM_TYPES;
    const char* names[] = {"Kleine Health Potion", "Medium Health Potion", 
                          "Grote Health Potion", "Power Glove", "Magisch Amulet"};
    int values[] = {RAND_RANGE(5, 10), RAND_RANGE(10, 20), RAND_RANGE(20, 35), 
                   RAND_RANGE(3, 6), RAND_RANGE(1, 10)};
    *it = (Item){it->type, values[it->type], names[it->type]};
    return it;
}

void connect_rooms(Room* a, Room* b) {
    if (a->num_doors < a->max_doors && b->num_doors < b->max_doors) {
        a->doors[a->num_doors++] = b;
        b->doors[b->num_doors++] = a;
    }
}

bool rooms_connected(Room* a, Room* b) {
    for (int i = 0; i < a->num_doors; i++)
        if (a->doors[i] == b) return true;
    return false;
}

void add_room_to_list(Dungeon* d, Room* room) {
    RoomNode* new_node = malloc(sizeof(RoomNode));
    new_node->room = room;
    new_node->next = NULL;
    
    if (!d->rooms_list) {
        d->rooms_list = new_node;
    } else {
        RoomNode* current = d->rooms_list;
        while (current->next) current = current->next;
        current->next = new_node;
    }
}

Room* find_room_by_id(Dungeon* d, int id) {
    RoomNode* current = d->rooms_list;
    while (current) {
        if (current->room->id == id) return current->room;
        current = current->next;
    }
    return NULL;
}

Dungeon* generate_dungeon(int num_rooms) {
    Dungeon* d = malloc(sizeof(Dungeon));
    d->num_rooms = num_rooms;
    d->rooms_list = NULL;
    
    // Create all rooms and add to linked list
    for (int i = 0; i < num_rooms; i++) {
        Room* r = create_room(i, RAND_RANGE(1, 4));
        add_room_to_list(d, r);
    }
    
    d->entrance = find_room_by_id(d, 0);
    d->player = (Player){0, 100, 100, RAND_RANGE(10, 20), false}; // current_room_id initialized to 0
    
    // Connect rooms in a tree structure
    for (int i = 1; i < num_rooms; i++) {
        Room* new_room = find_room_by_id(d, i);
        Room* existing_room = find_room_by_id(d, rand() % i);
        connect_rooms(existing_room, new_room);
    }
    
    // Add extra random connections
    RoomNode* current = d->rooms_list;
    while (current) {
        Room* r = current->room;
        for (int j = r->num_doors; j < r->max_doors; j++) {
            int target, attempts = 0;
            do { 
                target = rand() % num_rooms; 
            } while ((target == r->id || rooms_connected(r, find_room_by_id(d, target))) && ++attempts < 100);
            
            if (attempts < 100) {
                connect_rooms(r, find_room_by_id(d, target));
            }
        }
        current = current->next;
    }
    return d;
}

void populate_rooms(Dungeon* d) {
    RoomNode* current = d->rooms_list;
    while (current) {
        current->room->content.type = EMPTY;
        current->room->cleared = false;
        current = current->next;
    }

    int treasure = 1 + rand() % (d->num_rooms - 1);
    find_room_by_id(d, treasure)->content.type = TREASURE;
    
    int monster;
    do { monster = 1 + rand() % (d->num_rooms - 1); } while (monster == treasure);
    Room* monster_room = find_room_by_id(d, monster);
    monster_room->content.type = MONSTER;
    monster_room->content.content.monster = create_monster(rand() % MAX_MONSTER_TYPES);

    current = d->rooms_list;
    while (current) {
        if (current->room->id == 0 || current->room->id == treasure || current->room->id == monster) {
            current = current->next;
            continue;
        }
        
        int r = rand() % 100;
        if (r < 40) {
            current->room->content.type = MONSTER;
            current->room->content.content.monster = create_monster(rand() % MAX_MONSTER_TYPES);
        } else if (r < 75) {
            current->room->content.type = ITEM;
            current->room->content.content.item = create_item();
        }
        current = current->next;
    }
}

bool save_game(Dungeon* d, const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) return false;

    fwrite(&d->num_rooms, sizeof(int), 1, f);
    fwrite(&d->player, sizeof(Player), 1, f);

    RoomNode* current = d->rooms_list;
    while (current) {
        Room* r = current->room;
        fwrite(&r->id, sizeof(int), 1, f);
        fwrite(&r->num_doors, sizeof(int), 1, f);
        fwrite(&r->max_doors, sizeof(int), 1, f);
        fwrite(&r->visited, sizeof(bool), 1, f);
        fwrite(&r->cleared, sizeof(bool), 1, f);
        fwrite(&r->content.type, sizeof(ContentType), 1, f);

        if (r->content.type == MONSTER) {
            Monster* m = r->content.content.monster;
            fwrite(&m->type, sizeof(MonsterType), 1, f);
            fwrite(&m->hp, sizeof(int), 1, f);
            fwrite(&m->damage, sizeof(int), 1, f);
        } else if (r->content.type == ITEM) {
            Item* it = r->content.content.item;
            fwrite(&it->type, sizeof(ItemType), 1, f);
            fwrite(&it->value, sizeof(int), 1, f);
        }
        current = current->next;
    }

    current = d->rooms_list;
    while (current) {
        Room* r = current->room;
        for (int j = 0; j < r->num_doors; j++) {
            fwrite(&r->doors[j]->id, sizeof(int), 1, f);
        }
        current = current->next;
    }

    fclose(f);
    return true;
}

Dungeon* load_game(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;

    int num_rooms;
    fread(&num_rooms, sizeof(int), 1, f);

    Dungeon* d = malloc(sizeof(Dungeon));
    d->num_rooms = num_rooms;
    d->rooms_list = NULL;
    fread(&d->player, sizeof(Player), 1, f);

    // First pass: create all rooms
    for (int i = 0; i < num_rooms; i++) {
        int id, num_doors, max_doors;
        bool visited, cleared;
        ContentType type;
        
        fread(&id, sizeof(int), 1, f);
        fread(&num_doors, sizeof(int), 1, f);
        fread(&max_doors, sizeof(int), 1, f);
        fread(&visited, sizeof(bool), 1, f);
        fread(&cleared, sizeof(bool), 1, f);
        fread(&type, sizeof(ContentType), 1, f);

        Room* r = create_room(id, max_doors);
        r->num_doors = num_doors;
        r->visited = visited;
        r->cleared = cleared;
        r->content.type = type;

        if (type == MONSTER) {
            Monster* m = malloc(sizeof(Monster));
            fread(&m->type, sizeof(MonsterType), 1, f);
            fread(&m->hp, sizeof(int), 1, f);
            fread(&m->damage, sizeof(int), 1, f);
            m->name = m->type == GOBLIN ? "Goblin" : "Skeleton";
            m->action = m->type == GOBLIN ? goblin_special : skeleton_special;
            r->content.content.monster = m;
        } else if (type == ITEM) {
            Item* it = malloc(sizeof(Item));
            fread(&it->type, sizeof(ItemType), 1, f);
            fread(&it->value, sizeof(int), 1, f);
            const char* names[] = {"Kleine Health Potion", "Medium Health Potion", 
                                  "Grote Health Potion", "Power Glove", "Magisch Amulet"};
            it->name = names[it->type];
            r->content.content.item = it;
        }

        add_room_to_list(d, r);
    }

    d->entrance = find_room_by_id(d, 0);

    // Second pass: connect doors
    RoomNode* current = d->rooms_list;
    while (current) {
        Room* r = current->room;
        for (int j = 0; j < r->num_doors; j++) {
            int id;
            fread(&id, sizeof(int), 1, f);
            r->doors[j] = find_room_by_id(d, id);
        }
        current = current->next;
    }

    fclose(f);
    return d;
}

void free_dungeon(Dungeon* d) {
    RoomNode* current = d->rooms_list;
    while (current) {
        RoomNode* next = current->next;
        if (current->room->content.type == MONSTER) 
            free(current->room->content.content.monster);
        else if (current->room->content.type == ITEM) 
            free(current->room->content.content.item);
        free(current->room->doors);
        free(current->room);
        free(current);
        current = next;
    }
    free(d);
}
