#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define DEFAULT '~'
#define PLACED_SHIP 'O'
#define HIT '0'
#define MISS '*'
#define SUNK 'X'

#define INVALID 0
#define VALID_MISS 1
#define VALID_HIT 2

#define DEFAULT_COLOR "\033[0m"
#define RED_COLOR "\033[31m"
#define GREEN_COLOR "\033[32m"
#define BLUE_COLOR "\033[36m"
#define YELLOW_COLOR "\033[33m"
#define BRIGHT_RED_COLOR "\033[91m"
#define BRIGHT_GREEN_COLOR "\033[92m"
#define UNDERLINE_COLOR "\033[4;37m"

#define HORIZONTAL 1
#define VERTICAL 0

#define MAX_NAME 32
#define NULL_COORD {27, 27}     // bigger than max board-size => never used

#ifdef _WIN32
    #define CONSOLE system("cls") // makes ANSI work
    #include <windows.h>  //funcion sleep()  
#else
    #define CONSOLE 1 // literary makes nothing
    #include <unistd.h>			
#endif

typedef struct player_fleet {
    char nick[MAX_NAME];
    char* carrier[5];
    char* battleship[4];
    char* destroyer[3];
    char* submarine[3];
    char* patrol_boat[2];
    char** player_board;
    char* last_shot;
} PLAYER;

typedef struct ship {
    unsigned short orientation;
    unsigned short size;
    unsigned short x;
    unsigned short y;
} SHIP;

typedef struct coord {
    unsigned short x;
    unsigned short y;
} _COORD;

//////////////// CONSOLE GRAPHICS /////////////////

void clear_screen();
void logo_utn();

/////////////// PRINTING TO CONSOLE ///////////////

void print_one(PLAYER player, unsigned int board_size);
void print_both(PLAYER player_active, PLAYER player_opponent, unsigned int board_size);
void print_tables(PLAYER player_active, PLAYER player_opponent, unsigned int board_size);
void alive_ship_check(char* pShip[]);
void default_screen(PLAYER player_active, PLAYER player_opponent, unsigned int board_size);
void print_hint();

//////////////// UI, MENU and GUIDE ///////////////

void main_menu();
void main_menu_intro();
void main_menu_help();

/////////////////// INITIALIZING //////////////////

char** initialize(char** board, unsigned int board_size);
void free_board(char** board, unsigned int board_size);
PLAYER placement_of_ships_user(unsigned int board_size);
PLAYER placement_of_ships_computer(unsigned int board_size);
int place_ship(char** board, unsigned int board_size, SHIP active_ship);
void set_fleet(SHIP active_ship, unsigned short ship_size, int flag, PLAYER *result);

///////////////////// GAMEPLAY ////////////////////

void player_vs_player(unsigned int board_size);
int player_turn(PLAYER* player_active, PLAYER* player_opponent, unsigned int board_size);
_COORD get_coord();
int fire(PLAYER player_opponent, _COORD aim, unsigned int board_size);
int ship_hit_check(PLAYER player_opponent);
int victory_check(PLAYER player_opponent);


int main() {
    CONSOLE;    // makes ANSI work on Win CMD
    printf(DEFAULT_COLOR);
    char user_input[3];
    do {
        main_menu();    // game starts

        printf("\n\tEscribi 'R' para Reiniciar o cualquier tecla para salir: ");
        fgets(user_input, 3, stdin);
        user_input[0] = tolower(user_input[0]);
        clear_screen();
    } while(user_input[0] == 'r');  // loop until user types R for RESTART

    return 0;
}


/////////////////////////////////////////////////////
////////////////// CONSOLE GRAPHICS /////////////////
/////////////////////////////////////////////////////


void clear_screen() {
    /* Clears the screen and sets color to default */

    printf("\033c"DEFAULT_COLOR);
}


#include <stdio.h>

