/* This is the entirety of the game at the moment.
TODO
Add an element shift system
Add an equipment system
Add an initiative system
Add an item system
Add a leveling system
Add a status system
Add some kind of inut buffer/check
Break the functions out into their own header files
Make spell list
Make Monster list
Make equipment list
###EXIT CODES###
1 - missing file
*/

//Libraries were added as needed.
#include <math.h> //Needs -lm @ compile
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//Used to set name length limit.  Program is scaled to this value.
#define NAMEMAX 21

//String control made easy
typedef struct string {
    char *str;
    int len;
} string;

//This struct holds all the information for the player and whatever enemy they are fighting, for simplicity data is given in both save files and loaded as needed.
typedef struct character {
    string name;
    int level;
    int xp;
    int strength;
    int dexterity;
    int constitution;
    int numen;
    int intelligence;
    int resilience;
    int curHP;
    int maxHP;
    int defending;
    float elementX;
    float elementY;
    string element;
    float imbueX;
    float imbueY;
    string imbue;
    float pin1X;
    float pin1Y;
    float pin2X;
    float pin2Y;
} character;

//This struct holds the information concerning a spell that was loaded
typedef struct spell{
    string name;
    float x;
    float y;
    char type;
    struct spell *next;
} spell;

//These should be self documenting names.
float calculateElementalDistance (float x1, float y1, float x2, float y2);
void capitalizeName (char *name);
void castSpell (character *caster, character *target, int automate);
void changeImbuement (character *target, spell *spell, int automate);
int characterLoadCheck (character *player);
void characterPage (character *player);
void createCharacter (character **newChar);
void createSpell (spell **newSpell);
void damageCalculator (character *attacker, character *defender, char attackType);
void determineElement (float x, float y, string *target);
void fight (character *player);
void freeCharacter (character *oldChar);
void freeSpell (spell *oldSpell);
int generateCharacter (character *player);
void healingCalculator (character *caster, character *target);
void initializeString (string *newString);
void levelCharacter (character *player);
int loadCharacter (character *target, FILE *load);
void openEnemy (character *target);
int openPlayer (character *target, char *filename);
void readValueToString (string *string, char in);
void saveCharacter (character *player);
void saveCheck (character *player);
void selectSpell (character *target, int automate, spell *targetSpell);
void statusCalculator (character *caster, character *target, spell *spell);

//At load the character will not be loaded, it's simpler to use pointers and malloc when needed.  choice is the user's decision on the main game screen.  name is used in selecting a character to load.
int main () {
    character *player = NULL;
    char name[NAMEMAX];
    int choice;
//Seed the randomizer
    srand (time(NULL));
    printf ("Welcome to Sethloft");
// This do...while managaes the execution of the game.
    do {
        printf ("\n============================================================\n");
//Display the name of the loaded character, if any.
        if (player != NULL) { 
            printf ("Character: %s\n\n", player->name.str);
        }
//A simple request to the user.
        printf ("1. Fight!\n2. View Character Page\n3. Level up a Character\n4. Create New Character\n5. Save Current Character\n6. Load Saved Character\n0. Quit\nWhat would you like to do?\n");
        scanf ("%d%*c", &choice);
//This switch/case handles the execution of the user's selection.
        switch (choice) {
//Fight a monster.
            case 1:
//Only if a character is loaded.
                if (characterLoadCheck (player) == 1) {
                    fight (player);
                }
                break;
//Show the Character's stats.
            case 2:
//Only if a character is loaded.
                if (characterLoadCheck (player) == 1) {
                    characterPage (player);
                }
                break;
//Try to level up a character
            case 3:
                if (characterLoadCheck (player) == 1) {
                    levelCharacter (player);
                }
                break;
//Generate a new character.
            case 4:
//Save any that are loaded then free the memory.
                if (player != NULL) {
                    saveCheck (player);
                    freeCharacter (player);
                    player = NULL;
                }
                createCharacter (&player);
//Check that the creation executed properly.
                if (generateCharacter (player) == 0) {
                    printf ("Character creation failed, please try again.\n");
                } else {
                    printf ("%s successfully created.\n", player->name.str);
                }
                break;
//Save the current character.
            case 5:
//If there is one.
                if (characterLoadCheck (player) == 1) {
                    saveCharacter (player);
                }
                break;
//Load a saved character.
            case 6:
//Save if there is one loaded and free memory.
                if (player != NULL) {
                    saveCheck (player);
                    freeCharacter (player);
                    player = NULL;
                }
//A simple request to the user to figure out which character to load.
                printf ("Which character would you like to load?\n");
                scanf ("%[^\n]%*c", name);
                capitalizeName (name);
                createCharacter (&player);
//Check that the load was successfull.
                if (openPlayer (player, name) == 1) {
                    printf ("Character Load failed, please try again.\n");
                    freeCharacter (player);
                    player = NULL;
                } else {
                    printf ("%s loaded successfully.\n", player->name.str);
                }
                break;
//The quit command.
            case 0:
                if (player != NULL) {
                    saveCheck (player);
                    freeCharacter (player);
                    player = NULL;
                }
                break;
//Any other entered value.
            default:
                printf ("That is not a valid option, please select from the list.\n");
                break;
            }
        } while (choice != 0);
//Goodbye message
    printf ("Thanks for playing!\n");
    return 0;
}

