#include <X11/Xlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* gfx_open creates several X11 objects, and stores them in these globals
   the globals are used by the other functions in the library.  */

static Display *gfx_display=0;
static Display *disp=0;  // added Nov 5, 2015
static Window  gfx_window;
static GC      gfx_gc;
static Colormap gfx_colormap;
static int      gfx_fast_color_mode = 0;

/* These values are saved by gfx_wait then retrieved later by gfx_xpos and gfx_ypos. */
static int saved_xpos = 0;
static int saved_ypos = 0;

/* Open a new graphics window. */
void gfx_open( int width, int height, const char *title )
{
   gfx_display = XOpenDisplay(0);
   if(!gfx_display) {
      fprintf(stderr, "gfx_open: unable to open the graphics window.\n");
      exit(1);
   }

   Visual *visual = DefaultVisual(gfx_display,0);
   if(visual && visual->class==TrueColor) {
      gfx_fast_color_mode = 1;
   } else {
      gfx_fast_color_mode = 0;
   }

   int blackColor = BlackPixel(gfx_display, DefaultScreen(gfx_display));
   int whiteColor = WhitePixel(gfx_display, DefaultScreen(gfx_display));

   gfx_window = XCreateSimpleWindow(gfx_display, DefaultRootWindow(gfx_display),
                0, 0, width, height, 0, blackColor, blackColor);

   XSetWindowAttributes attr;
   attr.backing_store = Always;

   XChangeWindowAttributes(gfx_display,gfx_window,CWBackingStore,&attr);
   XStoreName(gfx_display,gfx_window,title);
   XSelectInput(gfx_display, gfx_window, StructureNotifyMask|
                  KeyPressMask|KeyReleaseMask|
                  ButtonPressMask|ButtonReleaseMask|
                  PointerMotionMask);
   XMapWindow(gfx_display,gfx_window);
   gfx_gc = XCreateGC(gfx_display, gfx_window, 0, 0);
   gfx_colormap = DefaultColormap(gfx_display, 0);
   XSetForeground(gfx_display, gfx_gc, whiteColor);

   /* Wait for the MapNotify event */
   for(;;) {
      XEvent e;
      XNextEvent(gfx_display, &e);
      if (e.type == MapNotify)
         break;
   }
}

/* Flush all previous output to the window. */
void gfx_flush()
{
   XFlush(gfx_display);
}

/* Change the current drawing color. */
void gfx_color( int r, int g, int b )
{
   XColor color;

   if(gfx_fast_color_mode) {
      /* If this is a truecolor display, we can just pick the color directly. */
      color.pixel = ((b&0xff) | ((g&0xff)<<8) | ((r&0xff)<<16) );
   } else {
      /* Otherwise, we have to allocate it from the colormap of the display. */
      color.pixel = 0;
      color.red = r<<8;
      color.green = g<<8;
      color.blue = b<<8;
      XAllocColor(gfx_display, gfx_colormap, &color);
   }

   XSetForeground(gfx_display, gfx_gc, color.pixel);
}

/* Clear the graphics window to the background color. */
void gfx_clear()
{
   XClearWindow(gfx_display,gfx_window);
}

/* Change the current background color. */
void gfx_clear_color( int r, int g, int b )
{
   XColor color;
   color.pixel = 0;
   color.red = r<<8;
   color.green = g<<8;
   color.blue = b<<8;
   XAllocColor(gfx_display, gfx_colormap, &color);

   XSetWindowAttributes attr;
   attr.background_pixel = color.pixel;
   XChangeWindowAttributes(gfx_display, gfx_window, CWBackPixel, &attr);
}

/* See if an event has occurred */
int gfx_event_waiting()
{
   XEvent event;

   gfx_flush();
   while (1) {
      if(XCheckMaskEvent(gfx_display, -1, &event)) {
        if(event.type==KeyPress) {
            XPutBackEvent(gfx_display,&event);
            return 1;
         } else if (event.type==KeyRelease) {
            XPutBackEvent(gfx_display,&event);
            return 2;
         } else if(event.type==ButtonPress) {
            XPutBackEvent(gfx_display,&event);
            return 3;
         } else if (event.type==ButtonRelease) {
            XPutBackEvent(gfx_display,&event);
            return 4;
         } else if (event.type==MotionNotify) {
            XPutBackEvent(gfx_display,&event);
            return 5;
         } else {
            return 0;
         }
      } else {
         return 0;
      }
   }
}

