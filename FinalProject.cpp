#include <iostream>
#include <GL/glut.h>
#include <deque>
#include <assert.h>
using namespace std;

 // colors for the program in rgb format, use float values
#define mapBgColor   1.0,0.78,0.69
#define snakeColor   0.16,0.49,0.25
#define foodColor    0.95,0.44,0.125
#define wallColor    0.2,0.2,0.2
#define splFoodColor 0.0,0.0,1.0

int map_size = 20;     // keep value bw 10 and 50
int luck = 5;          // keep between 1 and 100 to get the easter egg food.
int initialLives = 5; // saves when you bit yourself
int maxDifficulty = 7; // keep atleast 5.


// assigning codes to directions
#define UP     1
#define DOWN   2
#define LEFT   3
#define RIGHT  4

#define delay(n) for(int i = 0; i < 1e5;i++)for(int j = 0; j < n*1e3;j++);   

deque<pair<int, int>> snake_body; // dsa for storing snake cordinates
int food_pos[2];                 // x,y position of food.



int foodAvailable = 0;
int score = 0;
int special = 0;
int paused = 1;
int finished = 0;
int direction = RIGHT;
int Difficulty = maxDifficulty; //higher is easy
int currentLives = initialLives;
int gameSpeed = Difficulty * 20; // also timer val to be passed in glutTimerFunc 



void showFinalScore()
{
    cout << "***********************\n";
    cout << "Your Final Score is: " << score;
}

void guide()
{
    printf("\n*********************************************************");
    printf("\nUse WASD as movement keys.");
    printf("\nPress W for Up, A for left, S for Down and D for Right");
    printf("\nM to change difficuly.");
    printf("\nR to Restart Game.");
    //printf("\n+ to Increase map size.The game will restart though.");
    //printf("\n- to Decrease map size.The game will restart though.");
    printf("\nP to pause/resume the game.");
    printf("\nPress Esc or Q to  Quit.");
    printf("\nPress H for help.");
    printf("\n\nPress P to Start.");
    printf("\n*********************************************************\n");
}

void draw_body_part(int x, int y)
{
    glColor3f(snakeColor);
    glBegin(GL_POLYGON); // a 1x1 square
    glVertex2i(x + 1, y + 1);
    glVertex2i(x + 1, y + 0);
    glVertex2i(x + 0, y + 0);
    glVertex2i(x + 0, y + 1);
    glEnd();

    glLineWidth(2);
    glColor3f(mapBgColor);
    glBegin(GL_LINES); // vertical line through middle of square
    glVertex2f(x + 0.5, y + 1);
    glVertex2f(x + 0.5, y + 0);
    glEnd();
    glBegin(GL_LINES); // horizontal line through middle of square
    glVertex2f(x + 0, y + 0.5);
    glVertex2f(x + 1, y + 0.5);
    glEnd();
}

void food_texture(int x, int y)
{
    glBegin(GL_POLYGON);
    glVertex2f(x + 0.5, y + 1.0);
    glVertex2f(x + 0.0, y + 0.5);
    glVertex2f(x + 0.5, y + 0.0);
    glVertex2f(x + 1, y + 0.5);
    glEnd();
}

void drawBrick(int x, int y)
{
    glColor3f(wallColor);
    glBegin(GL_POLYGON);
    glVertex2f(x + 0.95, y + 0.95);
    glVertex2f(x + 0.05, y + 0.95);
    glVertex2f(x + 0.05, y + 0.05);
    glVertex2f(x + 0.95, y + 0.05);
    glEnd();

}

void drawWalls()
{   
    for (int i = 0; i <= map_size;i++)
    {
        glColor3f(wallColor);
        drawBrick(i, 0);         // bottom wall  
        drawBrick(i, map_size - 1);// top wall     
        drawBrick(0, i);         // left wall     
        drawBrick(map_size - 1, i);// right wall     
    }
}

void drawFood()
{
    if (!foodAvailable)
    {
        // create food
        int fx = rand() % (map_size - 2) + 1, fy = rand() % (map_size - 2) + 1;
        int lottery = 1 + rand() % 100;
        if (lottery <= luck && score != 0)
        {
            special = 1;
        }
        int overlap = 1;
        while (overlap) // make sure no overlap of food with snake body
        {
            for (auto part : snake_body)
            {
                if (part.first == fx && part.second == fy)
                {
                    fx = rand() % map_size + 1, fy = rand() % map_size + 1;
                    break;
                }
            }
            overlap = 0;
            food_pos[0] = fx;
            food_pos[1] = fy;
        }
    }
    foodAvailable = 1;

    if (special == 1)
        glColor3f(splFoodColor);
    else
        glColor3f(foodColor);

    food_texture(food_pos[0], food_pos[1]); // using a brick for food
}

void drawSnake()
{
    glColor3f(snakeColor);
    int hx = snake_body[0].first;  // head x value
    int hy = snake_body[0].second; // head y value

    glBegin(GL_POLYGON); // 1x1 square
    glVertex2i(hx + 1, hy + 1);
    glVertex2i(hx + 1, hy + 0);
    glVertex2i(hx + 0, hy + 0);
    glVertex2i(hx + 0, hy + 1);
    glEnd();

    glColor3f(mapBgColor);
    glBegin(GL_POLYGON); // making a small square for eye of snake
    glVertex2f(hx + 0.85, hy + 0.9);
    glVertex2f(hx + 0.85, hy + 0.65);
    glVertex2f(hx + 0.65, hy + 0.65);
    glVertex2f(hx + 0.65, hy + 0.9);
    glEnd();

    // draw body other than head
    glColor3f(snakeColor);
    int len = snake_body.size();
    for (int i = 1; i < len;i++)
    {
        draw_body_part(snake_body[i].first, snake_body[i].second);
    }
}

