package org.ab.x48;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;


import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.Configuration;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Window;
import android.view.WindowManager.LayoutParams;

public class X48 extends Activity {
    
	private HPView mainView;
	private boolean need_to_quit;
	static final private int LOAD_ID = Menu.FIRST +1;
	static final private int SAVE_ID = Menu.FIRST +2;
	static final private int SETTINGS_ID = Menu.FIRST +5 ;
	static final private int QUIT_ID = Menu.FIRST +3 ;
	static final private int RESET_ID = Menu.FIRST +4 ;
	static final private int KEYBLITE_ID = Menu.FIRST +6 ;
	
	static final private int ROM_ID = 123;
	
	private static EmulatorThread thread;
	
	
    // http://www.hpcalc.org/hp48/pc/emulators/gxrom-r.zip
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i("x48", "starting activity");
        AssetUtil.copyAsset(this, getResources().getAssets(), false);
        readyToGo() ;
        if (!AssetUtil.isFilesReady(this)) {
        	showDialog(DIALOG_ROM_KO);
        }
    }
    
    // http://stackoverflow.com/questions/9996333/openoptionsmenu-function-not-working-in-ics
    // todo: really need a proper system button
    @Override
    public void openOptionsMenu() {

        Configuration config = getResources().getConfiguration();

        if((config.screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK) 
                > Configuration.SCREENLAYOUT_SIZE_LARGE) {

            int originalScreenLayout = config.screenLayout;
            config.screenLayout = Configuration.SCREENLAYOUT_SIZE_LARGE;
            super.openOptionsMenu();
            config.screenLayout = originalScreenLayout;

        } else {
            super.openOptionsMenu();
        }
    }
 
    public void readyToGo() {
    	hp48s = PreferenceManager.getDefaultSharedPreferences(this).getBoolean("hp48s", false);
    	
    	requestWindowFeature(Window.FEATURE_NO_TITLE);
        requestWindowFeature(Window.FEATURE_PROGRESS);
        setContentView(R.layout.main);
        mainView = (HPView) findViewById(R.id.hpview);
        
        checkPrefs();
        
        thread = new EmulatorThread(this);
    	thread.start();
    	mainView.resume();
    }
    

    
    public void checkPrefs() {
    	SharedPreferences mPrefs = PreferenceManager.getDefaultSharedPreferences(this);
    	bitmapSkin = mPrefs.getBoolean("bitmapskin", false);
    	mainView.backBuffer = null;
    	mainView.needFlip = true;
		String port1 = mPrefs.getString("port1", "0");
		managePort(1, port1);
		String port2 = mPrefs.getString("port2", "0");
		managePort(2, port2);
		saveonExit = mPrefs.getBoolean("saveOnExit", false);
		if (mainView != null) {
			mainView.setHapticFeedbackEnabled(mPrefs.getBoolean("haptic", true));
			mainView.setFullWidth(mPrefs.getBoolean("large_width", false));
			mainView.setScaleControls(mPrefs.getBoolean("scale_buttons", false));
			mainView.setKeybLite(mPrefs.getBoolean("keybLite", false));
			mainView.setSound(mPrefs.getBoolean("sound", false));
		}
		if (mPrefs.getBoolean("fullScreen", false)) {
			getWindow().addFlags(LayoutParams.FLAG_FULLSCREEN);
			getWindow().clearFlags(LayoutParams.FLAG_FORCE_NOT_FULLSCREEN);
    	} else {
    		getWindow().addFlags(LayoutParams.FLAG_FORCE_NOT_FULLSCREEN);
			getWindow().clearFlags(LayoutParams.FLAG_FULLSCREEN);
		}
		mainView.requestLayout();
    }
        
    public void changeKeybLite() {
    	if (mainView != null) {
    		SharedPreferences mPrefs = PreferenceManager.getDefaultSharedPreferences(this);
	    	if (!mPrefs.getBoolean("disableLite", false)) {
	    		mainView.setKeybLite(!mainView.isKeybLite());
		    	Editor e = mPrefs.edit();
		    	e.putBoolean("keybLite", mainView.isKeybLite());
		    	e.commit();
		    	mainView.backBuffer = null;
		    	mainView.needFlip = true;
	    	}
    	}
    }
    
    public void refreshMainScreen(short data []) {
    	mainView.refreshMainScreen(data);
    }
    
    public int waitEvent() {
    	return mainView.waitEvent();
    }
    
    public void refreshIcons(boolean i []) {
    	mainView.refreshIcons(i);
    }
    
    public native void startHPEmulator();
    public native void resetHPEmulator();
    public native void saveState();
    public native void stopHPEmulator();
    public native int buttonPressed(int code);
    public native int buttonReleased(int code);
    public native void registerClass(X48 instance, String path, String rom_filename, String ram_filename, String conf_filename, String port1_filename, String port2_filename);
    public native int fillAudioData(short data []);
    public native int fillScreenData(short data [], boolean ann []);
    public native void flipScreen();
    public native int loadProg(String filename);
    public native void setBlankColor(short s);
    public native void openConditionVariable();
    public native void blockConditionVariable();

    public void emulatorReady() {
    	mainView.emulatorReady();
    }

    static {
        System.loadLibrary("droid48");
    }
	@Override
	protected void onResume() {
		super.onResume();
		Log.i("x48", "resume");
		if (mainView  != null)
			mainView.resume();
		Log.i("x48", "resumed");
	}

	 /**
    * Called when your activity's options menu needs to be created.
    */
   @Override
   public boolean onCreateOptionsMenu(Menu menu) {
       super.onCreateOptionsMenu(menu);

       // We are going to create two menus. Note that we assign them
       // unique integer IDs, labels from our string resources, and
       // given them shortcuts.
       //menu.add(0, RESET_ID, 0, R.string.reset);
       
       if (mainView.currentOrientation == Configuration.ORIENTATION_PORTRAIT)
    	   menu.add(0, KEYBLITE_ID, 0, R.string.show_lite_keyb);
       menu.add(0, SAVE_ID, 0, R.string.save_state);
       menu.add(0, LOAD_ID, 0, R.string.load_prog);
       menu.add(0, SETTINGS_ID, 0, R.string.settings);
       menu.add(0, RESET_ID, 0, R.string.reset_memory);
       menu.add(0, QUIT_ID, 0, R.string.button_quit);

       return true;
   }

   

   /**
    * Called when a menu item is selected.
    */
   @Override
   public boolean onMenuItemSelected(int featureId, MenuItem item) {
       switch (item.getItemId()) {
       case KEYBLITE_ID:
    	   	changeKeybLite();
    	   	return true;
       case RESET_ID:
	    	  AssetUtil.copyAsset(this, getResources().getAssets(), true);
	    	  //stopHPEmulator();
	           finish();
	           need_to_quit = true;
	           return true;
	       case SAVE_ID:
	    	  saveState();
	    	   return true;
	       
       	case LOAD_ID:
       		//loadProg("/data/data/org.ab.x48/SKUNK");
       		Intent loadFileIntent = new Intent();
       		loadFileIntent.setClass(this, ProgListView.class);
       		startActivityForResult(loadFileIntent, LOAD_ID);
       		//flipScreen();
       		break;
       	case SETTINGS_ID:
       		Intent settingsIntent = new Intent();
       		settingsIntent.setClass(this, Settings.class);
       		startActivityForResult(settingsIntent, SETTINGS_ID);
       		
       		break;
	       case QUIT_ID:
	    	   //stopHPEmulator();
	    	  // mainView.stop();
	           finish();
	           return true;
       }

       return super.onMenuItemSelected(featureId, item);
   }
   
   private static final int DIALOG_PROG_OK = 1;
   private static final int DIALOG_PROG_KO = 2;
   private static final int DIALOG_ROM_KO = 3;
   private static final int DIALOG_RAM_KO = 4;
   private static final int DIALOG_RAM_OK = 5;
   
   @Override
   protected Dialog onCreateDialog(int id) {
	   switch (id) {
       case DIALOG_PROG_OK: return new AlertDialog.Builder(X48.this)
	       .setIcon(R.drawable.alert_dialog_icon)
	       .setTitle(R.string.help)
	       .setMessage(R.string.prog_ok)
	       .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
	           public void onClick(DialogInterface dialog, int whichButton) {
	        	  
	           }
	       })
	       .create();
       case DIALOG_PROG_KO: return new AlertDialog.Builder(X48.this)
	   .setIcon(R.drawable.alert_dialog_icon)
	   .setTitle(R.string.help)
	   .setMessage(R.string.prog_ko)
	   .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
	       public void onClick(DialogInterface dialog, int whichButton) {
	
	           /* User clicked OK so do some stuff */
	       }
	   })
	   .create();
       case DIALOG_ROM_KO: return new AlertDialog.Builder(X48.this)
	   .setIcon(R.drawable.alert_dialog_icon)
	   .setTitle(R.string.help)
	   .setMessage(R.string.rom_ko)
	   .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
	       public void onClick(DialogInterface dialog, int whichButton) {
	
	    	   onDestroy();
	       }
	   })
	   .create();
       case DIALOG_RAM_KO: return new AlertDialog.Builder(X48.this)
	   .setIcon(R.drawable.alert_dialog_icon)
	   .setTitle(R.string.help)
	   .setMessage(R.string.ram_install_error)
	   .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
	       public void onClick(DialogInterface dialog, int whichButton) {
	
	    	   
	       }
	   })
	   .create();
       case DIALOG_RAM_OK: return new AlertDialog.Builder(X48.this)
	   .setIcon(R.drawable.alert_dialog_icon)
	   .setTitle(R.string.help)
	   .setMessage(R.string.ram_install_warning)
	   .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
	       public void onClick(DialogInterface dialog, int whichButton) {
	
	    	   
	       }
	   })
	   .create();
	   }
	   return null;
   }
   
   @Override
   protected void onActivityResult(final int requestCode, final int resultCode, final Intent extras) {
	   Log.i("x48", "requestCode: " + requestCode + " / " + resultCode);
   	super.onActivityResult(requestCode, resultCode, extras);
   	if (resultCode == RESULT_OK) {
   		switch (requestCode) {
   			case ROM_ID : {
   				/*if (true || isRomReady()) {
   	    		Log.i("x48", "Rom Ready... starting emulator");
   	    		readyToGo();
   	    	} else {
   	    		Log.i("x48", "Rom not Ready... quitting");
   	    		onDestroy();
   	    		finish();
   	    	}*/
   				break;
   			}
   			case LOAD_ID: {
   				final String filename = extras.getStringExtra("currentFile");
   				if (filename != null) {
   					int retCode = loadProg(filename);
   					if (retCode == 1) {
   						flipScreen();
   						SharedPreferences mPrefs = PreferenceManager.getDefaultSharedPreferences(this);
   						boolean msgbox = mPrefs.getBoolean("no_loadprog_msgbox", false);
   						if (!msgbox)
   							showDialog(DIALOG_PROG_OK);
   					} else {
   						showDialog(DIALOG_PROG_KO);
   					}
   				}
   				break;
   			}
   			case SETTINGS_ID: {
   				if (mainView != null)
   					mainView.updateContrast();
   				
   				checkPrefs();
   				
   			}
   		}
   	}
   }
   
   private boolean saveonExit;
   private boolean hp48s;
   private boolean bitmapSkin = false;
   
   public boolean isHp48s() {
	   return hp48s;
   }

   public boolean isBitmapSkin() {
	   return bitmapSkin;
   }

