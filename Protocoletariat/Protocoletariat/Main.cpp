/*----------------------------------------------------------------------
-- SOURCE FILE:	Main.cpp		- A wireless protocol communication
--								  terminal program that will send and
--								  read data from a text file in half-
--								  duplex communication.
--
-- PROGRAM:		Protocoletariat
--
-- FUNCTIONS:
--				int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
--								   LPSTR lspszCmdParam, int nCmdShow)
--				LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM)
--				boolean InitializeCommHandle(LPTSTR CommPort)
--				boolean SwitchCommPort(int commPort)
--				boolean ConfigureCommSettings(HWND hwnd)
--				void ClearQueue(std::queue<char*>* q)
--				void StartEngine(void)
--				void CleanUp(void)
--				void TerminateProgram(void)
--
--
--
-- DATE:		November 29, 2017
--
-- DESIGNER:	Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
--
-- PROGRAMMER:	Luke Lee
--
-- NOTES:
-- This is the entry point for the program. This method is responsible
-- for creating the GUI to make the application easy for users. It is
-- also responsible for initializing the connection to the serial port.
-- If the connection is successful then the program will move to the
-- next stage.
--
-- The program reads user keystroke as well as serial port input, sends
-- text character in a file uploaded by the user, and displays incoming
-- serial port input on screen.
--
-- Before starting data exchange, user can configure serial port or
-- communication settings such as bit per second, data bits, parity,
-- stop bit, and flow control.
--
-- To start data exchange, user can select Upload menu, and upload a
-- file, send a frame of characters at a time to serial port. At the
-- same time, the program reads incoming serial port input and
-- displays it on the screen.
----------------------------------------------------------------------*/
#pragma warning (disable: 4096)

#define STRICT

#include "Menu.h"
#include "Global.h"
#include "Main.h"
#include "FileUploader.h"
#include "FileDownloader.h"
#include "PrintData.h"
#include "ProtocolEngine.h"

using namespace protocoletariat;

/*----------------------------------------------------------------------
-- FUNCTION:	WinMain
--
-- DATE:		November 29, 2017
--
-- DESIGNER:	Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
--
-- PROGRAMMER:	Luke Lee
--
-- INTERFACE:	int WINAPI WinMain(HINSTANCE hInst
--								 , HINSTANCE hprevInstance,
--								 , LPSTR lspszCmdParam, int nCmdShow)
--
-- ARGUMENT: hInst				- A handle to the current instance of the
--								  application.
--			 hprevInstance		- A handle to the previous instance of
--								  the application.
--			 lspszCmdParam		- The command parameter for the
--								  application.
--			 nCmdShow			- A value to determine how the Window is
--								  is to be shown.
--
-- RETURNS:	 int				- 0 if this function terminates before
--								  entering the Message loop. If this
--								  function terminates by receiving a
--								  WM_QUIT Message, the exit value in that
--								  Message's wParam parameter.
--
-- NOTES:
-- Main function of this program. Its main role is creating Window and
-- initialize communciation Handler, as well as reading Windows Events
-- including user keystroke Events. Messages generated by Events captured
-- by this function are sent to WndProc
----------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
{
	MSG Msg;

	if (!InitializeWindows(hInst, nCmdShow))
	{
		return 0;
	}

	lpszCommPort = lpszDefaultCommPort;

	// initialize comm handle; if no serial port found, terminates the program
	if (!InitializeCommHandle(lpszCommPort))
	{
		return 0;
	}

	// successfully connected to serial port
	logfile = new LogFile();
	fileUploadParam = new paramFileUploader();
	fileDownloadParam = new paramFileDownloader();
	printDataParam = new paramPrintData();
	protocolParam = new paramProtocolEngine();

	uploadQ = new std::queue<char*>();
	downloadQ = new std::queue<char*>();
	dataToPrintQ = new std::queue<char*>();
	dlReady = false;
	protocolActive = true;
	RVIflag = false;
	globalRVI = false;

	StartEngine();

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}

/*----------------------------------------------------------------------
-- FUNCTION:	InitializeWindows
--
-- DATE:		November 29, 2017
--
-- DESIGNER:	Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
--
-- PROGRAMMER:	Luke Lee
--
-- INTERFACE:	bool InitializeWindows(HINSTANCE hInst, int nCmdShow)
--
-- ARGUMENT:	hInst			- A handle to the current instance of the
--								  application.
--				nCmdShow		- A value to determine how the Window is
--								  is to be shown.
--
-- RETURNS:		bool			- true if the main Windows is created
--								  successfully; false otherwise.
--
-- NOTES:
-- This function initializes required parameters for the wireless
-- terminal Windows and opens the windows. It also assigns a process to
-- the program which will listen to user's keystroke or menu item clicks
-- input and response accordingly.
----------------------------------------------------------------------*/
bool protocoletariat::InitializeWindows(HINSTANCE hInst, int nCmdShow)
{
	// application Window values
	const int intWindowW = 500; // Window width
	const int intWindowH = 650; // Window height

	WNDCLASSEX Wcl;

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW); // cursor style

	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	Wcl.lpszClassName = tchrProgramName;

	Wcl.lpszMenuName = TEXT("MYMENU"); // The menu Class
	Wcl.cbClsExtra = 0; // no extra memory needed
	Wcl.cbWndExtra = 0;

	if (!RegisterClassEx(&Wcl))
	{
		MessageBox(NULL, "Wireless Terminal couldn't start!", NULL, MB_OK | MB_ICONSTOP);
		return false;
	}

	hwnd = CreateWindow(tchrProgramName, tchrProgramName
		, WS_OVERLAPPEDWINDOW, 10, 10, intWindowW, intWindowH, NULL
		, NULL, hInst, NULL);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	return true;
}

