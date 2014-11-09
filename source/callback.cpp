#include"opencv_training.h"

// cairo_surface_t *surface = NULL;
static GdkWindow *drowable;
static GdkWindow *graphXdrawable = NULL;
static GdkWindow *graphYdrawable = NULL;
static GdkPixbuf *pixbufdata;
static MainDialog *dialog;
static GError *error = NULL;
static int xline = 0;
static int yline = 0;
static int w,h;


static void
draw_display(void)
{
	static cairo_t *cr;
	static cairo_t *line_x;
	static cairo_t *line_y;

	//GdkPixbuf *pixbuf = (GdkPixbuf * ) data;
	//GdkPixbuf *background;
	w = gdk_pixbuf_get_width(pixbufdata);
	h = gdk_pixbuf_get_height(pixbufdata);
	
	// GdkWindow からコンテキストを生成
	cr = gdk_cairo_create (drowable);
	// ソースをpixbufから読み込む
	gdk_cairo_set_source_pixbuf(cr,pixbufdata,0,0);
	// 画像サイズのパスを生成
	cairo_rectangle (cr, 0, 0,w, h);
	// line x, line y のコンテキストを生成
	line_x = cairo_create(cairo_get_target(cr));
	line_y = cairo_create(cairo_get_target(cr));
	// line x, line y のパスを生成
	cairo_rectangle(line_x,0,xline,w,1);
	cairo_rectangle(line_y,yline,0,1,h);
	// それぞれ表示
	cairo_fill (cr);
	cairo_fill(line_x);
	cairo_fill(line_y);
	cairo_destroy(cr);
	cairo_destroy(line_x);
	cairo_destroy(line_y);
}
static void
draw_graph(void)
{
	int i;
	int luminance;
	long line;
	guchar *pixel;
	cairo_t *graphXcr;
	cairo_t *graphYcr;
	cairo_t *whiteXcr;
	cairo_t *whiteYcr;
	// どちらかが宣言されていなければ終了
	if((graphXdrawable == NULL) || (graphYdrawable == NULL))
	{
		return;
	}
	// graph エリアを初期化(white)
	whiteXcr = gdk_cairo_create (graphXdrawable);
	whiteYcr = gdk_cairo_create (graphYdrawable);
	cairo_set_source_rgb(whiteXcr, 0.9, 0.9, 0.9);
	cairo_set_source_rgb(whiteYcr, 0.9, 0.9, 0.9);
	cairo_rectangle(whiteXcr,0,0,w+1,256);
	cairo_rectangle(whiteYcr,0,0,256,h+1);
	cairo_fill(whiteXcr);
	cairo_fill(whiteYcr);
	cairo_destroy(whiteXcr);
	cairo_destroy(whiteYcr);
	// GdkWindow からコンテキストを生成
	graphXcr = gdk_cairo_create (graphXdrawable);
	graphYcr = gdk_cairo_create (graphYdrawable);
	// pixel 取得
	pixel = gdk_pixbuf_get_pixels(pixbufdata);
	// graph X 描画
	for(i = 0;i < w*3;i+=3)
	{
		line = xline * w * 3;
		luminance = (int)(77 * pixel[i + 0 + line] + 150 * pixel[i + 1 + line] + 29 * pixel[i + 2 + line]) >> 8;
		cairo_rectangle (graphXcr, i/3,255 - luminance,2, 2);
	}
	cairo_rectangle(graphXcr,0,0,w,1);
	cairo_rectangle(graphXcr,0,127,w,1);
	cairo_rectangle(graphXcr,0,255,w,1);
	// graph Y 描画
	for(i = 0;i < h*3;i+=3)
	{
		line = yline * 3 + w * i;
		luminance = (int)(77 * pixel[0 + line] + 150 * pixel[1 + line] + 29 * pixel[2 + line]) >> 8;
		cairo_rectangle (graphYcr, 255-luminance,i/3,2, 2);
	}
	cairo_rectangle(graphYcr,0,0,1,h);
	cairo_rectangle(graphYcr,127,0,1,h);
	cairo_rectangle(graphYcr,255,0,1,h);

	cairo_fill(graphXcr);
	cairo_fill(graphYcr);
	cairo_destroy(graphXcr);
	cairo_destroy(graphYcr);
	
//	gtk_widget_set_size_request(dialog->window,w+10+10+255+100,h+10+10);
}
// expose event
gboolean
cb_expose(	GtkWidget *widget,
			GdkEventExpose *event,
			gpointer data)
{
	//GdkPixbuf *pixbuf = (GdkPixbuf * ) data;
	//GdkPixbuf *background;
	dialog = (MainDialog *)data;	
	if(pixbufdata == NULL)
	pixbufdata = (GdkPixbuf *)dialog->pixbuf;	
	w = gdk_pixbuf_get_width(pixbufdata);
	h = gdk_pixbuf_get_height(pixbufdata);

	gtk_widget_set_size_request(dialog->drowingarea,w,h);

//	gtk_widget_set_size_request(dialog->window,w+10+10+255+100,h+10+10);
	drowable = gtk_widget_get_window (widget);
	draw_display();

	return FALSE;
}
// graphX draw event
gboolean
cb_graphX(	GtkWidget *widget,
			GdkEventExpose *event,
			gpointer data)
{
	graphXdrawable = gtk_widget_get_window (widget);

	draw_graph();
	return FALSE;
}
// graphsY draw event
gboolean
cb_graphY(	GtkWidget *widget,
			GdkEventExpose *event,
			gpointer data)
{
	graphYdrawable = gtk_widget_get_window (widget);

	draw_graph();
	return FALSE;
}
// button up イベント
void cb_upButton(GtkWidget *widget, gpointer user_data)
{
	if(xline > 0){
		xline--;
	}
	draw_display();
	draw_graph();
}
void cb_downButton(GtkWidget *widget, gpointer user_data)
{
	if(xline < h){
		xline++;
	}
	draw_display();
	draw_graph();
}
void cb_rightButton(GtkWidget *widget, gpointer user_data)
{
	if(yline < w){
		yline++;
	}
	draw_display();
	draw_graph();
}
void cb_leftButton(GtkWidget *widget, gpointer user_data)
{
	if(yline > 0){
		yline--;
	}
	draw_display();
	draw_graph();
}