/* Wait for the user to press a key or mouse button. */
char gfx_wait()
{
   XEvent event;

   gfx_flush();
   while(1) {
      XNextEvent(gfx_display,&event);

      if(event.type==KeyPress) {
         saved_xpos = event.xkey.x;
         saved_ypos = event.xkey.y;
         return XLookupKeysym(&event.xkey,0);
      } else if(event.type==KeyRelease) {
         saved_xpos = event.xkey.x;
         saved_ypos = event.xkey.y;
         return XLookupKeysym(&event.xkey,0);
      } else if(event.type==ButtonPress) {
         saved_xpos = event.xkey.x;
         saved_ypos = event.xkey.y;
         return event.xbutton.button;
      } else if(event.type==ButtonRelease) {
         saved_xpos = event.xkey.x;
         saved_ypos = event.xkey.y;
         return event.xbutton.button;
      } else if(event.type==MotionNotify) {
         saved_xpos = event.xkey.x;
         saved_ypos = event.xkey.y;
         return event.xbutton.button;
      }
   }
}

/* Return the X coordinate of the last event. */
int gfx_xpos()
{
   return saved_xpos;
}

/* Return the  Y coordinate of the last event. */
int gfx_ypos()
{
   return saved_ypos;
}

/* get width and height of the screen */
int gfx_xsize()
{
   return XDisplayWidth(gfx_display, 0);
}

int gfx_ysize()
{
   return XDisplayHeight(gfx_display, 0);
}

void gfx_text( int x, int y , char *text )
{
   XDrawString(gfx_display, gfx_window, gfx_gc, x, y, text, strlen(text));
}

/* Draw a single point at (x,y) */
void gfx_point( int x, int y )
{
   XDrawPoint(gfx_display, gfx_window, gfx_gc, x, y);
}

/* Draw a line from (x1,y1) to (x2,y2) */
void gfx_line( int x1, int y1, int x2, int y2 )
{
   XDrawLine(gfx_display, gfx_window, gfx_gc, x1, y1, x2, y2);
}

/* Draw a circle centered at (xCtr,yCtr) and with radius r */
void gfx_circle( int xCtr, int yCtr, int r )
{
   XDrawArc(gfx_display, gfx_window, gfx_gc, xCtr-r, yCtr-r, 2*r, 2*r, 0, 360*64);
}

/* Draw an ellipse centered at (xCtr,yCtr) and with radii r1 and r2 */
void gfx_ellipse( int xCtr, int yCtr, int r1, int r2 )
{
   XDrawArc(gfx_display, gfx_window, gfx_gc, xCtr-r1, yCtr-r2, 2*r1, 2*r2, 0, 360*64);
}

/* Draw an arc whose top left corner of its bounding rectangle is at (xc,yc),
 * with width w and height h, starting at angle a1 and sweeping an angle of a2 (degrees);
 * a1 is at the 3 O'Clock position, and the a2 sweep is positive counter-clockwise */
void gfx_arc( int xc, int yc, int w, int h, int a1, int a2 )
{
   XDrawArc(gfx_display, gfx_window, gfx_gc, xc, yc, w, h, a1*64, a2*64);
}

/* Draw a filled arc; similar to gfx_arc */
void gfx_fill_arc( int xc, int yc, int w, int h, int a1, int a2 )
{
   XFillArc(gfx_display, gfx_window, gfx_gc, xc, yc, w, h, a1*64, a2*64);
}

/* Draw a rectangle with top-left corner at (x,y) with width w and height h */
void gfx_rectangle( int x, int y, int w, int h )
{
   XDrawRectangle(gfx_display, gfx_window, gfx_gc, x, y, w, h);
}