/*----------------------------------------------------------------------
-- FUNCTION:	WndProc
--
-- DATE:		November 29, 2017
--
-- DESIGNER:	Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
--
-- PROGRAMMER:	Luke Lee
--
-- INTERFACE:	LRESULT CALLBACK WndProc(HWND hwnd, UINT Message,
--										 WPARAM wParam, LPARAM lParam)
--
-- ARGUMENT:	hwnd			- A handle to the Window.
--				Message			- A Message to process.
--				wParam			- Additional Message information
--				lParam			- Additional Message information
--
-- RETURNS:		LRESULT			- 0 if defined behavior is successful.
--								  If the behavior for the received Message
--								  is not defined, the return value of
--								  DefWindowProc.
--
-- NOTES:
-- This function receives Messages from WinMain and determines behavior
-- in switch statements. Selecting on menu items defined in Menu.h and Menu.rc
-- is detected in this function, and it behaves accordingly.
----------------------------------------------------------------------*/
LRESULT CALLBACK protocoletariat::WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	char bufferWrite[2] = "";
	HDC hdc;
	PAINTSTRUCT paintstruct;
	TEXTMETRIC tm;
	HINSTANCE manInst;

	switch (Message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam)) // menu
		{
		case IDM_UPLOAD: // Start upload file thread

			OPENFILENAME ofn; // common dialog box structure
			char szFile[300]; // buffer for file name

			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwnd;
			ofn.lpstrFile = szFile;

			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			// Display the Open dialog box. 
			if (GetOpenFileName(&ofn) == TRUE)
			{
				fileUploadParam->filePath = ofn.lpstrFile;
				fileUploadParam->uploadQueue = uploadQ;
				uploadThrd = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FileUploader::LoadTextFile, fileUploadParam, 0, &uploadThrdID);
			}
			else
			{
				MessageBox(hwnd
					, TEXT("Unable to open specified file")
					, TEXT("Access Denied"), MB_ICONHAND | MB_OK);
			}
			break;

		case IDM_CONFIG: // Settings > Configure
			if (IDOK == MessageBox(hwnd
				, TEXT("Changing configuration while transmission will reset the connection. Continue?")
				, TEXT("Warning"), MB_ICONWARNING | MB_OKCANCEL))
			{
				CleanUp();
				ConfigureCommSettings(hwnd);
			}
			break;

		case IDM_COM1:
			if (IDOK == MessageBox(hwnd
				, TEXT("Switching COM port during transmission will reset the connection. Continue?")
				, TEXT("Warning"), MB_ICONWARNING | MB_OKCANCEL))
			{
				CleanUp();
				SwitchCommPort(1);
			}
			break;

		case IDM_COM2:
			if (IDOK == MessageBox(hwnd
				, TEXT("Switching COM port during transmission will reset the connection. Continue?")
				, TEXT("Warning"), MB_ICONWARNING | MB_OKCANCEL))
			{
				CleanUp();
				SwitchCommPort(2);
			}
			break;

		case IDM_COM3:
			if (IDOK == MessageBox(hwnd
				, TEXT("Switching COM port during transmission will reset the connection. Continue?")
				, TEXT("Warning"), MB_ICONWARNING | MB_OKCANCEL))
			{
				CleanUp();
				SwitchCommPort(3);
			}
			break;

		case IDM_COM4:
			if (IDOK == MessageBox(hwnd
				, TEXT("Switching COM port during transmission will reset the connection. Continue?")
				, TEXT("Warning"), MB_ICONWARNING | MB_OKCANCEL))
			{
				CleanUp();
				SwitchCommPort(4);
			}
			break;

		case IDM_ABOUT: // Open About dialog
			MessageBox(hwnd, "PTTP Wireless Protocol Terminal v.1\nCreated by M.Ariss, J.Lee, L.Lee, L.Tong",
				"About", MB_ICONINFORMATION | MB_OK);
			break;

		case IDM_HELP: // Open user manual
			manInst = ShellExecute(NULL, "open", "UserManual.pdf", NULL,
				NULL, SW_SHOW);
			if ((int)manInst == ERROR_FILE_NOT_FOUND)
			{
				MessageBox(NULL,
					TEXT("UserManual.pdf not found."), TEXT("Error"),
					MB_ICONHAND | MB_OK);
			}
			break;

		case IDM_EXIT: // Exit program
			if (IDOK == MessageBox(hwnd, "OK to close window?", "Exit", MB_ICONQUESTION | MB_OKCANCEL))
			{
				CleanUp();
				Sleep(500);
				TerminateProgram();
			}
			break;
		}
		break;

	case WM_CHAR: // process keystroke
		if (wParam == VK_ESCAPE)
		{
			if (IDOK == MessageBox(hwnd, "OK to close window?", "Exit", MB_ICONQUESTION | MB_OKCANCEL))
			{
				CleanUp();
				Sleep(500);
				TerminateProgram();
			}
			break;
		}
		else if (wParam == 0x72)
		{
			MessageBox(hwnd, "RVI key is detected", "RVI Event", MB_OK);
			ProtocolEngine::TransmitFrame(true, RVI_KEY);
			break;
		}
		break;

	case WM_PAINT: // process a repaint message
		hdc = BeginPaint(hwnd, &paintstruct); // Acquire DC
		GetTextMetrics(hdc, &tm); // get text metrics
		ReleaseDC(hwnd, hdc); // release device context
		EndPaint(hwnd, &paintstruct); // Release DC
		break;

	case WM_CLOSE: // terminate program
		if (IDOK == MessageBox(hwnd, "OK to close window?", "Exit", MB_ICONQUESTION | MB_OKCANCEL))
		{
			CleanUp();
			Sleep(500);
			TerminateProgram();
		}
		break;

	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}

	return 0;
}

