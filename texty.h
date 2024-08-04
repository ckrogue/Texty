void startRawMode(void);

/// @brief quits program with an exit code one. clean exit that returns terminal
///        to its previous state
/// @param s message to go before the error code message
void error(const char *s);

void initializeTexty(void);
void displayWelcomeScreen(void);
void refreshScreen(void);
void processKeyboardInput(void);
