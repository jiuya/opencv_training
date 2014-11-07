#ifndef OPENCV_TRAINING_H
#define OPENCV_TRAINING_H
#include <iostream>
#include <stdlib.h>
#include <math.h>

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// GTK+-2.0
#include<gtk/gtk.h>
#include<gdk/gdk.h>

// 構造体でウィジットを指定
typedef struct main_dialog_type {
	GtkWidget *window;
	GtkWidget *menuVbox;
	GtkWidget *upButton;
	GtkWidget *rightButton;
	GtkWidget *leftButton;
	GtkWidget *downButton;
	GdkPixbuf *pixbuf;;
	GtkWidget *vbox;
	GtkWidget *imgVbox;
	GtkWidget *hbox;
	GtkWidget *imgHbox;
	GtkWidget *drowingarea;
	GtkWidget *graphX;
	GtkWidget *graphY;
} MainDialog;
// expose event
gboolean cb_expose(	GtkWidget *widget,
					GdkEventExpose *event,
					gpointer data);
// graphX draw event
gboolean
cb_graphX(	GtkWidget *widget,
			GdkEventExpose *event,
			gpointer data);
// graphY draw event
gboolean
cb_graphY(	GtkWidget *widget,
			GdkEventExpose *event,
			gpointer data);

// button up イベント
void cb_upButton(GtkWidget *widget, gpointer user_data);
// button down イベント
void cb_downButton(GtkWidget *widget, gpointer user_data);
// button right イベント
void cb_rightButton(GtkWidget *widget, gpointer user_data);
// button left イベント
void cb_leftButton(GtkWidget *widget, gpointer user_data);

void cb_open(GtkAction *action, gpointer user_data);

void cb_quit(GtkAction *action, gpointer user_data);

#endif
