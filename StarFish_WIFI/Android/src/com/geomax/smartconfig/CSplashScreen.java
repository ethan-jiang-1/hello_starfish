package com.geomax.smartconfig;

import android.app.Activity;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.view.KeyEvent;

import com.geomax.smartconfig.utils.CConstants;
import com.geomax.smartconfig.utils.CUtils;
import com.geomax.wifi.R;

public class CSplashScreen extends Activity {

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		// Called initially to set the orientation to portrait only for mobile
		// and both for tablets
		CUtils.setProtraitOrientationEnabled(CSplashScreen.this);

		// Setting the splash screen view to Activity
		setContentView(R.layout.splash_screen);

		// Timer to start CC3XConfigActivity
		startTimer();
	}

	/**
	 * Starts the next activity with a little delay
	 */
	private void startTimer() {
		
		Handler splashTimer = new Handler();
		
		splashTimer.postDelayed(new Runnable() {
			public void run() {
				Intent configIntent = new Intent(CSplashScreen.this,
						CConfigActivity.class);
				startActivity(configIntent);
				finish();
			}
		}, CConstants.SPLASH_DELAY);
	}

	/**
	 * gets called in activity when a device is rotated in any side.. so we set
	 * the orientation based on screen size.
	 */
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		// TODO Auto-generated method stub
		super.onConfigurationChanged(newConfig);

		if (!(CUtils.isScreenXLarge(getApplicationContext()))) {
			return;
		}
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub

		return true;
	}

	/**
	 * Overriding back press function to ignore the back key event when user
	 * press back while processing the Splashscreen wait thread
	 */
	@Override
	public void onBackPressed() {
		return;
	}
}
