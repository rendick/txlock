#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <crypt.h>
#include <pwd.h>
#include <shadow.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define INPUT_LENGTH 512

#define randnum(max, min) ((rand() % ((max) - (min) + 1)) + (min))

unsigned long int bg_color = 0xffffff;
unsigned long int sqre_color = 0x000000;

void die(char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
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

int main(void) {
  srand(time(NULL));

  Display *dpy;
  Window win, root;
  GC gc, wrong_passwd;
  int screen;

  char user_input[INPUT_LENGTH] = {0};
  int user_input_length = 0;

  int x_rand_square_data[INPUT_LENGTH], y_rand_square_data[INPUT_LENGTH];
  int number_of_squares = 0;

  if (getuid())
    die("TXlock must be run under root!");

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

  XSetForeground(dpy, gc, sqre_color);
  XSetFillStyle(dpy, gc, FillSolid);

  XStoreName(dpy, win, "txlock");
  XSelectInput(dpy, win, StructureNotifyMask | KeyPressMask);
  XMapWindow(dpy, win);
  XGrabKeyboard(dpy, win, True, GrabModeAsync, GrabModeAsync, CurrentTime);

  for (;;) {
    XEvent event;
    XNextEvent(dpy, &event);
    switch (event.type) {
    case KeyPress:
      KeySym keysym = XLookupKeysym(&event.xkey, 0);
      if (keysym > 32 && keysym < 127 && user_input_length < INPUT_LENGTH) {
        int x_rand_coordinates =
                randnum((((DisplayWidth(dpy, screen) - 1270) / 2) + 1270) - 100,
                        (DisplayWidth(dpy, screen) - 1270) / 2),
            y_rand_coordinates =
                randnum((((DisplayHeight(dpy, screen) - 720) / 2) + 720) - 100,
                        (DisplayHeight(dpy, screen) - 720) / 2);
        x_rand_square_data[number_of_squares] = x_rand_coordinates,
        y_rand_square_data[number_of_squares] = y_rand_coordinates;

        XDrawRectangle(dpy, win, gc, x_rand_coordinates, y_rand_coordinates,
                       100, 100);

        user_input[user_input_length++] = (char)keysym;
        user_input[user_input_length + 1] = '\0';
        number_of_squares++;
        XFlush(dpy);
      } else if (keysym == XK_BackSpace && user_input_length > 0) {
        user_input[user_input_length - 1] = '\0';
        user_input_length--;

        x_rand_square_data[number_of_squares] = '\0',
        y_rand_square_data[number_of_squares] = '\0';
        number_of_squares--;

        XClearWindow(dpy, win);
        XFlush(dpy);

        for (int i = 0; i < number_of_squares; i++) {
          XDrawRectangle(dpy, win, gc, x_rand_square_data[i],
                         y_rand_square_data[i], 100, 100);
          XFlush(dpy);
        }
      } else if (keysym == XK_Return) {
        if (verify_passwd(user_input) == 0) {
          XCloseDisplay(dpy);
          return EXIT_SUCCESS;
        } else {
          XDrawString(dpy, win, wrong_passwd, 100, 100, "WRONG",
                      strlen("WRONG"));
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

  return EXIT_SUCCESS;
}
