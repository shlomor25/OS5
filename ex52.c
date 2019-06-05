/**
 * Name: Shlomo Rabinovich
 * ID:   308432517
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#define CLEAR "clear"
#define BUFFER_S 1
#define BOARD_S 20
#define BLOCK_S 3
#define INIT_POS 8
#define EMPTY_FIELD ' '
#define BLOCK_FIELD '-'
#define FRAME_FIELD '*'
#define BOARD_LIMIT 3
#define ALARM_WAIT 1

typedef enum {HORIZONTAL, VERTICAL} state;

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point *points;
    int rightMost;
    int leftMost;
    int downMost;
    int numOfPoints;
    state state;
} Block;

typedef struct {
    char spots[BOARD_S][BOARD_S+1];
    int shouldStop;
} Board;

Board *board;
Block *block;


/**
 * put block on board
 * @param board board
 * @param block block
 */
void RefreshBlockPosition(){
    int i, x, y;
    for(i = 0; i < block->numOfPoints; i++){
        x = block->points[i].x;
        y = block->points[i].y;
        board->spots[y][x] = BLOCK_FIELD;
    }
}


/**
 * boolean function. if block arrived to floor, erase it
 * @param block block
 * @return 1 for true, 0 for false
 */
int IsOnFloor(){
    if (block->downMost > BOARD_S - BOARD_LIMIT){
        return 1;
    }
    return 0;
}


/**
 * push down the block
 */
void PushDown(){
    if(block->downMost <= BOARD_S - BOARD_LIMIT) {
        int i;
        for (i = block->numOfPoints - 1; i >= 0; --i) {
            board->spots[block->points[i].y][block->points[i].x] = EMPTY_FIELD;
            block->points[i].y++;
        }
        block->downMost++;
    }
}


/**
 * push right the block
 */
void PushRight(){
    if(block->rightMost <= BOARD_S - BOARD_LIMIT - 1) {
        int i;
        for (i = block->numOfPoints - 1; i >= 0; --i) {
            board->spots[block->points[i].y][block->points[i].x] = EMPTY_FIELD;
            block->points[i].x++;
        }
        block->rightMost++;
        block->leftMost++;
    }
}


/**
 * push left the block
 */
void PushLeft(){
    if(BOARD_LIMIT <= block->leftMost) {
        int i;
        for (i = block->numOfPoints - 1; i >= 0; --i) {
            board->spots[block->points[i].y][block->points[i].x] = EMPTY_FIELD;
            block->points[i].x--;
        }
        block->leftMost--;
        block->rightMost--;
    }
}


/**
 * Display tetris.
 */
void DrawTetris(){
    system(CLEAR);
    int i;
    for (i = 0; i < BOARD_S; i++){
        printf("%s", board->spots[i]);
        printf("\n");
    }
}


void DeepCopy(Block *dest, Block *src){
    dest->numOfPoints = src->numOfPoints;
    dest->leftMost = src->leftMost;
    dest->rightMost = src->rightMost;
    dest->downMost = src->downMost;
    for (int i = 0; i < src->numOfPoints; i++){
        dest->points[i].x = src->points[i].x;
        dest->points[i].y = src->points[i].y;
    }
}


/**
 * Delete block from board.
 */
void ClearBoard(){
    int i;
    for (i = block->numOfPoints-1; i >= 0; --i){
        board->spots[block->points[i].y][block->points[i].x] = EMPTY_FIELD;
    }
}


/**
 * Flip block.
 */
void FlipBlock(){
    Block copy;
    copy.points = malloc(sizeof(Point) * block->numOfPoints);
    DeepCopy(&copy, block);
    Point center = copy.points[copy.numOfPoints / 2];
    int centerX, centerY, rightMost, leftMost, downMost;
    rightMost = leftMost = centerX = center.x;
    downMost = centerY = center.y;

    for(int i = copy.numOfPoints-1; i >=0; --i){
        Point p = copy.points[i];
        if (block->state == VERTICAL && p.x != centerX) {
            copy.points[i].x = centerX;
            copy.points[i].y = centerY + centerX - p.x;

        }  else if(block->state == HORIZONTAL &&  p.y != centerY)  {
            copy.points[i].x = centerX + centerY - p.y;
            copy.points[i].y = centerY;
        }
        if (copy.points[i].y > downMost){
            downMost = copy.points[i].y;
        }
        if (copy.points[i].x < leftMost){
            leftMost = copy.points[i].x;
        }
        if (copy.points[i].x > rightMost){
            rightMost = copy.points[i].x;
        }
    }
    if ((leftMost >= BOARD_LIMIT - 1) && (rightMost <= BOARD_S - BOARD_LIMIT) && (downMost < BOARD_S - BOARD_LIMIT)){
        block->state = (block->state + 1) % 2;
        ClearBoard();
        DeepCopy(block, &copy);
        block->downMost = downMost;
        block->leftMost = leftMost;
        block->rightMost = rightMost;
    }
    free(copy.points);
}