float calculateElementalDistance (float x1, float y1, float x2, float y2) {
    float deltaX = 0, deltaY = 0;
    deltaX = x1 - x2;
    if (deltaX > 0) {
        deltaX *= -1;
    }
    deltaY = y1 - y2;
    if (deltaY > 0) {
        deltaY *= -1;
    }
    return sqrt ((deltaX * deltaX) + (deltaY * deltaY));
}

//Format names properly
void capitalizeName (char *name) {
//Local Variables
    int i;
//Capitalize the first letter and go letter by letter afterwards.
    name[0] = toupper (name[0]);
    for (i = 1; i < strlen (name); i++) {
//This if checks if the character at i is the first in a new word.
        if (name[(i - 1)] == ' ') {
            name[i] = toupper (name[i]);
        } else {
            name[i] = tolower (name[i]);
        }
    }
    return;
}

//Control spell casting
void castSpell (character *caster, character *target, int automate) {
//Local variables, tempX and tempY are used to store caster imbuements, and spell is the selected spell
    float tempX, tempY;
    spell *spell = NULL;
    createSpell (&spell);
//Initialize values
    tempX = caster->imbueX;
    tempY = caster->imbueY;
    selectSpell (caster, automate, spell);
    printf ("%s casts %s\n", caster->name.str, spell->name.str);
//Damaging spell
    if (spell->type == 'D') {
        damageCalculator (caster, target, 'M');
//Healing spell
    } else if (spell->type == 'H') {
        healingCalculator (caster, caster);
//Status spell
    } else if (spell->type == 'S') {
        statusCalculator (caster, target, spell);
    }
    caster->imbueX = tempX;
    caster->imbueY = tempY;
    freeSpell (spell);
    return;
}

//Changes the imbue values of a character
void changeImbuement (character *target, spell *spell, int automate) {
    selectSpell (target, automate, spell);
    determineElement (target->elementX, target->elementY, &target->element);
    determineElement (target->imbueX, target->imbueY, &target->imbue);
    return;
}

//Check if a character is loaded
int characterLoadCheck (character *player) {
    if (player == NULL) {
        printf ("No character loaded!  Load or Create a character first.\n");
        return 0;
    } else {
        return 1;
    }
}

//Show stats of currently loaded character
void characterPage (character *player) {
    char choice = 0;
//list stats and ask ifthey want to reimbue
    printf ("\nName:\t\t%s\nLevel:\t\t%d\nXP:\t\t%d\n---Physical---\nStrength:\t%d\nDexterity:\t%d\nConstitution:\t%d\n---Magical---\nNumen:\t\t%d\nIntelligence:\t%d\nResilience:\t%d\nHP:\t\t%d/%d\nAttunement:\t%s\nImbuement:\t%s\nDo you want to change your current imbuement (Y/N)?\n", player->name.str, player->level, player->xp, player->strength, player->dexterity, player->constitution, player->numen, player->intelligence, player->resilience, player->curHP, player->maxHP, player->element.str, player->imbue.str);
//Put it in a do...while to make sure it's valid
    do {
        scanf ("%c%*c", &choice);
        if ((choice != 'Y') && (choice != 'y') && (choice != 'N') && (choice != 'n')) {
            printf ("That is not a valid option, please select from the list.\n");
        }
    } while ((choice != 'Y') && (choice != 'y') && (choice != 'N') && (choice != 'n'));
//If so choose the spell to imbue with
    if ((choice == 'Y') || (choice == 'y')) {
        spell *spell = NULL;
        createSpell (&spell);
        printf ("What spell would you like to imbue to your weapon?\n");
        changeImbuement (player, spell, 0);
        while (spell->type != 'D') {
            printf ("Only attack spells may be chosen for imbueing.\n");
            changeImbuement (player, spell, 0);
        }
        freeSpell (spell);
    }
    return;
}