void cb_open(GtkAction *action, gpointer user_data)
{
  GtkWidget *window;
  GtkWidget *dialog;
  gint response;

  window = GTK_WIDGET(user_data);
  //ファイル選択ダイアログを作成
  dialog = gtk_file_chooser_dialog_new("Open an image file", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
  //ファイル選択ダイアログを表示
  gtk_widget_show_all(dialog);
  //ファイル選択処理を開始
  response = gtk_dialog_run(GTK_DIALOG(dialog));
  //ファイル選択処理が終了した戻り値を受け取ったら
  if( response == GTK_RESPONSE_ACCEPT){
    gchar *filename;
    //選択したファイル名を取得
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	g_print("%s\n",filename);
    //取得したファイル名から画像を表示
	pixbufdata = gdk_pixbuf_new_from_file(filename,&error);
    //ファイル名取得に確保したメモリ領域を開放
    g_free(filename);
  }
  //ファイル選択ダイアログを閉じる
  gtk_widget_destroy(dialog);
}

void cb_quit(GtkAction *action, gpointer user_data)
{
  GObject *window = G_OBJECT(user_data);
  g_object_unref(g_object_get_data(window, "ui"));
  gtk_main_quit();
}

// マウスイベント
gint 
cb_button_press_event(GtkWidget *widget, GdkEventButton *event)
{
	int x, y, button;
	guint state;

	x = event->x;
	y = event->y;
	state = event->state;
	button = event->button;

	if(button == 1) {
		xline = y;
		yline = x;
		draw_display();
		draw_graph();
	}
	return TRUE;
}
