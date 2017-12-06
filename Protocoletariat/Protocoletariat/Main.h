#pragma once

#include <windows.h>
#include <stdio.h>
#include <strsafe.h>
#include <queue>

namespace protocoletariat
{
	// Main variables - for creating WinMain
	const TCHAR tchrProgramName[] = TEXT("Protocoletariat");
	const LPTSTR lpszDefaultCommPort = TEXT("COM1"); // default port

	HWND hwnd;
	LPTSTR lpszCommPort;
	COMMCONFIG ccfg;
	OVERLAPPED olRead, olWrite;
	HANDLE hComm, hEvent;

	int indexReadChar = 0;
	int X = 0, Y = 0; // current str coordinates
	bool protocolActive;
	bool globalRVI;

	// variables to pass - for Protocol Comm
	std::queue<char*>* uploadQ;
	std::queue<char*>* downloadQ;
	std::queue<char*>* dataToPrintQ;
	LogFile* logfile;
	bool dlReady; // flag for download ready
	bool RVIflag; // flag becomes true when RVI event detected

	// Thread handles
	HANDLE uploadThrd, downloadThrd, printThrd, protocolThrd;
	DWORD  uploadThrdID, downloadThrdID, printThrdID, protocolThrdID;
	// Parameter to pass to ThreadProc
	struct paramFileUploader* fileUploadParam;
	struct paramFileDownloader* fileDownloadParam;
	struct paramProtocolEngine* protocolParam;
	struct paramPrintData* printDataParam;
	DWORD readThreadExit;
	DWORD writeThreadExit;

	// Functions
	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	bool InitializeWindows(HINSTANCE hInst, int nCmdShow);
	bool InitializeCommHandle(LPTSTR CommPort);
	bool SwitchCommPort(int commPort);
	bool ConfigureCommSettings(HWND hwnd);
	void ClearQueue(std::queue<char*>* q);
	void StartEngine();
	void CleanUp();
	void TerminateProgram();
}
