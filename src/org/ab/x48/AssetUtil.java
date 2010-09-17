package org.ab.x48;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import android.content.res.AssetManager;
import android.util.Log;

public class AssetUtil {
	
	public static void copyAsset(AssetManager am, boolean force) {
		File sd = new File("/sdcard");
		if (sd.exists() && sd.isDirectory()) {
			File hpDir = new File("/sdcard/.hp48");
			copyAsset(am, hpDir.exists() || hpDir.mkdir(), force);
		} else {
			copyAsset(am, false, force);
		}
	}
	
	public static File getSDDir() {
		File hpDir = new File("/sdcard/.hp48");
		if (hpDir.exists())
			return hpDir;
		return null;
	}
	
	public static void copyAsset(AssetManager am, boolean sd, boolean force) {
		try {
			String assets[] = am.list( "" );
			for( int i = 0 ; i < assets.length ; i++ ) {
				boolean hp48 = assets[i].equals("hp48");
				boolean ram = assets[i].equals("ram");
				boolean rom = assets[i].equals("rom");
				int required = 0;
				if (ram)
					required = 131072;
				else if (rom)
					required = 524288;
				//boolean SKUNK = assets[i].equals("SKUNK");
				if (hp48 || rom || ram) {
					String rep = sd?"/sdcard/.hp48/":"/data/data/org.ab.x48/";
					File fout = new File(rep + assets[i]);
					if (!fout.exists() || fout.length() == 0 || (required > 0 && fout.length() != required) || force) {
						Log.i("x48", "Overwriting " + assets[i]);
						FileOutputStream out = new FileOutputStream(fout);
						InputStream in = am.open(assets[i]);
						byte buffer [] = new byte [8192];
						int n = -1;
						while ((n=in.read(buffer)) > -1) {
							out.write(buffer, 0, n);
						}
						out.close();
						in.close();
					}
				}
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public static boolean isFilesReady() {
		File hpDir = new File("/sdcard/.hp48");
		if (!hpDir.exists() || !hpDir.isDirectory()) {
			hpDir = new File("/data/data/org.ab.x48/");
			if (!hpDir.exists() || !hpDir.isDirectory())
				return false;
		}
		File hp = new File(hpDir, "hp48");
		File rom = new File(hpDir, "rom");
		File ram = new File(hpDir, "ram");
		return hp.exists() && hp.length() > 0 && rom.exists() && rom.length() > 0 && ram.exists() && ram.length() > 0;
	}

}