/**
 * define functional according keys
 * @param sig signal
 */
void SignalHandler(int sig){
    if(!IsOnFloor() && sig == SIGUSR2){
        char buffer[BUFFER_S];
        //  load char from ex51.c
        read(0, buffer, sizeof(buffer));
        char pushed = buffer[0];
        switch(pushed){
            case 'q':
                board->shouldStop = 1;
                break;
            case 'a':
                PushLeft();
                break;
            case 'd':
                PushRight();
                break;
            case 's':
                PushDown();
                break;
            case 'w':
                FlipBlock();
                break;
            default:
                break;
        }
        RefreshBlockPosition();
        DrawTetris();
    }
}


/**
 * Time passed.
 */
void AlarmHandle(int sig){
    if(!IsOnFloor() && sig == SIGALRM){
        PushDown();
        RefreshBlockPosition();
        DrawTetris();
    }
    alarm(ALARM_WAIT);
}


/**
 * Create board.
 * @return board
 */
Board* CreateTetrisBoard(){
    Board* board = malloc(sizeof(Board));
    board->shouldStop = 0;
    int i, j;
    for(i = BOARD_S - 2; i >= 0; --i){
        for(j = 1; j < BOARD_S - 1; j++){
            board->spots[i][j] = EMPTY_FIELD;
        }
    }
    // put * on margin
    for(i = BOARD_S-1; i >= 0; --i){
        board->spots[BOARD_S - 1][i] = FRAME_FIELD;
    }
    for(i = BOARD_S - 2; i >=0 ; --i) {
        board->spots[i][0] = FRAME_FIELD;
        board->spots[i][BOARD_S - 1] = FRAME_FIELD;
        board->spots[i][BOARD_S] = '\0';
    }
    return board;
}


/**
 * define block position.
 * @param block block
 * @return block
 */
Block* InitBlock(Block *block){
    int i;
    for (i = 0; i < block->numOfPoints; i++){
        block->points[i].y = 0;
        block->points[i].x = INIT_POS + i;
    }
    block->downMost = 0;
    block->rightMost = INIT_POS + block->numOfPoints - 1;
    block->leftMost = INIT_POS;
    block->state = VERTICAL;
    return block;
}


/**
 * Create block on top of board.
 * @param numOfPoints 3
 * @return block
 */
Block* CreateBlock(int numOfPoints){
    Block *block = malloc(sizeof(block));
    block->numOfPoints = numOfPoints;
    block->points = malloc(sizeof(Point) * numOfPoints);
    return (InitBlock(block));
}


/**
 * Free allocated memory.
 */
void NiceExit(){
    system(CLEAR);
    printf("---Quit!---\n");
    free(block->points);
    free(block);
    free(board);
}


/**
 * define signals.
 */
void signals(){
    /* Creating the Alarm Signal */
    struct sigaction usr_action1;
    sigset_t block_mask1;
    sigaddset(&block_mask1, SIGUSR2);
    usr_action1.sa_handler = AlarmHandle;
    usr_action1.sa_mask = block_mask1;
    usr_action1.sa_flags = 0;
    sigaction(SIGALRM, &usr_action1, NULL);

    /* Creating the SIGUSR Signal */
    struct sigaction usr_action2;
    sigset_t block_mask2;
    sigaddset(&block_mask2, SIGALRM);
    usr_action2.sa_handler = SignalHandler;
    usr_action2.sa_mask = block_mask2;
    usr_action2.sa_flags = 0;
    sigaction(SIGUSR2, &usr_action2, NULL);
}


/**
 * Main
 * Tetris game.
 * @return
 */
int main(){
    signals();
    sigset_t both_signals;
    sigaddset(&both_signals, SIGUSR2);
    sigaddset(&both_signals, SIGALRM);
    sigset_t pending;
    alarm(1);

    // init
    board = CreateTetrisBoard();
    block = CreateBlock(BLOCK_S);

    while(board->shouldStop == 0){
        // when bock is on floor
        if (IsOnFloor()){
            ClearBoard();
            InitBlock(block);
            DrawTetris();
            sigprocmask(SIG_UNBLOCK, &both_signals, NULL);
        }
        // listening to signals
        sigpending(&pending);
        if (sigismember(&pending, SIGALRM)){
            raise(SIGALRM);
        }
        if(sigismember(&pending, SIGUSR2)){
            raise(SIGUSR2);
        }
        pause();
    }
    NiceExit();
    return 0;
}