void logo_utn() {
    printf("\033[30m\n");
    // Parte superior del logo con arco y barra central
    printf("                             ####         #         #### \n");
    printf("                             ###          #          ### \n");
    printf("                             ##           #           ## \n");
    printf("                             ##           #           ## \n");
    printf("                             ##           #           ## \n");
    printf("                             ##           #           ## \n");
    printf("                               ##         #         ##   \n");
    printf("                                ###       #       ###    \n");
    printf("                                  ####    #    ####      \n");
    printf("                                     ###########         \n");

    // Línea horizontal central
    printf("                            #############################\n");

    // Parte inferior del logo con arco y barra central
    printf("                                     ###########         \n");
    printf("                                  ####    #    ####      \n");
    printf("                                ###       #       ###    \n");
    printf("                               ##         #         ##   \n");
    printf("                             ##           #           ## \n");
    printf("                             ##           #           ## \n");
    printf("                             ##           #           ## \n");
    printf("                             ##           #           ## \n");
    printf("                             ###          #          ### \n");
    printf("                             ####         #         #### \n");
}


///////////////////////////////////////////////////
/////////////// PRINTING TO CONSOLE ///////////////
///////////////////////////////////////////////////


void print_one(PLAYER player, unsigned int board_size) {
    /* Prints the game-board of given player while placing fleet */

    // first row
    printf(DEFAULT_COLOR"\n\t   ");
    for (int i = 0; i < board_size; ++i) {
        printf("%2d ", i + 1);  // 0 1 2 ...
    }
    printf("\n");

    // rows with tiles
    for (int i = 0; i < board_size; i++) {
        printf(DEFAULT_COLOR"\n\t%c  ", 'A' + i);
        for (int j = 0; j < board_size; j++) {
            if (player.player_board[j][i] == DEFAULT) printf(BLUE_COLOR);     // sets color
            else printf(GREEN_COLOR);

            printf(" %c ", player.player_board[j][i]);
        }
    }
    printf("\n\n"DEFAULT_COLOR);
}


void print_both(PLAYER player_active, PLAYER player_opponent, unsigned int board_size) {
    /* Prints the game-board of given player on left and prints
    other board, where one can see his shots and hits on right */

    // header
    printf(DEFAULT_COLOR"\n\ttu flota:");
    for (int i = 0; i < board_size; ++i) printf("   ");
    printf("\t\tTablero del rival:\n");

    // first row
    printf("\n\t   ");
    for (int i = 0; i < board_size; ++i) printf("%2d ", i + 1);  // left board
    printf("\t\t\t   ");

    for (int i = 0; i < board_size; ++i) printf("%2d ", i + 1); // right board
    printf("\n");

    // rows with tiles
    for (int i = 0; i < board_size; i++) {  // for each line:
        printf(DEFAULT_COLOR"\n\t%c  ", 'A' + i);
        for (int j = 0; j < board_size; j++) {    // left board
            if (player_active.player_board[j][i] == DEFAULT) printf(BLUE_COLOR);     // sets color according to tile
            else if (player_active.player_board[j][i] == PLACED_SHIP) printf(GREEN_COLOR);
            else if (player_active.player_board[j][i] == HIT) printf(BRIGHT_RED_COLOR);
            else if (player_active.player_board[j][i] == MISS) printf(YELLOW_COLOR);
            else if (player_active.player_board[j][i] == SUNK) printf(RED_COLOR);

            printf(" %c ", player_active.player_board[j][i]);   // prints sign
        }

        printf("\t\t");
        printf(DEFAULT_COLOR"\t%c  ", 'A' + i);
        for (int j = 0; j < board_size; j++) {    // right board
            if (player_opponent.player_board[j][i] == DEFAULT || player_opponent.player_board[j][i] == PLACED_SHIP) {
                printf(BLUE_COLOR);
                printf(" ~ ");          // cannot tell, if enemy ship's there
                continue;
            }
            else if (player_opponent.player_board[j][i] == HIT) printf(BRIGHT_RED_COLOR);
            else if (player_opponent.player_board[j][i] == MISS) printf(YELLOW_COLOR);
            else if (player_opponent.player_board[j][i] == SUNK) printf(RED_COLOR);

            printf(" %c ", player_opponent.player_board[j][i]);
        }
    }
    printf(DEFAULT_COLOR"\n\n");
}