private void managePort(int number, String value) {
	   int size = Integer.parseInt(value);
	   File f = AssetUtil.getSDDir(this);
	   if (f != null) {
		   boolean change = false;
		   File port = new File(f, "port" + number);
		   /*if (port.exists()) {
			   port.renameTo(new File(f, "bkp.port" + number));
		   }*/
		   if (size == 0) {
			   if (port.exists()) {
				   port.delete();
				   Log.i("x48", "Deleting port" + number + " file.");
				   change = true;
			   }
		   } else {
			   if (port.exists() && port.length() == 1024 * size) {
				   
			   } else {
				   Log.i("x48", "Port" + number + " file does not exists or is incomplete. Writing a blank file.");
				   byte data [] = new byte [1024];
				   for(int i=0;i<data.length;i++)
					   data[i] = 0;
				   try {
					FileOutputStream fout = new FileOutputStream(port);
					for(int l=0;l<size;l++)
						fout.write(data);
					fout.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
					change = true;
			   }
		   }
		   if (change)
			   showDialog(DIALOG_RAM_OK);
	   } else {
		   showDialog(DIALOG_RAM_KO);
	   }
   }

	@Override
	protected void onStop() {
		super.onStop();
		Log.i("x48", "stop");
	}
	
	@Override
	protected void onStart() {
		super.onStart();
		Log.i("x48", "start");
	}

	@Override
	protected void onPause() {
		super.onPause();
		Log.i("x48", "pause");
		if (mainView  != null)
			mainView.pause();
		Log.i("x48", "paused");
	}

	@Override
	protected void onDestroy() {
		Log.i("x48", "onDestroy");
		super.onDestroy();
		if (saveonExit)
			saveState();
		stopHPEmulator();
		if (mainView  != null)
			mainView.unpauseEvent();
		if (need_to_quit)
			System.exit(0);
	}
	
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
	}


	public void registerClass() {
		String files_path = AssetUtil.getSDDir(this).getAbsolutePath();
		if (!files_path.endsWith("/"))
			files_path = files_path + "/";
		registerClass(this, files_path, hp48s?"roms":"rom", hp48s?"rams":"ram", hp48s?"hp48s":"hp48",
				hp48s?"port1s":"port1", hp48s?"port2s":"port2");
	}

}