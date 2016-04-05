#include <curses.h>
#define UP 3
#define DOWN 2
#define RIGHT 5
#define LEFT 4

// Compile with
//   g++ SnakeGame.cpp -o SnakeGame -lncurses
//
// This has been shamelessly being snatched from a forum
//   http://cboard.cprogramming.com/c-programming/160991-help-ncurses-snake-game.html


int main(void){

    // activate the keypad to read from this terminal
    keypad(initscr(),TRUE);

    int pout = 0; //pen on/off (if even/odd)
    int xPos=1;     // initialise the x and y positions of the cursor
    int yPos=2;
    char map[1000][1000];
    char c='\0';  // input character initialise to null


    while( c!='q'){ // loop while the user has not pressed 'q'

        if(pout % 2 == 0){
            mvaddch(yPos,xPos,'*');
            map[yPos][xPos] = '*';
        }
        if(pout % 2 != 0){
            mvaddch(yPos,xPos,' ');
        }
        c=getch();    // read a single character

        if (c == 'p'){    //p changes even-->odd etc for toggle
            pout = pout + 1;
            noecho();
        }

        switch(c){
            case UP:
                yPos--;
                break;
            case DOWN:
                yPos++;
                break;
            case LEFT:
                xPos--;
                break;
            case RIGHT:
                xPos++;

        }
    }

    mvdelch(yPos,xPos+1);  //delete the q
    xPos = 1;
    yPos = 3;

    while( c!='f'){
        mvaddch(yPos,xPos,'O');    //This is the start of snake, snake won't move..
        // read a single character
        c=getch();
        // move according to cursor
        switch(c){
            case UP:
                if (map[yPos-1][xPos]== ' '){
                    yPos--;
                    map[yPos][xPos] = 'O';
                }
                break;
            case DOWN:
                if (map[yPos+1][xPos]== ' '){
                    yPos++;
                    map[yPos][xPos] = 'O';
                }
                break;
            case LEFT:
                if (map[yPos][xPos-1]== ' '){
                    xPos--;
                    map[yPos][xPos] = 'O';
                }
                break;
            case RIGHT:
                if (map[yPos][xPos+1]== ' '){
                    xPos++;
                    map[yPos][xPos] = 'O';
                }
        }
    }
}