void print_tables(PLAYER player_active, PLAYER player_opponent, unsigned int board_size) {
    /* Prints two tables of 5 ships. Active ships are printed green and sunk are red */

    // header
    printf(BLUE_COLOR"\tFlota de %s", player_active.nick);

    for (int i = 0; i < board_size*3 - strlen(player_active.nick) + 19; ++i) printf(" ");   // spaces

    printf(BRIGHT_RED_COLOR "Flota de %s", player_opponent.nick);
    printf(DEFAULT_COLOR"\n");

    // table
    // Carrier 5
    alive_ship_check(player_active.carrier);
    printf("\t   Portaaviones [5]");     // left table (active)
    for (int i = 0; i < board_size + 2; ++i) printf("   ");     // spaces
    alive_ship_check(player_opponent.carrier);
    printf("     Portaaviones [5]\n");   // right table (opponent)
    // Battleship 4
    alive_ship_check(player_active.battleship);
    printf("\t   Acorazado [4]");
    for (int i = 0; i < board_size + 4; ++i) printf("   ");
    alive_ship_check(player_opponent.battleship);
    printf("  Acorazado [4]\n");
    // Destroyer 3
    alive_ship_check(player_active.destroyer);
    printf("\t   Destructor [3]");
    for (int i = 0; i < board_size + 3; ++i) printf("   ");
    alive_ship_check(player_opponent.destroyer);
    printf("   Destructor [3]\n");
    // Submarine 3
    alive_ship_check(player_active.submarine);
    printf("\t   Submarino [3]");
    for (int i = 0; i < board_size + 4; ++i) printf("   ");
    alive_ship_check(player_opponent.submarine);
    printf("   Submarino [3]\n");
    // Patrol Boat 2
    alive_ship_check(player_active.patrol_boat);
    printf("\t   Patrullero [2]");
    for (int i = 0; i < board_size + 3; ++i) printf("   ");
    alive_ship_check(player_opponent.patrol_boat);
    printf("     Patrullero [2]\n");
}


void alive_ship_check(char* pShip[]) {
    /* Checks, whether tiles of given ship are set to SUNK. If the ship is sunk,
    color is set to red, otherwise the color is set to green */

    if (*pShip[0] == SUNK) printf(RED_COLOR);
    else printf(GREEN_COLOR);
}


void default_screen(PLAYER player_active, PLAYER player_opponent, unsigned int board_size) {
    /* Prints name of player who is on turn, sums up last round, prints his and opponent's boards and
    tables with active ships of both players */

    clear_screen();
    printf(DEFAULT_COLOR"Turno de\n");
    printf(BLUE_COLOR"\n\t%s", player_active.nick);
    

    printf("\n\tCONSOLA: ");     // dialogue
    if (player_active.last_shot) {  // based on last hit
        switch (*player_active.last_shot) {
            case MISS:
                printf(YELLOW_COLOR"'Fallaste!' ");
                break;
            case HIT:
                printf(BRIGHT_GREEN_COLOR"'Golpeaste un barco enemigo %s!' ", player_active.nick);
                break;
            case SUNK:
                printf(GREEN_COLOR"'Buen trabajo %s! El barco enemigo se esta hundiendo...' ", player_active.nick);
                break;
        }
    } else printf("Preparate %s", player_active.nick);    // first round

    if (player_opponent.last_shot) {
        switch (*player_opponent.last_shot) { // based on last hit
            case MISS:
                printf(YELLOW_COLOR"'El proyectil enemigo no nos alcanzo.'\n");
                break;
            case HIT:
                printf(BRIGHT_RED_COLOR"'Nos dieron!'\n");
                break;
            case SUNK:
                printf(RED_COLOR"'Se nos hunde un barco!'\n");
                break;
        }
    } else printf("... Haremos que visiten a Nemo!\n"DEFAULT_COLOR);

    print_both(player_active, player_opponent, board_size);
    print_tables(player_active, player_opponent, board_size);
    print_hint();
}


