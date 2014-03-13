//
// breakout.c
//
// Computer Science 50
// Problem Set 4
//

// standard libraries
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Stanford Portable Library
#include "gevents.h"
#include "gobjects.h"
#include "gwindow.h"

// height and width of game's window in pixels
#define HEIGHT 600
#define WIDTH 400

// height and width of paddle
#define PADDLEW 80
#define PADDLEH 15

// number of rows of bricks
#define ROWS 5

// number of columns of bricks
#define COLS 10

// radius of ball in pixels
#define RADIUS 10

// lives
#define LIVES 3

// prototypes
void initBricks(GWindow window);
GOval initBall(GWindow window);
GRect initPaddle(GWindow window);
GLabel initScoreboard(GWindow window);
void updateScoreboard(GWindow window, GLabel label, int points);
GObject detectCollision(GWindow window, GOval ball);
bool statusGame(GWindow window, int lives, int bricks);

int main(void)
{
    // seed pseudorandom number generator
    srand48(time(NULL));

    // instantiate window
    GWindow window = newGWindow(WIDTH, HEIGHT);

    // instantiate bricks
    initBricks(window);

    // instantiate ball, centered in middle of window
    GOval ball = initBall(window);

    // instantiate paddle, centered at bottom of window
    GRect paddle = initPaddle(window);

    // instantiate scoreboard, centered in middle of window, just above ball
    GLabel label = initScoreboard(window);

    // number of bricks initially
    int bricks = COLS * ROWS;

    // number of lives initially
    int lives = LIVES;

    // number of points initially
    int points = 0;
    
    // status of being unpaused equals 0, when paused it will be 1
    int paused = 0;

    // specifying velocity of ball in x-dimension (velocity) and y-dimension (angle_vel)
    double velocity;
    double angle_vel = 1;
    double random;
    do
    {
        random = drand48();
    } 
    while (random <=0.5);
    velocity = random;
    
    // deciding direction of the fall
    double coin_flip = drand48();
    if (coin_flip > 0.5)
    {
        velocity = - velocity;
    }
   
    //wait for click to start the game
    waitForClick();
    
    
    // keep playing until game over
    while (lives > 0 && bricks > 0)
    {
        updateScoreboard(window, label, points);    
        //check for mouse events
        GEvent event = getNextEvent(MOUSE_EVENT);
        
        //if we heard an event
        if (event != NULL)
        {
            if (getEventType(event) == MOUSE_MOVED)
            {
                double x = getX(event) - PADDLEW/2;
                double y = getHeight(window) - PADDLEH;
                setLocation(paddle, x, y);
            }
        }
        
        // move the ball
        move(ball, velocity, angle_vel);
        
        
        //bounce off the right wall
        if (getX(ball) + RADIUS*2 >= WIDTH)
        {
            //only change the horizontal direction, but vertical direction is the same
            velocity = -velocity;
        }
        //bounce off the left wall
        else if (getX(ball) <= 0)
        {
            velocity = -velocity;
        }
        //bounce of the top of the window
        else if (getY(ball) <= 0)
        {
            angle_vel = -angle_vel;
        }
        //if the ball fell past the paddle, stop the ball, reduce lives and stop game
        else if (getY(ball)+ getHeight(ball)>= HEIGHT)
        {
            velocity = 0;
            angle_vel = 0;
            lives --;
            setLocation(ball, 190, 290);
            paused = 1;
        }
        
        // check for any collision
        GObject object = detectCollision(window, ball);
        
        if (object != NULL)
        {
            // if what the ball hit is the paddle, change vertical direction of movement
            if (object == paddle)
            {
                angle_vel = -angle_vel;
            }
            // if what the ball hit is a brick, then change vertical direction of movement and remove the brick 
            else if (strcmp(getType(object), "GRect") == 0)
            {
                angle_vel = -angle_vel;
                removeGWindow(window, object);
                points++;
                bricks--;
            }
            
        }
        
        //linger before moving again
        pause(1.5);
        
        //check if a life was lost, and reset the game on Click
        if (paused == 1)
        {
            
            waitForClick();    
            paused = 0;
            velocity = random;
            angle_vel = 1;
        }       
    }
    
    updateScoreboard(window, label, points);
    
    // checking if the game was won or lost, update scoreboard to 50
    statusGame(window, lives, bricks);
    if (statusGame(window, lives, bricks))
        updateScoreboard(window, label, COLS * ROWS);
    
    // wait for click before exiting
    waitForClick();

    // game over
    closeGWindow(window);
    return 0;
}

