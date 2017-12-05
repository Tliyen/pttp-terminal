#pragma once

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include "atlstr.h"
#include <vector>
#include <queue>
#include "Main.h"


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
		static DWORD WINAPI PrintReceivedData(paramPrintData* param);
		static void DrawCharsByRow(const TCHAR* chars, unsigned int row);
		static void PrintData::UpdateLog(const TCHAR* chars, unsigned int row);

	private:
		static unsigned int mCurrentRow;
		static HWND* hwnd;
		
	};
}