void print_hint() {
    /* Prints one line with description of different chars shown on board */

    printf(BLUE_COLOR"\n\t%c", DEFAULT);
    printf(DEFAULT_COLOR" - agua no descubierta |");

    printf(GREEN_COLOR"  %c", PLACED_SHIP);
    printf(DEFAULT_COLOR" - tu barco |");

    printf(YELLOW_COLOR"  %c", MISS);
    printf(DEFAULT_COLOR" - fallaste (celda vacia) |");

    printf(BRIGHT_RED_COLOR"  %c", HIT);
    printf(DEFAULT_COLOR" - barco golpeado |");

    printf(RED_COLOR"  %c", SUNK);
    printf(DEFAULT_COLOR" - barco hundido");
}


///////////////////////////////////////////////////
//////////////// UI, MENU and GUIDE ///////////////
///////////////////////////////////////////////////


void main_menu() {
    /* Main menu where user gives all important information
    and gets needed guidance */

    clear_screen();
    char user_input[8];

    main_menu_intro();
    fgets(user_input, 8, stdin);    // chance to get help
    if (!strcmp(user_input, "h\n") || !strcmp(user_input, "H\n")) main_menu_help();

    printf(UNDERLINE_COLOR"\n\n\t########### CONFIGURACION DEL TABLERO ###########");
    printf(UNDERLINE_COLOR"\n\tIngresa el tamanio");
    printf(DEFAULT_COLOR" del tablero [5..10]: ");

    fgets(user_input, 8, stdin);    // inputs board size
    unsigned int board_size = strtol(user_input, NULL, 10);
    if (board_size > 10) board_size = 10;   // max size
    if (board_size < 5) board_size = 5;   // min size

    printf(UNDERLINE_COLOR"\n\n\t########### MODO DE JUEGO ###########"); 
    printf(DEFAULT_COLOR"\n\tEscribe [1] para el modo: Jugador vs Jugador;");
    printf("\n\tPresione 'ENTER' para continuar/salir: ");

    fgets(user_input, 8, stdin);    // inputs game-mode
    clear_screen();
    switch (user_input[0]) {
        case '1':
            player_vs_player(board_size);
            break;
        default:
            break;
    }
}


void main_menu_intro() {
    /* Only prints title and basic instructions to console */

    printf(BLUE_COLOR"\n\tBATALLA ");
    printf(RED_COLOR"NAVAL");
    printf("\n\t------------\n\t");
    logo_utn();
    printf(DEFAULT_COLOR"\n\tBienvenido al Menu Principal!");
    printf("\n\t'Batalla Naval' es un fascinante juego de estrategia por turnos para dos jugadores.");
    printf("\n\tCada jugador tiene una flota con");
    printf("\n\t5 barcos de diferentes longitudes. El objetivo del juego es hundir todos los barcos enemigos.");
    printf("\n\tEn primer lugar, colocas todos tus barcos en el tablero. Cuando comienza el juego, tienes que");
    printf("\n\tescribir las coordenadas del tablero enemigo para intentar encontrar sus barcos y hundirlos.");
    printf("\n\tEn el lado derecho de la pantalla, encontraras informacion sobre las naves activas de ambos jugadores.");
    printf("\n\tHay una barra de 'CONSOLA' en la parte superior de la pantalla, que resume la ronda reciente. Puedes elegir");
    printf("\n\tel tamanio del tablero de juego. Siendo un tablero pequeno un desafio sencillo y uno grande implica mayor tiempo de juego.");
    printf("\n\tPara batallas mas dificiles se recomienda un tablero de 10x10. Eso es todo, a divertirse!\n");
    printf("\n\tEscriba 'H' para obtener una pista adicional o pulse 'ENTER' para continuar: ");
}