/**
 * Initializes window with a grid of bricks.
 */
void initBricks(GWindow window)
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            GRect brick = newGRect(2 + (40*j), 50 + (14*i), 36, 12);
            setColor(brick, "BLUE");
            setFilled(brick, true);
            add(window, brick);
        }
    }
      
}

/**
 * Instantiates ball in center of window.  Returns ball.
 */
GOval initBall(GWindow window)
{
    // instantiate a red ball and add it to the window
    GOval ball = newGOval(190, 290, 20, 20);
    setColor(ball, "RED");
    setFilled(ball, true);
    add(window, ball);
    return ball;
}

/**
 * Instantiates paddle in bottom-middle of window.
 */
GRect initPaddle(GWindow window)
{
    // define the location of the future paddle
    double x = WIDTH/2 - PADDLEW/2;
    double y = HEIGHT - PADDLEH;
    
    // instantiate a paddle at that location
    GRect paddle = newGRect(x, y, 80, 15);
    setColor(paddle, "BLACK");
    setFilled(paddle, true);
    add(window, paddle);
    return paddle;
}

/**
 * Instantiates, configures, and returns label for scoreboard.
 */
GLabel initScoreboard(GWindow window)
{
    // Scoreboard will be represented as a label
    GLabel label = newGLabel(" ");
    setFont(label, "SansSerif-36");
    add(window, label);
    return label;
}

/**
 * Updates scoreboard's label, keeping it centered in window.
 */
void updateScoreboard(GWindow window, GLabel label, int points)
{
    // update label
    char s[12];
    sprintf(s, "%i", points);
    setLabel(label, s);

    // center label in window
    double x = (getWidth(window) - getWidth(label)) / 2;
    double y = (getHeight(window) - getHeight(label)) / 2;
    setLocation(label, x, y);
}

/**
 * Adds final label: Won or Lost.
 */
bool statusGame(GWindow window, int lives, int bricks)
{    
    GLabel label = newGLabel(" ");
    setFont(label, "SansSerif-36");
    add(window, label);
  
    // allocate memory for appropriate string
    if (lives >= 0 && bricks == 0)
    {
        // update label
        setLabel(label, "You Won");
        setColor(label, "GREEN");
        // center label
        setLocation(label, WIDTH / 2 - getWidth(label) / 2, 200);
        waitForClick();
        return true;    
    }   
    else 
    {
        // update label
        setLabel(label, "You Lost");
        setColor(label, "RED");
        // center label
        setLocation(label, WIDTH / 2 - getWidth(label) / 2, 200);
        waitForClick();
        return false;
    } 
    
}

/**
 * Detects whether ball has collided with some object in window
 * by checking the four corners of its bounding box (which are
 * outside the ball's GOval, and so the ball can't collide with
 * itself).  Returns object if so, else NULL.
 */
GObject detectCollision(GWindow window, GOval ball)
{
    // ball's location
    double x = getX(ball);
    double y = getY(ball);

    // for checking for collisions
    GObject object;

    // check for collision at ball's top-left corner
    object = getGObjectAt(window, x, y);
    if (object != NULL)
    {
        return object;
    }

    // check for collision at ball's top-right corner
    object = getGObjectAt(window, x + 2 * RADIUS, y);
    if (object != NULL)
    {
        return object;
    }

    // check for collision at ball's bottom-left corner
    object = getGObjectAt(window, x, y + 2 * RADIUS);
    if (object != NULL)
    {
        return object;
    }

    // check for collision at ball's bottom-right corner
    object = getGObjectAt(window, x + 2 * RADIUS, y + 2 * RADIUS);
    if (object != NULL)
    {
        return object;
    }

    // no collision
    return NULL;
}
