

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <string.h>

#include "texty.h"

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

  FILE *file = fopen("config.txt", "r");

  //handle configuration file
  if (!file) {
    return;
  }

  char optionOne[3] = "000";
  char optionTwo[3] = "000";
  for(int i = 0; i < 3; i++) {
    optionOne[i] = fgetc(file);
    if(optionOne[i] == '\n' || optionOne[i] == '\r' || optionOne[i] == EOF) {
      return;
    }
  }

  for(int i = 0; i < 3; i++) {
    optionTwo[i] = fgetc(file);
    if(optionTwo[i] == '\n' || optionTwo[i] == '\r' || optionTwo[i] == EOF) {
      break;
    }
  }

  fclose(file);

  handleConfigOptions(optionOne);
  handleConfigOptions(optionTwo);



}

void handleConfigOptions(char option[3]) {
  if(option[0] == '-' ) {
    if(option[1] == 'f') {
      switch (option[2]) {
        case 'd':
          break;
        case 'r':
          write(STDOUT_FILENO, "\x1b[31m", 5);
          break;
        case 'g':
          write(STDOUT_FILENO, "\x1b[32m", 5);
          break;
        case 'y':
          write(STDOUT_FILENO, "\x1b[33m", 5);
          break;
        case 'b':
          write(STDOUT_FILENO, "\x1b[34m", 5);
          break;
        case 'm':
          write(STDOUT_FILENO, "\x1b[35m", 5);
          break;
        case 'c':
          write(STDOUT_FILENO, "\x1b[36m", 5);
          break;
        case 'w':
          write(STDOUT_FILENO, "\x1b[37m", 5);
          break;
        
      }
    }
    if(option[1] == 'b') {
      switch (option[2]) {
        case 'd':
          write(STDOUT_FILENO, "\x1b[40m", 5);
          break;
        case 'r':
          write(STDOUT_FILENO, "\x1b[41m", 5);
          break;
        case 'g':
          write(STDOUT_FILENO, "\x1b[42m", 5);
          break;
        case 'y':
          write(STDOUT_FILENO, "\x1b[43m", 5);
          break;
        case 'b':
          write(STDOUT_FILENO, "\x1b[44m", 5);
          break;
        case 'm':
          write(STDOUT_FILENO, "\x1b[45m", 5);
          break;
        case 'c':
          write(STDOUT_FILENO, "\x1b[46m", 5);
          break;
        case 'w':
          write(STDOUT_FILENO, "\x1b[47m", 5);
          break;
    }
  }
  }
}
//taken from tutorial
void startRawMode(void) {
    if(tcgetattr(STDIN_FILENO, &original_termios) == -1) error("tcgetattr");
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
}

void drawRows(void) {
  int maxNumRoom = log10(rowList.numRows) + 2;
  for(int i = 0; i < windowInformation.rows -1; i++) {
    int rowNumber = i + windowInformation.rowOffset;
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
  //TODO

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
  fclose(file);
}

int main(int argc, char *argv[]) {
  initializeTexty();
  startRawMode();
  if(argc >= 2) {
    loadFile(argv[1]);
  }

  while (1) {
    refreshScreen();
    processKeyboardInput();
  }
}
