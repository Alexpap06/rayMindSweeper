//Simple minesweeper game with raylib.

#define RAYGUI_IMPLEMENTATION
#include "libs/raygui.h"
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
//Width and height of the minesweeper game
#define COLS 10
#define ROWS 10
//Screen width and height:
const int screenWidth = 800;
const int screenHeight = 800;
//Definition for each cell in the mindsweeper grid:

const int cellWidth = screenWidth/COLS;
const int cellHeight = screenHeight/ROWS;

typedef enum GameState{
    PLAYING,
    WIN,
    LOSE
}GameState;

typedef enum ScreenState{
    SCREEN_MENU,
    SCREEN_GAME
}ScreenState;

GameState state = PLAYING;
ScreenState screen = SCREEN_MENU;

//Create a struct for each cell so every cell has its own data: 
typedef struct cell{
    int r;
    int c;
    bool containsMine;
    bool revealed;
    int  totalMines;
    bool flagged;

}Cell;
//Create the grid array:
Cell grid[COLS][ROWS];
int cellsRevealed = 0; // Number of cells revealed so far
int CurrMinesPresent; // Total number of mines in the current game

//Global sound variables:
Sound win;
Sound lose;
Sound click;


//Cell functions:
void CellReveal(int,int);
void CellDraw(Cell,Texture2D,Texture2D,Font);
int CellCountMines(int,int);
void CellFlag(int,int); //Flag or unflag a cell
void CellClearence(int,int); //If a cell has no neighboring mines, reveal its neighbors

//General functions:
bool ValidIndex(int,int);
void GameInit(void);
void GameRestart(void);



int main()
{
    srand(time(NULL));

    InitWindow(screenWidth, screenHeight, "MindSweeper");
    SetExitKey(KEY_F4);
    InitAudioDevice();

    Image icon = LoadImage("imgs/icon.bmp");
    SetWindowIcon(icon);

    //Texture/sound loading:
    Texture2D cell = LoadTexture("imgs/block.png");
    Texture2D mine = LoadTexture("imgs/mine.png");
    Texture2D flag = LoadTexture("imgs/flag.png");
    Texture2D guibackground = LoadTexture("imgs/guibackground.png");
    Font guiFont = LoadFontEx("resources/minecraftfont.ttf", 24, 0, 0);
    Font cellFont = LoadFontEx("resources/minecraftfont.ttf", 64, 0, 0);
    win = LoadSound("sounds/win.ogg");
    lose = LoadSound("sounds/lose.ogg");
    click = LoadSound("sounds/click.ogg");
    int i,j;
    //Current GUI toggle state:
    int guistate = -1;
    //// Set GUI font to the loaded font:
    GuiSetFont(guiFont);

    GameInit();

    while (!WindowShouldClose())   
    {

        

    if(screen == SCREEN_GAME){
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            PlaySound(click);
            Vector2 mPos = GetMousePosition();
            int indxC = mPos.x / cellWidth;
            int indxR = mPos.y / cellHeight;
            
            if(state == PLAYING && ValidIndex(indxC,indxR)){

                CellReveal(indxC,indxR);
            }
        }
        else if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
            PlaySound(click);
            Vector2 mPos = GetMousePosition();
            int indxC = mPos.x / cellWidth;
            int indxR = mPos.y / cellHeight;

            if(state == PLAYING && ValidIndex(indxC,indxR)){

                CellFlag(indxC,indxR);
            }
        }

        if(IsKeyPressed(KEY_R)){
            GameRestart();
        }
    }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if(screen == SCREEN_MENU){
            DrawTexture(guibackground,0,0,WHITE);
            Rectangle guigroup= {(screenWidth-600)/2, (screenHeight-200)/2, 300, 60};
            char* toggleGroupText = "Play;Exit";
            GuiToggleGroup(guigroup,toggleGroupText,&guistate);
            // It is important to set the text size with GuiSetStyle,
            // since the imported font has a fixed original size.
            GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
            //By setting the text size to the same size as the font importation,
            // we get a 1.0 text scale, which gives maximum clarity.
            GuiSetStyle(TOGGLE,TEXT_COLOR_NORMAL, ColorToInt(SKYBLUE));
            GuiSetStyle(TOGGLE,TEXT_COLOR_FOCUSED, ColorToInt(SKYBLUE));
            GuiSetStyle(TOGGLE, BASE_COLOR_NORMAL, ColorToInt(BLACK));
            GuiSetStyle(TOGGLE, BASE_COLOR_FOCUSED, ColorToInt(GRAY));
            GuiSetStyle(TOGGLE, BASE_COLOR_PRESSED, ColorToInt(RAYWHITE));
            //The ColorToInt function takes a Color arguement (r,g,b,a from 0-255)
            //and it transforms it into a 32-bit int by taking the bits from every color
            //and adds them into one single int variable (This function is especially
            //useful in this case, since the raygui style system saves colors as ints).
            

            if(guistate == 0){
                GameRestart();
                screen = SCREEN_GAME;
            }
                
            else if(guistate == 1){ 
                EndDrawing();     
                CloseWindow();
            }
        }
        else if(screen == SCREEN_GAME){

            if(IsKeyPressed(KEY_ESCAPE)){
                    screen = SCREEN_MENU;
                    guistate = -1;
                }

            for(i=0;i<COLS;i++){
                for(j=0;j<ROWS;j++){
                    DrawTexture(cell,i*cellWidth,j*cellHeight,WHITE);
                    CellDraw(grid[i][j],mine,flag,cellFont);
                }
            }

            const char* restart = "Press 'r' to restart.";
            if(state == LOSE){
                DrawRectangle(0,0,screenWidth,screenHeight,Fade(LIGHTGRAY,0.6f));
                const char* loss = "YOU LOSE.";
                DrawText(loss,screenWidth/2 - MeasureText(loss,60) / 2 ,screenHeight/2 - 10,60,RED);
                DrawText(restart,screenWidth/2 - MeasureText(restart,20) / 2 ,screenHeight * 0.75f - 10,20,BLACK);
                // Reminder: In raylib, font size is essentially the text height.
            }else if(state == WIN){
                DrawRectangle(0,0,screenWidth,screenHeight,Fade(WHITE,0.8f));
                const char* win = "YOU WIN.";
                DrawText(win,screenWidth/2 - MeasureText(win,60) / 2 ,screenHeight/2 - 10,60,GREEN);
                DrawText(restart,screenWidth/2 - MeasureText(restart,20) / 2 ,screenHeight * 0.75f - 10,20,BLACK);

            }
            
    }
        EndDrawing();
}
    
    CloseAudioDevice();
    CloseWindow(); 

     return 0;
 }
