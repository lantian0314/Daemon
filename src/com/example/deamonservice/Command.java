package com.example.deamonservice;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import android.content.Context;
import android.content.res.AssetManager;
import android.os.Build;
import android.util.Log;

public class Command {

	private static final String TAG = "com.example.daemon";

	public static boolean install(Context context,String desDir,String filename){
		String abi=Build.CPU_ABI;
		if (!abi.startsWith("arm")) {
			return false;
		}
		try {
			File file=new File(context.getDir(desDir, Context.MODE_PRIVATE), filename);
			if (file.exists()) {
				Log.e(TAG, "binary has existed");
				return false;
			}
			copyAssets(context,filename,file,"0755");
			return true;
		} catch (Exception e) {
			
		}
		return false;
	}

	private static void copyAssets(Context context, String assetsFilename, File file,
			String mode) {
		try {
			AssetManager manager = context.getAssets();
			final InputStream is = manager.open(assetsFilename);
			copyFile(file, is, mode);
		} catch (IOException e) {
			e.printStackTrace();
		}
		
	}

	private static void copyFile(File file, InputStream is, String mode) {
		 try {
			String abspath = file.getAbsolutePath();
			 FileOutputStream fos=new FileOutputStream(file);
			 byte[] bt=new byte[1024];
			 int len=0;
			 while ((len=is.read(bt))>0) {
				fos.write(bt, 0, len);
			}
			fos.close();
			is.close();
			Runtime.getRuntime().exec("chmod " + mode + " " + abspath).waitFor();
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}