void main_menu_help() {
    /* Only prints hint to console */

    printf("\n\tLa flota esta compuesta por un Portaaviones(5), un Acorazado(4), un Destructor(3), un Submarino(3) y un Barco Patrulla(2).");
    printf("\n\tAntes del juego, colocaras tus barcos en cualquier lugar vertical u horizontalmente para abordar un tablero de tamanio elegido previamente.");
    printf("\n\tDurante la batalla, alternaras turnos con tu oponente. No espies su tablero, no hagas trampas!");
    printf("\n\tPara saber a quien le corresponde los turnos, sus nombres se mostraran en la parte superior de la pantalla.");
    printf("\n\tlA 'CONSOLA' te dira si tu ultimo disparo golpeo al enemigo o si fuimos golpeados.");
    printf("\n\tSe mostraran dos tableros. El de la izquierda muestra tus barcos y los ataques enemigos. El de la derecha registra tus disparos.");
    printf("\n\tDebajo de cada tablero hay un apartado con los barcos de los jugadores (ROJO: Hundido, VERDE: En flota). En cada turno tienes que");
    printf("\n\tescribir las coordenadas de una casilla enemiga (p. ej., A2). El objetivo es adivinar la posicion de los barcos oponentes y atacarlos.");
    printf("\n\tCuando el jugador no tiene ninguna nave en flota, su oponente gana y aparece la pantalla de victoria. A divertirse!\n");
}


///////////////////////////////////////////////////
/////////////////// INITIALIZING //////////////////
///////////////////////////////////////////////////


char** initialize(char** board, unsigned int board_size) {
    /* Allocates memory for game board and sets value of
    each tile to DEFAULT */

    board = (char**) malloc(sizeof(char*) * board_size);
    for (int i = 0; i < board_size; ++i) {
        board[i] = (char*) malloc(sizeof(char) * board_size);

        for (int j = 0; j < board_size; j++) board[i][j] = DEFAULT;
    }
    return board;
}


void free_board(char** board, unsigned int board_size) {
    /* Frees allocated memory for game board */

    for (int i = 0; i < board_size; ++i) free(board[i]);
    free(board);
}


PLAYER placement_of_ships_user(unsigned int board_size) {
    /* Memory for board will be allocated. Fleet of each player consists of five vessels.
    Player enters his nick, enters coordinates and places his ships to the board */

    clear_screen();
    PLAYER result;
    printf("\n\tEscribe tu usuario: ");
    fgets(result.nick, MAX_NAME, stdin);
    char* new_line = strchr(result.nick, '\n'); // removes '\n'
    if(new_line) *new_line = '\0';

    char** player_board = NULL;  // allocates memory
    player_board = initialize(player_board, board_size);
    result.player_board = player_board;
    result.last_shot = NULL;    // no last shot yet

    char buffer[8];
    int ship_size = 5; //parametro para modificar el tamanio de las naves
    int flag = 1;   // flag 1 = destroyer; flag 0 = submarine (both size 3)
    SHIP active_ship;

    while (ship_size >= 2) {    // this WHILE will run 5 times

        while(1) {  // this WHILE will run until active ship is placed (in case of formatting errors
            printf("\n\t%s esta colocando sus barcos!\n", result.nick);
            print_one(result, board_size);

            printf(UNDERLINE_COLOR);
            switch (ship_size) {
                case 5:
                    printf("\n\tEstas por colocar un: Portaaviones (5)\n");
                    break;
                case 4:
                    printf("\n\tEstas por colocar un: Acorazado (4)\n");
                    break;
                case 3:
                    if (flag) printf("\n\tEstas por colocar un: Destructor (3)\n");
                    else printf("\n\tEstas por colocar un: Submarino (3)\n");
                    break;
                default:
                    printf("\n\tEstas por colocar un: Bote Patrulla (2)\n");
                    break;
            }
            printf(DEFAULT_COLOR"\n\tIngresa las coordenadas ");
            printf(UNDERLINE_COLOR"[A..E] y [1..5].");
            printf(DEFAULT_COLOR"\n\tSeguido con ");
            printf(UNDERLINE_COLOR"'H' horizontal o 'V' vertical");
            printf(DEFAULT_COLOR".\n\tSi el lugar esta ocupado, podra volver a intentarlo.");
            printf("\n\n\tEscribe las coordenadas de la siguiente forma [A1 H]: ");

            fgets(buffer, 8, stdin);
            active_ship.x = strtol(buffer + 1, NULL, 10) - 1;   // reading x coordinate
            buffer[0] = toupper(buffer[0]);
            active_ship.y = buffer[0] - 'A';    // reading y coordinate

            char *space = strchr(buffer, ' ');
            if(space) { // if not NULL due to bad formatting
                space++;
                *space = toupper(*space);
                if (*space == 'V') active_ship.orientation = VERTICAL;      // reading orientation
                else active_ship.orientation = HORIZONTAL;

                active_ship.size = ship_size;
                clear_screen();
            }
            else {  // space == NULL -> formatting Error
                clear_screen();
                printf("\n\tERROR DE FORMATO\n");
                continue;   // repeat
            }

            if (place_ship(player_board, board_size, active_ship)) {     // checks if there is an obstacle
                printf(RED_COLOR"\n\tNo se puede colocar aca, proba de nuevo...\n"DEFAULT_COLOR);
            }
            else {      // ship has been placed
                result.player_board = player_board; // saves created placement
                set_fleet(active_ship, ship_size, flag, &result);
                break;  // ship is placed into board and into player struct - breaks inner WHILE
            }
        }

        if (ship_size == 3 && flag) { // taking care of two ships with size 3
            ship_size++;
            flag = 0;
        }
        ship_size--;    // placing ships from largest to smallest
    }

    printf("\n\t%s esta colocando sus barcos!\n", result.nick);  // shows the result
    print_one(result, board_size);
    printf("\n\tEscribe 'R' para Reiniciar.");
    printf("\n\tPulsa 'ENTER' para continuar: ");
    fgets(buffer, 8, stdin);
    buffer[0] = tolower(buffer[0]);

    if (buffer[0] == 'r') {
        free_board(result.player_board, board_size);
        return placement_of_ships_user(board_size); // recursion - repeating the process
    }
    else return result;     // user is satisfied - returns the result struct
}

