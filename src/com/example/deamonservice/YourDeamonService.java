package com.example.deamonservice;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

public class YourDeamonService extends Service{

	@Override
	public void onCreate() {
		super.onCreate();
		Daemon.run(this, YourDeamonService.class, Daemon.INTERVAL_ONE_MINUTE);
	}
	@Override
	public IBinder onBind(Intent intent) {
		// TODO Auto-generated method stub
		return null;
	}

	
}
