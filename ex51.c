//
// Created by Shlomo on 03/06/19.
//

#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>


#define ERROR "Error in system call\n"
#define DRAW_FILENAME "./draw.out"
#define ERROR_EXIT_STATUS 1





/**
 * listen to keyboard.
 *
 * @return char
 */
char GetChar() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror ("tcsetattr ~ICANON");
    return (buf);
}


void UpdateNewChar(int sonPid, int writeFd, char c){
    kill(sonPid, SIGUSR2);
    char buffer[] = {c};
    write(writeFd, buffer, sizeof(buffer));
}


/**
 * Check if key is legal.
 * @param c  key pressed
 * @return if it legal
 */
int IsLegalChar(char c){
    switch (c){
        case 'q':
        case 'a':
        case 'd':
        case 's':
        case 'w':
            return 1;
        default:
            return 0;
    }
}


/**
 * father task:
 *
 * @param sonPid
 * @param writeFd
 */
void ListenToKeyboard(int sonPid, int writeFd) {
    char pushed = 0;
    while(pushed != 'q'){
        pushed = GetChar();
        if (IsLegalChar(pushed)){
            UpdateNewChar(sonPid, writeFd, pushed);
        }
    }
    UpdateNewChar(sonPid, writeFd, 'q');
}


/**
 * run ex52.c, listen to keyboard
 * and send instruction via pipes.
 */
int main() {
    int pid;
    int fd[2];
    pipe(fd);

    // error
    if ((pid = fork()) < 0){
        write(2, ERROR, strlen(ERROR));
        exit(ERROR_EXIT_STATUS);
    }

    // child run exr52.c (after compilation)
    else if (pid == 0){
        dup2(fd[0], 0);
        close(fd[1]);
        execlp (DRAW_FILENAME, DRAW_FILENAME, NULL);

        // father listen to keyboard
    } else {
        close(fd[0]);
        ListenToKeyboard(pid, fd[1]);
    }
    return 0;
}