int place_ship(char** player_board, unsigned int board_size, SHIP active_ship) {
    /* Checks, whether ship can be placed to active_ship coordinates. If yes, function
    places ship to the board and returns 0. If it is impossible to place ship there,
    function returns 1 and doesn't edit the board */

    if (active_ship.x < 0 || active_ship.x >= board_size) return 1;
    if (active_ship.y < 0 || active_ship.y >= board_size) return 1;

    if (active_ship.orientation == HORIZONTAL) {  // horizontal orientation
        if (active_ship.x + active_ship.size > board_size) return 1;    // doesn't fit to board

        for (int i = 0; i < active_ship.size; ++i) {
            if (player_board[active_ship.x + i][active_ship.y] != DEFAULT) return 1;   // tile is occupied
        }

        for (int i = 0; i < active_ship.size; ++i) {
            player_board[active_ship.x + i][active_ship.y] = PLACED_SHIP;   // all good - placing ship
        }
    }

    else if (active_ship.orientation == VERTICAL) {  // vertical orientation
        if (active_ship.y + active_ship.size > board_size) return 1;    // doesn't fit to board

        for (int i = 0; i < active_ship.size; ++i) {
            if (player_board[active_ship.x][active_ship.y + i] != DEFAULT) return 1;   // tile is occupied
        }

        for (int i = 0; i < active_ship.size; ++i) {
            player_board[active_ship.x][active_ship.y + i] = PLACED_SHIP;   // all good - placing ship
        }
    }
    return 0;
}


