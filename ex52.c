/**
 * Name: Shlomo Rabinovich
 * ID:   308432517
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#define CLEAR "clear"
#define BUFFER_SIZE 1
#define BOARD_SIZE 20
#define BLOCK_SIZE 3
#define X_START_POINT 9
#define EMPTY_CHAR ' '
#define BLOCK_CHAR '-'
#define FRAME_CHAR '*'
#define BOARD_LIM 3
#define ALARM_WAIT 1

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
} Block;

typedef struct {
    char spots[BOARD_SIZE][BOARD_SIZE+1];
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
        board->spots[y][x] = BLOCK_CHAR;
    }
}


/**
 * boolean function. if block arrived to floor, erase it
 * @param block block
 * @return 1 for true, 0 for false
 */
int ShouldRemoveBlock(){
    if (block->downMost > BOARD_SIZE - BOARD_LIM){
        return 1;
    }
    return 0;
}


/**
 * push down the block
 */
void DownBlock(){
    if(block->downMost <= BOARD_SIZE - BOARD_LIM) {
        int i;
        for (i = block->numOfPoints - 1; i >= 0; --i) {
            board->spots[block->points[i].y][block->points[i].x] = EMPTY_CHAR;
            block->points[i].y++;
        }
        block->downMost++;
    }
}


/**
 * push right the block
 */
void RightBlock(){
    if(block->rightMost <= BOARD_SIZE - BOARD_LIM - 1) {
        int i;
        for (i = block->numOfPoints - 1; i >= 0; --i) {
            board->spots[block->points[i].y][block->points[i].x] = EMPTY_CHAR;
            block->points[i].x++;
        }
        block->rightMost++;
        block->leftMost++;
    }
}


/**
 * push left the block
 */
void LeftBlock(){
    if(BOARD_LIM <= block->leftMost) {
        int i;
        for (i = block->numOfPoints - 1; i >= 0; --i) {
            board->spots[block->points[i].y][block->points[i].x] = EMPTY_CHAR;
            block->points[i].x--;
        }
        block->leftMost--;
        block->rightMost--;
    }
}


/**
 * Display tetris.
 */
void DrawBoard(){
    system(CLEAR);
    int i,j;
    for (i = 0; i < BOARD_SIZE; i++){
        printf("%s", board->spots[i]);
        printf("\n");
    }
}


void CopyBlock(Block *dest, Block *src){
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
void ClearBlock(){
    int i;
    for (i = block->numOfPoints-1; i >= 0; --i){
        board->spots[block->points[i].y][block->points[i].x] = EMPTY_CHAR;
    }
}


void FlipBlock(){
    Block copy;
    copy.points = malloc(sizeof(Point) * block->numOfPoints);
    CopyBlock(&copy, block);
    Point center = copy.points[copy.numOfPoints / 2];
    int centerX = center.x;
    int centerY = center.y;
    int diff;
    int rightMost = centerX;
    int leftMost = centerX;
    int downMost = centerY;
    for(int i = 0; i < copy.numOfPoints; i++){
        Point p = copy.points[i];
        if (p.x != centerX && p.y == centerY){
            diff = centerX - p.x;
            copy.points[i].x = centerX;
            copy.points[i].y = centerY + diff;

            if (copy.points[i].x > rightMost){
                rightMost = copy.points[i].x;
            }
            if (copy.points[i].x < leftMost){
                leftMost = copy.points[i].x;
            }
            if (copy.points[i].y > downMost){
                downMost = copy.points[i].y;
            }

        } else if (p.y != centerY && p.x == centerX){
            diff = centerY - p.y;
            copy.points[i].x = centerX + diff;
            copy.points[i].y = centerY;

            if (copy.points[i].x > rightMost){
                rightMost = copy.points[i].x;
            }
            if (copy.points[i].x < leftMost){
                leftMost = copy.points[i].x;
            }
            if (copy.points[i].y > downMost){
                downMost = copy.points[i].y;
            }
        }
    }
    if (rightMost <= BOARD_SIZE - BOARD_LIM && leftMost >= BOARD_LIM && downMost < BOARD_SIZE - BOARD_LIM){
        ClearBlock();
        CopyBlock(block, &copy);
        block->rightMost = rightMost;
        block->leftMost = leftMost;
        block->downMost = downMost;
    }
    free(copy.points);
}


/**
 * define functional according keys
 * @param sig signal
 */
void SignalHandler(int sig){
    if(!ShouldRemoveBlock() && sig == SIGUSR2){
        char buffer[BUFFER_SIZE];
        //  load char from ex51.c
        read(0, buffer, sizeof(buffer));
        char pushed = buffer[0];
        switch(pushed){
            case 'q':
                board->shouldStop = 1;
                break;
            case 'a':
                LeftBlock();
                break;
            case 'd':
                RightBlock();
                break;
            case 's':
                DownBlock();
                break;
            case 'w':
                FlipBlock();
                break;
            default:
                break;
        }
        RefreshBlockPosition();
        DrawBoard();
    }
}


/**
 * Time passed.
 */
void AlarmHandle(int sig){
    if(!ShouldRemoveBlock() && sig == SIGALRM){
        DownBlock();
        RefreshBlockPosition();
        DrawBoard();
    }
    alarm(ALARM_WAIT);
}


/**
 * Create board.
 * @return board
 */
Board* CreateEmptyBoard(){
    Board* board = malloc(sizeof(Board));
    board->shouldStop = 0;
    int i, j;
    for(i = BOARD_SIZE - 2; i >= 0; --i){
        for(j = 1; j < BOARD_SIZE - 1; j++){
            board->spots[i][j] = EMPTY_CHAR;
        }
    }
    // put * on margin
    for(i = BOARD_SIZE-1; i >= 0; --i){
        board->spots[BOARD_SIZE - 1][i] = FRAME_CHAR;
    }
    for(i = BOARD_SIZE - 2; i >=0 ; --i) {
        board->spots[i][0] = FRAME_CHAR;
        board->spots[i][BOARD_SIZE - 1] = FRAME_CHAR;
        board->spots[i][BOARD_SIZE] = '\0';
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
        block->points[i].x = X_START_POINT + i;
    }
    block->downMost = 0;
    block->rightMost = X_START_POINT + block->numOfPoints - 1;
    block->leftMost = X_START_POINT;
    return block;
}


/**
 * Create block on top of board.
 * @param numOfPoints 3
 * @return block
 */
Block* CreateFirstBlock(int numOfPoints){
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
    board = CreateEmptyBoard();
    block = CreateFirstBlock(BLOCK_SIZE);

    while(board->shouldStop == 0){
        // when bock is on floor
        if (ShouldRemoveBlock()){
            ClearBlock();
            InitBlock(block);
            DrawBoard();
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
