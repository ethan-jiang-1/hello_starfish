// BowheadImager.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock.h>
#include "partition.h"
SOCKET m_Socket;
char g_cServerIP[]={"192.168.1.1"};
unsigned short g_ucServerPort=6000;
#define DISP_FUN_NAME(A) printf("*******->%s\r\n",A)

#define DISP_EXIT_FUN_NAME(A)  printf("------***%s\r\n",A)
#define TX_BUFFER_LEN 4096
bool SendDataOverTCP(char * buffer,int len);
#define MAX_DATA_BUFFER_LENTH 512
DWORD TcpIPClientRevThread();
HANDLE  g_socket_evelt=NULL; 
void Eink_Switch_Image( int index)
{
	char *pTxHead=(char *)malloc(100);
	memset(pTxHead,0,100);
	sprintf(pTxHead,"%cB1Q,100,%d,0,0\r\n",index);
	if(!SendDataOverTCP(pTxHead,strlen(pTxHead)))
	{
		printf("Error\r\n");
	}
	else
	{
		printf("E");
	}
	free(pTxHead);
	Sleep(10);
}

void flash_Erase_Chip( )
{
	char *pTxHead=(char *)malloc(100);
	memset(pTxHead,0,100);
	sprintf(pTxHead,"%cB1Q,802,0,0,0\r\n",0x25);
	if(!SendDataOverTCP(pTxHead,strlen(pTxHead)))
	{
		printf("Error\r\n");
	}
	else
	{
		printf("E");
	}
	free(pTxHead);
	Sleep(500);
}
void flash_Erase_Sector(int addr )
{
	char *pTxHead=(char *)malloc(100);
	memset(pTxHead,0,100);
	sprintf(pTxHead,"%cB1Q,801,%d,0,0\r\n",0x25,addr);
	if(!SendDataOverTCP(pTxHead,strlen(pTxHead)))
	{
		printf("Error\r\n");
	}
	else
	{
		printf(".");
	}
	Sleep(300);
	free(pTxHead);
	
}
bool flash_write_data (unsigned int WAddr, char *buf, int len )
{
	if(WAddr % 4096==0)
	{
		//flash_Erase_Sector( WAddr );
	}
	char *pTxHead=(char *)malloc(100);
	memset(pTxHead,0,100);
	sprintf(pTxHead,"%cB1Q,800,%d,%d,",0x25,WAddr,len);

	char *TxBuffer=(char *)malloc(TX_BUFFER_LEN);
	memset(TxBuffer,0,TX_BUFFER_LEN);

	memcpy((char *)TxBuffer,pTxHead,strlen(pTxHead));
	memcpy(TxBuffer+strlen(pTxHead),buf,len);
	memset(TxBuffer+strlen(pTxHead)+len,0x0d,1);
	memset(TxBuffer+strlen(pTxHead)+len+1,0x0a,1);

	if(!SendDataOverTCP(TxBuffer,len+strlen(pTxHead)+2))
	{
		printf("Error\r\n");
		free(pTxHead);
		free(TxBuffer);
		return false;
	}
	else
	{
		printf(".");
	}
	Sleep(300);
	free(pTxHead);
	free(TxBuffer);
	return true;
	
}
void write_Data(unsigned int startaddress, char *Pcmd, unsigned Len)
{
	int SendLen;
	
	while(Len>0)
	{
		if(Len>=MAX_DATA_BUFFER_LENTH)
		{
			SendLen=MAX_DATA_BUFFER_LENTH;
		}
		else
		{
			SendLen=Len;
		}
		flash_write_data(startaddress, Pcmd, SendLen);
		Pcmd+=SendLen;
		startaddress+=SendLen;
		Len-=SendLen;


	}
}
void write_baseinfo()
{

	unsigned int buf[4] = {CMDSET_LENGTH,CMDSET_START_ADDR, WAVEFORM_LENGTH, FIRMWARE_OFFSET };
	write_Data(FIRMWARE_OFFSET,(char *) buf, 16);
	
}
extern unsigned char cmdset[];
void write_cmdset()
{
	write_Data(CMDSET_START_ADDR,(char *) cmdset, CMDSET_LENGTH);
}
extern unsigned short wfm_set[];
void write_wav()
{
	write_Data(WAVFORM_START_ADDR,(char *) wfm_set, WAVEFORM_LENGTH);
}
extern  unsigned char Image_1[];
extern  unsigned char Image_2[];
extern  unsigned char Image_3[];
extern  unsigned char Image_4[];
void WriteImage()
{
	
	//----------1-----------
#if 0
	for(int i =0;i<IMAGE_SPACE_IN_FLASH;i+=4096)
	{
		flash_Erase_Sector(IMAGE_ADDR_1+i );
	}
	write_Data(IMAGE_ADDR_1,(char *) Image_1, IMAGE_LENGTH);
#endif

	//----------2-----------
#if 0
	for(int i =0;i<IMAGE_SPACE_IN_FLASH;i+=4096)
	{
		flash_Erase_Sector(IMAGE_ADDR_2+i );
	}
	write_Data(IMAGE_ADDR_2,(char *) Image_2, IMAGE_LENGTH);
#endif
	//----------3-----------
#if 0
	for(int i =0;i<IMAGE_SPACE_IN_FLASH;i+=4096)
	{
		flash_Erase_Sector(IMAGE_ADDR_3+i );
	}
	write_Data(IMAGE_ADDR_3,(char *) Image_3, IMAGE_LENGTH);
#endif

	//----------4-----------
#if 0
	for(int i =0;i<IMAGE_SPACE_IN_FLASH/2;i+=4096)
	{
		flash_Erase_Sector(IMAGE_ADDR_4+i );
	}
	write_Data(IMAGE_ADDR_4,(char *) Image_4, IMAGE_SPACE_IN_FLASH/2);

	for(int i =IMAGE_SPACE_IN_FLASH/2;i<IMAGE_SPACE_IN_FLASH;i+=4096)
	{
		flash_Erase_Sector(IMAGE_ADDR_4+i );
	}
	write_Data(IMAGE_ADDR_4+IMAGE_SPACE_IN_FLASH/2,(char *) Image_4+IMAGE_SPACE_IN_FLASH/2, IMAGE_LENGTH-(IMAGE_SPACE_IN_FLASH/2));

#endif

#if 1
	for(int i =0;i<IMAGE_SPACE_IN_FLASH;i+=4096)
	{
		flash_Erase_Sector(IMAGE_ADDR_4+i );
	}
	write_Data(IMAGE_ADDR_4,(char *) Image_4, IMAGE_LENGTH);
#endif

	//----------5-----------
#if 0
		for(int i =0;i<IMAGE_SPACE_IN_FLASH;i+=4096)
		{
			flash_Erase_Sector(IMAGE_ADDR_5+i );
		}
		write_Data(IMAGE_ADDR_5,(char *) Image_2, IMAGE_LENGTH);
#endif
	

}