void set_fleet(SHIP active_ship, unsigned short ship_size, int flag, PLAYER *result) {
    /* Based on active_ship data (size and orientation), only one of following branches is chosen.
    Within correct branch, pointers to tiles of specific ships are saved to PLAYER struct. E.g. when Carrier
    is placed on board, two pointers point to each of its 5 tiles (**player_board and **carrier) */

    if (active_ship.orientation == HORIZONTAL) {    // determine the orientation - horizontal
        switch (ship_size) {    // find out which type of ship is active
            case 5:
                for (int i = 0; i < ship_size; ++i)     // assign each active ship pointers to struct
                    result->carrier[i] = &result->player_board[active_ship.x + i][active_ship.y];
                break;
            case 4:
                for (int i = 0; i < ship_size; ++i)
                    result->battleship[i] = &result->player_board[active_ship.x + i][active_ship.y];
                break;
            case 3:
                if (flag)
                    for (int i = 0; i < ship_size; ++i)
                        result->destroyer[i] = &result->player_board[active_ship.x + i][active_ship.y];
                else
                    for (int i = 0; i < ship_size; ++i)
                        result->submarine[i] = &result->player_board[active_ship.x + i][active_ship.y];
                break;
            default:
                for (int i = 0; i < ship_size; ++i)
                    result->patrol_boat[i] = &result->player_board[active_ship.x + i][active_ship.y];
                break;
        }
    } else if (active_ship.orientation == VERTICAL) {   // vertical orientation
        switch (ship_size) {
            case 5:
                for (int i = 0; i < ship_size; ++i)
                    result->carrier[i] = &result->player_board[active_ship.x][active_ship.y + i];
                break;
            case 4:
                for (int i = 0; i < ship_size; ++i)
                    result->battleship[i] = &result->player_board[active_ship.x][active_ship.y + i];
                break;
            case 3:
                if (flag)
                    for (int i = 0; i < ship_size; ++i)
                        result->destroyer[i] = &result->player_board[active_ship.x][active_ship.y + i];
                else
                    for (int i = 0; i < ship_size; ++i)
                        result->submarine[i] = &result->player_board[active_ship.x][active_ship.y + i];
                break;
            default:
                for (int i = 0; i < ship_size; ++i)
                    result->patrol_boat[i] = &result->player_board[active_ship.x][active_ship.y + i];
                break;
        }
    }
}


///////////////////////////////////////////////////
///////////////////// GAMEPLAY ////////////////////
///////////////////////////////////////////////////


void player_vs_player(unsigned int board_size) {
    /* PvP mode. Two PLAYER structs are created. Players alternate turns until one destroys all enemy
    ships what breaks while loop. All allocated memory will be freed when finished */

    PLAYER player1, player2;

    player1 = placement_of_ships_user(board_size);
    player2 = placement_of_ships_user(board_size);

    while (1){
        // first player
        if (player_turn(&player1, &player2, board_size)) break;

        // second player
        if (player_turn(&player2, &player1, board_size)) break;
    }

    printf(DEFAULT_COLOR"\n\tFelicidades! Ganaste el juego");
    free_board(player1.player_board, board_size);
    free_board(player2.player_board, board_size);
}


int player_turn(PLAYER* player_active, PLAYER* player_opponent, unsigned int board_size) {
    _COORD aim;
    int flag;

    do {
        default_screen(*player_active, *player_opponent, board_size);
        aim = get_coord();
        flag = fire(*player_opponent, aim, board_size);
        
        while (flag == INVALID) { // Repite hasta que las coordenadas sean válidas
            printf(UNDERLINE_COLOR"\n\t¡No se puede disparar ahí! Prueba de nuevo.");
            aim = get_coord();
            flag = fire(*player_opponent, aim, board_size);
        }

        player_active->last_shot = &(player_opponent->player_board[aim.x][aim.y]);

        if (flag == VALID_HIT) { // Si el disparo fue un acierto
            if (ship_hit_check(*player_opponent)) { // Verifica si el barco fue hundido
                if (victory_check(*player_opponent)) { // Condición de victoria
                    default_screen(*player_active, *player_opponent, board_size);
                    printf(BRIGHT_GREEN_COLOR"\n\t##################################\n");
                    printf("\t##################################\n");
                    printf("\t  ------- VICTORIA DE %s -------\n", player_active->nick);
                    printf("\t##################################\n");
                    printf("\t##################################\n");
                    return 1;
                }
            }
            // Si el disparo fue exitoso, el jugador mantiene el turno
            printf(BRIGHT_GREEN_COLOR"\n\t¡Acertaste! Puedes disparar nuevamente.\n");
        } else {
            // Si el disparo falló, sale del bucle y cambia el turno
            printf(RED_COLOR"\n\t¡Fallaste! Es el turno del otro jugador.\n");
        }

    } while (flag == VALID_HIT); // Continúa mientras el jugador acierte (VALID_HIT)

    return 0;
}


_COORD get_coord() {
    /* Asks user for coordinates to shoots at and returns them as struct */

    printf(DEFAULT_COLOR"\n\n\tEscribe una coordenada para atacar al rival (por ejemplo: A1): ");
    _COORD result = NULL_COORD;    // default coordinates are too big so formatting error will be detected
    char buffer[8];
    fgets(buffer, 8, stdin);
    result.x = strtol(buffer + 1, NULL, 10) - 1;   // reading coordinates
    buffer[0] = toupper(buffer[0]);
    result.y = buffer[0] - 'A';
    return result;
}


