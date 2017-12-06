#pragma once

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include "atlstr.h"
#include <vector>
#include <queue>
#include "Global.h"
#include <sstream>

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
		static void PrintData::PrintLog(HWND* hwnd, const TCHAR* chars, unsigned int row);
		static void PrintData::PrintChar(HWND* hwnd, char* letter, unsigned int row, int* X, int*Y);
	};
}