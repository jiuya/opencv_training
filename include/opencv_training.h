#ifndef OPENCV_TRAINING_H
#define OPENCV_TRAINING_H
//#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// GTK+-3.0
#include<gtk/gtk.h>
#include<gdk/gdk.h>
#include<cairo.h>

// 構造体でウィジットを指定
typedef struct main_dialog_type {
	GtkWidget *window;
	// Box & Button
	GtkWidget *menuVbox;
	GtkWidget *upButton;
	GtkWidget *rightButton;
	GtkWidget *leftButton;
	GtkWidget *downButton;
	GtkWidget *gray3colorButton;
	GtkWidget *switchingFFTlumButton;
	GdkPixbuf *pixbuf;;
	GtkWidget *vbox;
	GtkWidget *imgVbox;
	GtkWidget *hbox;
	GtkWidget *imgHbox;
	GtkWidget *lowImgBox;
	GtkWidget *highImgBox;
	GtkWidget *outImgBox;
	// Drowingarea
	GtkWidget *scaleImgVbox;
	GtkWidget *drowingarea;
	GtkWidget *graphX;
	GtkWidget *graphY;
	GtkWidget *scaleImg;
	// Spinbutton
	GtkWidget *lowThresholdSpin;
	GtkWidget *highThresholdSpin;
	GtkWidget *lowNumberOfOpenSpin;
	GtkWidget *highNumberOfOpenSpin;
	GtkWidget *lowNumberOfDilate;
	GtkWidget *highNumberOfDilate;
	GtkWidget *lowNumberOfErode;
	GtkWidget *highNumberOfErode;
	GtkWidget *outNumberOfOpenSpin;
	// Label
	GtkWidget *highThresholdLabel;
	GtkWidget *lowThresholdLabel;
	GtkWidget *numberOfOpenLabel;
	GtkWidget *choiceOperatorLabel;
	GtkWidget *numberOfBlockLabel;
	GtkWidget *lowNumberOfDilateLabel;
	GtkWidget *highNumberOfDilateLabel;
	GtkWidget *lowNumberOfErodeLabel;
	GtkWidget *highNumberOfErodeLabel;
	//GtkWidget *switchingMorphologyLabel;

	// Combobox
	GtkWidget *choiceOperatorCombobox;
	GtkWidget *lowNumberOfBlockCombobox;
	GtkWidget *highNumberOfBlockCombobox;
	GtkWidget *lowSwitchingMorphologyCombobox;
	GtkWidget *highSwitchingMorphologyCombobox;
	GtkWidget *outSwitchingMorphologyCombobox;
	// Frame
	GtkWidget *lowImgFrame;
	GtkWidget *highImgFrame;
	GtkWidget *outImgFrame;
	// Grid
	GtkWidget *buttonGrid;
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
// scaleimg draw event
gboolean
cb_scaleImg(GtkWidget *widget,
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
// button gray/3color 切り替え イベント
void cb_gray3colorButton(GtkWidget *widget, gpointer user_data);
// button FFT 輝度断面切り替え　イベント
void cb_switchingFFTlumButton(GtkWidget *widget, gpointer user_data);

void cb_open(GtkAction *action, gpointer user_data);

void cb_quit(GtkAction *action, gpointer user_data);

gint cb_button_press_event(GtkWidget *widget, GdkEventButton *event);

void cb_lowThreshold_changed(GtkSpinButton *spinbutton, gpointer data);

void cb_highThreshold_changed(GtkSpinButton *spinbutton, gpointer data);

void cb_lowNumberOfOpen_changed(GtkSpinButton *spinbutton, gpointer data);

void cb_highNumberOfOpen_changed(GtkSpinButton *spinbutton, gpointer data);

void cb_choiceOperator_changed(GtkComboBox *spinbutton, gpointer data);

void cb_lowNumberOfBlock_changed(GtkComboBox *spinbutton, gpointer data);

void cb_highNumberOfBlock_changed(GtkComboBox *spinbutton, gpointer data);

void cb_lowSwitchingMorphology_changed(GtkComboBox *spinbutton, gpointer data);

void cb_highSwitchingMorphology_changed(GtkComboBox *spinbutton, gpointer data);

void cb_lowNumberOfDilate_changed(GtkSpinButton *spinbutton, gpointer data);

void cb_highNumberOfDilate_changed(GtkSpinButton *spinbutton, gpointer data);

void cb_lowNumberOfErode_changed(GtkSpinButton *spinbutton, gpointer data);

void cb_highNumberOfErode_changed(GtkSpinButton *spinbutton, gpointer data);

void cb_outNumberOfOpen_changed(GtkSpinButton *spinbutton, gpointer data);

void cb_outSwitchingMorphology_changed(GtkComboBox *spinbutton, gpointer data);
#endif
