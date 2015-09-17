package com.example.deamonservice;

import java.io.File;

import android.content.Context;
import android.util.Log;

public class Daemon {

	private static final String TAG = "com.example.daemon";
	public static final int INTERVAL_ONE_MINUTE = 5;

	public static void run(final Context context,final Class<?> daemonServiceClass,final int interval){
		new Thread(new Runnable() {
			@Override
			public void run() {
				Command.install(context, "bin", "daemon");
				start(context, daemonServiceClass, interval);
			}
		}).start();
	}

	protected static void start(Context context, Class<?> daemonServiceClass,
			int interval) {
		String cmd=context.getDir("bin", Context.MODE_PRIVATE)
				.getAbsolutePath() + File.separator + "daemon";
		
		StringBuilder cmdBuilder = new StringBuilder();
		cmdBuilder.append(cmd);
		cmdBuilder.append(" -p ");
		cmdBuilder.append(context.getPackageName());
		cmdBuilder.append(" -s ");
		cmdBuilder.append(daemonServiceClass.getName());
		cmdBuilder.append(" -t ");
		cmdBuilder.append(interval);
		
	
	  try {
		Runtime.getRuntime().exec(cmdBuilder.toString()).waitFor();
	} catch (Exception e) {
		Log.e(TAG, "start daemon error: " + e.getMessage());
	}
	
	}
}
