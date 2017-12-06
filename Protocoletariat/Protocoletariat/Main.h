#pragma once

#include <windows.h>
#include <stdio.h>
#include <strsafe.h>
#include <queue>

namespace protocoletariat
{
	// Global variables - for creating WinMain
	const TCHAR tchrProgramName[] = TEXT("PTTP Protocol");
	const LPTSTR lpszDefaultCommPort = TEXT("COM1"); // default port

	HWND		hwnd;
	LPTSTR		lpszCommPort;
	COMMCONFIG	ccfg;
	OVERLAPPED olRead, olWrite;
	HANDLE hThrd, hComm, hEvent;
	DWORD dwThreadID, dwThreadExit;

	// bCommOn to be used as global flag
	bool		bCommOn;

	int indexReadChar = 0;
	int X = 0, Y = 0; // current str coordinates

	// Global variables - for Protocol Comm
	std::queue<char*>* uploadQ;
	std::queue<char*>* downloadQ;
	std::queue<char*>* dataToPrintQ;
	LogFile* logfile;
	bool dlReady; // flag for download ready
	bool RVIflag; // flag becomes true when RVI event detected
	bool globalRVI;

	// Thread handles
	HANDLE uploadThrd, downloadThrd, printThrd, protocolThrd;
	DWORD  uploadThrdID, downloadThrdID, printThrdID, protocolThrdID;
	// Parameter to pass to ThreadProc
	struct paramFileUploader* fileUploadParam;
	struct paramFileDownloader* fileDownloadParam;
	//OVERLAPPED olRead;
	DWORD readThreadExit;
	struct paramProtocolEngine* protocolParam;
	//OVERLAPPED olWrite;
	DWORD writeThreadExit;
	struct paramPrintData* printDataParam;


	// Functions
	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	bool InitializeWindows(HINSTANCE hInst, int nCmdShow);
	bool InitializeCommHandle(LPTSTR CommPort);
	bool SwitchCommPort(int commPort);
	bool ConfigureCommSettings(HWND hwnd);
	boolean StartCommunication(HWND hwnd);
	DWORD WINAPI DoRead(LPVOID lpvThreadParm);

	void ClearQueue(std::queue<char*>* q);
	void CleanUp();
	void StartEngine();
}
