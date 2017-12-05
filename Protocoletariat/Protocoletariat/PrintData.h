#pragma once

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include "atlstr.h"
#include <vector>
#include <queue>

namespace protocoletariat
{
	static class PrintData
	{
	public:
		bool printActive;
		unsigned int mCurrentRow;

		//std::queue<char*> printQueue; Moved to main

		DWORD WINAPI  PrintReceivedData(const std::queue<char*> printQueue);
		void DrawCharsByRow(const TCHAR* chars, unsigned int row);
		void UpdateLog(const TCHAR* chars, unsigned int row);

	private:
	};
}