//Function Factory:
 void GameInit(void){
int i,j;

    for(i=0;i<COLS;i++){
            for(j=0;j<ROWS;j++){
                // Initialize every cell in the grid
                grid[i][j] = (Cell){.c = i, .r = j,.revealed = false,.containsMine = false,.flagged = false,.totalMines = 0};
                //The .c and .r are the struct's fields and we can access them in this way (C99 method).
                // Traditional initialization method:
                //grid[i][j].c = i;
                //grid[i][j].r = j;
                
            }
    }

    CurrMinesPresent = (int)(ROWS * COLS * 0.1f);
    int numOfMines = CurrMinesPresent;
    while(numOfMines > 0){
        j = rand() % ROWS;
        i = rand() % COLS;
        if(!grid[i][j].containsMine){
            grid[i][j].containsMine = true;
            numOfMines--;
        }
    }

    for(i=0;i<COLS;i++){
            for(j=0;j<ROWS;j++){     
                if(!grid[i][j].containsMine)
                    grid[i][j].totalMines = CellCountMines(i,j);
            }
    }

}

bool ValidIndex(int i,int j){
    return (i>=0 && i<COLS) && (j>=0 && j<ROWS);
}

void CellReveal(int i,int j){
    if(grid[i][j].flagged){
        return;
    }

    if(grid[i][j].revealed == false){
        grid[i][j].revealed = true;
        cellsRevealed++;
    }

    if(cellsRevealed >= (ROWS * COLS - CurrMinesPresent)){
        state = WIN;
        PlaySound(win);
    }

    if(grid[i][j].containsMine){
        state = LOSE;
        PlaySound(lose);
    }
    else{
        if(grid[i][j].totalMines == 0)
            CellClearence(i,j);
    }


}

void CellFlag(int i,int j){

    if(grid[i][j].revealed){
        return;
    }

    grid[i][j].flagged = !grid[i][j].flagged; 
}

