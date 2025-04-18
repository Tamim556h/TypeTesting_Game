#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define MAX_TEXT 1000
#define TIME_LIMIT 60  // 60 seconds time limit
#define NUM_LEVELS 4

const char *levels[NUM_LEVELS] = {"Easy", "Medium", "Hard" , "Exit"};

void read_text_from_file(char *buffer, size_t size, int level);
void save_high_score(int wpm);
int select_level();
void welcome_screen();

int main() {
    char text[MAX_TEXT];
    char user_input[MAX_TEXT];
    int level = 1, i = 0, correct_count = 0, error_count = 0;
    size_t start, current;
    int remaining_time = TIME_LIMIT;
    int rows, cols; // Store terminal size

    // Stats Calculation
    int total_chars = correct_count + error_count;
    int time_taken = current - start;
    int wpm = (correct_count / 5) / ((time_taken > 0 ? time_taken : 1) / 60.0);


    initscr();
    if (!has_colors()) {
        endwin();
        return 0;
    }
    start_color();
    noecho();
    cbreak(); // Line buffering disabled
    keypad(stdscr, TRUE); // Enable function keys

    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLACK, COLOR_CYAN);
    init_pair(5, COLOR_WHITE, COLOR_BLACK);
    init_pair(6, COLOR_BLUE, COLOR_BLACK);
    init_pair(7, COLOR_CYAN, COLOR_BLACK);

    getmaxyx(stdscr, rows, cols); // Get terminal size

    // Welcome screen
    welcome_screen(); 
    attron(COLOR_PAIR(7) | A_BOLD);
    mvprintw(rows / 2 + 8, (cols - 30) / 2, " Press any key to continue...");
    attroff(COLOR_PAIR(7) | A_BOLD);
    attron(COLOR_PAIR(3) | A_BOLD);
    char choice = getch();
    clear();
    if(choice == 27 || choice == '\x1B'){
        clear();
        endwin();
        return 0;
    }

   // **Level Selection**
    level = select_level();
    if (level == -1) {  // Exit if ESC is pressed
        endwin();
        return 0;
    }
    clear();
    
    // Read text based on difficulty
    read_text_from_file(text, MAX_TEXT, level);

    // Display Text
    mvprintw(rows / 2 - 3, (cols - 23) / 2, "Type the following text:\n ");
    attroff(COLOR_PAIR(3) | A_BOLD);
    attron(COLOR_PAIR(5) | A_BOLD);

    //waddstr(stdscr,text);
    mvprintw(rows / 2 -2 , 0, "%s", text);
    attroff(COLOR_PAIR(5) | A_BOLD);

    refresh();
    start = time(NULL);
    // mvprintw(rows / 2 +1, (cols - 22) / 2, "Start typing:");
    move(rows / 2 +4 , (cols - strlen(text)) / 2 );
    refresh();

    // Typing Process with Countdown Timer
    while (i < strlen(text)) {
        current = time(NULL);
        remaining_time = TIME_LIMIT - (current - start);

        nodelay(stdscr, TRUE);

        // **Handle Text Overflow (Move to Next Line if Needed)**
            int line_num = i / cols;  // Which line the character is on
            int y = (rows / 2 - 2) + line_num;  // Move down when wrapping
            int x = (i % cols);  // Reset x on a new line


        // Display Remaining Time at the Top
        attron(COLOR_PAIR(4) );
        mvprintw(rows / 2 - 5, cols /2 -10 , "Time Left: %d sec", remaining_time);
        attroff(COLOR_PAIR(4));
        refresh();

        if (remaining_time <= 0) break;  // Stop when time is up

        int ch = getch();
        if (ch == '\n') break; // Enter key to finish typing
        if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) { // Handle Backspace
            if (i > 0) {
                i--;
                int prev_x = i % cols;
                int prev_y = (rows / 2 - 2) + (i / cols);
                mvprintw(prev_y, prev_x, "%c", text[i]); 
                move(prev_y, prev_x);
            }

            continue;
        }
        if(ch == 27 || ch == '\x1B'){
            clear();
            endwin();
            return 0;
        }

        if (ch != ERR) {
            user_input[i] = ch;
            if (ch == text[i]) {
                attron(COLOR_PAIR(2) | A_BOLD);
                correct_count++;
            } else {
                attron(COLOR_PAIR(1) | A_BOLD);
                error_count++;
            }
            mvprintw(y, x, "%c", ch);
            
            attroff(COLOR_PAIR(1) | COLOR_PAIR(2) | A_BOLD);
            refresh();
            i++;
        }
        
        total_chars = correct_count + error_count;
        time_taken = current - start;
        wpm = (correct_count / 5) / ((time_taken > 0 ? time_taken : 1) / 60.0);



        attron(COLOR_PAIR(7) | A_BOLD);
        mvprintw(rows -8, 3, "Correct Characters: %d", correct_count);
        mvprintw(rows -7, 3, "Errors: %d", error_count);
        mvprintw(rows -6, 3, "Accuracy: %.2f%%", (correct_count * 100.0) / total_chars);
        mvprintw(rows -5, 3, "Characters Per Second (CPS): %.2f", total_chars / (float)(time_taken > 0 ? time_taken : 1));
        mvprintw(rows -4, 3, "Words Per Minute (WPM): %d", wpm);
        mvprintw(rows -3, 3, "Time Taken: %d sec", time_taken);
        attron(COLOR_PAIR(7) | A_BOLD);

    }
    nodelay(stdscr, FALSE);
    user_input[i] = '\0';
    current = time(NULL);

    clear();
    attron(COLOR_PAIR(4));
    mvprintw(rows / 2 - 4, (cols - 30) / 2, " Game Over! Here are your stats: ");
    attroff(COLOR_PAIR(4));

    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(rows / 2 - 2, (cols-30) / 2, "Correct Characters: %d", correct_count);
    mvprintw(rows / 2 - 1, (cols -30 ) / 2, "Errors: %d", error_count);
    mvprintw(rows / 2, (cols -30) / 2, "Accuracy: %.2f%%", (correct_count * 100.0) / total_chars);
    mvprintw(rows / 2 + 1, (cols -30 ) / 2, "Characters Per Second (CPS): %.2f", total_chars / (float)(time_taken > 0 ? time_taken : 1));
    mvprintw(rows / 2 + 2, (cols -30 ) / 2, "Words Per Minute (WPM): %d", wpm);
    mvprintw(rows / 2 + 3, (cols -30) / 2, "Time Taken: %d sec", time_taken);
    attron(COLOR_PAIR(5) | A_BOLD);

    save_high_score(wpm);

    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(rows / 2 + 5, (cols - 36) / 2, "Press 'Enter' to Continue or 'Esc' to Exit.");
    attroff(COLOR_PAIR(3));
    refresh();

    choice = getch();
    for( ; choice != 27 && choice != '\x1B'  ;){
        if (choice == '\n' || choice == 10) {
            clear();
            main(); // Restart the game
            clear();
            endwin();
            return 0;

        }
        else if ( choice == 27 || choice =='\x1B'){
            clear();
            endwin();
            return 0;
        }
        else{
            choice = getch();
        }
    }
    clear();
    endwin();
    return 0;
}

