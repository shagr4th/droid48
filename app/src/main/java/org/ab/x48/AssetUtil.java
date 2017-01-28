package org.ab.x48;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import android.content.Context;
import android.content.res.AssetManager;
import android.os.Environment;
import android.util.Log;

public class AssetUtil {
	
	public static boolean copyAsset(Context context, AssetManager am, boolean force) {
		File newDir = getSDDir(context);
		File sd = Environment.getExternalStorageDirectory();
		if (sd != null && sd.exists() && sd.isDirectory() && newDir != null && newDir.exists() && newDir.isDirectory()) {
			File hpDir = new File(sd, ".hp48");
			if (hpDir.exists()) {
				File allFiles [] = hpDir.listFiles();
				if (allFiles != null && allFiles.length > 0) {
					Log.i("x48", "Moving x48 files from the old dir " + sd.getAbsolutePath() + " to the proper one :");
					for(File file:allFiles) {
						File newFile = new File(newDir, file.getName());
						Log.i("x48", "Moving " + file.getAbsolutePath() + " to " + newFile);
						file.renameTo(newFile);
					}
				}
				Log.i("x48", "Deleting old directory");
				hpDir.delete();
			}
		}
		return copyAsset(am, newDir, force);
	}
	
	public static File getSDDir(Context context) {
		if (Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState())) {
			// We can read and write the media
			return context.getExternalFilesDir(null);
		} else {
			// Load another directory, probably local memory
			return context.getFilesDir();
		}
	}
	
	private static boolean copyAsset(AssetManager am, File rep, boolean force) {
		boolean existingInstall = false;
		try {
			String assets[] = am.list( "" );
			for( int i = 0 ; i < assets.length ; i++ ) {
				boolean hp48 = assets[i].equals("hp48");
				boolean hp48s = assets[i].equals("hp48s");
				boolean ram = assets[i].equals("ram");
				boolean rom = assets[i].equals("rom");
				boolean rams = assets[i].equals("rams");
				boolean roms = assets[i].equals("roms");
				int required = 0;
				if (ram)
					required = 131072;
				else if (rams)
					required = 32768;
				else if (rom)
					required = 524288;
				else if (roms)
					required = 262144;
				//boolean SKUNK = assets[i].equals("SKUNK");
				if (hp48 || rom || ram || hp48s || roms || rams) {
					File fout = new File(rep, assets[i]);
					existingInstall |= fout.exists();
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
		return existingInstall;
	}
	
	public static boolean isFilesReady(Context context) {
		File hpDir = getSDDir(context);
		if (!hpDir.exists() || !hpDir.isDirectory()) {
			return false;
		}
		File hp = new File(hpDir, "hp48");
		File rom = new File(hpDir, "rom");
		File ram = new File(hpDir, "ram");
		return hp.exists() && hp.length() > 0 && rom.exists() && rom.length() > 0 && ram.exists() && ram.length() > 0;
	}

}
