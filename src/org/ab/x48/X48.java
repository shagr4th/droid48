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
	static final private int LOAD_ID = Menu.FIRST +1;
	static final private int SAVE_ID = Menu.FIRST +2;
	static final private int SETTINGS_ID = Menu.FIRST +5 ;
	static final private int QUIT_ID = Menu.FIRST +3 ;
	static final private int RESET_ID = Menu.FIRST +4 ;
	
	static final private int ROM_ID = 123;
	
    // http://www.hpcalc.org/hp48/pc/emulators/gxrom-r.zip
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //if (savedInstanceState == null) {
     	   Log.i("x48", "starting activity");
     	  /*if (!isRomReady()) {
     		  // need to download the rom
     		  Log.i("x48", "Need to download the rom...");
     		 
     		  Intent intent = new Intent(this, ROMDownloadActivity.class);
     		 startActivityForResult(intent, 1);
     	  } else {*/
     	  AssetUtil.copyAsset(getResources().getAssets(), false);
     	  readyToGo() ;
     	  //}
     	 if (!AssetUtil.isFilesReady()) {
         	showDialog(DIALOG_ROM_KO);
         }
       // }
        
     	
    }
    
    
    /*
    private boolean isRomReady() {
    	SharedPreferences mPrefs = getSharedPreferences("x48", 0);
   	  String romLocation = mPrefs.getString("rom_location", null);
   	  return (romLocation != null && new File(romLocation).exists() && new File(romLocation).length() == 524288);
    }
   */
    
    public void readyToGo() {
    	requestWindowFeature(Window.FEATURE_NO_TITLE);
        requestWindowFeature(Window.FEATURE_PROGRESS);
        setContentView(R.layout.main);
        mainView = (HPView) findViewById(R.id.hpview);
        
        checkPrefs();
    }
    
    public void checkPrefs() {
    	SharedPreferences mPrefs = PreferenceManager.getDefaultSharedPreferences(this);
		saveonExit = mPrefs.getBoolean("saveOnExit", false);
		if (mainView != null) {
			mainView.setHapticFeedbackEnabled(mPrefs.getBoolean("haptic", true));
			mainView.setFullWidth(mPrefs.getBoolean("large_width", false));
			mainView.setKeybLite(mPrefs.getBoolean("keybLite", false));
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
    
    public native String startHPEmulator();
    public native String resetHPEmulator();
    public native String saveState();
    public native String stopHPEmulator();
    public native int buttonPressed(int code);
    public native int buttonReleased(int code);
    public native void registerClass(X48 instance);
    public native int fillScreenData(short data []);
    public native void flipScreen();
    public native int loadProg(String filename);
    public native void setBlankColor(short s);
    
    public void emulatorReady() {
    	mainView.emulatorReady();
    }
    
    public void pauseEvent() {
    	mainView.pauseEvent();
    }
    
    static {
        System.loadLibrary("droid48");
    }
	@Override
	protected void onResume() {
		super.onResume();
		Log.i("x48", "resuming");
		if (mainView  != null)
			mainView.resume();
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
       case RESET_ID:
	    	  AssetUtil.copyAsset(getResources().getAssets(), true);
	    	  stopHPEmulator();
	    	   mainView.stop();
	           finish();
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
   				
   				SharedPreferences mPrefs = PreferenceManager.getDefaultSharedPreferences(this);
   				String port1 = mPrefs.getString("port1", "0");
   				managePort(1, port1);
   				String port2 = mPrefs.getString("port2", "0");
   				managePort(2, port2);
   				checkPrefs();
   				
   			}
   		}
   	}
   }
   
   private boolean saveonExit;
    
   private void managePort(int number, String value) {
	   int size = Integer.parseInt(value);
	   File f = AssetUtil.getSDDir();
	   if (f != null) {
		   boolean change = false;
		   File port = new File(f, "port" + number);
		   /*if (port.exists()) {
			   port.renameTo(new File(f, "bkp.port" + number));
		   }*/
		   if (size == 0) {
			   if (port.exists()) {
				   port.delete();
				   change = true;
			   }
		   } else {
			   if (port.exists() && port.length() == 1024 * size) {
				   
			   } else {
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
		/*if (mainView  != null)
			mainView.stop();*/
		super.onStop();
		Log.i("x48", "stop");
		/*stopHPEmulator();
		mainView.stop();
		System.exit(1);*/
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
			mainView.pause(false);
		/*stopHPEmulator();
		mainView.stop();
		System.exit(1);*/
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
		
	}
	
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
	}

}