package org.ab.x48;

import android.content.Context;
import android.util.Log;

public class EmulatorThread extends Thread {
	
	private X48 x48;
	
	public EmulatorThread(Context x48) {
		this.x48 = (X48) x48;
	}
	
	public void run() {
		x48.registerClass();
		Log.i("x48", "startHPEmulator");
		x48.startHPEmulator();
		Log.i("x48", "endHPEmulator");
	}

	public void exit() {
		Log.i("x48", "stopHPEmulator");
		x48.stopHPEmulator();
		try {
			Thread.sleep(5000);
			Log.i("x48", "join");
			join(1000);
			Log.i("x48", "joined");
		} catch (InterruptedException e) {
			Log.e("x48", e.getMessage());
		}
	}
}
