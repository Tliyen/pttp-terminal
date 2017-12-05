#pragma once

#include <windows.h>
#include <stdio.h>
#include <strsafe.h>
#include <queue>
#include "global.h"

namespace protocoletariat
{
	// Global variables - for creating WinMain
	const TCHAR tchrProgramName[] = TEXT("PTTP Protocol");
	const LPTSTR lpszDefaultCommPort = TEXT("COM1"); // default port

	HWND		hwnd;
	LPTSTR		lpszCommPort;
	HANDLE		hComm;
	COMMCONFIG	ccfg;

	bool		bCommOn, bReading;

	int indexReadChar = 0;
	int X = 0, Y = 0; // current str coordinates

	// Global variables - for Protocol Comm
	std::queue<char*> uploadQ;
	std::queue<char*> downloadQ;
	std::queue<char*> dataToPrintQ;
	LogFile* logfile;
	bool globalRVI;

	// Thread handles
	HANDLE uploadThrd, downloadThrd, printThrd, protocolThrd;
	DWORD  uploadThrdID, downloadThrdID, printThrdID, protocolThrdID;
	// Parameter to pass to ThreadProc
	struct paramFileUploader* fileUploadParam;
	struct paramFileDownloader* fileDownloadParam;
	OVERLAPPED olRead;
	DWORD readThreadExit;
	struct paramProtocolEngine* protocolParam;
	OVERLAPPED olWrite;
	DWORD writeThreadExit;
	struct paramPrintData* printDataParam;


	// Functions
	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	bool InitializeWindows(HINSTANCE hInst, int nCmdShow);
	bool InitializeCommHandle(LPTSTR CommPort);
	bool SwitchCommPort(int commPort);
	bool ConfigureCommSettings(HWND hwnd);

	void ClearQueue(std::queue<char*> &q);
	void CleanUp();
	void StartEngine();
}
