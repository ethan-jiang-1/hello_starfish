package com.geomax.smartconfig;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import android.os.StrictMode;

public class CSmartConfig {
	
	private static final int MAX_SENDING_TIMES = 100;
	private static final int UDP_DATA_LEN  = 10;
	private static final int UDP_DATA_PORT = 9957;
	
	private DatagramSocket mUPDSocket;

	class SmartConfigData{	
		
		private static final int LEN_BYTES = 116;

		public String ap_ssid;	
		public String ap_passkey;	

		public byte[] ip   = new byte[4];
		public byte[] mask = new byte[4];
		public byte[] gw   = new byte[4];
		public byte[] dns  = new byte[4];
		
		private int catBytes(byte[] dstBuf, int pos, byte[] srcBuf) {
			
			for (int i = 0; i < srcBuf.length; i++)
				dstBuf[i + pos] = srcBuf[i];
			
			return srcBuf.length;
		}
		
		private byte checkSum(byte[] buf, int len) {
		    
			byte checksum = 0;

		    for (int i=0; i<len; i++) 
		        checksum += buf[i];

		    return (byte) (~checksum + 2);
		}
		
		public byte[] toBytes() {
			
			byte[] out = new byte[2 + ap_ssid.length() + ap_passkey.length() + 2 + 4 * 4];
			
			out[0] = (byte) out.length;
			out[1] = checkSum(out, 1);
			
			int index = 2;
			index += catBytes(out, index, ap_ssid.getBytes());

			out[index] = '\0';
			index ++;

			index += catBytes(out, index, ap_passkey.getBytes());

			out[index] = '\0';
			index ++;
			
			// IP Address
			index += catBytes(out, index, ip);
			index += catBytes(out, index, mask);
			index += catBytes(out, index, gw);
			index += catBytes(out, index, dns);
			
			return out;
		}
		
		public int getLength() {
			return LEN_BYTES;
		}
	};	
	
	private SmartConfigData mConfigData = new SmartConfigData();	
	
	public void transmitSettings() {

		// 解决了android 4.0上版本udp发送失败问题。
		if (android.os.Build.VERSION.SDK_INT >= 9) {
			StrictMode.ThreadPolicy old = StrictMode.getThreadPolicy();
			StrictMode.setThreadPolicy(new StrictMode.ThreadPolicy.Builder(old)
					.permitNetwork().build());
		}
		
		initUDP();
		
		final byte[] socketBuffer = getSmartConfigData();		
		final byte[] ipAddress = new byte[4];
		
		ipAddress[0] = (byte) 239;
		ipAddress[1] = (byte) 0;
		ipAddress[2] = (byte) 0;
		ipAddress[3] = (byte) 254;
		
		Thread t = new Thread(new Runnable() {
		
			public void run() {
		
				boolean exitThread = false;
				
				for (int i = 0; i < MAX_SENDING_TIMES && !exitThread; i++) {
				
					for(int j=0; j<socketBuffer.length; j++) {
						
						ipAddress[1] = (byte) j;
						ipAddress[2] = socketBuffer[j];
						
						InetAddress address;
						
						try {
							address = InetAddress.getByAddress(ipAddress);
							
							try {
								mUPDSocket.send(new DatagramPacket(socketBuffer, UDP_DATA_LEN,
										address, UDP_DATA_PORT));
							} catch (IOException e) {
								
								e.printStackTrace();
								exitThread = true;
								break; 
							}
							
							try {
								Thread.sleep(20);
							} catch (InterruptedException e) {
								
								e.printStackTrace();								
								exitThread = true;
								break; 			
							}
							
						} catch (UnknownHostException e1) {
							e1.printStackTrace();
							
							exitThread = true;
							break; 							
						}
					}							
				}
			}
		});
		
		t.start();	
	}
	
	public void stopTransmitting() {
		exitUDP();
	}
	
	CSmartConfig(String strSSID, String strPassword, 
			byte[] ip, byte[] mask, byte[] gateway, byte[] dns) {
		
		mConfigData.ap_ssid = strSSID;
		mConfigData.ap_passkey = strPassword;
		
		mConfigData.ip = ip;
		mConfigData.mask = mask;
		mConfigData.gw = gateway;
		mConfigData.dns = dns;
	}
	
	private byte[] getSmartConfigData()	{
		
		return mConfigData.toBytes();	
		
	}	
	
	private void initUDP() {
		
		try {
			mUPDSocket = new DatagramSocket();
			mUPDSocket.setSoTimeout(2 * 60 * 1000); // set 2mins
		} catch (SocketException e) {
			e.printStackTrace();
		}
	}

	private void exitUDP() {
		if (mUPDSocket != null) {
			mUPDSocket.close();
		}
	}	
}