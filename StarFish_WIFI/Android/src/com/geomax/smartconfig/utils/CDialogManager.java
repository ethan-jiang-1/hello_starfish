package com.geomax.smartconfig.utils;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;

import com.geomax.wifi.R;

/**
 * universal dialog class for app which pops up a dialog upon success or failure
 * or network failure cases
 */
public class CDialogManager implements OnClickListener {
	/**
	 * Called activity context
	 */
	private Context mContext = null;
	
	/**
	 * Alert dialog instance
	 */
	private AlertDialog.Builder mAlertDialog = null;

	/**
	 * Constructor for custom alert dialog.accepting context of called activity
	 * 
	 * @param mContext
	 */
	public CDialogManager(Context mContext) {
		this.mContext = mContext;
	}

	public void showCustomAlertDialog(int dialogType) {

		mAlertDialog = new AlertDialog.Builder(mContext);
		
		switch (dialogType) {

		case CConstants.DLG_NO_WIFI_AVAILABLE:
			mAlertDialog.setTitle(mContext.getResources().getString(
					R.string.alert_cc3x_title));
			mAlertDialog.setMessage(mContext.getResources().getString(
					R.string.alert_no_network_title));

			break;

		case CConstants.DLG_CONNECTION_SUCCESS:
			mAlertDialog.setTitle(mContext.getResources().getString(
					R.string.alert_cc3x_title));
			mAlertDialog.setMessage(mContext.getResources().getString(
					R.string.alert_successfully_connected));

			break;

		case CConstants.DLG_CONNECTION_FAILURE:
			mAlertDialog.setTitle(mContext.getResources().getString(
					R.string.alert_cc3x_title));
			mAlertDialog.setMessage(mContext.getResources().getString(
					R.string.alert_connection_failed));

			break;

		case CConstants.DLG_CONNECTION_TIMEOUT:
			mAlertDialog.setTitle(mContext.getResources().getString(
					R.string.alert_cc3x_title));
			mAlertDialog.setMessage(mContext.getResources().getString(
					R.string.alert_connection_timeout));

			break;

		case CConstants.DLG_SSID_INVALID:
			mAlertDialog.setTitle(mContext.getResources().getString(
					R.string.alert_invalid_input_title));
			mAlertDialog.setMessage(mContext.getResources().getString(
					R.string.alert_no_network_title));
			break;

		case CConstants.DLG_GATEWAY_IP_INVALID:
			mAlertDialog.setTitle(mContext.getResources().getString(
					R.string.alert_invalid_input_title));
			mAlertDialog.setMessage(mContext.getResources().getString(
					R.string.alert_no_network_title));
			break;

		case CConstants.DLG_KEY_INVALID:
			mAlertDialog.setTitle(mContext.getResources().getString(
					R.string.alert_invalid_input_title));
			mAlertDialog.setMessage(mContext.getResources().getString(
					R.string.alert_invalid_key_mesg));
			break;

		case CConstants.DLG_PASSWORD_INVALID:
			mAlertDialog.setTitle(mContext.getResources().getString(
					R.string.alert_invalid_input_title));
			mAlertDialog.setMessage(mContext.getResources().getString(
					R.string.alert_no_network_title));
			break;

		}
		
		mAlertDialog.setPositiveButton((mContext.getResources()
				.getString(R.string.cc3x_string_ok)).toUpperCase(), this);
		
		mAlertDialog.show();
	}

	@Override
	public void onClick(DialogInterface dialog, int dialogType) {

		if (dialogType == DialogInterface.BUTTON_POSITIVE) {
			dialog.dismiss();
		}
	}
}