void CellDraw(Cell cell,Texture2D mine,Texture2D flag,Font font){

    // Store the width and height of the text in a single variable
    Vector2 size = MeasureTextEx(font,TextFormat("%d", cell.totalMines),font.baseSize,0);
    //NOTE: the second parameter is a string representation of the number, specifically the number of total neighbor mines
    //for each cell.
    //Also, the font.baseSize gives us the font's size (in pixels) from the time it was loaded (in this case)
    //64.
    
    if(cell.revealed){
        
       if(cell.containsMine){
            DrawTexture(mine,cell.c*cellWidth,cell.r*cellHeight,WHITE);
       } 
       else{
            DrawRectangle(cell.c*cellWidth,cell.r*cellHeight,cellWidth,cellHeight,DARKGRAY);
            if(cell.totalMines > 0){
                if(cell.totalMines == 1)
                    DrawTextEx(font,TextFormat("%d", cell.totalMines),(Vector2){cell.c*cellWidth + (cellWidth - size.x) / 2,cell.r*cellHeight + (cellHeight - size.y) / 2},font.baseSize,0,SKYBLUE);
                else if(cell.totalMines == 2){
                    DrawTextEx(font,TextFormat("%d", cell.totalMines),(Vector2){cell.c*cellWidth + (cellWidth - size.x) / 2,cell.r*cellHeight + (cellHeight - size.y) / 2},font.baseSize,0,ORANGE);
                }
                else if(cell.totalMines == 3){
                    DrawTextEx(font,TextFormat("%d", cell.totalMines),(Vector2){cell.c*cellWidth + (cellWidth - size.x) / 2,cell.r*cellHeight + (cellHeight - size.y) / 2},font.baseSize,0,GREEN);
                }
                else{
                    DrawTextEx(font,TextFormat("%d", cell.totalMines),(Vector2){cell.c*cellWidth + (cellWidth - size.x) / 2,cell.r*cellHeight + (cellHeight - size.y) / 2},font.baseSize,0,BLACK);
                }
                //The thind parameter in the DrawTextEx function is a Vector2 variable which contains two parameters, one for the x and one for the
                // y position. For example, the cell.c/cell.r*cellWidth/cellHeight calculates the cell's position, but not centered (top-left corner). Then, we calculate
                //the cell's center position by removing the font size from the cell's width/height and dividing it by 2.In this way, we claim the 
                //cell's center so we can draw our element. 
//----------------------------------------------------------------------------------------------------------------------------------------------------
                //NOTES FROM THE PREVIOUS FUNCTION USED (DrawText):
                //The pre-last parameter of the DrawText function is the size of the font. We made it equal to cellHeight
                // so the size matches the cell's size, with a reduction of 8 pixels (4 from the top and 4 from the bottom, it 
                //basically fixes itself and adjusts its size).
            }
            
            
       }
    }
    else if(cell.flagged){
        DrawTexture(flag,cell.c*cellWidth,cell.r*cellHeight,WHITE);
        // This method works only for already scaled images.. If the image's size isn't already fixed, we program the following code:
        //Rectangle source  = {0,0,flag.width,flag.height}; //Source rectangle with original size image.
        //Rectangle dest = {flag,cell.c*cellWidth,cell.r*cellHeight,cellWidth,cellHeight}; //Fixed size and placement image (in the right cell).
        //Vector2 origin = {0,0}; //The equilibrium point of the image (not necessary in this case).
        //DrawTexturePro(flag,source,dest,origin,0.0f,WHITE);
    }

    DrawRectangleLines(cell.c*cellWidth,cell.r*cellHeight,cellWidth,cellHeight,BLACK);

}

int CellCountMines(int i,int j){

    int count = 0;
    for(int ii=-1;ii<=1;ii++){
            for(int jj=-1;jj<=1;jj++){
                if(ii == 0 && jj == 0){
                    continue;
                }

                if(ValidIndex(i + ii, j + jj)){

                    if(grid[i + ii][j + jj].containsMine){
                        count++;
                    }
                    else{
                        continue;
                    }
                }
                else{
                    continue;
                }
            
        }
        
    }

    return count;
}

void CellClearence(int i,int j){
    
    for(int ii=-1;ii<=1;ii++){
            for(int jj=-1;jj<=1;jj++){
                if(ii == 0 && jj == 0){
                    continue;
                }

                if(ValidIndex(i + ii, j + jj)){

                    if(!grid[i + ii][j + jj].revealed){
                        CellReveal(i + ii,j + jj);
                    }
                    else{
                        continue;
                    }
                }
                else{
                    continue;
                }
            
        }
        
    }
}

void GameRestart(void){
    GameInit();
    state = PLAYING;
    cellsRevealed = 0;
}