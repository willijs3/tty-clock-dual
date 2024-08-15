#include <assert.h>
#include <errno.h>
#include <locale.h>
#include <ncurses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* Default window size */
#define SECFRAMEW 58
#define NORMFRAMEW 38
#define DATEWINH 1

typedef struct {
    unsigned int x, y;
    unsigned int a, b;
    unsigned int w, h;
} geo_t;

typedef struct {
    int running;
    int option_second;
    int option_blink;
    int option_center;
    int option_box;
    int option_bold;
    int option_utc;
    int option_rebound;
    int option_date;
    int option_color;
    struct timespec option_sleep;
    int bg;
    time_t lt;
    struct tm *tm;
    char tty[32];
    char date[32];
    WINDOW *framewin;
    WINDOW *second_framewin;
    geo_t geo;
    geo_t geo_second;
} ttyclock_t;

ttyclock_t ttyclock;

void update_hour(void);
void draw_number(int n, int x, int y);
void signal_handler(int sig);
void set_center(int center);
void clock_move(int x, int y, int w, int h);
void clock_rebound(void);
void key_event(void);
void draw_clock(void);

void
init(void)
{
    struct sigaction sig;
    setlocale(LC_TIME, "");

    ttyclock.bg = COLOR_BLACK;

    /* Init ncurses */
    if (ttyclock.tty[0]) {
        FILE *ftty = fopen(ttyclock.tty, "r+");
        if (!ftty) {
            fprintf(stderr, "tty-clock: error: '%s' couldn't be opened: %s.\n",
                    ttyclock.tty, strerror(errno));
            exit(EXIT_FAILURE);
        }
        ttyclock.ttyscr = newterm(NULL, ftty, ftty);
        assert(ttyclock.ttyscr != NULL);
        set_term(ttyclock.ttyscr);
    } else
        initscr();

    cbreak();
    noecho();
    keypad(stdscr, true);
    start_color();
    curs_set(false);
    clear();

    /* Init default terminal color */
    if (use_default_colors() == OK)
        ttyclock.bg = -1;

    /* Init color pair */
    init_pair(0, ttyclock.bg, ttyclock.bg);
    init_pair(1, ttyclock.bg, ttyclock.option_color);
    init_pair(2, ttyclock.option_color, ttyclock.bg);
    refresh();

    /* Init signal handler */
    sig.sa_handler = signal_handler;
    sig.sa_flags = 0;
    sigaction(SIGTERM, &sig, NULL);
    sigaction(SIGINT, &sig, NULL);
    sigaction(SIGSEGV, &sig, NULL);

    /* Init global struct */
    ttyclock.running = true;
    ttyclock.option_second = 1;
    ttyclock.option_blink = 1;
    ttyclock.option_center = 1;
    ttyclock.option_box = 1;
    ttyclock.option_bold = 0;
    ttyclock.option_utc = 0;
    ttyclock.option_rebound = 0;
    ttyclock.option_date = 0;
    ttyclock.option_color = COLOR_GREEN;
    ttyclock.option_sleep.tv_sec = 1;
    ttyclock.option_sleep.tv_nsec = 0;

    if (!ttyclock.geo.x)
        ttyclock.geo.x = 0;
    if (!ttyclock.geo.y)
        ttyclock.geo.y = 0;
    if (!ttyclock.geo.a)
        ttyclock.geo.a = 1;
    if (!ttyclock.geo.b)
        ttyclock.geo.b = 1;
    ttyclock.geo.w = (ttyclock.option_second) ? SECFRAMEW : NORMFRAMEW;
    ttyclock.geo.h = 7;

    // Second clock geo setup
    ttyclock.geo_second.x = ttyclock.geo.x + ttyclock.geo.h + 1;
    ttyclock.geo_second.y = ttyclock.geo.y;
    ttyclock.geo_second.w = ttyclock.geo.w;
    ttyclock.geo_second.h = ttyclock.geo.h;

    ttyclock.lt = time(NULL);
    ttyclock.tm = localtime(&(ttyclock.lt));
    if (ttyclock.option_utc) {
        ttyclock.tm = gmtime(&(ttyclock.lt));
    }
    update_hour();

    /* Create clock win */
    ttyclock.framewin = newwin(ttyclock.geo.h,
                               ttyclock.geo.w,
                               ttyclock.geo.x,
                               ttyclock.geo.y);

    // Create second clock window
    ttyclock.second_framewin = newwin(ttyclock.geo_second.h,
                                      ttyclock.geo_second.w,
                                      ttyclock.geo_second.x,
                                      ttyclock.geo_second.y);

    if (ttyclock.option_box) {
        box(ttyclock.framewin, 0, 0);
        box(ttyclock.second_framewin, 0, 0);
    }

    if (ttyclock.option_bold) {
        wattron(ttyclock.framewin, A_BLINK);
        wattron(ttyclock.second_framewin, A_BLINK);
    }

    set_center(ttyclock.option_center);
    nodelay(stdscr, true);

    wrefresh(ttyclock.framewin);
    wrefresh(ttyclock.second_framewin);

    return;
}

void
update_hour(void)
{
    ttyclock.lt = time(NULL);
    if (ttyclock.option_utc) {
        ttyclock.tm = gmtime(&(ttyclock.lt));
    } else {
        ttyclock.tm = localtime(&(ttyclock.lt));
    }
}

void
draw_number(int n, int x, int y)
{
    /* Here would be the code to draw each number based on `n` */
    /* Use waddch or mvwaddch to draw in the frame window */
}

