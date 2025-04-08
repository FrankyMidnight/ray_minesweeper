#include <raylib.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

bool init();
void cleanup();
bool setup_game();
void draw();
void read_input();
bool load_image_textures();

int WIN_WIDTH = 600;
int WIN_HEIGHT = 600;
const char *WIN_TITLE = "Ray Minesweeper 0.1";
const int FPS = 60;
const size_t SMALL_COLS = 9;
const size_t SMALL_ROWS = 9;
const size_t mines = 5;



typedef enum 
{
    SIZE_SMALL = 0,
    SIZE_MEDIUM,
    SIZE_LARGE,
    SIZE_EXTRA_LARGE
}board_size_t;

typedef struct 
{
    Rectangle src_rect;
    int nearby_mines;
    bool is_revealed;
    bool is_mine;
    bool is_checked;
    bool has_number;
}board_t;

typedef struct 
{
    board_size_t board_size;
    size_t cols;
    size_t rows;
    size_t mines;
}game_t;

board_t *board;
game_t *game;

Texture2D board_texture;
Texture2D borders_texture;
Texture2D digitback_texture;
Texture2D digits_texture;
Texture2D faces_texture;
Texture2D main_window_texture;

Image icon_image;

int main(void)
{
    // INIT
    if (!init())
    {
        TraceLog(LOG_ERROR, "Error initializing Raylib ... Bye");
        return EXIT_FAILURE;
    }
    // SETUP GAME
    if(!setup_game())
    {
        TraceLog(LOG_ERROR, "Error setting up game ... Bye");
        return EXIT_FAILURE;
    }
    // MAIN LOOP
    while (!WindowShouldClose())
    {
        // RENDER
        draw();
        // READ USER INPUT
        read_input();
    }
    cleanup();
    return EXIT_SUCCESS;
}

