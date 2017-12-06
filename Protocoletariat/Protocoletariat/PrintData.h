#pragma once

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include "atlstr.h"
#include <vector>
#include <queue>
#include "global.h"
#include <sstream>
//#include "Main.h"

namespace protocoletariat
{
	struct paramPrintData
	{
		std::queue<char*>* printQueue;
		HWND* hwnd;
		HANDLE* hComm;
		int* X;
		int* Y;
		LogFile* logfile;
	};

	static class PrintData
	{
	public:
		static DWORD WINAPI PrintData::PrintReceivedData(paramPrintData* param);
		static void PrintData::PrintPayload(HWND* hwnd, const TCHAR* chars, unsigned int row, int* X, int*Y);
		static void PrintData::PrintLog(HWND* hwnd, const TCHAR* chars, unsigned int row, int* X, int*Y);

		//static bool printDataActive;

	private:
		//static unsigned int mCurrentRow;
		//static HWND* hwnd;
		
	};
}