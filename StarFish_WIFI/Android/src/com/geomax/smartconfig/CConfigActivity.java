package com.geomax.smartconfig;

import java.net.InetAddress;
import java.util.Timer;
import java.util.TimerTask;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.NetworkInfo.DetailedState;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.geomax.smartconfig.utils.CConstants;
import com.geomax.smartconfig.utils.CDialogManager;
import com.geomax.smartconfig.utils.CUtils;
import com.geomax.smartconfig.utils.CWifiManager;
import com.geomax.wifi.R;


public class CConfigActivity extends Activity implements OnClickListener,
		OnCheckedChangeListener {
	
	// WIFI Manager instance which gives the network related information like
	// WIFI ,SSID etc.
	private CWifiManager mWifiManager = null;

	private Button mSendDataPackets = null;

	private EditText mSSIDInputField     = null;
	private EditText mPasswordInputField = null;
	private EditText mGateWayInputField  = null;
	private EditText mNetMaskInputField  = null;
	private EditText mIpInputField       = null;
	private EditText mDnsInputField      = null;

	/**
	 * A Dialog instance which is responsible to generate all dialogs in the app
	 */
	private CDialogManager mDialogManager = null;

	/**
	 * A check box which when checked sends the encryption key to server as a
	 * input and checks if key is exactly 16chars
	 */
	private CheckBox mConfigIPCheckbox = null;

	private ProgressBar mConfigProgress = null;

	/**
	 * Called initially and loading of views are done here Orientation is
	 * restricted for mobile phones to portrait only and both orientations for
	 * tablet
	 */

	/**
	 * Timer instance to activate network wifi check for every few minutes Popps
	 * out an alert if no network or if connected to any network
	 */
	private Timer timer = new Timer();

	/**
	 * A boolean to check if network alert is visible or invisible. If visible
	 * we are dismissing the existing alert
	 */
	public static boolean sIsNetworkAlertVisible = false;

	// Boolean to check if network is enabled or not
	private boolean mIsNetworkConnecting = false;

	/**
	 * Dialog ID to trigger no network dialog
	 */
	private static final int NO_NETWORK_DIALOG_ID = 002;

	private CSmartConfig mSmartConfig = null;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		// Disable orientation if launched in mobile
		CUtils.setProtraitOrientationEnabled(CConfigActivity.this);

		setContentView(R.layout.configration);

		// Check for WIFI initially
		if (!mIsNetworkConnecting && !sIsNetworkAlertVisible) {
			checkNetwork("ONCREATE");
		}

		// Initialize all view components in screen
		initViews();

		// Initialize all click listeners to views
		setViewClickListeners();

		// Set initial data to all non editable components
		initData();

		timerDelayForAPUpdate();

		// Initializing the intent Filter for network check cases and
		// registering the events to broadcast receiver
		
		IntentFilter intentFilter = new IntentFilter();
		intentFilter.addAction(WifiManager.SUPPLICANT_CONNECTION_CHANGE_ACTION);
		intentFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
		intentFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
		intentFilter.addAction(WifiManager.SUPPLICANT_STATE_CHANGED_ACTION);

		registerReceiver(mBroadcastReceiver, intentFilter);
	}

	// returns the WIFI Manager instance which gives the network related
	// information like WIFI ,SSID etc.
	public CWifiManager getWiFiManagerInstance() {

		if (mWifiManager == null) {
			mWifiManager = new CWifiManager(CConfigActivity.this);
		}

		return mWifiManager;
	}

	public static byte[] ipToBytesByInet(String ipAddr) {
    
		try {
            return InetAddress.getByName(ipAddr).getAddress();
        } catch (Exception e) {
            return null;
        }
	}
	
	// Initialize all view components in screen with input data
	private void initData() {

		if (getWiFiManagerInstance().getCurrentSSID() != null
				&& getWiFiManagerInstance().getCurrentSSID().length() > 0) {

			mSSIDInputField.setText(getWiFiManagerInstance().getCurrentSSID());

			/**
			 * removing the focus of SSID when field is already configured from
			 * Network
			 */
			mSSIDInputField.setEnabled(false);
			mSSIDInputField.setFocusable(false);
			mSSIDInputField.setFocusableInTouchMode(false);
		}

		mGateWayInputField.setText(getWiFiManagerInstance().getGateway());
	}

	// Check for WIFI network if not throw a alert to user
	private boolean checkNetwork(String str) {

		if (!(getWiFiManagerInstance().isWifiConnected())) {
			sIsNetworkAlertVisible = true;
			mDialogManager = new CDialogManager(CConfigActivity.this);
			showDialog(NO_NETWORK_DIALOG_ID);
			return false;
		} else {
			return true;
		}
	}

	private void initViews() {

		mSendDataPackets    = (Button) findViewById(R.id.config_start_button);
		mSSIDInputField     = (EditText) findViewById(R.id.config_ssid_input);
		mPasswordInputField = (EditText) findViewById(R.id.config_passwd_input);
		mIpInputField       = (EditText) findViewById(R.id.ip_address_input);
		mNetMaskInputField  = (EditText) findViewById(R.id.net_mask_input);
		mGateWayInputField  = (EditText) findViewById(R.id.gateway_input);
		mDnsInputField      = (EditText) findViewById(R.id.dns_input);
		mConfigProgress     = (ProgressBar) findViewById(R.id.config_progress);
		mConfigIPCheckbox   = (CheckBox) findViewById(R.id.config_ip_checkbox);

		mConfigIPCheckbox.setChecked(false);
		mConfigIPCheckbox.setOnCheckedChangeListener(this);

		if (!mConfigIPCheckbox.isChecked()) {

			mIpInputField.setEnabled(false);
			mNetMaskInputField.setEnabled(false);
			mGateWayInputField.setEnabled(false);
			mDnsInputField.setEnabled(false);
		}
	}

	// Init the click listeners of all required views
	private void setViewClickListeners() {
		mSendDataPackets.setOnClickListener(this);
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {

		case R.id.config_start_button:
			
			// Check network
			if (checkNetwork("bUTTON")) {
				try {
					sendPacketData();
				} catch (Exception e) {

				}
			}
			
			break;
		}
	}

	public boolean mIsCalled = false;

	// Send the packet data to server
	private void sendPacketData() throws Exception {

		if (!mIsCalled) {

			mIsCalled = true;
			mSendDataPackets.setText(getResources().getString(
					R.string.stop_label));

			mSmartConfig = getSmartConfigInstance();
			mSmartConfig.transmitSettings();

			mSendDataPackets
					.setBackgroundResource(R.drawable.selection_focus_btn);

			mConfigProgress.setVisibility(ProgressBar.VISIBLE);
		} else {
			if (mSendDataPackets != null) {
				stopPacketData();
			}
		}
	}

	// Stops the transmission of live packets to server. callback of FTC_SUCCESS
	// or failure also will trigger this method.
	private void stopPacketData() {
		
		if (mIsCalled) {
		
			try {
				mIsCalled = false;

				mSendDataPackets.setText(getResources().getString(
						R.string.start_label));

				mSendDataPackets.setBackgroundResource(R.drawable.selection);
				
				if (mConfigProgress != null) {
					mConfigProgress.setVisibility(ProgressBar.INVISIBLE);
				}

				mSmartConfig.stopTransmitting();

			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	private CSmartConfig getSmartConfigInstance() {

		String ssidFieldTxt = mSSIDInputField.getText().toString().trim();
		String passwdText = mPasswordInputField.getText().toString().trim();
		
		byte[] ip      = ipToBytesByInet(mIpInputField.getText().toString().trim());
		byte[] mask    = ipToBytesByInet(mNetMaskInputField.getText().toString().trim());
		byte[] gateway = ipToBytesByInet(mGateWayInputField.getText().toString().trim());
		byte[] dns     = ipToBytesByInet(mDnsInputField.getText().toString().trim());
		
		return new CSmartConfig(ssidFieldTxt, passwdText, ip, mask, gateway, dns);
	}

	// Logic to restrict the orientation in mobile phones only because of size
	// constraint
	
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		
		super.onConfigurationChanged(newConfig);
		
		if (!(CUtils.isScreenXLarge(getApplicationContext()))) {
			return;
		}
	}

	// Show timeout alert
	private void showConnectionTimedOut(int dialogType) {

		if (mDialogManager == null) {
			mDialogManager = new CDialogManager(CConfigActivity.this);
		}

		mDialogManager.showCustomAlertDialog(dialogType);
	}

	// Show Failure alert
	private void showFailureAlert(int dialogType) {

		if (mDialogManager == null) {
			mDialogManager = new CDialogManager(CConfigActivity.this);
		}

		mDialogManager.showCustomAlertDialog(dialogType);
	}

	// Throws an alert to user stating the success message received after
	// configuration
	private void showConnectionSuccess(int dialogType) {

		if (mDialogManager == null) {
			mDialogManager = new CDialogManager(CConfigActivity.this);
		}

		mDialogManager.showCustomAlertDialog(dialogType);
	}

	@Override
	protected void onResume() {
		
		super.onResume();
		
		// Check for network cases when app is minimized again
		
		if (!sIsNetworkAlertVisible) {
			
			if (mIsNetworkConnecting) {

			} else {
				if (!(getWiFiManagerInstance().isWifiConnected())) {
					showDialog(NO_NETWORK_DIALOG_ID);
				}
			}
			
			sIsNetworkAlertVisible = false;
		}
	}

	@Override
	protected void onStop() {
		super.onStop();
		/**
		 * Calling Stop once again
		 */
		stopPacketData();
	}

	@Override
	protected void onPause() {
		super.onPause();
	}

	@Override
	public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {

		if (isChecked) {
			mIpInputField.setEnabled(true);
			mNetMaskInputField.setEnabled(true);
			mGateWayInputField.setEnabled(true);
			mDnsInputField.setEnabled(true);
		} else {
			mIpInputField.setEnabled(false);
			mNetMaskInputField.setEnabled(false);
			mGateWayInputField.setEnabled(false);
			mDnsInputField.setEnabled(false);
		}
	}

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {

			switch (msg.what) {
			case CConstants.DLG_CONNECTION_FAILURE:
				showFailureAlert(CConstants.DLG_CONNECTION_FAILURE);
				break;

			case CConstants.DLG_CONNECTION_SUCCESS:
				showConnectionSuccess(CConstants.DLG_CONNECTION_SUCCESS);
				break;

			case CConstants.DLG_CONNECTION_TIMEOUT:
				showConnectionTimedOut(CConstants.DLG_CONNECTION_TIMEOUT);
				break;
			}
			/**
			 * Stop transmission
			 */
			stopPacketData();
		}
	};

	/**
	 * Timer to check network periodically at some time intervals. If network is
	 * available then the current access point details are set to the SSID field
	 * in next case else a alert dialog is shown
	 */
	void timerDelayForAPUpdate() {

		int periodicDelay = 1000;   // delay for 1 sec.
		int timeInterval  = 180000; // repeat every 3minutes.

		timer.scheduleAtFixedRate(new TimerTask() {
			
			public void run() {
			
				runOnUiThread(new Runnable() {
				
					public void run() {
						
						// stuff that updates ui
						
						if (!sIsNetworkAlertVisible) {
							if (!(getWiFiManagerInstance().isWifiConnected())) {
								showDialog(NO_NETWORK_DIALOG_ID);
							}
						}
					}
				});
			}
		}, periodicDelay, timeInterval);
	}

	@Override
	protected void onDestroy() {
		
		super.onDestroy();
		
		// back end time for network check is unregistered
		timer.cancel();

		// the Network change or WIFI change broadcast registered in onCreate is
		// unregistered at the finish of activity to ensure there is no
		// IntentFilter leak in the onDestroy()
		
		unregisterReceiver(mBroadcastReceiver);
	}

	// Common Alert dialog instance to show no network or AP change message
	private AlertDialog alert = null;

	/**
	 * Dialog creation is done here
	 */
	@Override
	protected Dialog onCreateDialog(int id) {
		AlertDialog.Builder builder1;
		try {

			if (alert != null) {
				if (alert.isShowing()) {
					alert.dismiss();
				}
			}
		} catch (Exception e) {

		}

		switch (id) {
		case NO_NETWORK_DIALOG_ID:
			sIsNetworkAlertVisible = true;
			builder1 = new AlertDialog.Builder(this);
			builder1.setCancelable(true)
					.setTitle(
							getResources().getString(R.string.alert_cc3x_title))
					.setMessage(
							getResources().getString(
									R.string.alert_no_network_title))
					.setCancelable(false)
					.setPositiveButton("OK",
							new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog,
										int id) {
									sIsNetworkAlertVisible = false;

								}
							});

			alert = builder1.create();
			alert.show();
			break;
		}
		return super.onCreateDialog(id);
	}

	private void UpdateIPInputField() {
		
		if(mIsNetworkConnecting) {
			
			mSSIDInputField.setText(getWiFiManagerInstance()
					.getCurrentSSID());

			mSSIDInputField.setEnabled(false);
			mSSIDInputField.setFocusable(false);
			mSSIDInputField.setFocusableInTouchMode(false);

			mGateWayInputField.setText(getWiFiManagerInstance()
					.getGateway());

			mNetMaskInputField.setText(getWiFiManagerInstance()
					.getNetMask());

			mDnsInputField.setText(getWiFiManagerInstance().getDns());
		}
	
	}
	
	// A broadcast receiver which is registered to notify the app about the
	// changes in network or Access point is switched by the Device WIfimanager
	
	BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {

			final String action = intent.getAction();

			if (action.equals(WifiManager.SUPPLICANT_CONNECTION_CHANGE_ACTION)) {

				if (intent.getBooleanExtra(WifiManager.EXTRA_SUPPLICANT_CONNECTED, false)) {

				} else {
					
					// WIFI connection was lost
					if (!sIsNetworkAlertVisible) {
						if (!(getWiFiManagerInstance().isWifiConnected())) {

						}
					}
					
					// Clearing the previous SSID Gateway ip if access point is
					// disconnected in between connection or usage
					
					mSSIDInputField.setText("");
					
					mIpInputField.setText("0.0.0.0");
					mNetMaskInputField.setText("0.0.0.0");
					mGateWayInputField.setText("0.0.0.0");
					mDnsInputField.setText("0.0.0.0");
				}
			}

			if (action.equals(WifiManager.NETWORK_STATE_CHANGED_ACTION)) {

				NetworkInfo info = intent
						.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
	
				if (info.getDetailedState() == DetailedState.CONNECTED) {

					mIsNetworkConnecting = true;

					UpdateIPInputField();
				}
			}
		}
	};
}