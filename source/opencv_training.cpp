#include"../include/opencv_training.h"

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
	GtkListStore    *store;
	GtkTreeIter      iter;
	GtkCellRenderer *cell;

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
		dialog.scaleImgVbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
		gtk_box_pack_start(GTK_BOX(dialog.imgHbox),dialog.scaleImgVbox,TRUE,TRUE,0);

		dialog.graphX = gtk_drawing_area_new();
		gtk_widget_set_size_request(dialog.graphX,300,256);
		gtk_box_pack_start(GTK_BOX(dialog.scaleImgVbox),dialog.graphX,TRUE,TRUE,0);
		g_signal_connect(G_OBJECT(dialog.graphX),"draw",
									G_CALLBACK(cb_graphX),0);
		// 拡大・縮小用 vbox
		dialog.scaleImgVbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
		gtk_box_pack_start(GTK_BOX(dialog.imgHbox),dialog.scaleImgVbox,FALSE,FALSE,5);
		
		dialog.scaleImg = gtk_drawing_area_new();
		gtk_widget_set_size_request(dialog.scaleImg,256,256);
		gtk_box_pack_start(GTK_BOX(dialog.scaleImgVbox),dialog.scaleImg,TRUE,TRUE,0);
		g_signal_connect(G_OBJECT(dialog.scaleImg),"draw",
									G_CALLBACK(cb_scaleImg),0);


	}

	// 操作用box
	dialog.vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
	gtk_box_pack_start(GTK_BOX(dialog.hbox),dialog.vbox,FALSE,TRUE,0);
	// buttonを配置
	{
		// gridを作成
		dialog.buttonGrid = gtk_grid_new();
		gtk_box_pack_start(GTK_BOX(dialog.vbox),dialog.buttonGrid,FALSE,FALSE,0);

		dialog.upButton = gtk_button_new_with_label("↑");
		//gtk_box_pack_start(GTK_BOX(dialog.vbox),dialog.upButton,FALSE,FALSE,0);
		gtk_grid_attach(GTK_GRID(dialog.buttonGrid),dialog.upButton,1,0,1,1);
		dialog.downButton = gtk_button_new_with_label("↓");
		gtk_grid_attach(GTK_GRID(dialog.buttonGrid),dialog.downButton,1,2,1,1);
		//gtk_box_pack_start(GTK_BOX(dialog.vbox),dialog.downButton,FALSE,FALSE,0);
		dialog.rightButton = gtk_button_new_with_label("→");
		gtk_grid_attach(GTK_GRID(dialog.buttonGrid),dialog.rightButton,2,1,1,1);
		//gtk_box_pack_start(GTK_BOX(dialog.vbox),dialog.rightButton,FALSE,FALSE,0);
		dialog.leftButton = gtk_button_new_with_label("←");
		gtk_grid_attach(GTK_GRID(dialog.buttonGrid),dialog.leftButton,0,1,1,1);
		//gtk_box_pack_start(GTK_BOX(dialog.vbox),dialog.leftButton,FALSE,FALSE,0);
		// ３色からの輝度グラフを表示するのと３色それぞれのグラフを表示するのを切り替えるボタン
		// on-off に切り替えるかも
		dialog.gray3colorButton = gtk_button_new_with_label("GRAY/3COLOR");
		gtk_box_pack_start(GTK_BOX(dialog.vbox),dialog.gray3colorButton,FALSE,FALSE,0);
		// グラフに表示する内容を切り替える（輝度断面＜ー＞FFT）
		// on-off に切り替えるかも
		dialog.switchingFFTlumButton = gtk_button_new_with_label("輝度/FFT");
		gtk_box_pack_start(GTK_BOX(dialog.vbox),dialog.switchingFFTlumButton,FALSE,FALSE,0);
		
		/*** Low 操作用***/
		dialog.lowImgFrame = gtk_frame_new("Low");
		gtk_box_pack_start(GTK_BOX(dialog.vbox), dialog.lowImgFrame, FALSE, FALSE, 0);
		dialog.lowImgBox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
		gtk_container_add(GTK_CONTAINER(dialog.lowImgFrame), dialog.lowImgBox);
		
		// Opening Closing 切り替えCombobox
		store = gtk_list_store_new( 1, G_TYPE_STRING );
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, "Closing", -1 );
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, "Opening", -1 );
		/* Create combo box with store as data source. */
		dialog.lowSwitchingMorphologyCombobox= gtk_combo_box_new_with_model( GTK_TREE_MODEL( store ) );
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(dialog.lowSwitchingMorphologyCombobox),&iter);
		gtk_box_pack_start(GTK_BOX(dialog.lowImgBox),dialog.lowSwitchingMorphologyCombobox , FALSE, FALSE, 0);
		/* Create cell renderer. */
		cell = gtk_cell_renderer_text_new();
		/* Pack it into the combo box. */
		gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( dialog.lowSwitchingMorphologyCombobox ), cell, TRUE );
		/* Connect renderer to data source. */
		gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( dialog.lowSwitchingMorphologyCombobox ), cell, "text", 0, NULL );



		// 下限ラベル
		dialog.lowThresholdLabel = gtk_label_new("閾値");
		gtk_box_pack_start(GTK_BOX(dialog.lowImgBox), dialog.lowThresholdLabel, FALSE, FALSE, 0);
		//gtk_container_add(GTK_CONTAINER(dialog.lowImgFrame), dialog.lowThresholdLabel);
		
		// ２値化の下限決定SpinButton
		dialog.lowThresholdSpin = gtk_spin_button_new_with_range(0,255,1);
		// 値の桁数を指定
		gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dialog.lowThresholdSpin), 0);
		//上限値と下限値を設ける
		gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(dialog.lowThresholdSpin), TRUE);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog.lowThresholdSpin),60);
		gtk_box_pack_start(GTK_BOX(dialog.lowImgBox),dialog.lowThresholdSpin , FALSE, FALSE, 0);
		
		// 膨張回数決定label
		dialog.lowNumberOfDilateLabel = gtk_label_new("膨張回数");
		gtk_box_pack_start(GTK_BOX(dialog.lowImgBox), dialog.lowNumberOfDilateLabel, FALSE, FALSE, 0);
		
		//膨張回数決定SpinButton
		dialog.lowNumberOfDilate = gtk_spin_button_new_with_range(0,255,1);
		// 値の桁数を指定
		gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dialog.lowNumberOfDilate), 0);
		//上限値と下限値を設ける
		gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(dialog.lowNumberOfDilate), TRUE);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog.lowNumberOfDilate),1);
		gtk_box_pack_start(GTK_BOX(dialog.lowImgBox),dialog.lowNumberOfDilate , FALSE, FALSE, 0);
		
		// 収縮回数決定label
		dialog.lowNumberOfErodeLabel = gtk_label_new("収縮回数");
		gtk_box_pack_start(GTK_BOX(dialog.lowImgBox), dialog.lowNumberOfErodeLabel, FALSE, FALSE, 0);
		
		// 収縮回数決定SpinButton
		dialog.lowNumberOfErode = gtk_spin_button_new_with_range(0,255,1);
		// 値の桁数を指定
		gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dialog.lowNumberOfErode), 0);
		//上限値と下限値を設ける
		gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(dialog.lowNumberOfErode), TRUE);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog.lowNumberOfErode),1);
		gtk_box_pack_start(GTK_BOX(dialog.lowImgBox),dialog.lowNumberOfErode , FALSE, FALSE, 0);	

		// 試行回数を決定SpinButton
		// 試行回数ラベル
		dialog.numberOfOpenLabel = gtk_label_new("試行回数");
		//gtk_box_pack_start(GTK_BOX(dialog.vbox), dialog.numberOfOpenLabel, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(dialog.lowImgBox),dialog.numberOfOpenLabel , FALSE, FALSE, 0);

		//２値化の上限決定SpinButton
		dialog.lowNumberOfOpenSpin = gtk_spin_button_new_with_range(0,10,1);
		// 値の桁数を指定
		gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dialog.lowNumberOfOpenSpin), 0);
		//上限値と下限値を設ける
		gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(dialog.lowNumberOfOpenSpin), TRUE);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog.lowNumberOfOpenSpin),1);
		gtk_box_pack_start(GTK_BOX(dialog.lowImgBox),dialog.lowNumberOfOpenSpin , FALSE, FALSE, 0);

		// ブロック数の決定Label
		dialog.numberOfOpenLabel = gtk_label_new("ブロック数");
		gtk_box_pack_start(GTK_BOX(dialog.lowImgBox), dialog.numberOfOpenLabel, FALSE, FALSE, 0);

		// ブロック数を決定ComboBox
		store = gtk_list_store_new( 1, G_TYPE_STRING );
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, "5", -1 );
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, "3", -1 );
		/* Create combo box with store as data source. */
		dialog.lowNumberOfBlockCombobox= gtk_combo_box_new_with_model( GTK_TREE_MODEL( store ) );
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(dialog.lowNumberOfBlockCombobox),&iter);
		gtk_box_pack_start(GTK_BOX(dialog.lowImgBox),dialog.lowNumberOfBlockCombobox , FALSE, FALSE, 0);
		/* Create cell renderer. */
		cell = gtk_cell_renderer_text_new();
		/* Pack it into the combo box. */
		gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( dialog.lowNumberOfBlockCombobox ), cell, TRUE );
		/* Connect renderer to data source. */
		gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( dialog.lowNumberOfBlockCombobox ), cell, "text", 0, NULL );

		/*** High 操作用***/
		dialog.highImgFrame = gtk_frame_new("High");
		gtk_box_pack_start(GTK_BOX(dialog.vbox), dialog.highImgFrame, FALSE, FALSE, 0);
		dialog.highImgBox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
		gtk_container_add(GTK_CONTAINER(dialog.highImgFrame), dialog.highImgBox);

		// Opening Closing 切り替えCombobox
		store = gtk_list_store_new( 1, G_TYPE_STRING );
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, "Closing", -1 );
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, "Opening", -1 );
		/* Create combo box with store as data source. */
		dialog.highSwitchingMorphologyCombobox= gtk_combo_box_new_with_model( GTK_TREE_MODEL( store ) );
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(dialog.highSwitchingMorphologyCombobox),&iter);
		gtk_box_pack_start(GTK_BOX(dialog.highImgBox),dialog.highSwitchingMorphologyCombobox , FALSE, FALSE, 0);
		/* Create cell renderer. */
		cell = gtk_cell_renderer_text_new();
		/* Pack it into the combo box. */
		gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( dialog.highSwitchingMorphologyCombobox ), cell, TRUE );
		/* Connect renderer to data source. */
		gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( dialog.highSwitchingMorphologyCombobox ), cell, "text", 0, NULL );
	

		// 上限ラベル
		dialog.highThresholdLabel = gtk_label_new("閾値");
		gtk_box_pack_start(GTK_BOX(dialog.highImgBox), dialog.highThresholdLabel, FALSE, FALSE, 0);


		//２値化の上演決定SpinButton
		dialog.highThresholdSpin = gtk_spin_button_new_with_range(0,255,1);
		// 値の桁数を指定
		gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dialog.highThresholdSpin), 0);
		//上限値と下限値を設ける
		gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(dialog.highThresholdSpin), TRUE);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog.highThresholdSpin),200);
		gtk_box_pack_start(GTK_BOX(dialog.highImgBox),dialog.highThresholdSpin , FALSE, FALSE, 0);
	
		// 膨張回数決定label
		dialog.highNumberOfDilateLabel = gtk_label_new("膨張回数");
		gtk_box_pack_start(GTK_BOX(dialog.highImgBox), dialog.highNumberOfDilateLabel, FALSE, FALSE, 0);
		
		//膨張回数決定SpinButton
		dialog.highNumberOfDilate = gtk_spin_button_new_with_range(0,255,1);
		// 値の桁数を指定
		gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dialog.highNumberOfDilate), 0);
		//上限値と下限値を設ける
		gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(dialog.highNumberOfDilate), TRUE);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog.highNumberOfDilate),1);
		gtk_box_pack_start(GTK_BOX(dialog.highImgBox),dialog.highNumberOfDilate , FALSE, FALSE, 0);
		
		// 収縮回数決定label
		dialog.highNumberOfErodeLabel = gtk_label_new("収縮回数");
		gtk_box_pack_start(GTK_BOX(dialog.highImgBox), dialog.highNumberOfErodeLabel, FALSE, FALSE, 0);
		
		// 収縮回数決定SpinButton
		dialog.highNumberOfErode = gtk_spin_button_new_with_range(0,255,1);
		// 値の桁数を指定
		gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dialog.highNumberOfErode), 0);
		//上限値と下限値を設ける
		gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(dialog.highNumberOfErode), TRUE);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog.highNumberOfErode),1);
		gtk_box_pack_start(GTK_BOX(dialog.highImgBox),dialog.highNumberOfErode , FALSE, FALSE, 0);	



		// 試行回数を決定SpinButton
		// 試行回数ラベル
		dialog.numberOfOpenLabel = gtk_label_new("試行回数");
		//gtk_box_pack_start(GTK_BOX(dialog.vbox), dialog.numberOfOpenLabel, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(dialog.highImgBox),dialog.numberOfOpenLabel , FALSE, FALSE, 0);

		//２値化の上限決定SpinButton
		dialog.highNumberOfOpenSpin = gtk_spin_button_new_with_range(0,10,1);
		// 値の桁数を指定
		gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dialog.highNumberOfOpenSpin), 0);
		//上限値と下限値を設ける
		gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(dialog.highNumberOfOpenSpin), TRUE);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog.highNumberOfOpenSpin),1);
		gtk_box_pack_start(GTK_BOX(dialog.highImgBox),dialog.highNumberOfOpenSpin , FALSE, FALSE, 0);

		// ブロック数の決定Label
		dialog.numberOfOpenLabel = gtk_label_new("ブロック数");
		gtk_box_pack_start(GTK_BOX(dialog.highImgBox), dialog.numberOfOpenLabel, FALSE, FALSE, 0);

		// ブロック数を決定ComboBox
		store = gtk_list_store_new( 1, G_TYPE_STRING );
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, "5", -1 );
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, "3", -1 );
		/* Create combo box with store as data source. */
		dialog.highNumberOfBlockCombobox= gtk_combo_box_new_with_model( GTK_TREE_MODEL( store ) );
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(dialog.highNumberOfBlockCombobox),&iter);
		gtk_box_pack_start(GTK_BOX(dialog.highImgBox),dialog.highNumberOfBlockCombobox , FALSE, FALSE, 0);
		/* Create cell renderer. */
		cell = gtk_cell_renderer_text_new();
		/* Pack it into the combo box. */
		gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( dialog.highNumberOfBlockCombobox ), cell, TRUE );
		/* Connect renderer to data source. */
		gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( dialog.highNumberOfBlockCombobox ), cell, "text", 0, NULL );


		// 演算方法の決定Label
		dialog.numberOfOpenLabel = gtk_label_new("演算方法");
		gtk_box_pack_start(GTK_BOX(dialog.vbox), dialog.numberOfOpenLabel, FALSE, FALSE, 0);

		// 演算方法を決定ComboBox
		store = gtk_list_store_new( 1, G_TYPE_STRING );
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, "&", -1 );
		gtk_list_store_prepend( store, &iter );
		gtk_list_store_set( store, &iter, 0, "|", -1 );
		gtk_list_store_insert( store, &iter, 1 );
		gtk_list_store_set( store, &iter, 0, "^", -1 );
		/* Create combo box with store as data source. */
		dialog.choiceOperatorCombobox= gtk_combo_box_new_with_model( GTK_TREE_MODEL( store ) );
		gtk_box_pack_start(GTK_BOX(dialog.vbox),dialog.choiceOperatorCombobox , FALSE, FALSE, 0);
		/* Create cell renderer. */
		cell = gtk_cell_renderer_text_new();
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(dialog.choiceOperatorCombobox),&iter);
		/* Pack it into the combo box. */
		gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( dialog.choiceOperatorCombobox ), cell, TRUE );
		/* Connect renderer to data source. */
		gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( dialog.choiceOperatorCombobox ), cell, "text", 0, NULL );


		/*** Out画像用 ***/
		dialog.outImgFrame = gtk_frame_new("Out");
		gtk_box_pack_start(GTK_BOX(dialog.vbox), dialog.outImgFrame, FALSE, FALSE, 0);
		dialog.outImgBox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
		gtk_container_add(GTK_CONTAINER(dialog.outImgFrame), dialog.outImgBox);


		// Out画像用 Opening Closing 切り替えCombobox
		store = gtk_list_store_new( 1, G_TYPE_STRING );
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, "Closing", -1 );
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, "Opening", -1 );
		/* Create combo box with store as data source. */
		dialog.outSwitchingMorphologyCombobox= gtk_combo_box_new_with_model( GTK_TREE_MODEL( store ) );
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(dialog.outSwitchingMorphologyCombobox),&iter);
		gtk_box_pack_start(GTK_BOX(dialog.outImgBox),dialog.outSwitchingMorphologyCombobox , FALSE, FALSE, 0);
		/* Create cell renderer. */
		cell = gtk_cell_renderer_text_new();
		/* Pack it into the combo box. */
		gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( dialog.outSwitchingMorphologyCombobox ), cell, TRUE );
		/* Connect renderer to data source. */
		gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( dialog.outSwitchingMorphologyCombobox ), cell, "text", 0, NULL );
		
		// 試行回数を決定SpinButton
		// 試行回数ラベル
		dialog.numberOfOpenLabel = gtk_label_new("試行回数");
		//gtk_box_pack_start(GTK_BOX(dialog.vbox), dialog.numberOfOpenLabel, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(dialog.outImgBox),dialog.numberOfOpenLabel , FALSE, FALSE, 0);

		//２値化の上限決定SpinButton
		dialog.outNumberOfOpenSpin = gtk_spin_button_new_with_range(0,10,1);
		// 値の桁数を指定
		gtk_spin_button_set_digits(GTK_SPIN_BUTTON(dialog.outNumberOfOpenSpin), 0);
		//上限値と下限値を設ける
		gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(dialog.outNumberOfOpenSpin), TRUE);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog.outNumberOfOpenSpin),1);
		gtk_box_pack_start(GTK_BOX(dialog.outImgBox),dialog.outNumberOfOpenSpin , FALSE, FALSE, 0);




		// コールバック関数を定義
		g_signal_connect(G_OBJECT(dialog.upButton), "clicked", G_CALLBACK(cb_upButton), NULL);
		g_signal_connect(G_OBJECT(dialog.downButton), "clicked", G_CALLBACK(cb_downButton), NULL);
		g_signal_connect(G_OBJECT(dialog.rightButton), "clicked", G_CALLBACK(cb_rightButton), NULL);
		g_signal_connect(G_OBJECT(dialog.leftButton), "clicked", G_CALLBACK(cb_leftButton), NULL);
		g_signal_connect(G_OBJECT(dialog.gray3colorButton), "clicked", G_CALLBACK(cb_gray3colorButton), NULL);
		g_signal_connect(G_OBJECT(dialog.switchingFFTlumButton), "clicked", G_CALLBACK(cb_switchingFFTlumButton), NULL);
		// spinの変更時のコールバック
		g_signal_connect(G_OBJECT(dialog.lowThresholdSpin), "value-changed", G_CALLBACK(cb_lowThreshold_changed), NULL);
		g_signal_connect(G_OBJECT(dialog.highThresholdSpin), "value-changed", G_CALLBACK(cb_highThreshold_changed), NULL);
		g_signal_connect(G_OBJECT(dialog.lowNumberOfOpenSpin), "value-changed", G_CALLBACK(cb_lowNumberOfOpen_changed), NULL);
		g_signal_connect(G_OBJECT(dialog.highNumberOfOpenSpin), "value-changed", G_CALLBACK(cb_highNumberOfOpen_changed), NULL);
		g_signal_connect(G_OBJECT(dialog.lowNumberOfDilate), "value-changed", G_CALLBACK(cb_lowNumberOfDilate_changed), NULL);
		g_signal_connect(G_OBJECT(dialog.highNumberOfDilate), "value-changed", G_CALLBACK(cb_highNumberOfDilate_changed), NULL);
		g_signal_connect(G_OBJECT(dialog.lowNumberOfErode), "value-changed", G_CALLBACK(cb_lowNumberOfErode_changed), NULL);
		g_signal_connect(G_OBJECT(dialog.highNumberOfErode), "value-changed", G_CALLBACK(cb_highNumberOfErode_changed), NULL);
		g_signal_connect(G_OBJECT(dialog.outNumberOfOpenSpin), "value-changed", G_CALLBACK(cb_outNumberOfOpen_changed), NULL);
		// Comboboxの変更時のコールバック
		g_signal_connect(G_OBJECT( dialog.choiceOperatorCombobox ), "changed",G_CALLBACK( cb_choiceOperator_changed ), NULL );
		g_signal_connect(G_OBJECT( dialog.lowNumberOfBlockCombobox ), "changed",G_CALLBACK( cb_lowNumberOfBlock_changed ), NULL );
		g_signal_connect(G_OBJECT( dialog.highNumberOfBlockCombobox ), "changed",G_CALLBACK( cb_highNumberOfBlock_changed ), NULL );
		g_signal_connect(G_OBJECT( dialog.lowSwitchingMorphologyCombobox ),  "changed",G_CALLBACK( cb_lowSwitchingMorphology_changed ), NULL );
		g_signal_connect(G_OBJECT( dialog.highSwitchingMorphologyCombobox ), "changed",G_CALLBACK( cb_highSwitchingMorphology_changed ), NULL );
		g_signal_connect(G_OBJECT( dialog.outSwitchingMorphologyCombobox ), "changed",G_CALLBACK( cb_outSwitchingMorphology_changed ), NULL );
	}

	gtk_widget_show_all(dialog.window);
	gtk_main();
	return 0;
}