// Function to read text from a file
void read_text_from_file(char *buffer, size_t size, int level) {
    FILE *file = fopen("input.txt", "r");
    if (!file) {
        strcpy(buffer, "Error: Could not open word file.");
        return;
    }

    char easy_words[500][50], medium_words[500][50], hard_words[500][50];
    int easy_count = 0, medium_count = 0, hard_count = 0;
    char word[50];

    // Read words and categorize them
    while (fscanf(file, "%49s", word) == 1) {
        int len = strlen(word);

        if (len >= 1 && len <= 4) {
            strcpy(easy_words[easy_count++], word);
        } else if (len >= 5 && len <= 7) {
            strcpy(medium_words[medium_count++], word);
        } else if (len >= 8) {
            strcpy(hard_words[hard_count++], word);
        }
    }
    fclose(file);

    if ((level == 1 && easy_count == 0) || (level == 2 && medium_count == 0) || (level == 3 && hard_count == 0)) {
        strcpy(buffer, "Error: No valid words for this level.");
        return;
    }

    int num_words = (level == 1) ? 20 : (level == 2) ? 40 : 60; // Adjust passage length
    char (*word_list)[50] = (level == 1) ? easy_words : (level == 2) ? medium_words : hard_words;
    int word_count = (level == 1) ? easy_count : (level == 2) ? medium_count : hard_count;

    srand(time(NULL));
    buffer[0] = '\0';

    for (int i = 0; i < num_words; i++) {
        int random_index = rand() % word_count;
        strcat(buffer, word_list[random_index]);
        if (i < num_words - 1) strcat(buffer, " ");
    }
}
// **Interactive Level Selection**
int select_level() {
    int highlight = 0;
    int key;
    int rows, cols; // Store terminal size

    getmaxyx(stdscr, rows, cols); 

    while (1) {
        clear();
        mvprintw(rows / 2 - 3, (cols - 18) / 2, "Select Your Level:");
        for (int i = 0; i < NUM_LEVELS; i++) {
            if (i == highlight) attron(A_REVERSE);
            mvprintw( (rows/2) - 1 + i, (cols - 10) / 2, "%d. %s ", i + 1, levels[i]);
            if (i == highlight) attroff( A_REVERSE);
        }
        refresh();

        key = getch();
        switch (key) {
            case KEY_UP:
                highlight = (highlight == 0) ? NUM_LEVELS - 1 : highlight - 1;
                break;
            case KEY_DOWN:
                highlight = (highlight == NUM_LEVELS - 1) ? 0 : highlight + 1;
                break;
            case 10: 
                if (highlight == 3) {  // If user selects "Exit"
                    endwin();
                    exit(0);  // Exit the game
                }
                return highlight + 1; // Return level (1-3)
            case 27: 
                endwin();
                exit(0);

        }
    }
}

void save_high_score(int wpm) {
    FILE *file = fopen("highscores.txt", "a");
    if (file) {
        fprintf(file, "WPM: %d\n", wpm);
        fclose(file);
    }
}

void welcome_screen( ){
    
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    
    attron(COLOR_PAIR(7) | A_BOLD);
    mvprintw(rows / 2 - 6, (cols - 30) / 2, "**********************************");
    mvprintw(rows / 2 - 5, (cols - 30) / 2, "*      TYPING SPEED TEST GAME    *");
    mvprintw(rows / 2 - 4, (cols - 30) / 2, "**********************************");

    mvprintw(rows/2 , (cols -30)/2 -6,   " W   W  EEEEE  L      CCCCC  OOOOO  M   M  EEEEE ");
    mvprintw(rows/2 +1, (cols -30)/2 -6, " W   W  E      L     C       O   O  MM MM  E     ");
    mvprintw(rows/2 +2, (cols -30)/2 -6, " W W W  EEEE   L     C       O   O  M M M  EEEE  ");
    mvprintw(rows/2 +3, (cols -30)/2 -6, " WW WW  E      L     C       O   O  M   M  E     ");
    mvprintw(rows/2 +4, (cols -30)/2 -6, " W   W  EEEEE  LLLLL  CCCCC  OOOOO  M   M  EEEEE ");
    attroff(COLOR_PAIR(7) | A_BOLD);
}