package org.ab.x48;

import java.nio.ShortBuffer;
import java.util.List;
import java.util.Vector;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Paint.Style;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.util.Log;
import android.view.HapticFeedbackConstants;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class HPView extends SurfaceView implements SurfaceHolder.Callback, Runnable {

	private static final int MAX_TOUCHES = 49;
	private static EmulatorThread thread;
	private Thread drawThread;
	private X48 x48;
	private Bitmap mainScreen;
	private SurfaceHolder mSurfaceHolder;
	//private Bitmap  mBackgroundImage;
	//private Bitmap  mBackgroundImageLand;
	//private Bitmap  mTouchImage;
	private Bitmap  annImages [];
	boolean ann [];
	int ann_pos [] = { 62, 105, 152, 197, 244, 287 };
	private List<Integer> queuedCodes;
	private boolean touches [] = new boolean [MAX_TOUCHES];
	//private int touches_x [] = new int [MAX_TOUCHES];
	//private int touches_y [] = new int [MAX_TOUCHES];
	
	private short buf [];
	private int currentOrientation;
	
	//private float scaleX = 1.0f;
	//private float scaleY = 1.0f;
	
	int buttons_coords [][] = new int [MAX_TOUCHES][4];
    int icons_coords [][] = new int [6][2];
    /*int screen_coord [] = new int[2];
    float screen_factor_x = 1;
    float back_factor_x = 1;

    float screen_factor_y = 1;
    float back_factor_y = 1;

    int buttons_coords_land [][] = new int [MAX_TOUCHES][4];
    int icons_coords_land [][] = new int [6][2];
    int screen_coord_land [] = new int[2];
    float screen_factor_land_x = 1;
    float back_factor_land_x = 1;
    
    float screen_factor_land_y = 1;
    float back_factor_land_y = 1;*/
    
    Matrix matrixScreen;
    Matrix matrixBack;
    Paint paint;

	public HPView(Context context, AttributeSet attrs) {
		super(context, attrs);
		setFocusable(true);
        setFocusableInTouchMode(true);
		x48 = ((X48) context);
		mSurfaceHolder = getHolder();
		mSurfaceHolder.addCallback(this);
        mainScreen = Bitmap.createBitmap(262, 14+128, Bitmap.Config.RGB_565);
        queuedCodes = new Vector<Integer>();
        ann = new boolean [6];
        buf = new short [(14+128)*262];
        annImages = new Bitmap [6];
        updateContrast();
        matrixScreen = new Matrix();
        matrixBack= new Matrix();
        annImages [0] = BitmapFactory.decodeResource(x48.getResources(), R.drawable.ann01);
        annImages [1] = BitmapFactory.decodeResource(x48.getResources(), R.drawable.ann02);
        annImages [2] = BitmapFactory.decodeResource(x48.getResources(), R.drawable.ann03);
        annImages [3] = BitmapFactory.decodeResource(x48.getResources(), R.drawable.ann04);
        annImages [4] = BitmapFactory.decodeResource(x48.getResources(), R.drawable.ann05);
        annImages [5] = BitmapFactory.decodeResource(x48.getResources(), R.drawable.ann06);
       // mBackgroundImageLand = BitmapFactory.decodeResource(x48.getResources(), R.drawable.skin_landscape);
        //mBackgroundImage = BitmapFactory.decodeResource(x48.getResources(), R.drawable.skin);
        
        for(int i=0;i<MAX_TOUCHES;i++) {
        	keys[i] = BitmapFactory.decodeResource(x48.getResources(), R.drawable.k01 + i);
        }
        
        paint = new Paint(); 
        paint.setStyle(Style.FILL); 
        paint.setARGB(128, 250, 250, 250); 

        
	}
	/*
	private void initGraphicsElements() {
		
        
        icons_coords = new int [][] { { 62, 0 }, {105, 0}, {152, 0}, {197, 0}, {244, 0}, {287, 0} };
        screen_coord = new int [] { 46-15, 1 };
        screen_factor_x = 1;
        screen_factor_y = 1;
        back_factor_x = 1;
        back_factor_y = 1;
        buttons_coords = new int [][] {
            {15, 139, 83, 170},
            {83, 139, 129, 170},
            {129, 139, 175, 170},
            {175, 139, 221, 170},
            {221, 139, 267, 170},
            {267, 139, 1000, 170},
            {15, 170, 83, 206},
            {83, 170, 129, 206},
            {129, 170, 175, 206},
            {175, 170, 221, 206},
            {221, 170, 267, 206},
            {267, 170, 1000, 206},
            {15, 206, 83, 242},
            {83, 206, 129, 242},
            {129, 206, 175, 242},
            {175, 206, 221, 242},
            {221, 206, 267, 242},
            {267, 206, 1000, 242},
            {15, 242, 83, 276},
            {83, 242, 129, 276},
            {129, 242, 175, 276},
            {175, 242, 221, 276},
            {221, 242, 267, 276},
            {267, 242, 1000, 276},
            {15, 276, 129, 312}, // enter
            {129, 276, 175, 312},
            {175, 276, 221, 312},
            {221, 276, 267, 312},
            {267, 276, 1000, 312},
            {15, 312, 91, 348},
            {91, 312, 147, 348},
            {147, 312, 203, 348},
            {203, 312, 259, 348},
            {259, 312, 1000, 348},
            {15, 348, 91, 383},
            {91, 348, 147, 383},
            {147, 348, 203, 383},
            {203, 348, 259, 383},
            {259, 348, 1000, 383},
            {15, 383, 91, 418},
            {91, 383, 147, 418},
            {147, 383, 203, 418},
            {203, 383, 259, 418},
            {259, 383, 1000, 418},
            {15, 418, 91, 1000},
            {91, 418, 147, 1000},
            {147, 418, 203, 1000},
            {203, 418, 259, 1000},
            {259, 418, 1000, 1000}
        };
        for(int i=0;i<buttons_coords.length;i++) {
        	buttons_coords[i][0] -= 15;
        	buttons_coords[i][2] -= 15;
        }
        
        float aspect = (float) width / (float) height;
        if (aspect < 1)
        	aspect = 1/ aspect;
        
        float ratio_x = (float) width / 320;
        if (height < width)
        	ratio_x = (float) width / 480;
        
        float ratio_y = ratio_x;
        
    

        icons_coords_land = new int [][] { { 62-34, 0 }, {105-34, 0}, {152-34, 0}, {197-34, 0}, {244-34, 0}, {287-34, 0} };
        screen_coord_land = new int [] { 12, 1 };
        screen_factor_x = ratio_x;
        screen_factor_y = ratio_y;
        back_factor_x = ratio_x;
        back_factor_y = ratio_y;
        screen_factor_land_x = ratio_x;
        screen_factor_land_y = ratio_y;
        back_factor_land_x = ratio_x;
        back_factor_land_y = ratio_y;
        buttons_coords_land = new int [][] {
            {0, 130, 49, 170},
            {49, 130, 95, 170},
            {95, 130, 141, 170},
            {141, 130, 187, 170},
            {187, 130, 233, 170},
            {233, 130, 286, 170},
            {0, 170, 49, 206},
            {49, 170, 95, 206},
            {95, 170, 141, 206},
            {141, 170, 187, 206},
            {187, 170, 233, 206},
            {233, 170, 286, 206},
            {0, 206, 49, 242},
            {49, 206, 95, 242},
            {95, 206, 141, 242},
            {141, 206, 187, 242},
            {187, 206, 233, 242},
            {233, 206, 286, 242},
            {0, 242, 49, 1000},
            {49, 242, 95, 1000},
            {95, 242, 141, 1000},
            {141, 242, 187, 1000},
            {187, 242, 233, 1000},
            {233, 242, 286, 1000},
            {391, 40, 1000, 76}, // enter, 24
            {391, 76, 433, 111},
            {433, 76, 1000, 111},
            {391, 111, 433, 146},
            {433, 111, 1000, 146},
            {286, 0, 337, 40},
            {302, 149, 358, 186},
            {358, 149, 414, 186},
            {414, 149, 1000, 186},
            {337, 0, 391, 40},
            {286, 40, 337, 76},
            {302, 186, 358, 224},
            {358, 186, 414, 224},
            {414, 186, 1000, 224}, // 37
            {337, 40, 391, 76},
            {286, 76, 337, 111},
            {302, 224, 358, 258},
            {358, 224, 414, 258},
            {414, 224, 1000, 258}, // 42
            {337, 76, 391, 111},
            {286, 111, 337, 146},
            {302, 258, 358, 1000},
            {358, 258, 414, 1000},
            {414, 258, 1000, 1000}, // 47
            {337, 111, 391, 146}
        };
        
        if ((width > 750 || height > 750) && mBackgroundImage.getWidth() == 480) //hdpi mode
        {
           screen_factor_x = 1.5f;
           screen_factor_y = 1.5f;
           screen_factor_land_x = 1.5f;
           screen_factor_land_y = 1.5f;
           
           ratio_x = (float) width / 480;
           if (height < width) // land
        	   ratio_x = (float) width / 800;
           
           ratio_y = ratio_x;
           
        
           back_factor_x = ratio_x;
           back_factor_y = ratio_y;
           back_factor_land_x = ratio_x;
           back_factor_land_y = ratio_y;
          
           screen_factor_land_x = screen_factor_land_x * ratio_x;
           screen_factor_land_y = screen_factor_land_y * ratio_y;
           buttons_coords = new int [][] {
        		{0, 215, 80, 274},
        		{80, 215, 160, 274},
        		{160, 215, 240, 274},
        		{240, 215, 320, 274},
        		{320, 215, 400, 274},
        		{400, 215, 480, 274},
        		{0, 274, 80, 336},
        		{80, 274, 160, 336},
        		{160, 274, 240, 336},
        		{240, 274, 320, 336},
        		{320, 274, 400, 336},
        		{400, 274, 480, 336},
        		{0, 336, 80, 396},
        		{80, 336, 160, 396},
        		{160, 336, 240, 396},
        		{240, 336, 320, 396},
        		{320, 336, 400, 396},
        		{400, 336, 480, 396},
        		{0, 396, 80, 454},
        		{80, 396, 160, 454},
        		{160, 396, 240, 454},
        		{240, 396, 320, 454},
        		{320, 396, 400, 454},
        		{400, 396, 480, 454},
        		{0, 454, 160, 514}, // enter
        		{160, 454, 240, 514},
        		{240, 454, 320, 514},
        		{320, 454, 400, 514},
        		{400, 454, 480, 514},
        		{0, 514, 96, 576},
        		{96, 514, 192, 576},
        		{192, 514, 288, 576},
        		{288, 514, 384, 576},
        		{384, 514, 480, 576},
        		{0, 576, 96, 638},
        		{96, 576, 192, 638},
        		{192, 576, 288, 638},
        		{288, 576, 384, 638},
        		{384, 576, 480, 638},
        		{0, 638, 96, 698},
        		{96, 638, 192, 698},
        		{192, 638, 288, 698},
        		{288, 638, 384, 698},
        		{384, 638, 480, 698},
        		{0, 698, 96, 1000},
        		{96, 698, 192, 1000},
        		{192, 698, 288, 1000},
        		{288, 698, 384, 1000},
        		{384, 698, 480, 1000}
           };
        } else {
        	for(int i=0;i<MAX_TOUCHES;i++) {
          	   buttons_coords[i][0] =(int) ((float) buttons_coords[i][0] * screen_factor_x);
          	   buttons_coords[i][1] =(int) ((float) buttons_coords[i][1] * screen_factor_y);
          	   buttons_coords[i][2] =(int) ((float) buttons_coords[i][2] * screen_factor_x);
          	   buttons_coords[i][3] =(int) ((float) buttons_coords[i][3] * screen_factor_y);
             }
             
             
        }
        
        for(int i=0;i<MAX_TOUCHES;i++) {
     	   buttons_coords_land[i][0] =(int) ((float) buttons_coords_land[i][0] * screen_factor_land_x);
     	   buttons_coords_land[i][1] =(int) ((float) buttons_coords_land[i][1] * screen_factor_land_y);
     	   buttons_coords_land[i][2] =(int) ((float) buttons_coords_land[i][2] * screen_factor_land_x);
     	   buttons_coords_land[i][3] =(int) ((float) buttons_coords_land[i][3] * screen_factor_land_y);
        }
        
        matrixScreen = new Matrix();
        matrixBack= new Matrix();
        
        boolean land = currentOrientation == Configuration.ORIENTATION_LANDSCAPE;
    	matrixBack.postScale(land?back_factor_land_x:back_factor_x, land?back_factor_land_y:back_factor_y);
        
    	matrixScreen.preTranslate(land?screen_coord_land[0]:screen_coord[0], land?screen_coord_land[1]:screen_coord[1]);
        matrixScreen.postScale(land?screen_factor_land_x:screen_factor_x, land?screen_factor_land_y:screen_factor_y);
        
	}
	*/
	public void updateContrast() {
		SharedPreferences mPrefs = PreferenceManager.getDefaultSharedPreferences(x48);
        String cString = mPrefs.getString("contrast", "1");
        int c = Integer.parseInt(cString);
        if (c < 0)
        	c = 0;
        if (c > 2)
        	c = 2;
        setContrast(0.5 * c);
        //x48.flipScreen();
	}
	
	private void setContrast(double contrast_factor) {
		x48.setBlankColor( (short) (((int)(15*contrast_factor+16) << 11) + ((int)(30*contrast_factor+33) << 5) + (int)(15*contrast_factor+13)));
	}
	
	//private short data [];
	private Bitmap keys [] = new Bitmap[MAX_TOUCHES];
	private Bitmap backBuffer;
	private boolean newBuffer = true;
	
	public void refreshMainScreen(short data []) {
		Canvas c = null;
		try {
            c = mSurfaceHolder.lockCanvas(null);
            synchronized (mSurfaceHolder) {
            	boolean land = currentOrientation == Configuration.ORIENTATION_LANDSCAPE;
            	
            	if (c != null) {
            		
            		if (land) {
            			keybLite = false;
            		}
            		
            		if (newBuffer) {
	            		if (backBuffer == null) {
	            			Log.i("x48", "init backBuffer !: " + keybLite);
	            			backBuffer = Bitmap.createBitmap(c.getWidth(), c.getHeight(), Bitmap.Config.ARGB_8888);
	            			Canvas backCanvas = new Canvas(backBuffer);
	            			
	            			int w = backBuffer.getWidth();
							int h = backBuffer.getHeight();
							Paint p = new Paint();
							int srcColor = Color.rgb(48, 56, 72);
							p.setColor(srcColor);
							backCanvas.drawRect(0, 0, w, h, p);
							float lcd_ratio = (land?h:w) / 131;
							int start_w = (int) (131*lcd_ratio);
							int start_h = (int) (71*lcd_ratio);
							float usable_w = w;
							float remaning_w = 0;
							float usable_h = h - start_h;
							if (land) {
								usable_w = ((float)w) * 5f / 9f;
								remaning_w = w - usable_w;
							}
							int lcd_pos_x = land?(((int)usable_w-start_w)/2):((w - start_w)/2);
							int lcd_pos_y = 0;
							int lcd_pos_x_end = lcd_pos_x+start_w;
							int lcd_pos_y_end = lcd_pos_y+start_h;
							float regular_key_height = usable_h / (8f + 11f/18f);
							float regular_key_height_right = h / 7f;
							float menu_key_height = regular_key_height*11/18;
							if (land) {
								regular_key_height = usable_h / (3f + 11f/18f);
								menu_key_height = regular_key_height*11/18;
							}
							
							icons_coords = new int [][] { { lcd_pos_x, 0 }, {(int)(lcd_pos_x+21*lcd_ratio), 0}, {(int)(lcd_pos_x+45*lcd_ratio), 0},
									{(int)(lcd_pos_x+67*lcd_ratio), 0}, {(int)(lcd_pos_x+91*lcd_ratio), 0}, {(int)(lcd_pos_x+112*lcd_ratio), 0} };
							int green = Color.rgb(80, 96, 104);
							p.setColor(green);
							if (!keybLite)
								backCanvas.drawRect(0, 0, usable_w, start_h+menu_key_height, p);
							
							matrixScreen = new Matrix();
							
							matrixScreen.preScale(lcd_ratio/2, lcd_ratio/2);
							matrixScreen.postTranslate(lcd_pos_x, lcd_pos_y);
							p.setColor(Color.WHITE);
							backCanvas.drawRect(lcd_pos_x, lcd_pos_y, lcd_pos_x_end, lcd_pos_y_end, p);
							Paint keyPaint = new Paint();
							keyPaint.setFilterBitmap(true);
							
							for(int k=0;k<keys.length;k++) {
								float key_x = 0f;
								float key_width = 0f;
								float key_y = 0f;
								float key_height = 0f;
								if (!keybLite) {
									if (k < 6) {
										// A, B, C...
										key_width = usable_w / 6;
										key_height = menu_key_height;
										key_x = key_width*k;
										key_y = start_h;
									} else if (k < 24) {
										key_width = usable_w / 6;
										key_height = regular_key_height;
										key_x = key_width*(k % 6);
										key_y = start_h + menu_key_height + regular_key_height*((k / 6)-1);
									} else if (!land) {
										if (k == 24) {
											// ENTER
											key_width = usable_w / 3;
											key_height = regular_key_height;
											key_x = 0;
											key_y = start_h + menu_key_height + regular_key_height*3f;
										} else if (k > 24 && k < 29) {
											key_width = usable_w / 6;
											key_height = regular_key_height;
											key_x = key_width*((k+1) % 6);
											int rank = ((k+1) / 6);
											key_y = start_h + menu_key_height + regular_key_height*(rank-1);
										} else if (k >= 29) {
											key_width = usable_w / 5;
											key_height = regular_key_height;
											key_x = key_width*((k+1) % 5);
											key_y = start_h + menu_key_height + regular_key_height*(((k+1) / 5)-2);
										}
									} else if (land) {
										if (k == 24) {
											key_width = remaning_w / 2;
											key_height = regular_key_height_right;
											key_x = w-key_width;
											int rank = 7;
											key_y = regular_key_height_right*(rank-1);
										} else if (k == 29 || k == 34 || k == 39 || k == 44) {
											key_width = remaning_w / 4;
											key_height = regular_key_height_right;
											int xrank = 4;
											if (k == 34)
												xrank = 3;
											else if (k == 39)
												xrank = 2;
											else if (k == 44)
												xrank = 1;
											key_x = w-key_width*xrank;
											key_y = 0;
										} else {
											key_width = remaning_w / 4;
											key_height = regular_key_height_right;
											int xrank = 0;
											int yrank = 0;
											if (k == 25) {
												xrank = 4;
												yrank = 0;
											} else if (k == 26) {
												xrank = 3;
												yrank = 0;
											} else if (k == 27) {
												xrank = 2;
												yrank = 0;
											} else if (k == 28) {
												xrank = 1;
												yrank = 0;
											} else if (k == 30) {
												xrank = 4;
												yrank = 1;
											} else if (k == 31) {
												xrank = 3;
												yrank = 1;
											} else if (k == 32) {
												xrank = 2;
												yrank = 1;
											} else if (k == 33) {
												xrank = 1;
												yrank = 1;
											} else if (k == 35) {
												xrank = 4;
												yrank = 2;
											} else if (k == 36) {
												xrank = 3;
												yrank = 2;
											} else if (k == 37) {
												xrank = 2;
												yrank = 2;
											} else if (k == 38) {
												xrank = 1;
												yrank = 2;
											} else if (k == 40) {
												xrank = 4;
												yrank = 3;
											} else if (k == 41) {
												xrank = 3;
												yrank = 3;
											} else if (k == 42) {
												xrank = 2;
												yrank = 3;
											} else if (k == 43) {
												xrank = 1;
												yrank = 3;
											} else if (k == 45) {
												xrank = 4;
												yrank = 4;
											} else if (k == 46) {
												xrank = 3;
												yrank = 4;
											} else if (k == 47) {
												xrank = 2;
												yrank = 4;
											} else if (k == 48) {
												xrank = 1;
												yrank = 4;
											}
											key_x = w-key_width*xrank;
											key_y = regular_key_height_right*(2+yrank-1);
										}
									}
								} else {
									if (k == 24) {
										key_width = w / 2;
										key_height = usable_h/5f;
										key_x = 0;
										key_y = start_h;
									} else if (k == 27) {
										key_width = w / 4;
										key_height = usable_h/5f;
										key_x = 2*key_width;
										key_y = start_h;
									} else if (k == 28) {
										key_width = w / 4;
										key_height = usable_h/5f;
										key_x = 3*key_width;
										key_y = start_h;
									} else if (k >= 30 && k <= 48 && k !=34 && k != 39 && k !=44) {
										key_width = w / 4;
										key_height = usable_h/5f;
										int xrank = 0;
										int yrank = 0;
										if (k == 30) {
											xrank = 4;
											yrank = 1;
										} else if (k == 31) {
											xrank = 3;
											yrank = 1;
										} else if (k == 32) {
											xrank = 2;
											yrank = 1;
										} else if (k == 33) {
											xrank = 1;
											yrank = 1;
										} else if (k == 35) {
											xrank = 4;
											yrank = 2;
										} else if (k == 36) {
											xrank = 3;
											yrank = 2;
										} else if (k == 37) {
											xrank = 2;
											yrank = 2;
										} else if (k == 38) {
											xrank = 1;
											yrank = 2;
										} else if (k == 40) {
											xrank = 4;
											yrank = 3;
										} else if (k == 41) {
											xrank = 3;
											yrank = 3;
										} else if (k == 42) {
											xrank = 2;
											yrank = 3;
										} else if (k == 43) {
											xrank = 1;
											yrank = 3;
										} else if (k == 45) {
											xrank = 4;
											yrank = 4;
										} else if (k == 46) {
											xrank = 3;
											yrank = 4;
										} else if (k == 47) {
											xrank = 2;
											yrank = 4;
										} else if (k == 48) {
											xrank = 1;
											yrank = 4;
										}
										key_x = key_width*(4-xrank);
										key_y = start_h + regular_key_height_right*(yrank);
									} else
										key_width = 0;
								}
								if (key_width == 0) {
									buttons_coords[k][0] = buttons_coords[k][1] = buttons_coords[k][2] = buttons_coords[k][3] = 0;
									continue;
								}
								buttons_coords[k][0] = (int) key_x;
								buttons_coords[k][1] = (int) key_y;
								buttons_coords[k][2] = (int) (key_x+key_width);
								buttons_coords[k][3] = (int) (key_y+key_height);
								int bw = keys[k].getWidth();
								int bh = keys[k].getHeight();
								int delta_x = 0;
								int delta_y = 0;
								float ratio_kx = 0.0f;
								float ratio_ky = 0.0f;
								if (bw < (int) key_width) {
									delta_x = ((int)key_width-bw)/2;
								} else if (bw > (int) key_width) {
									//ratio_kx = key_width / (float) bw;
									delta_x = ((int)key_width-bw)/2;
								}
								if (bh < (int) key_height) {
									delta_y = ((int)key_height-bh)/2;
								} else if (bh > (int) key_height) {
									//ratio_ky = key_height / (float) bh;
									delta_y = ((int)key_height-bh)/2;
								}
								if (!keybLite && !land && (k == 30 || k == 31 || k == 32 ||
										k == 35 || k == 36 || k == 37 ||
										k == 40 || k == 41 || k == 42 || k == 39)) {
									Paint p2 = new Paint();
									p2.setColor(green);
									backCanvas.drawRect(buttons_coords[k][0], buttons_coords[k][1], buttons_coords[k][2], buttons_coords[k][3], p2);
								}
								// slight off:
								buttons_coords[k][1] += bh*5/36;
								buttons_coords[k][3] += bh*5/36;
								Matrix matrixKey = new Matrix();
								if (ratio_kx != 0 && ratio_ky != 0) {
									matrixKey.preScale(ratio_kx, ratio_ky);
								}
								matrixKey.postTranslate(key_x + delta_x, key_y + delta_y);
								backCanvas.drawBitmap(keys[k], matrixKey, keyPaint);
								
							}
	            		}
	            		
	            		c.drawBitmap(backBuffer, 0, 0, null);
	            		if (data != null)
	            			mainScreen.copyPixelsFromBuffer(ShortBuffer.wrap(data));
						c.drawBitmap(mainScreen, matrixScreen, null);
						for(int i=0;i<MAX_TOUCHES;i++) {
							if (touches[i]) {
								c.drawRoundRect(new RectF(new Rect(buttons_coords[i][0], buttons_coords[i][1], buttons_coords[i][2], buttons_coords[i][3])), 12f, 12f, paint);
							}
						}
						
						for(int i=0;i<6;i++) {
							if (ann[i])
								c.drawBitmap(annImages[i], icons_coords[i][0], icons_coords[i][1], null);
						}
            		} else {
            	/*
		            	if ((land?mBackgroundImageLand:mBackgroundImage) != null) {
		            		c.drawBitmap(land?mBackgroundImageLand:mBackgroundImage, matrixBack, null);
		                
			                mainScreen.copyPixelsFromBuffer(ShortBuffer.wrap(data));
			                c.drawBitmap(mainScreen, matrixScreen, null);
			                
			                for(int i=0;i<MAX_TOUCHES;i++) {
			                    if (touches[i]) {
			                        if (land)
			                        	c.drawRoundRect(new RectF(new Rect(buttons_coords_land[i][0], buttons_coords_land[i][1], buttons_coords_land[i][2], buttons_coords_land[i][3])), 12f, 12f, paint);
			                        else
			                        	c.drawRoundRect(new RectF(new Rect(buttons_coords[i][0], buttons_coords[i][1], buttons_coords[i][2], buttons_coords[i][3])), 12f, 12f, paint);
			                    }
			                }
			                
			                for(int i=0;i<6;i++) {
			                    if (ann[i])
			                        c.drawBitmap(annImages[i], land?icons_coords_land[i][0]:icons_coords[i][0], land?icons_coords_land[i][1]:icons_coords[i][1], null);
			                }
		            	}
		            	*/
            		}
            	} else {
            		//Log.i("x48", "null canvas !");
            	}
            }
        } finally {
            // do this in a finally so that if an exception is thrown
            // during the above, we don't leave the Surface in an
            // inconsistent state
            if (c != null) {
                mSurfaceHolder.unlockCanvasAndPost(c);
            }
        }
		//Log.i("x48", "data: " + data.length);
	}
	
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		synchronized (mSurfaceHolder) {
			int action = event.getAction();
			float x = event.getX();
			float y = event.getY();
			//Log.i("x48", "action: " + action + " / x: " + x + " / y:" + y);
			if (action != MotionEvent.ACTION_DOWN && action != MotionEvent.ACTION_UP)
				return false;
			int code = -1;
			
			 if (currentOrientation != Configuration.ORIENTATION_LANDSCAPE || newBuffer) {
	                for(int i=0;i<MAX_TOUCHES;i++) {
	                    if (x >= buttons_coords[i][0] && x < buttons_coords[i][2] && y >= buttons_coords[i][1] && y < buttons_coords[i][3])
	                    {
	                        code = i;
	                        break;
	                    }
	                }
	                if (code == -1 && action == MotionEvent.ACTION_DOWN && currentOrientation != Configuration.ORIENTATION_LANDSCAPE ) {
	                	//x48.flipkeyboard();
	                	keybLite = !keybLite;
	                	backBuffer = null;
	                	refreshMainScreen(null);
	                	return true;
	                }
	            } else {

	                /*for(int i=0;i<MAX_TOUCHES;i++) {
	                    if (x >= buttons_coords_land[i][0] && x < buttons_coords_land[i][2] && y >= buttons_coords_land[i][1] && y < buttons_coords_land[i][3])
	                    {
	                        code = i;
	                        break;
	                    }
	                }
	                */
	            }
			 
			//Log.i("x48", "action: " + action + " code: " + code);
			if (code > -1) {
				key(code, action == MotionEvent.ACTION_DOWN);
				return action == MotionEvent.ACTION_DOWN;
			}
		}
        
        return false;
	}
	
	private boolean keybLite = false;
	
	public synchronized void key(int code, boolean down) {
		//Log.i("x48", "code: " + code + " / " + down);
		if (code < MAX_TOUCHES) {
			if (down) {
				for(int i=0;i<MAX_TOUCHES;i++) {
					if (touches[i]) {
						Log.i("x48", "no multitouch !, force up of " + i);
						queuedCodes.add(i + 100);
						touches [i] = false;
						break;
					}
				}
				Integer cI = code+1;
				if (!queuedCodes.contains(cI)) {
					queuedCodes.add(cI);
					touches [code] = true;
					performHapticFeedback(HapticFeedbackConstants.LONG_PRESS,
							HapticFeedbackConstants.FLAG_IGNORE_GLOBAL_SETTING );
					/*x48.flipScreen();*/
				} else {
					Log.i("x48", "rejected down");
				}
			}
			else {
				Integer cI = code+100;
				if (!queuedCodes.contains(cI) && touches [code]) {
					queuedCodes.add(cI);
					touches [code] = false;
					/*x48.flipScreen();*/
				} else {
					Log.i("x48", "rejected up");
					for(int i=0;i<MAX_TOUCHES;i++) {
						if (touches[i]) {
							Log.i("x48", "forced up of " + i);
							queuedCodes.add(i + 100);
							touches [i] = false;
						}
					}
					//queuedCodes.add(code + 1);
					//queuedCodes.add(cI);
				}
			}
			x48.flipScreen();
			this.notify();
	    	//pressed++;
		}
	}
	
	
	
	public synchronized void pauseEvent() {
		//Log.i("x48", "pauseEvent begin");
		try {
			this.wait();
		} catch (InterruptedException e) {
			//Log.i("x48", "pauseEvent: " + e.getMessage());
		}
		//Log.i("x48", "pauseEvent end");
	}
	
	public synchronized void unpauseEvent() {
		//Log.i("x48", "unpauseEvent");
		this.notify();
	}
	
	public synchronized int waitEvent() {
		if (queuedCodes.size() == 0) {
			return 0;
		}
		else {
			int c = queuedCodes.remove(0);
			return c;
		}
    }
	
	protected int width;
	protected int height;

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		Log.i("x48", "width: " + width + " / height: " + height);
		this.width = width;
		this.height = height;
		if (width < height)
			currentOrientation = Configuration.ORIENTATION_PORTRAIT;
		else
			currentOrientation = Configuration.ORIENTATION_LANDSCAPE;
		//initGraphicsElements();
		backBuffer = null;
		x48.flipScreen();
	}
	
	public void emulatorReady() {
		resume();
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		Log.i("x48", "Surface created");
		if (thread == null) {
			thread = new EmulatorThread(x48);
			thread.start();
		}
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		Log.i("x48", "Surface destroyed");
		/*if (thread != null) {
	        while (retry) {
	            try {
	                thread.join();
	                retry = false;
	            } catch (InterruptedException e) {
	            }
	        }
		}*/
	}

	@Override
    public void onWindowFocusChanged(boolean hasWindowFocus) {
        if (!hasWindowFocus) {
        	//mRun = false;
        }
    }
	
	public void stop() {
		Log.i("x48", "Stopping..");
		if (thread != null)
			thread.interrupt();
		thread = null;
	}
	
	public void refreshIcons(boolean ann []) {
		this.ann = ann;
		//x48.flipScreen();
		//refreshMainScreen(null);
	}
	
	private boolean mRun;
	
	public void pause(boolean fast) {
		mRun = false;
		if (fast && drawThread != null) {
			try {
				drawThread.join(0);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}
	
	public void resume() {
		Log.i("x48", "resume");
		if (thread != null) {
			mRun = true;
			drawThread = new Thread(this);
			drawThread.start();
		}
	}

	@Override
	public void run() {
		Log.i("x48", "drawing thread started");
		x48.flipScreen();
		while (mRun) {
			try {
			Thread.sleep(40);
			
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			/*
			short data [] = x48.getScreen();
			if (data != null && data.length > 0)
				refreshMainScreen(data);*/
			if (x48.fillScreenData(buf) == 1)
				refreshMainScreen(buf);
		}
		Log.i("x48", "drawing thread stopped");
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		//Log.i("x48", "-->"+keyCode);
		return actionKey(true, keyCode);
	}
	
	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		//Log.i("x48", "--<"+keyCode);
		return actionKey(false, keyCode);
		
	}
	
	private boolean actionKey(boolean d, int code) {
		switch (code) {
			case KeyEvent.KEYCODE_BACK: {
				SharedPreferences mPrefs = PreferenceManager.getDefaultSharedPreferences(x48);
		        String bString = mPrefs.getString("backkey", "0");
		        if (bString.equals("0"))
		        	return false;
		        else if (bString.equals("1")) { 
		        	if (d) key(28, true); else key(28, false); return true;
		        } else if (bString.equals("2")) { 
		        	if (d) key(44, true); else key(44, false); return true;
		        } 
			}
			
			case KeyEvent.KEYCODE_0: if (d) key(45, true); else key(45, false); return true;
			case KeyEvent.KEYCODE_1: if (d) key(40, true); else key(40, false); return true;
			case KeyEvent.KEYCODE_2: if (d) key(41, true); else key(41, false); return true;
			case KeyEvent.KEYCODE_3: if (d) key(42, true); else key(42, false); return true;
			case KeyEvent.KEYCODE_4: if (d) key(35, true); else key(35, false); return true;
			case KeyEvent.KEYCODE_5: if (d) key(36, true); else key(36, false); return true;
			case KeyEvent.KEYCODE_6: if (d) key(37, true); else key(37, false); return true;
			case KeyEvent.KEYCODE_7: if (d) key(30, true); else key(30, false); return true;
			case KeyEvent.KEYCODE_8: if (d) key(31, true); else key(31, false); return true;
			case KeyEvent.KEYCODE_9: if (d) key(32, true); else key(32, false); return true;
			case KeyEvent.KEYCODE_ENTER: if (d) key(24, true); else key(24, false); return true;
			case KeyEvent.KEYCODE_DEL: if (d) key(28, true); else key(28, false); return true;
			case KeyEvent.KEYCODE_PERIOD: if (d) key(46, true); else key(46, false); return true;
			case KeyEvent.KEYCODE_AT: if (d) key(29, true); else key(29, false); return true;
			
			case KeyEvent.KEYCODE_A: if (d) key(0, true); else key(0, false); return true;
			case KeyEvent.KEYCODE_B: if (d) key(1, true); else key(1, false); return true;
			case KeyEvent.KEYCODE_C: if (d) key(2, true); else key(2, false); return true;
			case KeyEvent.KEYCODE_D: if (d) key(3, true); else key(3, false); return true;
			case KeyEvent.KEYCODE_E: if (d) key(4, true); else key(4, false); return true;
			case KeyEvent.KEYCODE_F: if (d) key(5, true); else key(5, false); return true;
			case KeyEvent.KEYCODE_G: if (d) key(6, true); else key(6, false); return true;
			case KeyEvent.KEYCODE_H: if (d) key(7, true); else key(7, false); return true;
			case KeyEvent.KEYCODE_I: if (d) key(8, true); else key(8, false); return true;
			case KeyEvent.KEYCODE_J: if (d) key(9, true); else key(9, false); return true;
			case KeyEvent.KEYCODE_K: if (d) key(10, true); else key(10, false); return true;
			case KeyEvent.KEYCODE_L: if (d) key(11, true); else key(11, false); return true;
			case KeyEvent.KEYCODE_M: if (d) key(12, true); else key(12, false); return true;
			case KeyEvent.KEYCODE_N: if (d) key(13, true); else key(13, false); return true;
			case KeyEvent.KEYCODE_O: if (d) key(14, true); else key(14, false); return true;
			case KeyEvent.KEYCODE_P: if (d) key(15, true); else key(15, false); return true;
			case KeyEvent.KEYCODE_Q: if (d) key(16, true); else key(16, false); return true;
			case KeyEvent.KEYCODE_R: if (d) key(17, true); else key(17, false); return true;
			case KeyEvent.KEYCODE_S: if (d) key(18, true); else key(18, false); return true;
			case KeyEvent.KEYCODE_T: if (d) key(19, true); else key(19, false); return true;
			case KeyEvent.KEYCODE_U: if (d) key(20, true); else key(20, false); return true;
			case KeyEvent.KEYCODE_V: if (d) key(21, true); else key(21, false); return true;
			case KeyEvent.KEYCODE_W: if (d) key(22, true); else key(22, false); return true;
			case KeyEvent.KEYCODE_X: if (d) key(23, true); else key(23, false); return true;
			case KeyEvent.KEYCODE_Y: if (d) key(25, true); else key(26, false); return true;
			case KeyEvent.KEYCODE_Z: if (d) key(26, true); else key(25, false); return true;
			case KeyEvent.KEYCODE_SPACE: if (d) key(47, true); else key(47, false); return true;
			
			
			//case KeyEvent.KEYCODE_SHIFT_LEFT: if (d) key(34, true); else key(34, false); return true;
			//case KeyEvent.KEYCODE_SHIFT_RIGHT: if (d) key(39, true); else key(39, false); return true;
			
			case KeyEvent.KEYCODE_DPAD_UP: if (d) key(10, true); else key(10, false); return true;
			case KeyEvent.KEYCODE_DPAD_DOWN: if (d) key(16, true); else key(16, false); return true;
			case KeyEvent.KEYCODE_DPAD_LEFT: if (d) key(15, true); else key(15, false); return true;
			case KeyEvent.KEYCODE_DPAD_RIGHT: if (d) key(17, true); else key(17, false); return true;
			case KeyEvent.KEYCODE_DPAD_CENTER: if (d) key(24, true); else key(24, false); return true;
			
			default: return false;
		}
	}

	@Override
	public boolean onTrackballEvent(MotionEvent event) {
		/*float x = event.getX();
		float y = event.getY();
		int hs = event.getHistorySize();
		Log.i("x48", "tevent: " + hs + " x: " + x + " - " + y);
		return true;*/
		return super.onTrackballEvent(event);
	}
	
	
}