//create a new character object
void createCharacter (character **newChar) {
    *newChar = malloc (sizeof (**newChar));
    initializeString (&(*newChar)->name);
    initializeString (&(*newChar)->element);
    initializeString (&(*newChar)->imbue);
    return;
}

//Create a new spell object
void createSpell (spell **newSpell) {
    *newSpell = malloc (sizeof (**newSpell));
    initializeString (&(*newSpell)->name);
    (*newSpell)->next = NULL;
    (*newSpell)->type = 0;
    return;
}

//Determine damage beterrn attacker and defender returns damage done, if any
void damageCalculator (character *attacker, character *defender, char attackType) {
//The basic calculation of a damage value.
    float attack;
    int damage, defence;
//Load the right attack value
    if (attackType == 'P') {
        attack = ((10 * attacker->strength) * (1) * (1 + calculateElementalDistance (attacker->imbueX, attacker->imbueY, defender->elementX, defender->elementY)));
        defence = ((7 * defender->constitution) * (1));
    } else {
        attack = ((10 * attacker->numen) * (1) * (1 + calculateElementalDistance (attacker->imbueX, attacker->imbueY, defender->elementX, defender->elementY)));
        defence = ((7 * defender->resilience) * (1));
    }
    if (defender->defending == 1) {
        defence *= 2;
    }
    damage = attack - defence;
//In case of too high defence compared to attack.
    if (damage < 1) {
        printf ("%s blocks the attack.\n", defender->name.str);
    } else {
        printf ("%s does %d damage to %s.\n", attacker->name.str, damage, defender->name.str);
        defender->curHP -= damage;
    }
    return;
}

//Convert elemental coordinates to a string
void determineElement (float x, float y, string *target) {
//Local variables, element is the string returned
//Left side
    if (x < 0) {
//Light: Lower-left
        if (y < 0) {
            target->len = 6;
            target->str = realloc (target->str, target->len);
            strcpy (target->str, "Light");
//Electricity: Upper-left
        } else if (y > 0) {
            target->len = 12;
            target->str = realloc (target->str, target->len);
            strcpy (target->str, "Electricity");
//Fire: True-left
        } else {
            target->len = 5;
            target->str = realloc (target->str, target->len);
            strcpy (target->str, "Fire");
        }
//Right side
    } else if (x > 0) {
//Ice: Lower-right
        if (y < 0) {
            target->len = 4;
            target->str = realloc (target->str, target->len);
            strcpy (target->str, "Ice");
//Dark: Upper-right
        } else if (y > 0) {
            target->len = 5;
            target->str = realloc (target->str, target->len);
            strcpy (target->str, "Dark");
//Water: True-Right
        } else {
            target->len = 6;
            target->str = realloc (target->str, target->len);
            strcpy (target->str, "Water");
        }
//Center
    } else {
//Earth: Lower-center
        if (y < 0) {
            target->len = 6;
            target->str = realloc (target->str, target->len);
            strcpy (target->str, "Earth");
//Air: Upper-center
        } else if (y > 0) {
            target->len = 4;
            target->str = realloc (target->str, target->len);
            strcpy (target->str, "Air");
//Neutral: True Center
        } else {
            target->len = 8;
            target->str = realloc (target->str, target->len);
            strcpy (target->str, "Neutral");
        }
    }
    return;
}

