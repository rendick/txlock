#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <crypt.h>
#include <pthread.h>
#include <pwd.h>
#include <shadow.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define INPUT_LENGTH 512

#define randnum(max, min) ((rand() % ((max) - (min) + 1)) + (min))

// potentially unsafe part of the code (global variables)
unsigned long int bg_color = 0xffffff;
unsigned long int sq_color = 0x000000;
unsigned short int sq_width = 100;
unsigned short int sq_height = 100;

char user_input[INPUT_LENGTH] = {0};
int user_input_length = 0;

int x_rand_square_data[INPUT_LENGTH], y_rand_square_data[INPUT_LENGTH];

int number_of_squares = 0;

void die(char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

int strwid(char *str, XFontStruct *font_struct) {
  return XTextWidth(font_struct, str, strlen(str));
}

int verify_passwd(char *password) {
  struct passwd *pe = getpwnam("root");
  if (!pe)
    die("User does not exist!");
  if (strcmp(pe->pw_passwd, "x") == 0) {
    struct spwd *se = getspnam("root");
    if (!se)
      die("Failed to read shadow!");
    return strcmp(se->sp_pwdp, crypt(password, se->sp_pwdp));
  }
  return strcmp(pe->pw_passwd, crypt(password, pe->pw_passwd));
}

void purge_sq(Display *dpy, Window win, GC gc) {
  printf("UI: %d AND NS: %d\n", user_input_length, number_of_squares);
  user_input[user_input_length - 1] = '\0';
  user_input_length--;

  x_rand_square_data[number_of_squares] = '\0',
  y_rand_square_data[number_of_squares] = '\0';
  number_of_squares--;

  XClearWindow(dpy, win);
  XFlush(dpy);

  for (int i = 0; i < number_of_squares; i++) {
    XFillRectangle(dpy, win, gc, x_rand_square_data[i], y_rand_square_data[i],
                   sq_width, sq_height);
    XFlush(dpy);
  }
}

int lockscreen() {
  srand(time(NULL));

  Display *dpy;
  Window win, root;
  GC gc, wrong_passwd;
  Font font;
  XFontStruct *font_struct;
  int screen;

  // if (getuid())
  //   die("TXlock must be run under root!");

  if ((dpy = XOpenDisplay(NULL)) == NULL)
    die("Cannon open X11 display");

  XSetWindowAttributes attrs;
  attrs.override_redirect = True;
  attrs.background_pixel = bg_color;
  root = DefaultRootWindow(dpy);
  screen = DefaultScreen(dpy);
  win = XCreateWindow(dpy, root, 0, 0, DisplayWidth(dpy, screen),
                      DisplayHeight(dpy, screen), 0, DefaultDepth(dpy, screen),
                      CopyFromParent, DefaultVisual(dpy, screen),
                      CWOverrideRedirect | CWBackPixel, &attrs);
  gc = XCreateGC(dpy, win, 0, NULL);
  wrong_passwd = XCreateGC(dpy, win, 0, NULL);

  font_struct = XLoadQueryFont(dpy, "fixed");

  font = font_struct->fid;
  XSetFont(dpy, wrong_passwd, font);

  XSetForeground(dpy, gc, sq_color);
  XSetFillStyle(dpy, gc, FillSolid);

  XStoreName(dpy, win, "txlock");
  XSelectInput(dpy, win, StructureNotifyMask | KeyPressMask | FocusChangeMask);
  XMapWindow(dpy, win);
  XGrabKeyboard(dpy, win, True, GrabModeAsync, GrabModeAsync, CurrentTime);

  for (;;) {
    XEvent event;
    XNextEvent(dpy, &event);
    printf("%s\n", user_input);
    switch (event.type) {
    case KeyPress:
      KeySym keysym = XLookupKeysym(&event.xkey, 0);
      if ((event.xkey.state & ControlMask) && keysym == XK_u) {
        number_of_squares = 1, user_input_length = 1;
        purge_sq(dpy, win, gc);
        break;
      }
      if (keysym > 32 && keysym < 127 && user_input_length < INPUT_LENGTH) {
        int x_rand_coordinates =
                randnum((((DisplayWidth(dpy, screen) - 1270) / 2) + 1270) - 100,
                        (DisplayWidth(dpy, screen) - 1270) / 2),
            y_rand_coordinates =
                randnum((((DisplayHeight(dpy, screen) - 720) / 2) + 720) - 100,
                        (DisplayHeight(dpy, screen) - 720) / 2);
        x_rand_square_data[number_of_squares] = x_rand_coordinates,
        y_rand_square_data[number_of_squares] = y_rand_coordinates;
        XFillRectangle(dpy, win, gc, x_rand_coordinates, y_rand_coordinates,
                       sq_width, sq_height);
        XFlush(dpy);

        user_input[user_input_length++] = (char)keysym;
        user_input[user_input_length + 1] = '\0';
        number_of_squares++;
        XFlush(dpy);
      } else if (keysym == XK_BackSpace && user_input_length > 0) {
        purge_sq(dpy, win, gc);
      } else if (keysym == XK_Return) {
        if (verify_passwd(user_input) == 0) {
          XCloseDisplay(dpy);
          return EXIT_SUCCESS;
        } else {
          XDrawString(dpy, win, wrong_passwd,
                      (DisplayWidth(dpy, screen) / 2) -
                          strwid("WRONG", font_struct),
                      DisplayHeight(dpy, screen) / 2, "WRONG", strlen("WRONG"));
          XFlush(dpy);
        }
      }
      break;
    case FocusOut:
      XGrabKeyboard(dpy, win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
      break;
    }
  }

  XCloseDisplay(dpy);
}

int main(int argc, char **argv) {
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-bgcolor") == 0) {
      bg_color = strtoul(argv[i + 1], NULL, 0);
    } else if (strcmp(argv[i], "-sqcolor") == 0) {
      sq_color = strtoul(argv[i + 1], NULL, 0);
    } else if (strcmp(argv[i], "-sqwidth") == 0) {
      sq_width = atoi(argv[i + 1]);
    } else if (strcmp(argv[i], "-sqheight") == 0) {
      sq_height = atoi(argv[i + 1]);
    } else if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "-h") == 0) {
      die("-fgcolor\tForeground color\n-bgcolor\tBackground color");
    } else if (strcmp(argv[i], "-version") == 0 || strcmp(argv[i], "-v") == 0) {
      die("txlock v0.1");
    }
  }

  lockscreen();

  return EXIT_SUCCESS;
}
