#include"opencv_training.h"

static const gchar *menu_info =
  "<ui>"
  "  <menubar name='Menubar'>"
  "    <menu name='File' action='File'>"
  "      <menuitem name='Open' action='Open'/>"
  "      <separator/>"
  "      <menuitem name='Quit' action='Quit'/>"
  "    </menu>"
  "  </menubar>"
  "</ui>";

static GtkActionEntry entries[] = {
  {"File", NULL, "_File"},
  {"Open", GTK_STOCK_OPEN, "_Open", "<control>O", "Open an image", G_CALLBACK(cb_open)},
  {"Quit", GTK_STOCK_QUIT, "_Quit", "<control>Q", "Quit this program", G_CALLBACK(cb_quit)},
};


static GtkUIManager* create_menu(GtkWidget *parent)
{
  GtkUIManager *ui;
  GtkActionGroup *actions;
  GError *gErrors = NULL;
  ui = gtk_ui_manager_new();
  actions = gtk_action_group_new("menu");
  gtk_action_group_add_actions(actions, entries, sizeof(entries) / sizeof(entries[0]), parent);
  gtk_ui_manager_insert_action_group(ui, actions, 0);
  gtk_ui_manager_add_ui_from_string(ui, menu_info, -1, NULL);
  gtk_window_add_accel_group(GTK_WINDOW(parent), gtk_ui_manager_get_accel_group(ui));
  return ui;
}
// Main roop
int main(int argc, char ** argv)
{
	GtkUIManager *ui;
	GtkWidget *menubar;
	MainDialog dialog;
	GError *error = NULL;

	gtk_init(&argc,&argv);
	
	// windowの設定
	dialog.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request(dialog.window,300,200);
	gtk_window_set_title(GTK_WINDOW(dialog.window),"画像処理練習");
	gtk_window_set_position(GTK_WINDOW(dialog.window),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(dialog.window),10);
	g_signal_connect(G_OBJECT(dialog.window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
	dialog.menuVbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
	gtk_container_add(GTK_CONTAINER(dialog.window),dialog.menuVbox);
	// mnue ber
	ui = create_menu(dialog.window);
	g_object_set_data(G_OBJECT(dialog.window), "ui", (gpointer)ui);
	menubar = gtk_ui_manager_get_widget(ui, "/Menubar");
	gtk_box_pack_start(GTK_BOX(dialog.menuVbox), menubar, FALSE, FALSE, 0);
	// hbox メインと操作部の分割用のbox
	dialog.hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
	gtk_box_pack_start(GTK_BOX(dialog.menuVbox), dialog.hbox, FALSE, FALSE, 0);

	// 表示用box
	dialog.vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
	gtk_box_pack_start(GTK_BOX(dialog.hbox),dialog.vbox,FALSE,TRUE,0);
	{
		// img用+グラフy用 hbox
		dialog.imgHbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
		gtk_box_pack_start(GTK_BOX(dialog.vbox),dialog.imgHbox,TRUE,TRUE,0);
		// img 用　vbox
		dialog.imgVbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
		gtk_box_pack_start(GTK_BOX(dialog.imgHbox),dialog.imgVbox,FALSE,TRUE,0);
		// img drawing_area
		dialog.drowingarea = gtk_drawing_area_new();
		gtk_box_pack_start(GTK_BOX(dialog.imgVbox),dialog.drowingarea,TRUE,TRUE,0);
		// img pixbuf
		dialog.pixbuf = gdk_pixbuf_new_from_file("../lenna.png",&error);
		gtk_widget_set_size_request(dialog.drowingarea,
									gdk_pixbuf_get_width(dialog.pixbuf),
									gdk_pixbuf_get_height(dialog.pixbuf));
		g_signal_connect(G_OBJECT(dialog.drowingarea),"draw",
									G_CALLBACK(cb_expose),(gpointer)&dialog);
		g_signal_connect(G_OBJECT(dialog.drowingarea), "button_press_event",
				                   G_CALLBACK(cb_button_press_event), NULL);
		gtk_widget_set_events(dialog.drowingarea, GDK_BUTTON_PRESS_MASK);

		// グラフy用 vbox
		dialog.imgVbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
		gtk_box_pack_start(GTK_BOX(dialog.imgHbox),dialog.imgVbox,TRUE,TRUE,5);
		// グラフy用 drawing_area
		dialog.graphY = gtk_drawing_area_new();
		gtk_widget_set_size_request(dialog.graphY,256,0);
		gtk_box_pack_start(GTK_BOX(dialog.imgVbox),dialog.graphY,TRUE,TRUE,0);
		g_signal_connect(G_OBJECT(dialog.graphY),"draw",
									G_CALLBACK(cb_graphY),0);


		// グラフx用 hbox
		dialog.imgHbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
		gtk_box_pack_start(GTK_BOX(dialog.vbox),dialog.imgHbox,TRUE,TRUE,5);
		// グラフx用 drawing_area
		dialog.graphX = gtk_drawing_area_new();
		gtk_widget_set_size_request(dialog.graphX,300,256);
		gtk_box_pack_start(GTK_BOX(dialog.imgHbox),dialog.graphX,TRUE,TRUE,0);
		g_signal_connect(G_OBJECT(dialog.graphX),"draw",
									G_CALLBACK(cb_graphX),0);
	}

	// 操作用box
	dialog.vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
	gtk_box_pack_start(GTK_BOX(dialog.hbox),dialog.vbox,FALSE,TRUE,0);
	// buttonを配置
	{
		dialog.upButton = gtk_button_new_with_label("UP");
		gtk_box_pack_start(GTK_BOX(dialog.vbox),dialog.upButton,FALSE,FALSE,0);
		dialog.downButton = gtk_button_new_with_label("DOWN");
		gtk_box_pack_start(GTK_BOX(dialog.vbox),dialog.downButton,FALSE,FALSE,0);
		dialog.rightButton = gtk_button_new_with_label("RIGHT");
		gtk_box_pack_start(GTK_BOX(dialog.vbox),dialog.rightButton,FALSE,FALSE,0);
		dialog.leftButton = gtk_button_new_with_label("LEFT");
		gtk_box_pack_start(GTK_BOX(dialog.vbox),dialog.leftButton,FALSE,FALSE,0);

		dialog.gray3colorButton = gtk_button_new_with_label("GRAY/3COLOR");
		gtk_box_pack_start(GTK_BOX(dialog.vbox),dialog.gray3colorButton,FALSE,FALSE,0);

		g_signal_connect(G_OBJECT(dialog.upButton), "clicked", G_CALLBACK(cb_upButton), NULL);
		g_signal_connect(G_OBJECT(dialog.downButton), "clicked", G_CALLBACK(cb_downButton), NULL);
		g_signal_connect(G_OBJECT(dialog.rightButton), "clicked", G_CALLBACK(cb_rightButton), NULL);
		g_signal_connect(G_OBJECT(dialog.leftButton), "clicked", G_CALLBACK(cb_leftButton), NULL);
		g_signal_connect(G_OBJECT(dialog.gray3colorButton), "clicked", G_CALLBACK(cb_gray3colorButton), NULL);
	}

	gtk_widget_show_all(dialog.window);
	gtk_main();
	return 0;
}