//Combat 'sequence'
void fight (character *player) {
//Local variables.  playerAction and enemyAction are what the respective character does from their possible list, tempX and tempY are storage values for a spell casting, spell is a selected spell.
    character *enemy = NULL;
    int enemyAction, playerAction;
    char enemyStatus[12], attackType, typeChoice;
    float tempX, tempY;
    spell *spell = NULL;
//Initialize the temp values and populate it from the monster list.
    createSpell (&spell);
    createCharacter (&enemy);
    openEnemy (enemy);
//This keeps the combat up only while those fighting have HP.
    while ((player->curHP > 0) && (enemy->curHP > 0)) {
//Calculate the enemy's health and give is a vague descriptive status for the player to guess at the enemy's HP.
        if (((enemy->curHP * 100) / enemy->maxHP) > 49) {
            strcpy (enemyStatus, "healthy");
        } else if (((enemy->curHP * 100) / enemy->maxHP) < 25) {
            strcpy (enemyStatus, "almost dead");
        } else {
            strcpy (enemyStatus, "injured");
        }
//Reset the defending values;
        player->defending = 0;
        enemy->defending = 0;
//A simple request to the user for the player's turn.
        do {
            printf ("\n\n\n%s looks %s\n\n\n\n%s: %d/%d HP\nWhat do you want to do?\n1. Attack\n2. Spell\n3. Defend\n4. Change Imbument\n5. Heal\n6. Flee!\n", enemy->name.str, enemyStatus, player->name.str, player->curHP, player->maxHP);
            scanf ("%d%*c", &playerAction);
            if ((playerAction < 1) || (playerAction > '6')) {
                printf ("That is not a valid option, please select from the list.\n");
            }
        } while ((playerAction < 1) || (playerAction > '6'));
//For now it's a random selection between available actions.
        enemyAction = rand () % 4;
//If the enemy defends
        if (enemyAction == 3) {
            enemy->defending = 1;
        }
//This switch/case handles the player's possible actions.
        switch (playerAction) {
//A physical attack.
            case 1:
                damageCalculator (player, enemy, 'P');
                break;
//Cast a spell
            case 2:
/*GIVING WEIRDNESS*/
                castSpell (player, enemy, 0);
                break;
//Defending doubles the defence score for one round.
            case 3:
                player->defending = 1;
                break;
//Change the elemental imbuement on the weapon
            case 4:
                printf ("What spell would you like to imbue to your weapon?\n");
                changeImbuement (player, spell, 0);
                break;
//Recover 50hp
            case 5:
                printf ("You drink a Health Potion!\n");
                player->curHP += 50;
//Make sure the health never exceeds the max
                if (player->curHP > player->maxHP) {
                    player->curHP = player->maxHP;
                }
                break;
//Try to run away
            case 6:
                printf ("You run for your life...\n");
                int escapeChance, escape;
//Odds of escaping are <player level>/<enemy level>, if player outranks enemy it's automatic
                if (player->level > enemy->level) {
                    escapeChance = 100;
                } else {
                    escapeChance = (player->level * 100) / enemy->level;
                }
                escape = rand () % 100;
                if (escape <= escapeChance) {
                    printf ("and escape %s!\n", enemy->name.str);
                    freeCharacter (enemy);
                    enemy = NULL;
                    return;
                }
                printf ("but fail to evade %s.\n", enemy->name.str);
                break;
        }
        if (enemy->curHP < 1) {
            break;
        }
//A physical attack by the enemy
        if (enemyAction == 0) {
            damageCalculator (enemy, player, 'P');
//A spell cast by the enemy
        } else if (enemyAction == 1) {
            castSpell (enemy, player, 1);
//Change enemy imbuement
        } else if (enemyAction == 2) {
            changeImbuement (player, spell, 1);
            printf ("%s changes its imbuement!\n", enemy->name.str);
        }
    }
//If the player dies.
    if (player->curHP < 1) {
        printf ("You are dead!\n");
        freeCharacter (player);
        player = NULL;
//If the enemy died.
    } else {
        printf ("%s is dead!\nYou gain %d XP.\n", enemy->name.str, enemy->xp);
        player->xp += enemy->xp;
    }
    freeCharacter (enemy);
    freeSpell (spell);
    return;
}

//Frees a character object
void freeCharacter (character *oldChar) {
    free (oldChar->name.str);
    free (oldChar->element.str);
    free (oldChar->imbue.str);
    free (oldChar);
    return;
}

