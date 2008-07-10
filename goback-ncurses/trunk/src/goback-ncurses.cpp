//#include <libgoback.h>
#include <iostream>
#include <DataSource.h>
#include <WorkMode.h>

#include <curses.h>

// command mode like vim
bool cmdmode=false;
// insert mode like vim
bool insertmode=false;


/*
 * Initialize ncurses environtment.
 */
void init_ncurses() {
	initscr();
	if (has_colors() == FALSE) {
		endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}
	keypad(stdscr, TRUE);
	//(void) nonl();      /* tell curses not to do NL->CR/NL on output */
	cbreak();
	noecho();
	//nodelay(stdscr, TRUE);
	//idlok(stdscr, TRUE);    /* allow use of insert/delete line */
	start_color();
	if (use_default_colors() == OK)
		init_pair(1, COLOR_RED, -1);

	attron(COLOR_PAIR(1));
}

/*
 * Destroy ncurses environtment.
 */
void close_ncurses() {
	attroff(COLOR_PAIR(1));
	endwin();
}

/*
 * Print a line.
 */
void print_line(ViewLine line, int numline) {
	// Print the line number
	printw("%08x  ", numline);
	// Print the blocks
	ViewLine::iterator j = line.begin();
	while (j != line.end()) {
		printw("%s", j->_str.c_str());
		j++;
	}
}

/*
 * Exec a command.
 */
bool exec_command(char *str) {
	return true;
}

/*
 * Print main view.
 */
void print_view(WorkMode *wm, int numlines, int startline) {
	ViewLine line;
	int i;
	char str[256];
	// Print all the lines generated by the WorkMode
	for (i = 0; i < numlines && i < LINES; i++) {
		line = wm->getLine(i + startline);
		move(i, 0);
		print_line(line, i+startline);
	}
	
	move(i-1, 0);
	if (cmdmode) {
		printw(":");
		clrtoeol();
		echo();
		refresh();
		getstr(str);
		exec_command(str);
		noecho();
		cmdmode=false;
		move(i, 0);
		print_line(line,i+startline);
	}
	
	refresh();
}

int main(int argc, char *argv[]) {
	int key;
	int startline = 0;

	if (argc != 2){
		std::cout << "Usage: " << argv[0] << " file" << std::endl;
		exit(-1);
	}

	DataSource *ds = DataSource::create("file");
	if (!ds->open(argv[1])) {
		perror("open");
		delete ds;
		return 1;
	}

	WorkMode *wm = WorkMode::create("hex", ds);
	int numlines = wm->getNumberLines();

	init_ncurses();

	print_view(wm, numlines, 0);
	for (;;) {
		key = getch();
		switch (key) {
		case KEY_DOWN:
			(startline == numlines-LINES) ? numlines : startline++;
			break;
		case KEY_UP:
			(startline == 0) ? 0 : startline--;
			break;
		case KEY_NPAGE:
			(startline + LINES*2 > numlines) ? startline=numlines-LINES : startline += LINES;
			break;
		case KEY_PPAGE:
			(startline - LINES < 0) ? startline=0 : startline -= LINES;
			break;
		case ':':
			cmdmode=true;
			break;
		case 'q':
			goto exit;
			break;
		}
		print_view(wm, numlines, startline);
	}

	exit:
	close_ncurses();
	ds->close();
	return 0;
}
