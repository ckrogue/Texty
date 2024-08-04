

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <string.h>

#include "texty.h"

struct termios original_termios;

struct row {
  char *text;
  int length;
};
struct rowList {
  struct row *rows;
  int numRows;
}; 
struct rowList rowList;
char *contents = NULL;


void initializeTexty(void) {
  rowList.rows = NULL;
  rowList.numRows = 0;
}
//taken from tutorial
void startRawMode(void) {
    if(tcgetattr(STDIN_FILENO, &original_termios) == -1) error("tcgetattr");
    //atexit(disableRawMode);

    struct termios raw = original_termios;
    raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | ICRNL | IXON);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cflag |= (CS8);
    raw.c_oflag &= ~(OPOST);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) error("tcsetattr");
}

void error(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}

void addRow(char *line, size_t len) {
  rowList.rows = realloc(rowList.rows, sizeof(struct row) * (rowList.numRows + 1));
  int index = rowList.numRows;
  rowList.rows[index].length = len;
  rowList.rows[index].text = malloc(len + 3);
  memcpy(rowList.rows[index].text, line, len);
  rowList.rows[index].text[len] = '\0';
  rowList.numRows++;
  //write(STDOUT_FILENO, rowList.rows[index].text, rowList.rows[index].length);
}

void drawRows(void) {

  int maxNumRoom = log10(rowList.numRows) + 2;
  for(int i = 0; i < rowList.numRows; i++) {
    //int rowNumLen = log10(rowList.rows[i].length);
    int rowNumLen;
    if(i == 0) {
      rowNumLen = 1;
    }
    else {
      rowNumLen = log10(i +1) + 1;
    }
    char rowNum[maxNumRoom];
    sprintf(rowNum, "%d", i+1); 

    for(int j = rowNumLen; j < maxNumRoom; j++) {
      rowNum[j] = ' ';
    }
    write(STDOUT_FILENO, rowNum, maxNumRoom);
    write(STDOUT_FILENO, rowList.rows[i].text, rowList.rows[i].length);
    write(STDOUT_FILENO, "\r", 1);
  }
}

void displayWelcomeScreen(void) {
  //rowList = realloc(rowList, sizeof(struct row) * (rowList.numRows + 1));

}

void refreshScreen(void) {

}

void processKeyboardInput(void) {
  int rread;
  char c;

  while ((rread = read(STDIN_FILENO, &c, 1)) != 1) {
    if(rread == -1 && errno != EAGAIN) error("read");
  }
  if(c == 'q') {
    exit(0);
  }

}

void loadFile(char *fileName) {
  FILE *file = fopen(fileName, "r");
  if (!file) error("fopen");
  
  char *line =  NULL;
  size_t linecap = 0;
  ssize_t len;
  while((len = getline(&line, &linecap, file)) != -1) {
    addRow(line, len);
  }
  //write(STDOUT_FILENO, line, 10);
  
  
}


int main(int argc, char *argv[]) {
  initializeTexty();
  startRawMode();
  if (argc >= 2) {
    loadFile(argv[1]);
    drawRows();
  }

  else {
    displayWelcomeScreen();
  }

  while (1) {
    refreshScreen();
    processKeyboardInput();
  }
}