/*----------------------------------------------------------------------
-- FUNCTION:	InitializeCommHandle
--
-- DATE:		October 4, 2017
--
-- DESIGNER:	Jeremy Lee
--
-- PROGRAMMER:	Jeremy Lee
--
-- INTERFACE:	bool InitializeCommHandle(LPTSTR CommPort)
--
-- ARGUMENT:	CommPort		- a pointer to the string representing name
--								  of the COM port.
--
-- RETURNS:		bool			- true if a COM connection is established
--								  successfully.
--
-- NOTES:
-- This function creates communication Handle and applies the
-- configuration settings to it. If a communication connection is
-- established succefully, it sets the global flag protocolActive to true.
----------------------------------------------------------------------*/
bool protocoletariat::InitializeCommHandle(LPTSTR CommPort)
{
	// create communcation handle
	hComm = CreateFile(lpszCommPort, GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hComm == INVALID_HANDLE_VALUE) // failed to create handle
	{
		MessageBox(NULL, "No Serial COM port found.", NULL, MB_OK | MB_ICONSTOP);
		// TODO: release hComm memory
		return false;
	}

	ccfg.dwSize = sizeof(COMMCONFIG);
	ccfg.wVersion = 0x100;
	GetCommConfig(hComm, &ccfg, &ccfg.dwSize);
	if (!CommConfigDialog(lpszCommPort, hwnd, &ccfg))
	{
		MessageBox(NULL, "Error getting the COM port configuration dialog", TEXT("Error"), MB_OK);
		return false;
	}
	if (!SetCommState(hComm, &ccfg.dcb))
	{
		MessageBox(NULL, "Error setting the COM port configuration", TEXT("Error"), MB_OK);
		return false;
	}

	protocolActive = true;
	return true;
}