bool TcpIpConnect()
{
	DISP_FUN_NAME(__FUNCDNAME__);
	HRESULT hr = E_FAIL;
	HRESULT hrCoinit = E_FAIL;

	WSADATA wsaData = {0};

	sockaddr_in clientService;

	INT bytesSent = SOCKET_ERROR;
	INT bytesRecv = SOCKET_ERROR;
	INT iRet = 0;

	bool fSockInitialized = false;
	// Initialize Winsock.
	iRet = WSAStartup( MAKEWORD(2,2), &wsaData);
	if(iRet != NO_ERROR )
	{
		hr = E_FAIL;
		goto Exit;
	}

	fSockInitialized = TRUE;

	// Create socket
	m_Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_Socket == INVALID_SOCKET )
	{
		iRet = WSAGetLastError();
		hr = HRESULT_FROM_WIN32(iRet);
		goto Exit;
	}
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(g_cServerIP);
	clientService.sin_port = htons( g_ucServerPort );

	// Connect to the Server
	iRet = connect(m_Socket, (SOCKADDR*) &clientService, sizeof(clientService));
	if(SOCKET_ERROR == iRet)
	{
		iRet = WSAGetLastError();
		hr = HRESULT_FROM_WIN32(iRet);
		printf(" connect error\r\n");

	}
	printf(" connect Ok\r\n");
	DISP_EXIT_FUN_NAME(__FUNCDNAME__);
	return true;
