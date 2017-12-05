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
//#include "Main.h"

namespace protocoletariat
{
	//static HWND* hwnd = nullptr;
	int mCurrentRow = 4;
	bool printDataActive = true;

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

		std::queue<char*>* printQ = param->printQueue;
		LogFile* logfile = param->logfile;
		HWND* hwnd = param->hwnd;
		//PrintData::hwnd = hwnd;

		int* pX = param->X;
		int* pY = param->Y;

		while (printDataActive) 
		{
			if (!printQ->empty())
			{
			// Load up payload
			char* payload = new char[512];

			payload = printQ->front();

			// wchar_t Conversion
			//wchar_t printPayload[513];
			//size_t outSize;
			//mbstowcs_s(&outSize, printPayload, 513, payload, 512);

			PrintPayload(hwnd,(TCHAR*)payload, mCurrentRow, pX, pY);

			// Remove Data from queue.
			printQ->pop();

			// Print Sent Packets
			std::string logSent;
			logSent.append("Sent Packets: ");
			logSent.append(logfile->sent_packet + " ");
			PrintLog(hwnd,(TCHAR*)logSent.c_str(), 0, pX, pY);

			// Print Lost Packets
			std::string logLost;
			logLost.append("Lost Packets: ");
			logLost.append(logfile->lost_packet + " ");
			PrintLog(hwnd,(TCHAR*)logLost.c_str(), 1, pX, pY);

			// Print Recieved Packets
			std::string logReceive;
			logReceive.append("Recieved Packets: ");
			logReceive.append(logfile->received_packet + " ");
			PrintLog(hwnd,(TCHAR*)logReceive.c_str(), 2, pX, pY);

			// Print Corrupt Packets
			std::string logCorrupt;
			logCorrupt.append("Lost/Corrupt Packets: ");
			logCorrupt.append(logfile->received_corrupted_packet + "");
			PrintLog(hwnd,(TCHAR*)logCorrupt.c_str(), 3, pX, pY);

			Sleep(2000);
			}
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
	void PrintData::PrintPayload(HWND* hwnd, const TCHAR* chars, unsigned int row, int* X, int* Y)
	{
		HDC hdc;
		TEXTMETRIC tm;
		SIZE size;
		RECT rect;

		const int offsetRightSide = 25;

		*X = 0; // move to the beginning of line
		*Y += mCurrentRow; // move to the last empty line

		hdc = GetDC(*hwnd); // Acquire DC
		GetTextMetrics(hdc, &tm); // get text metrics
		GetTextExtentPoint32(hdc, (char*)*chars, _tcslen(chars), &size); // compute length of a string 
		TextOut(hdc, *X, *Y, (char*)*chars, _tcslen(chars));  // Display string
		*X += size.cx; // advance to end of previous string
		if (GetWindowRect(*hwnd, &rect))
		{
			int width = rect.right - rect.left; // get Window width
			if (*X >= width - offsetRightSide)
			{
				*X = 0;
				*Y = *Y + tm.tmHeight + tm.tmExternalLeading; // next line
			}
		}

		// draws the specified input string
		//TextOut(hdc, X, Y, chars, _tcslen(chars));

		mCurrentRow = *Y; // Set Current row to last row printed
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
	void PrintData::PrintLog(HWND* hwnd, const TCHAR* chars, unsigned int row, int* X, int* Y)
	{
		HDC hdc;
		TEXTMETRIC tm;

		hdc = GetDC(*hwnd); // Acquire DC
		GetTextMetrics(hdc, &tm); // get text metrics

		*X = 0; // move to the beginning of line
		*Y = 0; // move to the first line

		// move to the specified row
		while (row > 0)
		{
			*Y += tm.tmHeight + tm.tmExternalLeading; // next line
			row--;
		}

		TCHAR eraser[128];
		memset(eraser, ' ', 128);
		eraser[127] = '\0';
		// removes previous printing
		TextOut(hdc, *X, *Y, eraser, _tcslen(eraser));
		// draws the specified input string
		TextOut(hdc, *X, *Y, chars, _tcslen(chars));
		ReleaseDC(*hwnd, hdc); // release device context
	}
}