//Frees a spell object
void freeSpell (spell *oldSpell) {
    free (oldSpell->name.str);
    free (oldSpell);
    return;
}

//Generate base stats for a new character
int generateCharacter (character *player) {
//class will be used as a basis for stat augmentation (probably end up being added to the struct later), in is used in the custom allocation and HP calc, total and valid are only used in custom allocation.
    int class;
//Start with the name
    player->name.str = realloc (player->name.str, NAMEMAX);
    printf ("What would you like your character's name to be?\n");
    scanf ("%[^\n]%*c", player->name.str);
//CapitalizeName sets the name to a singular format, regardless of how it is entered.
    capitalizeName (player->name.str);
    player->name.len = strlen (player->name.str);
//A simple request to the user.
    printf ("What class would you like to be:\n1. Brigand (Physical attacker)\n2. Warrior (Physical balanced)\n3. Knight (Physical defender)\n4. Paladin (Neutral attacker)\n5. Monk (Neutral balanced)\n6. Cleric (Neutral defender)\n7. Mage (Magical attacker)\n8. Sorcerer (Magical balanced)\n9. Wizard (Magical defender)\n10. Prodigy (Custom)\nWhat is your choice? ");
    scanf ("%d%*c", &class);
//Simple error check.
    while ((1 > class) || (class > 10)) {
        printf ("That is not a valid class option, please choose again.\n");
        scanf ("%d%*c", &class);
    }
//This switch/case handles alterations based on class.
    switch (class) {
//Brigand: Physical attacker.
        case 1:
            player->strength = 5;
            player->dexterity = 4;
            player->constitution = 3;
            player->numen = 3;
            player->intelligence = 2;
            player->resilience = 1;
            break;
//Warrior: Physical balanced.
        case 2:
            player->strength = 4;
            player->dexterity = 4;
            player->constitution = 4;
            player->numen = 2;
            player->intelligence = 2;
            player->resilience = 2;
            break;
//Knight: Physical defender.
        case 3:
            player->strength = 3;
            player->dexterity = 4;
            player->constitution = 5;
            player->numen = 1;
            player->intelligence = 2;
            player->resilience = 3;
            break;
//Paladin: Neutral attacker.
        case 4:
            player->strength = 4;
            player->dexterity = 3;
            player->constitution = 2;
            player->numen = 4;
            player->intelligence = 3;
            player->resilience = 2;
            break;
//Monk: Neutral balanced.
        case 5:
            player->strength = 3;
            player->dexterity = 3;
            player->constitution = 3;
            player->numen = 3;
            player->intelligence = 3;
            player->resilience = 3;
            break;
//Cleric: Neutral defender.
        case 6:
            player->strength = 2;
            player->dexterity = 3;
            player->constitution = 4;
            player->numen = 2;
            player->intelligence = 3;
            player->resilience = 4;
            break;
//Mage: Magical attacker.
        case 7:
            player->strength = 3;
            player->dexterity = 2;
            player->constitution = 1;
            player->numen = 5;
            player->intelligence = 4;
            player->resilience = 3;
            break;
//Sorcerer: Magical balanced.
        case 8:
            player->strength = 3;
            player->dexterity = 3;
            player->constitution = 3;
            player->numen = 4;
            player->intelligence = 4;
            player->resilience = 4;
            break;
//Wizard: Magical defender.
        case 9:
            player->strength = 1;
            player->dexterity = 2;
            player->constitution = 3;
            player->numen = 3;
            player->intelligence = 4;
            player->resilience = 5;
            break;
//Prodigy: Custom stat distribution.
        case 10:
/*ABSTRACT THIS OUT TO A REPEATED CALL*/
            player->strength = 1;
            player->dexterity = 1;
            player->constitution = 1;
            player->numen = 1;
            player->intelligence = 1;
            player->resilience = 1;
            int in, total = 18, valid = 0;
//Make sure it's an allowable value
            while (valid == 0) {
                printf ("You have %d points to spend, what would you like your Strength (Physical, Attack) to be?  It must be within 1 and %d\n", total, total - 5);
                scanf ("%d%*c", &in);
//Check the range
                if ((in < 1) || (in > (total - 5))) {
                    printf ("Outside acceptable range\n");
                } else {
                    valid = 1;
                }
            }
//Adjsut as needed
            total -= in;
            player->strength = in;
//Reset and make sure it's an allowable value
            valid = 0;
            while (valid == 0) {
                printf ("You have %d points to spend, what would you like your Dexterity (Physical, Agility) to be?  It must be within 1 and %d\n", total, total - 5);
                scanf ("%d%*c", &in);
//Check the range
                if ((in < 1) || (in > (total - 5))) {
                    printf ("Outside acceptable range\n");
                } else {
                    valid = 1;
                }
            }
//Adjsut as needed
            total -= in;
            player->dexterity = in;
//Reset and make sure it's an allowable value
            valid = 0;
            while (valid == 0) {
                printf ("You have %d points to spend, what would you like your Constitution (Physical, Defence) to be?  It must be within 1 and %d\n", total, total - 5);
                scanf ("%d%*c", &in);
//Check the range
                if ((in < 1) || (in > (total - 5))) {
                    printf ("Outside acceptable range\n");
                } else {
                    valid = 1;
                }
            }
//Adjsut as needed
            total -= in;
            player->constitution = in;
//Reset and make sure it's an allowable value
            valid = 0;
            while (valid == 0) {
                printf ("You have %d points to spend, what would you like your Numen (Magical, Attack) to be?  It must be within 1 and %d\n", total, total - 5);
                scanf ("%d%*c", &in);
//Check the range
                if ((in < 1) || (in > (total - 5))) {
                    printf ("Outside acceptable range\n");
                } else {
                    valid = 1;
                }
            }
//Adjsut as needed
            total -= in;
            player->numen = in;
//Reset and make sure it's an allowable value
            valid = 0;
            while (valid == 0) {
                printf ("You have %d points to spend, what would you like your Intelligence (Magical, Agility) to be?  It must be within 1 and %d\n", total, total - 5);
                scanf ("%d%*c", &in);
//Check the range
                if ((in < 1) || (in > (total - 5))) {
                    printf ("Outside acceptable range\n");
                } else {
                    valid = 1;
                }
            }
//Adjsut as needed
            total -= in;
            player->intelligence = in;
//All that's left has only one place to go
            printf ("Your remaining %d points have been allocated to your Resilience (Magical, Defence).\n", total);
            player->resilience = total;
//if this gets triggered somehow a mistake was made, freeing the data seems best.
        default:
            freeCharacter (player);
            player = NULL;
            return 0;
    }
//The static stats
    player->level = 1;
    player->xp = 0;
//HP has a random element associated to it.  Minimum 90 points per soak, + up to 20 bonus per level
    player->maxHP = (((player->constitution + player->resilience) * 90) + (rand () % 21));
    player->curHP = player->maxHP;
    player->elementX = 0;
    player->elementY = 0;
    player->imbueX = 0;
    player->imbueY = 0;
    determineElement (player->elementX, player->elementY, &player->element);
    determineElement (player->imbueX, player->imbueY, &player->imbue);
    return 1;
}