void moveSnake(int newDirection)
{
    // A snake moves in the desired direction by adding new blocks/elements 
    // in front and popping block/element from the back so as to give an illusion
    // that the snake is moving
    direction = newDirection;
    int delX = 0;
    int delY = 0;
    int mapEdge = 0;
    int snake_part_axis = 0;
    switch (direction)
    {
    case UP:
        delY = 1;
        break;
    case DOWN:
        delY = -1;
        break;
    case LEFT:
        delX = -1;
        break;
    case RIGHT:
        delX = 1;
        break;
    }

    // Check if snake gets bitten by itself or not
    for (auto part : snake_body)
    {
        if ((part.first == (snake_body[0].first + delX)) && (part.second == (snake_body[0].second + delY)))
        {
            currentLives--;
            cout << "Be Careful! You got bit.\n";
            if (currentLives <= 0)
            {
                cout << "Game Over.";
                showFinalScore();
                delay(2);
                exit(0);
            }
        }
    }

    // check if the snake runs into the wall 
    if (snake_body[0].first <= 0 || snake_body[0].first >= map_size - 1 || snake_body[0].second <= 0 || snake_body[0].second >= map_size - 1)
    {
        cout << "Oh NO! You ran into wall. Game Over.\n";
        showFinalScore();
        delay(2);
        exit(0);
    }

    // check if snake hits the food and make him grow by pushing a new block inside 
    //the deque without popping last entry
    int grow = 0;
    if (snake_body[0].first + delX == food_pos[0] && snake_body[0].second + delY == food_pos[1])
    {
        grow = 1;
        if (special)
        {
            grow = 100;
            special = 0;
            cout << "Legendary Food!!! +100 score.\n";
        }
        score += grow;
        foodAvailable = 0;
    }

    // Whenever snake eats food we push a new block inside the deque
    snake_body.push_front({ snake_body[0].first + delX,snake_body[0].second + delY });

    // While the snake is moving we pop the blocks/elements from the back of the 
    // snakes body
    if (!grow)
    {
        snake_body.pop_back();
    }
    glutPostRedisplay();
}

void TimerFunc(int val)
{
    if (!paused)
        moveSnake(direction);
    glutTimerFunc(gameSpeed, TimerFunc, 0);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, map_size, 0, map_size);
    glMatrixMode(GL_MODELVIEW);
    drawSnake();
    drawFood();
    drawWalls();
    glutSwapBuffers();
}

void initializeGame()
{
    // some assertions for initial parameters
    assert(map_size >= 15);
    assert(map_size <= 50);
    assert(luck <= 100);
    assert(luck >= 1);
    assert(maxDifficulty >= 5);
    assert(initialLives > 0);
    srand(time(0));
    // glutOrtho2d()

    glClearColor(mapBgColor, 0);
    score = 0;
    currentLives = initialLives;

    // create size 3 snake
    snake_body.clear();
    snake_body.push_back({ 5,map_size / 2 });
    snake_body.push_back({ 4,map_size / 2 });
    snake_body.push_back({ 3,map_size / 2 });
    moveSnake(RIGHT);
    paused = 1;
    foodAvailable = 0;
    guide(); // show game guide
}

void keyboard(unsigned char key, int, int) {
    switch (key)
    {
        case 'W':
        case 'w':
        {
            if (!paused)
                if (direction == LEFT || direction == RIGHT) {
                    moveSnake(UP);
                }

            break;
        }
        case 'S':
        case 's':
        {
            if (!paused)
                if (direction == LEFT || direction == RIGHT) {
                    moveSnake(DOWN);
                }

            break;
        }
        case 'A':
        case 'a':
        {
            if (!paused)
                if (direction == UP || direction == DOWN) {
                    moveSnake(LEFT);
                }

            break;
        }
        case 'D':
        case 'd':
        {
            if (!paused)
                if (direction == UP || direction == DOWN) {
                        moveSnake(RIGHT);
                }
            break;
        }
        case 'M':
        case 'm':
        {
            Difficulty = (Difficulty - 1) % (maxDifficulty + 1);
            if (Difficulty <= 0)
                Difficulty = maxDifficulty;
            gameSpeed = Difficulty * 20;
            cout << "New Difficulty is " << (maxDifficulty + 1) - Difficulty << endl;
            break;
        }
        case 'P':
        case 'p':
        {   if (paused == 0)
            cout << "You Stopped\nPaused the game\n";
        else
            cout << "You Resumed/Started Game\n";
        paused = !paused;
        break;
        }
        case 'H':
        case 'h':
        {
            guide();
            break;
        }
        case 'R':
        case 'r':
        {

            showFinalScore();
            cout << "You restarted the Game.\nRestarting...\n.All the best!!!.\n";
            initializeGame();
            break;
        }
        case 27:
        case 'q':
        {
            cout << "You pressed exit.\n";
            showFinalScore();
            exit(0);
        }
    }
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(map_size * 20, map_size * 20);
    glutInitWindowPosition(250, 100);
    glutCreateWindow("Snake Game");
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(gameSpeed, TimerFunc, 0);
    initializeGame();
    glutMainLoop();
    return 0;
}