Exit:
	return false;
}
bool SendDataOverTCP(char * buffer,int len)
{
	
	INT bytesSent = SOCKET_ERROR;
	HRESULT hr = E_FAIL;
	INT iRet = 0;
	char Buffer[4096];

	do
	{	
		bytesSent = send(m_Socket, buffer,len, 0);
		iRet = WSAGetLastError();
		hr = HRESULT_FROM_WIN32(iRet);
		//return false;
		if(SOCKET_ERROR == bytesSent)
		{
			while(!TcpIpConnect())
			{
				Sleep(1000);
				printf("C\r\n");
			}
			
		}
		
			WaitForSingleObject(g_socket_evelt,5000);
		


	}while(SOCKET_ERROR == bytesSent);

	
	return true;
}

bool SendFrameBuffer()
{
	DISP_FUN_NAME(__FUNCDNAME__);
	static int iFrameCnt=0;
	INT bytesSent = SOCKET_ERROR;
	HRESULT hr = E_FAIL;
	INT iRet = 0;
	char Buffer[1024];
	for(int i=0;i<sizeof(Buffer);i++)
	{
		Buffer[i]='a'+i%26;

	}
	int Sendlen=10;
	//DWORD dwStartTickCount;


	for(int i=0;i<=sizeof(Buffer)-bytesSent;)
	{
		if(NULL==m_Socket)
		{
			return false;
		}

		bytesSent = send(m_Socket, (char *)Buffer+i*Sendlen,Sendlen, 0);
		i+=Sendlen;
	}
	
	

	//dwStartTickCount=GetTickCount();
	//bytesSent = send(m_Socket, (char *)g_pImageDataBuffer,iBufferLen, 0);
	//dwStartTickCount=GetTickCount()-dwStartTickCount;
	if(SOCKET_ERROR == bytesSent)
	{
		iRet = WSAGetLastError();
		hr = HRESULT_FROM_WIN32(iRet);
		return false;

	}
	else
	{
		iFrameCnt++;

	}
	return true;
}
void Disp_baseinfo()
{
	unsigned int startaddress = FIRMWARE_OFFSET;
	unsigned int buf[4] = {CMDSET_LENGTH,CMDSET_START_ADDR, WAVEFORM_LENGTH, FIRMWARE_OFFSET };
	unsigned char *Pcmd = (unsigned char *)buf;
	unsigned char temp;
	unsigned int len=sizeof(buf);
	printf("\r\n-----------------BASIC INFO[%d]-Star--------------------\r\n",FIRMWARE_OFFSET);
	for(int i=0; i < 16; i++)
	{
		printf("%x ",Pcmd[i]);
	}
	printf("\r\n-----------------BASIC INFO [%d]-End--------------------\r\n",FIRMWARE_OFFSET);
}
void Disp_cmdset()
{
	unsigned long int startaddress = CMDSET_START_ADDR;
	unsigned char *Pcmd = cmdset;
	unsigned char temp;

	printf("\r\n-----------------CMDSET[%d]-Star--------------------\r\n",CMDSET_START_ADDR);
	for(int i=0; i < CMDSET_LENGTH; i++)
	{
		if(i%16==0)
			printf("\r\n [%08d]  ",CMDSET_START_ADDR+i);
		printf("%02x ",Pcmd[i]);
	}
	printf("\r\n-----------------CMDSET[%d]-End--------------------\r\n",CMDSET_START_ADDR);
}
void Disp_wav()
{

	printf("\r\n-----------------WAVFORM[%d]-Star--------------------\r\n",WAVFORM_START_ADDR);
	unsigned long int startaddress = WAVFORM_START_ADDR;
	unsigned char *Pcmd = (unsigned char *)wfm_set;
	for(int i=0; i < WAVEFORM_LENGTH; i++)
	{
		if(i%16==0)
			printf("\r\n [%08d]  ",WAVFORM_START_ADDR+i);
		printf("%02x ",Pcmd[i]);
	}
	printf("\r\n-----------------WAVFORM[%d]-End--------------------\r\n",WAVFORM_START_ADDR);
}
void Disp_Image1()
{;
	printf("\r\n-----------------IMAGE1[%d]-Star--------------------\r\n",IMAGE_ADDR_1);
	for(int i=0; i < 1026; i++)
	{
		if(i%16==0)
			printf("\r\n [%08d]  ",IMAGE_ADDR_1+i);
		printf("%02x ",Image_1[i]);
	}
	printf("\r\n-----------------WAVFORM[%d]-End--------------------\r\n",IMAGE_ADDR_1);
}
void Disp_Image2()
{;
printf("\r\n-----------------IMAGE1[%d]-Star--------------------\r\n",IMAGE_ADDR_2);
for(int i=0; i < 1026; i++)
{
	if(i%16==0)
		printf("\r\n [%08d]  ",IMAGE_ADDR_2+i);
	printf("%02x ",Image_2[i]);
}
printf("\r\n-----------------WAVFORM[%d]-End--------------------\r\n",IMAGE_ADDR_2);
}
int _tmain(int argc, _TCHAR* argv[])
{
	char TestData[16];
	for(int i=0;i<16;i++)
	{
		TestData[i]=0x41+i;
	}
	
#if 1
	HANDLE  g_socket_evelt=NULL; 
	g_socket_evelt = CreateEvent(NULL, FALSE, FALSE, NULL);
	ResetEvent(g_socket_evelt);
	/*while(!TcpIpConnect())
		Sleep(500);*/
	TcpIpConnect();
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TcpIPClientRevThread, 0, 0, NULL);
	/*flash_Erase_Chip();
	write_baseinfo();
	write_cmdset();
	write_wav();*/
	WriteImage();
	Eink_Switch_Image(4);
	if(m_Socket)
	closesocket(m_Socket);