void
signal_handler(int sig)
{
    if (sig == SIGTERM || sig == SIGINT) {
        ttyclock.running = 0;
    }
}

void
set_center(int center)
{
    if (center) {
        ttyclock.geo.x = (LINES - ttyclock.geo.h) / 2;
        ttyclock.geo.y = (COLS - ttyclock.geo.w) / 2;
    } else {
        ttyclock.geo.x = ttyclock.geo.a;
        ttyclock.geo.y = ttyclock.geo.b;
    }
}

void
clock_move(int x, int y, int w, int h)
{
    /* Code for moving the clock */
}

void
clock_rebound(void)
{
    /* Code for rebound effect, if any */
}

void
key_event(void)
{
    /* Code to handle key events */
}

void
draw_clock(void)
{
    if (ttyclock.option_date && !ttyclock.option_rebound &&
        strcmp(ttyclock.date, ttyclock.date) != 0) {
        clock_move(ttyclock.geo.x,
                   ttyclock.geo.y,
                   ttyclock.geo.w,
                   ttyclock.geo.h);
    }

    /* Draw hour numbers */
    draw_number(ttyclock.tm->tm_hour / 10, 1, 1);
    draw_number(ttyclock.tm->tm_hour % 10, 1, 8);

    chtype dotcolor = COLOR_PAIR(1);
    if (ttyclock.option_blink && time(NULL) % 2 == 0)
        dotcolor = COLOR_PAIR(2);

    /* 2 dot for number separation */
    wbkgdset(ttyclock.framewin, dotcolor);
    mvwaddstr(ttyclock.framewin, 2, 16, "  ");
    mvwaddstr(ttyclock.framewin, 4, 16, "  ");

    /* Draw minute numbers */
    draw_number(ttyclock.tm->tm_min / 10, 1, 20);
    draw_number(ttyclock.tm->tm_min % 10, 1, 27);

    /* Draw second if the option is enabled */
    if (ttyclock.option_second) {
        /* Again 2 dot for number separation */
        wbkgdset(ttyclock.framewin, dotcolor);
        mvwaddstr(ttyclock.framewin, 2, NORMFRAMEW, "  ");
        mvwaddstr(ttyclock.framewin, 4, NORMFRAMEW, "  ");

        /* Draw second numbers */
        draw_number(ttyclock.tm->tm_sec / 10, 1, 39);
        draw_number(ttyclock.tm->tm_sec % 10, 1, 46);
    }

    // Draw second clock
    if (ttyclock.option_second) {
        draw_number(ttyclock.tm->tm_hour / 10, 1, 1);
        draw_number(ttyclock.tm->tm_hour % 10, 1, 8);
        wbkgdset(ttyclock.second_framewin, dotcolor);
        mvwaddstr(ttyclock.second_framewin, 2, 16, "  ");
        mvwaddstr(ttyclock.second_framewin, 4, 16, "  ");
        draw_number(ttyclock.tm->tm_min / 10, 1, 20);
        draw_number(ttyclock.tm->tm_min % 10, 1, 27);
    }

    wrefresh(ttyclock.framewin);
    wrefresh(ttyclock.second_framewin);
}

int
main(int argc, char **argv)
{
    int c;

    ttyclock.tty[0] = '\0';

    while ((c = getopt(argc, argv, "iuvsScbtrhBxnDC:f:d:T:a:")) != -1)
    {
        switch (c) {
        case 's':
            ttyclock.option_second = 0;
            break;
        case 'S':
            ttyclock.option_blink = 0;
            break;
        case 'c':
            ttyclock.option_center = 0;
            break;
        case 'b':
            ttyclock.option_box = 0;
            break;
        case 'B':
            ttyclock.option_bold = 1;
            break;
        case 'u':
            ttyclock.option_utc = 1;
            break;
        case 'r':
            ttyclock.option_rebound = 1;
            break;
        case 'h':
            /* print help and exit */
            printf("Usage: %s [-iuvsScbtrhBxnDC:f:d:T:a:]\n", argv[0]);
            exit(EXIT_SUCCESS);
        case 'C':
            ttyclock.option_color = atoi(optarg);
            break;
        case 'f':
            strcpy(ttyclock.tty, optarg);
            break;
        case 'd':
            strcpy(ttyclock.date, optarg);
            break;
        case 'T':
            ttyclock.option_date = 1;
            break;
        case 'a':
            ttyclock.geo.a = atoi(optarg);
            ttyclock.geo.b = atoi(optarg);
            break;
        case 'n':
            ttyclock.option_second = 1;
            ttyclock.option_blink = 1;
            ttyclock.option_center = 1;
            ttyclock.option_box = 1;
            ttyclock.option_bold = 0;
            ttyclock.option_utc = 0;
            ttyclock.option_rebound = 0;
            ttyclock.option_date = 0;
            ttyclock.option_color = COLOR_GREEN;
            ttyclock.option_sleep.tv_sec = 1;
            ttyclock.option_sleep.tv_nsec = 0;
            break;
        default:
            /* print help and exit */
            printf("Usage: %s [-iuvsScbtrhBxnDC:f:d:T:a:]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    init();

    while (ttyclock.running)
    {
        key_event();
        draw_clock();
        if (ttyclock.option_rebound)
            clock_rebound();
        nanosleep(&ttyclock.option_sleep, NULL);
    }

    endwin();

    return 0;
}