bool init()
{
    InitWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE);
    if(!IsWindowReady())
    {
        TraceLog(LOG_ERROR, "Error creating window ... Bye");
        return false;
    }
    SetTargetFPS(FPS);
    load_image_textures();
    return true;
}
void cleanup()
{
    free(game);
    free(board);
    UnloadTexture(board_texture);
    UnloadTexture(borders_texture);
    UnloadTexture(digitback_texture);
    UnloadTexture(digits_texture);
    UnloadTexture(faces_texture);
    UnloadTexture(main_window_texture);
    CloseWindow();
}
bool setup_game()
{
    
    // Allocate game
    game = calloc(1, sizeof(game_t));
    if(!game)
    {
        TraceLog(LOG_ERROR, "Error allocating game ...");
        return false;
    }
    // setup size
    game->board_size = SIZE_SMALL;


    // SET BOARD SIZE ACCORDING TO GAME SIZE
    switch(game->board_size)
    {
        case SIZE_SMALL:
            game->cols = 9;
            game->rows = 9;
            game->mines = 6;
            break;
        case SIZE_MEDIUM:
            game->cols = 19;
            game->rows = 19;
            game->mines = 12;
            break;
        case SIZE_LARGE:
            game->cols = 29;
            game->rows = 29;
            game->mines = 18;
            break;
        case SIZE_EXTRA_LARGE:
            game->cols = 39;
            game->rows = 39;
            game->mines = 24;
            break;
        default:
            break;
    }
    // Allocate board
    board = calloc(game->cols * game->rows, sizeof(board_t));
    // check
    if(!board)
    {
        TraceLog(LOG_ERROR, "Error allocating board ...");
        return false;
    }
    // generate seed
    srand((uint32_t)time(NULL));
    // restore the board
    for (size_t row = 0; row < game->rows; row++) 
    {
        for (size_t col = 0; col < game->cols; col++) 
        {
               
            // set default state no mines none is checked and none is visible
            board[game->cols * row + col].is_checked = false;    
            board[game->cols * row + col].is_revealed = false;    
            board[game->cols * row + col].is_mine = false;  
            board[game->cols * row + col].nearby_mines = 0;  
            board[game->cols * row + col].has_number = false;  
        }
    }
    // place the mines
    for (size_t i = 0; i < game->mines; i++) 
    {
        size_t row = (size_t)rand() % game->rows;
        size_t col = (size_t)rand() % game->cols;
        board[game->cols * row + col].is_mine = true;
    }
    
    
    // set numbers for nearby mines
    // loop the board
    for (int row = 0; row < (int)game->rows; row++) 
    {
        for (int col = 0; col < (int)game->cols; col++) 
        {
            // set the index of the current cell
            int index = (int)game->cols * row + col;
            // loop extra column right and left and an extra row below and up
            for (int add_row = -1; add_row < 2; add_row++) 
            {
                for (int add_col = -1; add_col < 2; add_col++) 
                {
                    // set checking col and row
                    int check_col = col + add_col;
                    int check_row = row + add_row;
                    // if check is in bound
                    if(check_col >= 0 && check_row >= 0 && 
                        check_col < (int)game->cols && check_row < (int)game->cols)
                    {
                        // and if it is a mine
                        if(board[(int)game->cols * check_row + check_col].is_mine)
                        {
                            // increase mine count for the base cell
                            board[index].nearby_mines++;
                            board[index].has_number = true;
                        }
                    }
                }
            
            }
        }
    }
    // Assign rectangles
    for (size_t row = 0; row < game->rows; row++) 
    {
        for (size_t col = 0; col < game->cols; col++) 
        {
            size_t cell_index = game->cols * row + col;
            switch (board[cell_index].nearby_mines) 
            {
            case 0:
                board[cell_index].src_rect = (Rectangle){.x = 0.0f, .y = 0.0f, .width = 16.0f, .height = 16.0f};
                break;
            case 1:
                board[cell_index].src_rect = (Rectangle){.x = 16.0f, .y = 0.0f, .width = 16.0f, .height = 16.0f};
                break;
            case 2:
                board[cell_index].src_rect = (Rectangle){.x = 32.0f, .y = 0.0f, .width = 16.0f, .height = 16.0f};
                break;
            case 3:
                board[cell_index].src_rect = (Rectangle){.x = 48.0f, .y = 0.0f, .width = 16.0f, .height = 16.0f};
                break;
            case 4:
                board[cell_index].src_rect = (Rectangle){.x = 64.0f, .y = 0.0f, .width = 16.0f, .height = 16.0f};
                break;
            case 5:
                board[cell_index].src_rect = (Rectangle){.x = 80.0f, .y = 0.0f, .width = 16.0f, .height = 16.0f};
                break;
            case 6:
                board[cell_index].src_rect = (Rectangle){.x = 96.0f, .y = 0.0f, .width = 16.0f, .height = 16.0f};
                break;
            case 7:
                board[cell_index].src_rect = (Rectangle){.x = 112.0f, .y = 0.0f, .width = 16.0f, .height = 16.0f};
                break;
            case 8:
                board[cell_index].src_rect = (Rectangle){.x = 0.0f, .y = 16.0f, .width = 16.0f, .height = 16.0f};
                break;
            default:
            // on error set mine icon with an X
                board[cell_index].src_rect = (Rectangle){.x = 112.0f, .y = 16.0f, .width = 16.0f, .height = 16.0f};
                break;
            }
        }
    }

    return true;
}
void draw()
{
    BeginDrawing();
    // Calculate dest_rect dimensions for the cell texture
    float dest_rect_width = (float)WIN_WIDTH / (float)game->cols;
    float dest_rect_height = (float)WIN_HEIGHT / (float)game->rows;
    // walk the board to draw cells
    for (size_t row = 0; row < game->rows; row++) 
    {
        for (size_t col = 0; col < game->cols; col++) 
        {
            size_t index = game->cols * row + col;
            // Calculate the dest x y coordinate for the cell texture
            float dest_posX = dest_rect_width * (float)col;
            float dest_posY = dest_rect_height * (float)row;
            // DRAW MINE
            if(board[index].is_mine)
            {
                // DrawRectangle(posX, posY, rect_width, rect_height , RED);
                DrawTexturePro(board_texture, 
                    (Rectangle){.x = 32.0f, .y = 16.0f , .width = 16.0f, .height = 16.0f},
                    (Rectangle){ . x = dest_posX , .y = dest_posY , .width = dest_rect_width, .height = dest_rect_height}, 
                    (Vector2){.x = 0.0f , .y = 0.0f}, 
                    0.0f, 
                    WHITE);
            }
            else if (board[index].is_revealed) 
            // DRAW REVEALED CELL
            {
            
                // DrawRectangle(posX, posY, rect_width, rect_height , DARKGRAY);
                DrawTexturePro(board_texture, 
                    board[index].src_rect,
                    (Rectangle){ . x = dest_posX , .y = dest_posY , .width = dest_rect_width, .height = dest_rect_height}, 
                    (Vector2){.x = 0.0f , .y = 0.0f}, 
                    0.0f, 
                    WHITE);
            }
            else  
            // DRAW COVERED CELL
            {
                // DrawRectangle(posX, posY, rect_width, rect_height , DARKGRAY);
                DrawTexturePro(board_texture, 
                    (Rectangle){.x = 16.0f, .y = 16.0f , .width = 16.0f, .height = 16.0f},
                    (Rectangle){ . x = dest_posX , .y = dest_posY , .width = dest_rect_width, .height = dest_rect_height}, 
                    (Vector2){.x = 0.0f , .y = 0.0f}, 
                    0.0f, 
                    WHITE);
            }
        }
    }

    EndDrawing();
}
void read_input()
{
    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        // Reveal cell under mouse click
        Vector2 mouse_pos = GetMousePosition();
        // column = x pos / (win_width / game_cols)
        int col = (int)(mouse_pos.x /((float)WIN_WIDTH / (float)game->cols));
        int row = (int)(mouse_pos.y /((float)WIN_HEIGHT / (float)game->rows));
        TraceLog(LOG_INFO, "Col : %i\tRow : %i", col, row);
        TraceLog(LOG_INFO, "Mouse_X : %.2f\tMouse_Y : %.2f", mouse_pos.x, mouse_pos.y);
        // if click is on a mine game over
        if(board[game->cols * (size_t)row + (size_t)col].is_mine)
        {
            TraceLog(LOG_INFO, "You clicked a mine : GAME OVER !!");
        }
        else  
        // just reveal the cell
        {
        board[game->cols * (size_t)row + (size_t)col].is_revealed = true;
        TraceLog(LOG_INFO, "Nearby mines : %i",board[game->cols * (size_t)row + (size_t)col].nearby_mines);
        }
    }
}
bool load_image_textures()
{


    // load game textures
    board_texture = LoadTexture("images/board.png");
    if(!IsTextureValid(board_texture))
    {
        TraceLog(LOG_ERROR, "Error loading texture images/board.png");
        return false;
    }
    // load game textures
    borders_texture = LoadTexture("images/borders.png");
    if(!IsTextureValid(borders_texture))
    {
        TraceLog(LOG_ERROR, "Error loading texture images/borders.png");
        return false;
    }
    // load game textures
    digitback_texture = LoadTexture("images/digitback.png");
    if(!IsTextureValid(digitback_texture))
    {
        TraceLog(LOG_ERROR, "Error loading texture images/digitback.png");
        return false;
    }
    // load game textures
    digits_texture = LoadTexture("images/digits.png");
    if(!IsTextureValid(digits_texture))
    {
        TraceLog(LOG_ERROR, "Error loading texture images/digits.png");
        return false;
    }
    // load game textures
    faces_texture = LoadTexture("images/faces.png");
    if(!IsTextureValid(faces_texture))
    {
        TraceLog(LOG_ERROR, "Error loading texture images/faces.png");
        return false;
    }
    // load game textures
    main_window_texture = LoadTexture("images/main_window.png");
    if(!IsTextureValid(main_window_texture))
    {
        TraceLog(LOG_ERROR, "Error loading texture images/main_window.png");
        return false;
    }

    // Load window icon
    icon_image = LoadImage("images/icon.png");
    if(!IsImageValid(icon_image))
    {
        TraceLog(LOG_ERROR, "Error loading icon image images/icon.png");
        return false;
    }
    SetWindowIcon(icon_image);
    UnloadImage(icon_image);
    return true;
}