#else
	/*Disp_baseinfo();
	Disp_cmdset();
	Disp_wav();*/
	//Disp_Image1();
	Disp_Image2();
#endif
	//SendFrameBuffer();

	

	return 0;
}

DWORD TcpIPClientRevThread()
{
	HRESULT hr = E_FAIL;

	INT bytesRecv = SOCKET_ERROR;
	INT iRet = 0;
	char *PRecvbuf=(char *)malloc(4096);

	while(1)
	{
		int iRecvLen=4096;
		int iBytesRecved=0;
		iBytesRecved = recv(m_Socket, PRecvbuf, iRecvLen, 0);
		if(iBytesRecved==0)
		{
			printf(" Recieved byte 0,Conect lost!!\r\n");
			closesocket(m_Socket);
			m_Socket = INVALID_SOCKET;
			TcpIpConnect();
			Sleep(500);
		}
		else if(iBytesRecved==-1)
		{
			printf(" Recieved byte-1 Conect lost!!\r\n");
			closesocket(m_Socket);
			m_Socket = INVALID_SOCKET;
			TcpIpConnect();
			Sleep(500);
		}
		else if(iBytesRecved>0)
		{

			printf("R");
			SetEvent(g_socket_evelt);

		}

		if(SOCKET_ERROR == iBytesRecved)
		{
			iRet = WSAGetLastError();
			if(WSAENOTCONN ==iRet)
			{
				TcpIpConnect();
				Sleep(500);
			}

			hr = HRESULT_FROM_WIN32(iRet);

		}
	}
	return 0;
}