//Determine the amount of health restored with the healing spell
void healingCalculator (character *caster, character *target) {
    float healed;
    healed = ((50 * caster->numen) * (1 + calculateElementalDistance (caster->imbueX, caster->imbueY, 0, 0)));
//Make sure the health never exceeds the max
    if (target->curHP > target->maxHP) {
        healed -= (target->curHP - target->maxHP);
        target->curHP = target->maxHP;
    }
    printf ("%s heals %s for %.0f HP.\n", caster->name.str, target->name.str, healed);
    return;
}

//Sets minimum values to a string
void initializeString (string *newString) {
    newString->len = 1;
    newString->str = malloc (1);
    newString->str[0] = '\0';
    return;
}

/*CHANGE THIS*/
//Level up the character.  1000XP/level for now
void levelCharacter (character *player) {
//If there's enough XP
    if (player->xp >= (1000)) {
        player->level++;
        player->xp -= 1000;
        player->strength++;
        player->dexterity++;
        player->constitution++;
        player->numen++;
        player->intelligence++;
        player->resilience++;
        player->maxHP += (180 + (rand () % 21));
        player->curHP = player->maxHP;
        printf ("You have reached level %d.\n", player->level);
//If not
    } else {
        printf ("You do not have enough experience, collect %d more XP\n", 1000 - player->xp);
    }
    return;
}