/*----------------------------------------------------------------------
-- FUNCTION:	SwitchCommPort
--
-- DATE:		October 4, 2017
--
-- DESIGNER:	Jeremy Lee
--
-- PROGRAMMER:	Jeremy Lee
--
-- INTERFACE:	bool SwitchCommPort(int commPort)
--
-- ARGUMENT:	commPort		- an int representation of the port number
--
-- RETURNS:		bool			- true if switching a COM port is successful;
--								  false otherwise.
--
-- NOTES:
-- This function is called by user Menu click in WndProc, and sets the
-- target COM port based on the user selection. There are currently 4
-- COM ports available.
----------------------------------------------------------------------*/
bool protocoletariat::SwitchCommPort(int commPort)
{
	switch (commPort)
	{
	case 1:
		lpszCommPort = TEXT("COM1");
		break;
	case 2:
		lpszCommPort = TEXT("COM2");
		break;
	case 3:
		lpszCommPort = TEXT("COM3");
		break;
	case 4:
		lpszCommPort = TEXT("COM4");
		break;
	default:
		return false;
	}

	PurgeComm(hComm, PURGE_RXCLEAR); // clean out pending bytes
	PurgeComm(hComm, PURGE_TXCLEAR); // clean out pending bytes
	CloseHandle(hComm);
	if (!InitializeCommHandle(lpszCommPort))
		return false;

	return true;
}

/*----------------------------------------------------------------------
-- FUNCTION:	ConfigureCommSettings
--
-- DATE:		October 4, 2017
--
-- DESIGNER:	Jeremy Lee
--
-- PROGRAMMER:	Jeremy Lee
--
-- INTERFACE:	bool ConfigureCommSettings(HWND hwnd)
--
-- ARGUMENT:	hwnd		- A handle to the Window.
--
-- RETURNS:		bool		- true if comm setting is configured successfully;
--							  false otherwise.
--
-- NOTES:
-- This function is called by user Menu click that is processed in
-- WndProc. Once called, it displays a separate Window containing
-- communication settings. On that Window, user can configure values for
-- communication properties, and apply it for the next connection. If
-- a connection is established successfully, the global flag protocolActive
-- is set to true.
----------------------------------------------------------------------*/
bool protocoletariat::ConfigureCommSettings(HWND hwnd)
{
	ccfg.dwSize = sizeof(COMMCONFIG);
	ccfg.wVersion = 0x100;
	GetCommConfig(hComm, &ccfg, &ccfg.dwSize);
	if (!CommConfigDialog(lpszCommPort, hwnd, &ccfg))
	{
		MessageBox(NULL, "Error getting the COM port configuration dialog", TEXT("Error"), MB_OK);
		return false;
	}
	if (!SetCommState(hComm, &ccfg.dcb))
	{
		MessageBox(NULL, "Error setting the COM port configuration", TEXT("Error"), MB_OK);
		return false;
	}

	protocolActive = true;
	return true;
}

