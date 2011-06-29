package org.ab.x48;

import java.nio.ShortBuffer;
import java.util.ArrayList;
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
	private Bitmap  annImages [];
	boolean ann [];
	int ann_pos [] = { 62, 105, 152, 197, 244, 287 };
	private List<Integer> queuedCodes;
	private int touches [] = new int [MAX_TOUCHES];
	protected boolean needFlip;
	private short buf [];
	private int currentOrientation;
	private boolean multiTouch;
	
	int buttons_coords [][] = new int [MAX_TOUCHES][4];
    int icons_coords [][] = new int [6][2];
   
    Matrix matrixScreen;
    Matrix matrixBack;
    Paint paint;
    Paint screenPaint = null;

	public HPView(Context context, AttributeSet attrs) {
		super(context, attrs);
		setFocusable(true);
        setFocusableInTouchMode(true);
		x48 = ((X48) context);
		multiTouch = Wrapper.supportsMultitouch(x48);
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
        
        BitmapFactory.Options opts = new BitmapFactory.Options();
        opts.inScaled = false;
        for(int i=0;i<MAX_TOUCHES;i++) {
        	keys[i] = BitmapFactory.decodeResource(x48.getResources(), R.drawable.k01 + i, opts);
        }
        
        paint = new Paint(); 
        paint.setStyle(Style.FILL); 
        paint.setARGB(128, 250, 250, 250); 

        screenPaint = null;
		screenPaint = new Paint();
		
        
	}
	
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
	protected volatile Bitmap backBuffer;
	private boolean fullWidth;
	
	public boolean isFullWidth() {
		return fullWidth;
	}

	public void setFullWidth(boolean fullWidth) {
		this.fullWidth = fullWidth;
	}

	public void refreshMainScreen(short data []) {
		Canvas c = null;
		try {
            c = mSurfaceHolder.lockCanvas(null);
            synchronized (mSurfaceHolder) {
            	boolean land = currentOrientation == Configuration.ORIENTATION_LANDSCAPE;
            	
            	if (c != null) {
            		
            		
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
							screenPaint.setFilterBitmap(false);
							if (!land && fullWidth) {
								screenPaint.setFilterBitmap(true);
								lcd_ratio = (land?h:(float)w) / 131;
							}
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
							if (!keybLite || land)
								backCanvas.drawRect(0, 0, usable_w, start_h+menu_key_height, p);
							
							matrixScreen = new Matrix();
							
							matrixScreen.preScale(lcd_ratio/2, lcd_ratio/2);
							matrixScreen.postTranslate(lcd_pos_x, lcd_pos_y);
							p.setColor(Color.WHITE);
							backCanvas.drawRect(lcd_pos_x, lcd_pos_y, lcd_pos_x_end, lcd_pos_y_end, p);
							Paint keyPaint = new Paint();
							keyPaint.setFilterBitmap(true);
							
							ArrayList<Integer> orderKeys = new ArrayList<Integer>();
							for(int k=0;k<keys.length;k++) {
								orderKeys.add(k);
							}
							orderKeys.add(0, orderKeys.remove(30));
							orderKeys.add(0, orderKeys.remove(31));
							orderKeys.add(0, orderKeys.remove(32));
							
							for(int k:orderKeys) {
								float key_x = 0f;
								float key_width = 0f;
								float key_y = 0f;
								float key_height = 0f;
								if (!keybLite || land) {
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
										key_width = 2f * (float)w / 5f;
										key_height = usable_h/5f;
										key_x = 0;
										key_y = start_h;
									} else if (k == 25) {
										key_width = w / 5;
										key_height = usable_h/5f;
										key_x = 2*key_width;
										key_y = start_h;
									} else if (k == 27) {
										key_width = w / 5;
										key_height = usable_h/5f;
										key_x = 3*key_width;
										key_y = start_h;
									} else if (k == 28) {
										key_width = w / 5;
										key_height = usable_h/5f;
										key_x = 4*key_width;
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
										key_y = start_h + key_height*(yrank);
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
						c.drawBitmap(mainScreen, matrixScreen, screenPaint);
						for(int i=0;i<MAX_TOUCHES;i++) {
							if (touches[i] != 0) {
								c.drawRoundRect(new RectF(new Rect(buttons_coords[i][0], buttons_coords[i][1], buttons_coords[i][2], buttons_coords[i][3])), 12f, 12f, paint);
							}
						}
						
						for(int i=0;i<6;i++) {
							if (ann[i])
								c.drawBitmap(annImages[i], icons_coords[i][0], icons_coords[i][1], null);
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
			float x, y;
			int action = event.getAction();
			int actionCode = action & MotionEvent.ACTION_MASK;
			int code = -1;
			int pointerID = 0;

			if (multiTouch) {
				if( actionCode == MotionEvent.ACTION_DOWN || actionCode == MotionEvent.ACTION_UP ||
						actionCode == MotionEvent.ACTION_POINTER_DOWN || actionCode == MotionEvent.ACTION_POINTER_UP ) {
					pointerID = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
					x = Wrapper.MotionEvent_getX(event, pointerID);
					y = Wrapper.MotionEvent_getY(event, pointerID);
					pointerID = Wrapper.MotionEvent_getPointerId(event, pointerID) + 1;
				} else {
					return false;
				}
				
				// *_DOWN : lookup by coordinates
				// *_UP : lookup by pointer pressed
				if( actionCode == MotionEvent.ACTION_DOWN || actionCode == MotionEvent.ACTION_POINTER_DOWN ) {
		            for(int i=0;i<MAX_TOUCHES;i++) {
		                if (x >= buttons_coords[i][0] && x < buttons_coords[i][2] && y >= buttons_coords[i][1] && y < buttons_coords[i][3])
		                {
		                    code = i;
		                    break;
		                }
		            }
	            } else {
	            	for(int i=0;i<MAX_TOUCHES;i++) {
	            		if(touches[i] == pointerID)
	            			code = i;
	            	}
		            }	            
	            if (code == -1 && actionCode == MotionEvent.ACTION_DOWN && currentOrientation != Configuration.ORIENTATION_LANDSCAPE ) {
	            	//x48.flipkeyboard();
	            	((X48) getContext()).changeKeybLite();
	            	return true;
	            }
	       
				if (code > -1) {
					key(code, actionCode == MotionEvent.ACTION_DOWN || actionCode == MotionEvent.ACTION_POINTER_DOWN, pointerID);
					return true;
				}
			} else {
				// old code used before the 1.29 version: 
				x = event.getX();
				y = event.getY();
				
				if (action != MotionEvent.ACTION_DOWN && action != MotionEvent.ACTION_UP)
					return false;
				
	            for(int i=0;i<MAX_TOUCHES;i++) {
	                if (x >= buttons_coords[i][0] && x < buttons_coords[i][2] && y >= buttons_coords[i][1] && y < buttons_coords[i][3])
	                {
	                    code = i;
	                    break;
	                }
	            }
	            if (code == -1 && action == MotionEvent.ACTION_DOWN && currentOrientation != Configuration.ORIENTATION_LANDSCAPE ) {
	            	((X48) getContext()).changeKeybLite();
	            	return true;
	            }
	       
				if (code > -1) {
					key(code, action == MotionEvent.ACTION_DOWN);
					return action == MotionEvent.ACTION_DOWN;
				}
			}
		}
        
        return false;
	}
	
	private boolean keybLite = false;
	
	public boolean isKeybLite() {
		return keybLite;
	}

	public void setKeybLite(boolean keybLite) {
		this.keybLite = keybLite;
	}

	public void key(int code, boolean down) {
		key(code, down, 255); // Use pointerID 255 for keyboard
	}
	
	public synchronized void key(int code, boolean down, int pointerID) {
		//Log.i("x48", "code: " + code + " / " + down);
		if (code < MAX_TOUCHES) {
			if (down) {
				if (!multiTouch) {
					for(int i=0;i<MAX_TOUCHES;i++) {
						if (touches[i] != 0) {
							Log.i("x48", "no multitouch !, force up of " + i);
							queuedCodes.add(i + 100);
							touches [i] = 0;
							break;
						}
					}
				}
				Integer cI = code+1;
				if (!queuedCodes.contains(cI)) {
					queuedCodes.add(cI);
					touches [code] = pointerID;
					performHapticFeedback(HapticFeedbackConstants.LONG_PRESS,
							HapticFeedbackConstants.FLAG_IGNORE_GLOBAL_SETTING );
				} else {
					Log.i("x48", "rejected down");
				}
			}
			else {
				Integer cI = code+100;
				if (!queuedCodes.contains(cI) && touches [code] != 0) {
					queuedCodes.add(cI);
					touches [code] = 0;
				} else {
					Log.i("x48", "rejected up");
					if (!multiTouch) {
						for(int i=0;i<MAX_TOUCHES;i++) {
							if (touches[i] != 0) {
								Log.i("x48", "forced up of " + i);
								queuedCodes.add(i + 100);
								touches [i] = 0;
							}
						}
					}
				}
			}
			x48.flipScreen();
			this.notify();
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
			if (needFlip || x48.fillScreenData(buf) == 1) {
				needFlip = false;
				refreshMainScreen(buf);
			}
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
			case KeyEvent.KEYCODE_Y: if (d) key(25, true); else key(25, false); return true;
			case KeyEvent.KEYCODE_Z: if (d) key(26, true); else key(26, false); return true;
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
	
}
