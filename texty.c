

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

enum Direction {
  LEFT,
  RIGHT,
  UP,
  DOWN
};
struct termios original_termios;

struct cursorCoords {
  int x;
  int y;
};
struct cursorCoords cursorCoords;

struct windowInformation {
  int cols;
  int rows;
  int rowOffset;
};
struct windowInformation windowInformation;

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
  struct winsize ws;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
  windowInformation.cols = ws.ws_col;
  windowInformation.rows = ws.ws_row;
  windowInformation.rowOffset = 0;
  rowList.rows = NULL;
  rowList.numRows = 0;
  cursorCoords.x = 0;
  cursorCoords.y = 0;
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
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
  perror(s);
  exit(1);
}

void addRow(char *line, size_t len) {
  rowList.rows = realloc(rowList.rows, sizeof(struct row) * (rowList.numRows + 1));
  int index = rowList.numRows;
  rowList.rows[index].length = len;
  rowList.rows[index].text = malloc(len + 1);
  memcpy(rowList.rows[index].text, line, len);
  rowList.rows[index].text[len] = '\0';
  rowList.numRows++;
  //write(STDOUT_FILENO, rowList.rows[index].text, rowList.rows[index].length);
}

void drawRows(void) {
  int maxNumRoom = log10(rowList.numRows) + 2;
  for(int i = 0; i < windowInformation.rows -1; i++) {

    int rowNumber = i + windowInformation.rowOffset;
    //int rowNumLen = log10(rowList.rows[i].length);
    int rowNumLen;
    if(i == 0) {
      rowNumLen = 1;
    }
    else {
      rowNumLen = log10(rowNumber +1) + 1;
    }
    char rowNum[maxNumRoom];
    sprintf(rowNum, "%d", rowNumber+1); 

    for(int j = rowNumLen; j < maxNumRoom; j++) {
      rowNum[j] = ' ';
    }
    write(STDOUT_FILENO, rowNum, maxNumRoom);
    write(STDOUT_FILENO, "\x1b[K", 3);
    write(STDOUT_FILENO, rowList.rows[rowNumber].text, rowList.rows[rowNumber].length);
    write(STDOUT_FILENO, "\r", 2);
  }
}

void displayWelcomeScreen(void) {
  //rowList = realloc(rowList, sizeof(struct row) * (rowList.numRows + 1));

}
void moveCursor(enum Direction direction) {
  switch (direction) {
    case LEFT:
      if(cursorCoords.x > 0) {
        cursorCoords.x--;
      }
      break;
    case RIGHT:
      if(cursorCoords.x < windowInformation.rows - 1) {
        cursorCoords.x++;
      }
      break;
    case UP:
      if(cursorCoords.y > 0) {
        cursorCoords.y--;
      }
      break;
    case DOWN:
      if(cursorCoords.y < rowList.numRows - 1) {
        cursorCoords.y++;
      }
      break;
  }
}

void cleanExit(void) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
  exit(0);
}

void refreshScreen(void) {
  if(cursorCoords.y > windowInformation.rows + windowInformation.rowOffset) {
    windowInformation.rowOffset++;
    cursorCoords.y = windowInformation.rows + windowInformation.rowOffset;
  }
  write(STDOUT_FILENO, "\x1b[31m", 5);
  write(STDOUT_FILENO, "\x1b[?25l", 6);
  write(STDOUT_FILENO, "\x1b[H", 3);
  drawRows();
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", cursorCoords.y + 1, cursorCoords.x + 1);
  write(STDOUT_FILENO, buf, strlen(buf));
  write(STDOUT_FILENO, "\x1b[?25h]", 6);

}

void processKeyboardInput(void) {
  int rread;
  char c;

  while ((rread = read(STDIN_FILENO, &c, 1)) != 1) {
    if(rread == -1 && errno != EAGAIN) error("read");
  }
  if(c == 'q') {
    cleanExit();
  }
  if(c == '\x1b') {
    char keyPresses[2];
    if(read(STDIN_FILENO, &keyPresses[0], 1) != 1) return;
    if(read(STDIN_FILENO, &keyPresses[1], 1) != 1) return;

    if(keyPresses[0] == '[') {
      switch (keyPresses[1]) {
        case 'A':
          moveCursor(UP);
          break;
        case 'B':
          moveCursor(DOWN);
          break;
        case 'C':
          moveCursor(RIGHT);
          break;
        case 'D':
          moveCursor(LEFT);
          break;
      }

    }
  }

}

void loadFile(char *fileName) {
  FILE *file = fopen(fileName, "r");
  if (!file) error("fopen");
  
  char *line =  NULL;
  size_t linecap = 0;
  ssize_t len;
  while((len = getline(&line, &linecap, file)) != -1) {
    while(len > 0  && (line[len - 1] == 'n' || line[len-1] == 'r')) {
      len--;
    }
    addRow(line, len);
  }
  //write(STDOUT_FILENO, line, 10);
  
  
}
void handleArguments(int argc, char * argv[]) {
  char red[3] = "red";
  for (int i = 0; i < argc; i++) {
    switch (argv[i]) {
      case red:
        break;
    }

  }

}


int main(int argc, char *argv[]) {
  initializeTexty();
  startRawMode();
  if (argc >= 2) {
    loadFile(argv[1]);
  }

  else {
    displayWelcomeScreen();
  }

  while (1) {
    refreshScreen();
    processKeyboardInput();
  }
}