/*----------------------------------------------------------------------
-- FUNCTION:	StartEngine
--
-- DATE:		December 4, 2017
--
-- DESIGNER:	Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
--
-- PROGRAMMER:	Luke Lee
--
-- INTERFACE:	void StartEngine(void)
--
-- ARGUMENT:	void
--
-- RETURNS:		void
--
-- NOTES:
-- This function is responsible for initializing and starting the download
-- thread, print data thread, and the main protocol engine thread. It
-- initializes the required parameters in custom structs and passes them
-- to the thread process in each of their respective classes.
----------------------------------------------------------------------*/
void protocoletariat::StartEngine()
{
	// initialize download (read) thread
	fileDownloadParam->downloadQueue = downloadQ;
	fileDownloadParam->olRead = &olRead;
	fileDownloadParam->dwThreadExit = &readThreadExit;
	fileDownloadParam->handle = &hComm;
	fileDownloadParam->dlReady = &dlReady;
	fileDownloadParam->RVIflag = &RVIflag;
	fileDownloadParam->hEvent = &hEvent;
	fileDownloadParam->logfile = logfile;
	fileDownloadParam->printQueue = dataToPrintQ;

	PurgeComm(hComm, PURGE_RXCLEAR); // clean out pending bytes
	PurgeComm(hComm, PURGE_TXCLEAR); // clean out pending bytes

	COMMTIMEOUTS cto; // timeout

	// set timeouts
	cto.ReadIntervalTimeout = 1;
	cto.ReadTotalTimeoutMultiplier = 1;
	cto.ReadTotalTimeoutConstant = 1;
	cto.WriteTotalTimeoutMultiplier = 1;
	cto.WriteTotalTimeoutConstant = 1;

	if (!SetCommTimeouts(hComm, &cto))
	{
		MessageBox(NULL, "Error setting port time-outs", TEXT("Error"), MB_OK);
	}
	if (!EscapeCommFunction(hComm, CLRDTR))
	{
		MessageBox(NULL, "Error clearing DTR", TEXT("Error"), MB_OK);
	}
	Sleep(200);
	if (!EscapeCommFunction(hComm, SETDTR))
	{
		MessageBox(NULL, "Error setting DTR", TEXT("Error"), MB_OK);
	}
	downloadThrd = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FileDownloader::ReadSerialPort, fileDownloadParam, 0, &downloadThrdID);

	// initialize print data thread
	printDataParam->printQueue = dataToPrintQ;
	printDataParam->hwnd = &hwnd;
	printDataParam->hComm = &hComm;
	printDataParam->X = &X;
	printDataParam->Y = &Y;
	printDataParam->logfile = logfile;
	printThrd = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PrintData::PrintReceivedData, printDataParam, 0, &printThrdID);

	// initialize main protocol engine thread
	olWrite = { 0 };
	protocolParam->uploadQueue = uploadQ;
	protocolParam->downloadQueue = downloadQ;
	protocolParam->printQueue = dataToPrintQ;
	protocolParam->olWrite = olWrite;
	protocolParam->dwThreadExit = writeThreadExit;
	protocolParam->hComm = &hComm;
	protocolParam->logfile = logfile;
	protocolParam->dlReady = &dlReady;
	protocolParam->RVIflag = &RVIflag;
	protocolThrd = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ProtocolEngine::ProtocolThread, protocolParam, 0, &protocolThrdID);
}

/*----------------------------------------------------------------------
-- FUNCTION:	ClearQueue
--
-- DATE:		December 4, 2017
--
-- DESIGNER:	Luke Lee
--
-- PROGRAMMER:	Luke Lee
--
-- INTERFACE:	void ClearQueue(std::queue<char*> &q)
--
-- ARGUMENT:	q				- a reference to a queue
--
-- RETURNS:		void
--
-- NOTES:
-- This function is responsible for clearing all the items in a queue
-- given as a parameter in the function.
----------------------------------------------------------------------*/
void protocoletariat::ClearQueue(std::queue<char*>* q)
{
	while (!q->empty())
	{
		q->pop();
	}
}

/*----------------------------------------------------------------------
-- FUNCTION:	CleanUp
--
-- DATE:		December 4, 2017
--
-- DESIGNER:	Luke Lee
--
-- PROGRAMMER:	Luke Lee
--
-- INTERFACE:	void CleanUp(void)
--
-- ARGUMENT:	void
--
-- RETURNS:		void
--
-- NOTES:
-- This function is responsible for setting the global connection flag
-- to false, and clearing all the queues (upload, download, print-data
-- queues).
----------------------------------------------------------------------*/
void protocoletariat::CleanUp()
{
	protocolActive = false;
	Sleep(500);
	ClearQueue(uploadQ);
	ClearQueue(downloadQ);
	ClearQueue(dataToPrintQ);
}

/*----------------------------------------------------------------------
-- FUNCTION:	CleanUp
--
-- DATE:		December 5, 2017
--
-- DESIGNER:	Luke Lee
--
-- PROGRAMMER:	Luke Lee
--
-- INTERFACE:	void TerminateProgram(void)
--
-- ARGUMENT:	void
--
-- RETURNS:		void
--
-- NOTES:
-- This function is responsible for closing all the handles and threads,
-- (serial port handle, upload, download, print-data, and main protocol
-- threads) and deleting all the allocated memory structure (logfile,
-- upload file parameter, download file parameter, print-data parameter).
-- After all the cleanups are finished, the program is terminated.
----------------------------------------------------------------------*/
void protocoletariat::TerminateProgram()
{
	PurgeComm(hComm, PURGE_RXCLEAR); // clean out pending bytes
	PurgeComm(hComm, PURGE_TXCLEAR); // clean out pending bytes
	CloseHandle(hComm);

	CloseHandle(uploadThrd);
	CloseHandle(downloadThrd);
	CloseHandle(printThrd);
	CloseHandle(protocolThrd);

	delete logfile;
	delete fileUploadParam;
	delete fileDownloadParam;
	delete printDataParam;
	delete protocolParam;
	delete uploadQ;
	delete downloadQ;
	delete dataToPrintQ;

	PostQuitMessage(0);
}
