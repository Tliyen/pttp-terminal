/*----------------------------------------------------------------------
-- SOURCE FILE: PrintData.cpp
--
-- PROGRAM: Protocoleriat
--
-- FUNCTIONS:
--		void PrintReceivedData();
--		void DrawCharsByRow();
--		void UpdateLog();
--
-- DATE: December 1, 2017
--
-- DESIGNER: Jeremy Lee & Luke Lee
--
-- PROGRAMMER: Li-Yan Tong and Morgan Ariss
--
-- NOTES:
-- The program is responsible for displaying transfered data and
----------------------------------------------------------------------*/

#include "PrintData.h"

namespace protocoletariat
{
	static HWND* hwnd = nullptr;

	/*----------------------------------------------------------------------
	-- FUNCTION: PrintReceivedData()
	--
	-- DATE: December 1, 2017
	--
	-- DESIGNER: Jeremy Lee, Luke Lee
	--
	-- PROGRAMMER: Li-Yan Tong
	--
	-- INTERFACE: void DrawLetter(HWND hwnd, char* letter)
	--
	-- RETURNS: void
	--
	-- NOTES:
	-- This function takes a string and displays it (one character) on a
	-- device context. One major benefit of this function is that it
	-- calculates the position of the last character written on screen, and
	-- determines if it is off the Window by calculating the Window's width.
	----------------------------------------------------------------------*/
	DWORD WINAPI PrintData::PrintReceivedData(paramPrintData* param)
	{
		mCurrentRow = 4;

		std::queue<char*>* printQ = param->printQueue;
		LogFile* logfile = param->logfile;
		HWND* hwnd = param->hwnd;
		PrintData::hwnd = hwnd;

		while (!printQ->empty()) {
			// Load up
			char* payload = new char[512];

			payload = printQ->front();

			wchar_t printPayload[513];

			size_t outSize;

			mbstowcs_s(&outSize, printPayload, 513, payload, 512);

			DrawCharsByRow((TCHAR*)printPayload, mCurrentRow);
			mCurrentRow++;

			// Remove Data from queue.
			printQ->pop();

			// Print Sent Packets
			std::string logSent;
			logSent.append("Sent Packets: ");
			logSent.append(logfile->sent_packet + " ");
			PrintLog((TCHAR*)logSent.c_str(), 0);

			// Print Lost Packets
			std::string logLost;
			logLost.append("Lost Packets: ");
			logLost.append(logfile->lost_packet + " ");
			PrintLog((TCHAR*)logLost.c_str(), 1);

			// Print Recieved Packets
			std::string logReceive;
			logReceive.append("Recieved Packets: ");
			logReceive.append(logfile->received_packet + " ");
			PrintLog((TCHAR*)logReceive.c_str(), 2);

			// Print Corrupt Packets
			std::string logCorrupt;
			logCorrupt.append("Lost/Corrupt Packets: ");
			logCorrupt.append(logfile->received_corrupted_packet + "");
			PrintLog((TCHAR*)logCorrupt.c_str(), 3);

			Sleep(1000);
			
			return 1;
		}

		return 0;
	}

	/*----------------------------------------------------------------------
	-- FUNCTION:	DrawCharsByRow
	--
	-- DATE:		October 15, 2017
	--
	-- DESIGNER:	Jeremy Lee, Luke Lee
	--
	-- PROGRAMMER:	Li-Yan Tong
	--
	-- INTERFACE:	void DrawCharsByRow(const TCHAR* chars,
	--									unsigned int row)
	--
	-- ARGUMENT:	chars		- Pointer to the beginning of a character
	--							  string to draw on the Window.
	--				row			- Line number to draw a character string on.
	--							  Starts from 0.
	--
	-- RETURNS: void
	--
	-- NOTES:
	-- A function to draw a character string to the Window.
	-- Finds the x and y coordinate to start drawing the input character
	-- string from. Continues to print data on a new line specified by the
	-- row input, and then draws the input character string.
	----------------------------------------------------------------------*/
	void PrintData::DrawCharsByRow(const TCHAR* chars, unsigned int row)
	{
		HDC hdc;
		TEXTMETRIC tm;
		SIZE size;
		RECT rect;

		const int offsetRightSide = 25;

		hdc = GetDC(*hwnd); // Acquire DC
		GetTextMetrics(hdc, &tm); // get text metrics
		GetTextExtentPoint32(hdc, (char*)*chars, 1, &size); // compute length of a string 
		TextOut(hdc, X, Y, (char*)*chars, 1);
		X += size.cx; // advance to end of previous string
		if (GetWindowRect(*hwnd, &rect))
		{
			int width = rect.right - rect.left; // get Window width
			if (X >= width - offsetRightSide)
			{
				X = 0;
				Y = Y + tm.tmHeight + tm.tmExternalLeading; // next line
			}
		}

		hdc = GetDC(*hwnd); // Acquire DC
		GetTextMetrics(hdc, &tm); // get text metrics

		X = 0; // move to the beginning of line
		Y = 0; // move to the first line

		// move to the specified row
		while (row > 0)
		{
			Y += tm.tmHeight + tm.tmExternalLeading; // next line
			row--;
		}

		// draws the specified input string
		TextOut(hdc, X, Y, chars, _tcslen(chars));
		ReleaseDC(*hwnd, hdc); // release device context
	}

	/*----------------------------------------------------------------------
	-- FUNCTION:	UpdateLog
	--
	-- DATE:		October 15, 2017
	--
	-- DESIGNER:	Jeremy Lee, Luke Lee
	--
	-- PROGRAMMER:	Li-Yan Tong
	--
	-- INTERFACE:	void DrawCharsByRow(const TCHAR* chars,
	--									unsigned int row)
	--
	-- ARGUMENT:	chars		- Pointer to the beginning of a character
	--							  string to draw on the Window.
	--				row			- Line number to draw a character string on.
	--							  Starts from 0.
	--
	-- RETURNS: void
	--
	-- NOTES:
	-- A function to draw a character string to the Window.
	-- Finds the x and y coordinate to start drawing the input character
	-- string from. Erases the previously log information string on the line
	-- specified by the row input, and then draws the input character string
	-- on the same line.
	----------------------------------------------------------------------*/
	void PrintData::PrintLog(const TCHAR* chars, unsigned int row)
	{
		HDC hdc;
		TEXTMETRIC tm;

		hdc = GetDC(*hwnd); // Acquire DC
		GetTextMetrics(hdc, &tm); // get text metrics

		X = 0; // move to the beginning of line
		Y = 0; // move to the first line

		// move to the specified row
		while (row > 0)
		{
			Y += tm.tmHeight + tm.tmExternalLeading; // next line
			row--;
		}

		TCHAR eraser[128];
		memset(eraser, ' ', 128);
		eraser[127] = '\0';
		// removes previous printing
		TextOut(hdc, X, Y, eraser, _tcslen(eraser));
		// draws the specified input string
		TextOut(hdc, X, Y, chars, _tcslen(chars));
		ReleaseDC(*hwnd, hdc); // release device context
	}
}