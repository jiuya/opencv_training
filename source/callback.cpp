#include"../include/opencv_training.h"

// cairo_surface_t *surface = NULL;
static GdkWindow *drowable;
static GdkWindow *graphXdrawable = NULL;
static GdkWindow *graphYdrawable = NULL;
static GdkWindow *scaleImgdrawable = NULL;
static GdkPixbuf *pixbufdata;
static MainDialog *dialog;
static GError *error = NULL;
static int xline = 0;
static int yline = 0;
static int w,h;
static int graphline = 0;

// noise 変数
static int noise_lowThreshold = 60;
static int noise_highThreshold = 200;
static int noise_n = 1;
static int noise_operation = '^';
static int noise_size = 3;

// 膨張収縮画像の生成
static void
noiseExtractionImage(GdkPixbuf* const inputPixbuf,GdkPixbuf*& outputPixbuf,int threshold1,int threshold2,int n,char operation,int size)
{
	// pixbuf からcv::Mat に変換
	cv::Mat colorScale(
			cv::Size(
				gdk_pixbuf_get_width(inputPixbuf),
				gdk_pixbuf_get_height(inputPixbuf)
			),
			CV_8UC3,
			(uchar*)gdk_pixbuf_get_pixels(inputPixbuf),
			gdk_pixbuf_get_rowstride(inputPixbuf));
	// 画像のグレースケール変換
	cv::Mat grayScale(w,h,CV_8UC3);
	cv::cvtColor(colorScale, grayScale, CV_RGB2GRAY,1);
	// 画像の２値化
	cv::Mat lowBinImg;
	cv::threshold(grayScale, lowBinImg, threshold1, 255, cv::THRESH_BINARY);
	cv::Mat highBinImg;
	cv::threshold(grayScale, highBinImg, threshold2, 255, cv::THRESH_BINARY);
	// 画像の膨張収縮
	cv::Mat element(size,size,CV_8U, cv::Scalar::all(255));
	cv::morphologyEx(lowBinImg, lowBinImg , cv::MORPH_OPEN, element, cv::Point(-1,-1), n);
	cv::morphologyEx(highBinImg,highBinImg, cv::MORPH_OPEN, element, cv::Point(-1,-1), n);
	// 画像の演算
	cv::Mat outputMat;
	if(operation == '+')
	{
		outputMat = lowBinImg + highBinImg;
	}
	else if(operation == '-')
	{
		outputMat = lowBinImg - highBinImg;
	}
	else if(operation == '*')
	{
		outputMat = lowBinImg * highBinImg;
	}
	else if(operation == '/')
	{
		outputMat = lowBinImg / highBinImg;
	}
	else if(operation == '&')
	{
		outputMat = lowBinImg & highBinImg;
	}
	else if(operation == '|')
	{
		outputMat = lowBinImg | highBinImg;
	}
	else if(operation == '^')
	{
		outputMat = lowBinImg ^ highBinImg;
	}
	else
	{
		outputMat = lowBinImg;
	}
	// cv::Mat から pixbufに変換
	cv::cvtColor(outputMat,outputMat,CV_GRAY2RGB,3);
	GdkPixbuf* const tmpPixbuf = gdk_pixbuf_new_from_data(
			(guchar*)outputMat.data,
			GDK_COLORSPACE_RGB,
			FALSE,
			8,
			outputMat.cols,
			outputMat.rows,
			outputMat.step,
			NULL,
			NULL
			);
	outputPixbuf = gdk_pixbuf_copy(tmpPixbuf);
}
// 膨張収縮の右下画像を表示
static void 
draw_noiseDisplay()
{	
	cairo_t *scaleImg_cr;
	cairo_t *scaleImg_back;
	GdkPixbuf *resultImg;	

	if(scaleImgdrawable != NULL)
	{
		// 背景を消す
		scaleImg_back = gdk_cairo_create (scaleImgdrawable);
		cairo_set_source_rgb(scaleImg_back,0.9,0.9,0.9);
		cairo_rectangle (scaleImg_back, 0, 0,255, 255);
		cairo_fill(scaleImg_back);
		cairo_destroy(scaleImg_back);
		// 画像を描画
		scaleImg_cr = gdk_cairo_create (scaleImgdrawable);
		// モルフォロジーを用い微小なゴミを抽出
		noiseExtractionImage(pixbufdata,resultImg,noise_lowThreshold,noise_highThreshold,noise_n,noise_operation,noise_size);
		gdk_cairo_set_source_pixbuf(scaleImg_cr,resultImg,-yline,-xline);
		cairo_rectangle (scaleImg_cr, 0, 0,255, 255);
		cairo_fill (scaleImg_cr);
		cairo_destroy(scaleImg_cr);
	}

}
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
	// 右下の画像を表示
	draw_noiseDisplay();	
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
	int i,j;
	int rows;
	int channel;
	guchar luminance;
	long line;
	double *linexBuffer[3];
	double *lineyBuffer[3];
	guchar *pixel;
	cairo_t *graphXcr[3];
	cairo_t *graphYcr[3];
	cairo_t *graphbarX;
	cairo_t *graphbarY;
	cairo_t *whiteXcr;
	cairo_t *whiteYcr;
	cairo_t *line_x_low;
	cairo_t *line_y_low;
	cairo_t *line_x_high;
	cairo_t *line_y_high;

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
	graphXcr[0]   = gdk_cairo_create (graphXdrawable);
	graphXcr[1]   = gdk_cairo_create (graphXdrawable);
	graphXcr[2]   = gdk_cairo_create (graphXdrawable);
	graphYcr[0]   = gdk_cairo_create (graphYdrawable);
	graphYcr[1]   = gdk_cairo_create (graphYdrawable);
	graphYcr[2]   = gdk_cairo_create (graphYdrawable);
	graphbarX	  = gdk_cairo_create (graphXdrawable);
	graphbarY	  = gdk_cairo_create (graphYdrawable);
	// line x, line y のコンテキストを生成
	line_x_low = gdk_cairo_create(graphXdrawable);
	line_x_high = gdk_cairo_create(graphXdrawable);
	line_y_low = gdk_cairo_create(graphYdrawable);
	line_y_high = gdk_cairo_create(graphYdrawable);
	// line x, line y のパスを生成
	cairo_set_source_rgb(line_x_low,1,0,0);
	cairo_set_source_rgb(line_x_high,1,0,0);
	cairo_set_source_rgb(line_y_low,1,0,0);
	cairo_set_source_rgb(line_y_high,1,0,0);
	cairo_rectangle(line_x_low ,0,255 - noise_lowThreshold ,w,1);
	cairo_rectangle(line_x_high,0,255 - noise_highThreshold,w,1);
	cairo_rectangle(line_y_low ,255 - noise_lowThreshold ,0,1,h);
	cairo_rectangle(line_y_high,255 - noise_highThreshold,0,1,h);


	// 色選択
	if(!(graphline & 1))
	{
		cairo_set_source_rgb(graphXcr[0],1,0,0);
		cairo_set_source_rgb(graphXcr[1],0,1,0);
		cairo_set_source_rgb(graphXcr[2],0,0,1);
		cairo_set_source_rgb(graphYcr[0],1,0,0);
		cairo_set_source_rgb(graphYcr[1],0,1,0);
		cairo_set_source_rgb(graphYcr[2],0,0,1);
	}	
	// pixel 取得
	pixel = gdk_pixbuf_get_pixels(pixbufdata);
	channel = gdk_pixbuf_get_n_channels(pixbufdata);
	rows = gdk_pixbuf_get_rowstride(pixbufdata);
	//g_print("rows = %d\n",rows);
	// FFT切り替え
	if(graphline & 2)
	{
		linexBuffer[0] = (double *)malloc(sizeof(double) * w);
		lineyBuffer[0] = (double *)malloc(sizeof(double) * h);
		// グレイスケール
		if(graphline & 1)
		{
			// graph X 描画
			for(i = 0;i < w;i++)
			{
				line = xline * rows + i * 3;
				luminance = (int)(77 * pixel[0 + line] + 150 * pixel[1 + line] + 29 * pixel[2 + line]) >> 8;
				linexBuffer[0][i] = (double)luminance;
			}
			// graph Y 描画
			for(i = 0;i < h;i++)
			{
				line = yline * 3 + rows * i;
				luminance = (int)(77 * pixel[0 + line] + 150 * pixel[1 + line] + 29 * pixel[2 + line]) >> 8;
				lineyBuffer[0][i] = (double)luminance;
			}
			cv::Mat linexBufferMat(1,w,CV_32FC1,linexBuffer[0]);
			cv::Mat lineyBufferMat(1,h,CV_32FC1,lineyBuffer[0]);
			cv::Mat linexBufferDFTOut(1,w,CV_32FC1);
			cv::Mat lineyBufferDFTOut(1,h,CV_32FC1);
			cv::dft(linexBufferMat,linexBufferDFTOut,0,0);
			cv::dft(lineyBufferMat,lineyBufferDFTOut,0,0);
			// 4で割るのはDFTで半分、有効なデータが半分だからである
			for(i = 0;i < w/4;i++)
			{
				cairo_rectangle (graphXcr[0], i*4,255,2,255-(int)pow(linexBufferDFTOut.at<double>(i)/2,2));
			}
			for(i = 0;i < h/4;i++)
			{
				cairo_rectangle (graphYcr[0],255,i*4,255-(int)pow(lineyBufferDFTOut.at<double>(i)/2,2), 2);
			}
			if(linexBuffer[0])
			{
				free(linexBuffer[0]);
				linexBuffer[0] = NULL;
			}
			if(lineyBuffer[0])
			{
				free(lineyBuffer[0]);
				lineyBuffer[0] = NULL;
			}
		}
		// 三色
		else
		{
			linexBuffer[1] = (double *)malloc(sizeof(double) * w);
			lineyBuffer[1] = (double *)malloc(sizeof(double) * h);
			linexBuffer[2] = (double *)malloc(sizeof(double) * w);
			lineyBuffer[2] = (double *)malloc(sizeof(double) * h);
			
			// graph X 
			for(i = 0;i < w;i++)
			{
				line = xline * rows + i * 3;
				luminance = (int)pixel[0+line];
				linexBuffer[0][i] = (double)luminance;
				luminance = (int)pixel[1+line];
				linexBuffer[1][i] = (double)luminance;
				luminance = (int)pixel[2+line];
				linexBuffer[2][i] = (double)luminance;
			}
			// graph Y 
			for(i = 0;i < h;i++)
			{
				line = yline * 3 + rows * i;
				luminance = (int)pixel[0 + line];
				lineyBuffer[0][i] = (double)luminance;
				luminance = (int)pixel[1 + line];
				lineyBuffer[1][i] = (double)luminance;
				luminance = (int)pixel[2 + line];
				lineyBuffer[2][i] = (double)luminance;
			}
			cv::Mat linexBufferDFTOut(1,w,CV_32FC1);
			cv::Mat lineyBufferDFTOut(1,h,CV_32FC1);
			for(j = 0;j < 3;j++)
			{
				cv::Mat linexBufferMat(1,w,CV_32FC1,linexBuffer[j]);
				cv::Mat lineyBufferMat(1,h,CV_32FC1,lineyBuffer[j]);
				cv::dft(linexBufferMat,linexBufferDFTOut,0,0);
				cv::dft(lineyBufferMat,lineyBufferDFTOut,0,0);
				// 4で割るのはDFTで半分、有効なデータが半分だからである
				for(i = 0;i < w/4;i++)
				{
					cairo_rectangle (graphXcr[j], i*4+j,128,1,(int)(linexBufferDFTOut.at<double>(i)));
				}
				for(i = 0;i < h/4;i++)
				{
					cairo_rectangle (graphYcr[j],128,i*4+j,(int)(lineyBufferDFTOut.at<double>(i)), 1);
				}
				if(linexBuffer[j])
				{
					free(linexBuffer[j]);
					linexBuffer[j] = NULL;
				}
				if(lineyBuffer[j])
				{
					free(lineyBuffer[j]);
					lineyBuffer[j] = NULL;
				}
			}
		}
	}
	// Not FFT
	else
	{
		// グレイスケール
		if(graphline & 1)
		{
			// graph X 描画
			for(i = 0;i < w;i++)
			{
				line = xline * rows + i * 3;
				luminance = (int)(77 * pixel[0 + line] + 150 * pixel[1 + line] + 29 * pixel[2 + line]) >> 8;
				cairo_rectangle (graphXcr[0], i,255 - luminance,2, 2);
			}
			// graph Y 描画
			for(i = 0;i < h;i++)
			{
				line = yline * 3 + rows * i;
				luminance = (int)(77 * pixel[0 + line] + 150 * pixel[1 + line] + 29 * pixel[2 + line]) >> 8;
				cairo_rectangle (graphYcr[0], 255-luminance,i,2, 2);
			}
		}
		// 三色
		else
		{
			// graph X 描画
			for(i = 0;i < w;i++)
			{
				line = xline * rows + i * 3;
				luminance = (int)pixel[0+line];
				cairo_rectangle (graphXcr[0], i,255 - luminance,2, 2);
				luminance = (int)pixel[1+line];
				cairo_rectangle (graphXcr[1], i,255 - luminance,2, 2);
				luminance = (int)pixel[2+line];
				cairo_rectangle (graphXcr[2], i,255 - luminance,2, 2);

			}
			// graph Y 描画
			for(i = 0;i < h;i++)
			{
				line = yline * 3 + rows * i;
				luminance = (int)pixel[0 + line];
				cairo_rectangle (graphYcr[0], 255-luminance,i,2, 2);
				luminance = (int)pixel[1 + line];
				cairo_rectangle (graphYcr[1], 255-luminance,i,2, 2);
				luminance = (int)pixel[2 + line];
				cairo_rectangle (graphYcr[2], 255-luminance,i,2, 2);
			}
		}
	}
	cairo_rectangle(graphbarX,0,0,w,1);
	cairo_rectangle(graphbarX,0,127,w,1);
	cairo_rectangle(graphbarX,0,255,w,1);

	cairo_rectangle(graphbarY,0,0,1,h);
	cairo_rectangle(graphbarY,127,0,1,h);
	cairo_rectangle(graphbarY,255,0,1,h);

	cairo_fill(graphbarX);
	cairo_fill(graphbarY);
	cairo_destroy(graphbarX);
	cairo_destroy(graphbarY);

	if(!(graphline & 1))
	{
		cairo_fill(graphXcr[0]);
		cairo_fill(graphXcr[1]);
		cairo_fill(graphXcr[2]);
		cairo_fill(graphYcr[0]);
		cairo_fill(graphYcr[1]);
		cairo_fill(graphYcr[2]);
		
		cairo_destroy(graphXcr[0]);
		cairo_destroy(graphXcr[1]);
		cairo_destroy(graphXcr[2]);
		cairo_destroy(graphYcr[0]);
		cairo_destroy(graphYcr[1]);
		cairo_destroy(graphYcr[2]);
	}
	else
	{
		
		cairo_fill(graphXcr[0]);
		cairo_fill(graphYcr[0]);
		cairo_destroy(graphXcr[0]);
		cairo_destroy(graphYcr[0]);
		if(!(graphline & 2))
		{
			cairo_fill(line_x_low);
			cairo_fill(line_x_high);
			cairo_fill(line_y_low);
			cairo_fill(line_y_high);

			cairo_destroy(line_x_low);
			cairo_destroy(line_x_high);
			cairo_destroy(line_y_low);
			cairo_destroy(line_y_high);
		}
	}
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

