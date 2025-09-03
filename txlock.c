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

#define randnum(max, min) ((rand() % ((max) - (min) + 1)) + (min))

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
  GC gc;
  GC remove_sqre;

  char user_input[512] = {0};
  int user_input_length = 0;

  int x_rand_square_data[512], y_rand_square_data[512];
  int number_of_squares = 0;

  if (getuid())
    die("TXlock must be run under root!");

  if ((dpy = XOpenDisplay(NULL)) == NULL)
    die("Cannon open X11 display");

  XSetWindowAttributes attrs;
  attrs.override_redirect = True;

  root = DefaultRootWindow(dpy);
  win = XCreateWindow(dpy, root, 0, 0, DisplayWidth(dpy, DefaultScreen(dpy)),
                      DisplayHeight(dpy, DefaultScreen(dpy)), 0,
                      DefaultDepth(dpy, DefaultScreen(dpy)), CopyFromParent,
                      DefaultVisual(dpy, DefaultScreen(dpy)),
                      CWOverrideRedirect | CWBackPixel, &attrs);
  gc = XCreateGC(dpy, win, 0, NULL);
  remove_sqre = XCreateGC(dpy, win, 0, NULL);

  XSetForeground(dpy, remove_sqre, 0x000000);
  XSetFillStyle(dpy, remove_sqre, FillSolid);
  XSetForeground(dpy, gc, 0xffffff);
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
      if (keysym > 32 && keysym < 127) {
        int x_rand_coordinates = randnum(1920, 200),
            y_rand_coordinates = randnum(1080, 200);
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

        XDrawRectangle(dpy, win, remove_sqre,
                       x_rand_square_data[number_of_squares - 1],
                       y_rand_square_data[number_of_squares - 1], 100, 100);
        x_rand_square_data[number_of_squares] = '\0',
        y_rand_square_data[number_of_squares] = '\0';
        number_of_squares--;

        XFlush(dpy);
      } else if (keysym == XK_Return) {
        if (verify_passwd(user_input) == 0) {
          XCloseDisplay(dpy);

          return EXIT_SUCCESS;
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
