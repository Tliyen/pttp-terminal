#pragma once

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include "atlstr.h"
#include <vector>
#include <queue>

<<<<<<< HEAD
bool printActive;
std::queue<char*> printQueue;
=======
namespace protocoletariat
{
	static class PrintData
	{
	public:
		bool printActive;
		unsigned int mCurrentRow;

		std::queue<char*> printQueue;

		void PrintReceivedData();
		void DrawCharsByRow(const TCHAR* chars, unsigned int row);
		void UpdateLog(const TCHAR* chars, unsigned int row);

	private:
	};

>>>>>>> c455689ac91d98120a6ecb8d433868d79e6be1cf

}