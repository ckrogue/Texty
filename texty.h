#include <stdlib.h>

/// @brief type for storing direction cursor is moving
enum Direction {
  LEFT,
  RIGHT,
  UP,
  DOWN
};

/// @brief puts the terminal into raw input and output mode
/// @param  void
void startRawMode(void);

/// @brief quits the program and sends an error message to the terminal
/// @param s the part of the program causing the error
void error(const char *s);

/// @brief adds a row of text to the row list
/// @param line the text to be added
/// @param len  length of the line to be added
void addRow(char *line, size_t len);

/// @brief draws all the rows in the row list to the terminal. adds on
///        row numbers to the front
/// @param void 
void drawRows(void);

/// @brief opens a file and puts all of its context into rowList
/// @param fileName name of the text file
void loadFile(char *fileName);

/// @brief exits the program cleanly
/// @param void
void cleanExit(void);

/// @brief updates the cursors position so that it will be drawn in the righ place
/// @param direction direction in which the cursor will move
void moveCursor(enum Direction direction);

/// @brief turns the terminal the appropriate color based on what option is passed in
/// @param option a command from the config file
void handleConfigOptions(char option[3]);

/// @brief initializes internal state
/// @param void
void initializeTexty(void);

/// @brief displaces the welcome screen NOTE TODO
/// @param void
void displayWelcomeScreen(void);

/// @brief Redraws the screen in the terminal
/// @param void
void refreshScreen(void);

/// @brief Processes user input from the keyboard and updates internal state
/// @param void
void processKeyboardInput(void);