int fire(PLAYER player_opponent, _COORD aim, unsigned int board_size) {
    /* Checks whether given coordinates are valid (within board, repetitive strikes). If not, INVALID is returned.
    If shot hits water, VALID_MISS is returned and VALID_HIT is returned upon hitting ship. Function edits the board */

    // checks if within board
    if (aim.x < 0 || aim.x >= board_size) return INVALID;
    if (aim.y < 0 || aim.y >= board_size) return INVALID;
    // actually fire
    if (player_opponent.player_board[aim.x][aim.y] == DEFAULT) {
        player_opponent.player_board[aim.x][aim.y] = MISS;  // hits water
        return VALID_MISS;
    }
    else if (player_opponent.player_board[aim.x][aim.y] == PLACED_SHIP) {
        player_opponent.player_board[aim.x][aim.y] = HIT;  // hits ship
        return VALID_HIT;
    }
    else return INVALID;    // shoots where it is not allowed (repetitive strikes)
}


int ship_hit_check(PLAYER player_opponent) {
    /* Called when player hits something and checks whether it was deadly strike. Function goes through all
    enemy ships. If it finds ship with all tiles set to HIT, it returns 1. Else 0 is returned */

    int flag = 0;
    // check Carrier
    for (int i = 0; i < 5; ++i)
        if (*player_opponent.carrier[i] != HIT) {
            flag++;
            break;  // not all of them HIT
        }
    if (flag == 0) {    // newly sunk ship found (all tiles were HIT)
        for (int i = 0; i < 5; ++i) *player_opponent.carrier[i] = SUNK; // sinks the ship
        return 1;
    }
    flag = 0;
    // check Battleship
    for (int i = 0; i < 4; ++i)
        if (*player_opponent.battleship[i] != HIT) {
            flag++;
            break;  // not all of them HIT
        }
    if (flag == 0) {    // newly sunk ship found
        for (int i = 0; i < 4; ++i) *player_opponent.battleship[i] = SUNK;
        return 1;
    }
    flag = 0;
    // check Destroyer
    for (int i = 0; i < 3; ++i)
        if (*player_opponent.destroyer[i] != HIT) {
            flag++;
            break;  // not all of them HIT
        }
    if (flag == 0) {    // newly sunk ship found
        for (int i = 0; i < 3; ++i) *player_opponent.destroyer[i] = SUNK;
        return 1;
    }
    flag = 0;
    // check Submarine
    for (int i = 0; i < 3; ++i)
        if (*player_opponent.submarine[i] != HIT) {
            flag++;
            break;  // not all of them HIT
        }
    if (flag == 0) {    // newly sunk ship found
        for (int i = 0; i < 3; ++i) *player_opponent.submarine[i] = SUNK;
        return 1;
    }
    flag = 0;
    // check Patrol Boat
    for (int i = 0; i < 2; ++i)
        if (*player_opponent.patrol_boat[i] != HIT) {
            flag++;
            break;  // not all of them HIT
        }
    if (flag == 0) {    // newly sunk ship found
        for (int i = 0; i < 2; ++i) *player_opponent.patrol_boat[i] = SUNK;
        return 1;
    }
    return 0;
}


int victory_check(PLAYER player_opponent) {
    /* Goes through all opponents ships. If it finds active ship , function returns 0 (didn't win).
    If all ships are SUNK, enemy fleet is sunk (win) and functions return 1 */

    // check Carrier
    if (*player_opponent.carrier[0] != SUNK) return 0;
    // check Battleship
    if (*player_opponent.battleship[0] != SUNK) return 0;
    // check Destroyer
    if (*player_opponent.destroyer[0] != SUNK) return 0;
    // check Submarine
    if (*player_opponent.submarine[0] != SUNK) return 0;
    // check Patrol Boat
    if (*player_opponent.patrol_boat[0] != SUNK) return 0;
    return 1;
}