int loadCharacter (character *target, FILE *load) {
//Local variables, buffer is used to store all the data at once, in is used to read the data from file.
    string buffer;
    char in;
//Initialize
    initializeString (&buffer);
//Read the save file
    while (1) {
        in = fgetc (load);
//Break conditions
        if (((ferror (load)) || (feof (load)))) {
            break;
        }
//Name first
        if (target->name.len == 1) {
            while (in != '\t') {
                readValueToString (&target->name, in);
                in = fgetc (load);
            }
        } else {
//Then stats
            while (in != '\n') {
                readValueToString (&buffer, in);
                in = fgetc (load);
            }
/*ADD EQUIPMENT HERE*/
         
/*ADD ITEMS HERE*/
               
        }
    }
//Parse out stats
    sscanf (buffer.str, "%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%f%*c%f%*c%f%*c%f", &target->level, &target->xp, &target->strength, &target->dexterity, &target->constitution, &target->numen, &target->intelligence, &target->resilience, &target->curHP, &target->maxHP, &target->elementX, &target->elementY, &target->imbueX, &target->imbueY);
    determineElement (target->elementX, target->elementY, &target->element);
    determineElement (target->imbueX, target->imbueY, &target->imbue);
    fclose (load);
    free (buffer.str);
    return 0;
}

//Load a character struct from a saved file
void openEnemy (character *enemy) {
//Local variables, monsters is the monster list Sonmy is working on, entryNumber is which monster to load, monsterCount is how many viable entries are in the monster file, in is used to search the monster file for the '#' mark denoting the end of the viable entries.
    FILE *monsters;
    int entryNumber, monsterCount = -1, i;
    char in = 0;
//Initialize
    monsters = fopen ("monsters", "r");
//A check that the file exists.
    if (monsters == NULL) {
        printf ("Monsters list missing.\n");
        exit (1);
    }
//Detect how many monsters have been added to the list.
    in = fgetc (monsters);
    do {
        in = fgetc (monsters);
        if (in == '\n') {
            monsterCount++;
        }
    } while (in != '#');
//Reset the file pointer and randomly pick a monster to load.
    rewind (monsters);
    entryNumber = ((rand () % monsterCount) + 1);
    for (i = 0; i < entryNumber; i++) {
        fscanf (monsters, "%*[^\n]%*c");
    }
    loadCharacter (enemy, monsters);
    return;
}

//Load a character struct from a saved file
int openPlayer (character *target, char *name) {
//Local variables.  load is the file the data is coming from.
    FILE *load;
    load = fopen (name, "r");
//A check that the file exists.
    if (load == NULL) {
        printf ("Can't access %s file.\n", name);
        return 1;
    }
    loadCharacter (target, load);
    return 0;
}

//Adds a character to a string, adjsuting size as needed
void readValueToString (string *string, char in) {
    string->str = realloc (string->str, ++(string->len));
    string->str[((string->len) - 2)] = in;
    string->str[((string->len) - 1)] = '\0';
    return;
}

//Generate the save file storing the current character's stats
void saveCharacter (character *player) {
//Local Variables, save is the file pointer that will save the data
    FILE *save;
//The character saves are sorted by the character name.
    save = fopen (player->name.str, "w");
//A check to make sure the file is generated properly.
    if (save == NULL) {
        printf ("Save failed, please try again.\n");
        return;
    }
    fprintf (save, "%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d/%d\t%f,%f\t%f,%f\n", player->name.str, player->level, player->xp, player->strength, player->dexterity, player->constitution, player->numen, player->intelligence, player->resilience, player->curHP, player->maxHP, player->elementX, player->elementY, player->imbueX, player->imbueY);
    fclose (save);
    printf ("%s saved.\n", player->name.str);
    return;
}

//Ask the player if they want to save
void saveCheck (character *player) {
//Local variables, choice is used to store the player's input.
    char choice;
//A simple request to the user.
    do {
        printf ("Save %s (Y/N)?\n", player->name.str);
        scanf ("%c%*c", &choice);
        if ((choice != 'Y') && (choice != 'y') && (choice != 'N') && (choice != 'n')) {
            printf ("That's not a valid option\n");
        }
//This ensures the valid input is given.
    } while ((choice != 'Y') && (choice != 'y') && (choice != 'N') && (choice != 'n'));
    if ((choice == 'Y') || (choice == 'y')) {
        saveCharacter (player);
    }
    return;
}

