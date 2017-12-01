#pragma once

#include <windows.h>
#include <stdio.h>
#include <strsafe.h>
#include <queue>

namespace protocoletariat
{
	struct LogFile {
		int sent_packet;
		int lost_packet;
		int received_packet;
		int received_corrupted_packet;

		LogFile()
			: sent_packet(0)
			, lost_packet(0)
			, received_packet(0)
			, received_corrupted_packet(0)
		{
		}
	};

	// Global variables - for creating WinMain
	const TCHAR tchrProgramName[] = TEXT("PTTP Protocol");
	const LPTSTR lpszDefaultCommPort = TEXT("COM1"); // default port

	HWND		hwnd;
	LPTSTR		lpszCommPort;
	HANDLE		hComm;
	COMMCONFIG	ccfg;
	boolean		bCommOn, bReading;

	int indexReadChar = 0;
	int X = 0, Y = 0; // current str coordinates

					  // Thread handles
	HANDLE		uploadThrd, downloadThrd, printThrd, mainThrd;
	DWORD		uploadThrdID, downloadThrdID, printThrdID, mainThrdID;

	// Global variables - for Protocol Comm
	std::queue<char*> uploadQ;
	std::queue<char*> downloadQ;
	std::queue<char*> dataToPrintQ;
	LogFile* logfile;

	// Functions
	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	boolean InitializeWindows(HINSTANCE hInst, int nCmdShow);
	boolean InitializeCommHandle(LPTSTR CommPort);
	boolean SwitchCommPort(int commPort);
	boolean ConfigureCommSettings(HWND hwnd);
	//boolean StartCommunication(HWND hwnd);
	//boolean StopCommunication();
	//DWORD WINAPI DoRead(LPVOID lpvThreadParm);
	//void DrawLetter(HWND hwnd, char* letter);
	//void ReportError(LPTSTR lpszFunction);

	void ClearQueue(std::queue<char*> &q);
	void CleanUp();
	void StartEngine();

	// Parameter to pass to ThreadProc
	struct paramFileUploader* fileUploadParam;
}