/* Draw a filled rectangle; similar to gfx_rectangle */
void gfx_fill_rectangle( int x, int y, int w, int h )
{
   XFillRectangle(gfx_display, gfx_window, gfx_gc, x, y, w, h);
}

/* Draw a polygon; makes use of Xlib's XPoint struct;
 *   the num_pts points are in the pointsarr array *
 *   (if the last point is the same as the first point,
 *   the polygon will be a closed one) */
void gfx_polygon( XPoint *pointsarr, int num_pts )
{
   XDrawLines(gfx_display, gfx_window, gfx_gc, pointsarr, num_pts, CoordModeOrigin);
}

/* Draw a filled polygon */
void gfx_fill_polygon( XPoint *pointsarr, int num_pts )
{
   XFillPolygon(gfx_display, gfx_window, gfx_gc,
                    pointsarr, num_pts, Complex, CoordModeOrigin);
}

/* Changes the text font that will be used by gfx_text() */
void gfx_changefont(char *font_str)
{
   Font font;
   font = XLoadFont(gfx_display, font_str);
   XSetFont(gfx_display, gfx_gc, font);
}

/* Get the width and the height of the screen (monitor)
 *   (can be used prior to opening a window */
int gfx_screenwidth()
{
   disp = XOpenDisplay(0);
   int wid = XDisplayWidth(disp,DefaultScreen(disp));
   XCloseDisplay(disp);
   return wid;
}

int gfx_screenheight()
{
   disp = XOpenDisplay(0);
   int ht = XDisplayHeight(disp, DefaultScreen(disp));
   XCloseDisplay(disp);
   return ht;
}

// new functions added 11/14/15:

/* Get the width and the height of the display window
 *   (useful in case the display window has been resized */
int gfx_windowwidth()
{
  XWindowAttributes attr;
  XGetWindowAttributes(gfx_display,gfx_window,&attr);
  return attr.width;
}

int gfx_windowheight()
{
  XWindowAttributes attr;
  XGetWindowAttributes(gfx_display,gfx_window,&attr);
  return attr.height;
}

/* get the red, green, or blue value of the current display's color */
// note: these three color functions can give inconsistent results on
// various platforms; it is much better to use global variables for
// your R,G,B colors to get the desired results.

int gfx_getred()
{
   XColor c;
   XQueryColor(gfx_display, gfx_colormap, &c);
   return c.red/256;
}

int gfx_getgreen()
{
   XColor c;
   XQueryColor(gfx_display, gfx_colormap, &c);
   return c.green/256;
}

int gfx_getblue()
{
   XColor c;
   XQueryColor(gfx_display, gfx_colormap, &c);
   return c.blue/256;
}

/* find the pixel width of a string (text) in a given font (fontname) */
int gfx_textpixelwidth(char *text, char *fontname)
{
  int n;
  XFontStruct *font_info;
  font_info = XLoadQueryFont(gfx_display,fontname);
  n = XTextWidth(font_info,text,strlen(text));
  return n;
}

/* find the pixel height of a string (text) in a given font (fontname) */
int gfx_textpixelheight(char *text, char *fontname)
{
  int n;
  XFontStruct *font_info;
  font_info = XLoadQueryFont(gfx_display,fontname);
  n = font_info->ascent + font_info->descent;
  return n;
}

/* Draw a filled circle centered at (xCtr,yCtr) and with radius r */
void gfx_fill_circle(int xCtr, int yCtr, int r)
{ 
  gfx_fill_arc(xCtr-r, yCtr-r, 2*r, 2*r, 0, 360);
}

// new functions added 11/29/15:

/* clear an area whose top left corner is at x,y and with width w and height h */
void gfx_cleararea(int x, int y, int w, int h)
{
   XClearArea(gfx_display, gfx_window, x, y, w, h, 1);
}

/* change the cursor (mouse pointer)
 * see the file /usr/include/X11/cursorfont.h for possible cursors */
void gfx_changecursor(int cursorcode)
{
   Cursor mycursor = XCreateFontCursor(gfx_display, cursorcode);
   XDefineCursor(gfx_display, gfx_window, mycursor);
}