//Select a spell from a list of available spells, has an automated function for the enemy use.  Is also used to change imbuement
void selectSpell (character *caster, int automate, spell *targetSpell) {
//Local variables, spells is the file of spells, choice is the selected spell, optionsCount is the total available to choose from, spellX and spellY are coordinates of a spell from the list, maxDistance is the furthest allowable distance, firSpell and curSpell store the data from the list, in is used to read spell names from file
    FILE *spellList;
    int choice = 0, optionsCount = 0;
    float spellX = 0, spellY = 0, maxDistance;
    spell *firSpell = NULL, *curSpell = NULL, *prevSpell = NULL;
    char in;
    maxDistance = ((float)caster->level / 20);
    createSpell (&firSpell);
    curSpell = firSpell;
//make and check the file.
    spellList = fopen ("spells", "r");
    if (spellList == NULL) {
        printf ("Spells list missing.\n");
        exit (1);
    }
//Load the spells from the spellList
    fscanf (spellList, "%*[^\n]%*c");
    while (1) {
        in = fgetc (spellList);
//Break conditions
        if (((ferror (spellList)) || (feof (spellList)))) {
            break;
        }
//scan input
        fscanf (spellList, "%f%*c%f%*c", &spellX, &spellY);
//check if the spell is within allowable range
        if (calculateElementalDistance (spellX, spellY, caster->elementX, caster->elementY) <= maxDistance) {
            curSpell->x = spellX;
            curSpell->y = spellY;
//Update type
            curSpell->type = in;
            in = fgetc (spellList);
//Get name
            while (in != '\n') {
                readValueToString (&curSpell->name, in);
                in = fgetc (spellList);
            }
//get ready for next spell
            createSpell (&curSpell->next);
            prevSpell = curSpell;
            curSpell = curSpell->next;
            optionsCount++;
//Skip the line otherwise
        } else {
            fscanf (spellList, "%*[^\n]%*c");
        }
    }
    prevSpell->next = NULL;
    freeSpell (curSpell);
//Randomly select a spell if automate is selected
    if (automate == 1) {
        choice = ((rand () % optionsCount) + 1);
//Or ask the player what they choose
    } else {
//do...while to ensure valid choices only
        do {
            int i = 0, lineCount = 1;
            curSpell = firSpell;
            printf ("You can cast\n");
//List all possible spells
            while (i < optionsCount) {
                printf ("%d. %s", ++i, curSpell->name.str);
                curSpell = curSpell->next;
                if (lineCount++ == 3) {
                    printf ("\n");
                    lineCount = 1;
                } else {
                    printf ("\t");
                }
            }
//Get user's choice
            printf ("\nWhat would you like to cast: ");
            scanf ("%d%*c", &choice);
//Error checking
            if ((choice < 1) || (choice > optionsCount)) {
                printf ("That is not a valid option, please select from the list.\n");
                curSpell = firSpell;
            }
        } while ((choice < 1) || (choice > optionsCount));
    }
//Find the right spell
    curSpell = firSpell;
    while (choice-- > 1) {
        curSpell = curSpell->next;
    }
//Load the selected stats to targetSpell
    targetSpell->x = curSpell->x;
    targetSpell->y = curSpell->y;
    targetSpell->type = curSpell->type;
    targetSpell->name.len = curSpell->name.len;
    targetSpell->name.str = realloc (targetSpell->name.str, targetSpell->name.len);
    strcpy (targetSpell->name.str, curSpell->name.str);
//Update the imbued stats with the spell values
    caster->imbueX = targetSpell->x;
    caster->imbueY = targetSpell->y;
//Close and free
    fclose (spellList);
    curSpell = firSpell;
    while (firSpell != NULL) {
        firSpell = firSpell->next;
        freeSpell (curSpell);
        curSpell = firSpell;
    }
    return;
}

/*BUILD THIS*/
//Determine the status effect change
void statusCalculator (character *caster, character *target, spell *spell) {
    
    return;
}