// scaleimg draw event
gboolean
cb_scaleImg(GtkWidget *widget,
			GdkEventExpose *event,
			gpointer data)
{
	scaleImgdrawable = gtk_widget_get_window(widget);
	draw_display();	
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
void cb_gray3colorButton(GtkWidget *widget, gpointer user_data)
{
	if(graphline & 1)
	{
		graphline &= 2;
	}
	else
	{
		graphline |= 1;
	}
	draw_display();
	draw_graph();
}
void cb_switchingFFTlumButton(GtkWidget *widget, gpointer user_data)
{
	if(graphline & 2)
	{
		graphline &= 1;
	}
	else
	{
		graphline |= 2;
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

void cb_lowThreshold_changed(GtkSpinButton *spinbutton, gpointer data)
{
	noise_lowThreshold = gtk_spin_button_get_value(spinbutton);
	draw_noiseDisplay();
	draw_graph();
}

void cb_highThreshold_changed(GtkSpinButton *spinbutton, gpointer data)
{
	noise_highThreshold = gtk_spin_button_get_value(spinbutton);
	draw_noiseDisplay();
	draw_graph();
}

void cb_numberOfOpen_changed(GtkSpinButton *spinbutton, gpointer data)
{
	noise_n = gtk_spin_button_get_value(spinbutton);
	draw_noiseDisplay();
	draw_graph();
}
void cb_choiceOperator_changed(GtkComboBox *combo, gpointer data)
{
	GtkTreeIter   iter;
	gchar        *string = NULL;
	GtkTreeModel *model;
	/* Obtain currently selected item from combo box.
	 * If nothing is selected, do nothing. */
	if( gtk_combo_box_get_active_iter( combo, &iter ) )
	{
		/* Obtain data model from combo box. */
		model = gtk_combo_box_get_model( combo );
		/* Obtain string from model. */
		gtk_tree_model_get( model, &iter, 0, &string, -1 );
	}
	/* Print string to the console - if string is NULL, print NULL. */
	if(string != NULL)
	{
		noise_operation = *string;
	}
	/* Free string (if not NULL). */
	if( string )
		g_free( string );	
	draw_noiseDisplay();
	draw_graph();
}
void cb_numberOfBlock_changed(GtkComboBox *combo, gpointer data)
{
	GtkTreeIter   iter;
	gchar        *string = NULL;
	GtkTreeModel *model;
	/* Obtain currently selected item from combo box.
	 * If nothing is selected, do nothing. */
	if( gtk_combo_box_get_active_iter( combo, &iter ) )
	{
		/* Obtain data model from combo box. */
		model = gtk_combo_box_get_model( combo );
		/* Obtain string from model. */
		gtk_tree_model_get( model, &iter, 0, &string, -1 );
	}
	/* Print string to the console - if string is NULL, print NULL. */
	if(string != NULL)
	{
		noise_size = atoi(string);
	}
	/* Free string (if not NULL). */
	if( string )
		g_free( string );	
	draw_noiseDisplay();
	draw_graph();
}

