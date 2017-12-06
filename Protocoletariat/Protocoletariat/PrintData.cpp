/*----------------------------------------------------------------------
-- SOURCE FILE: PrintData.cpp
--
-- PROGRAM: Protocoletariat
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
-- PROGRAMMER: Li-Yan Tong
--
-- NOTES:
-- This part of the program is responsible for displaying Packet 
-- transfer information and sucessfully transfered text data.
----------------------------------------------------------------------*/

#include "PrintData.h"

namespace protocoletariat
{
	int mCurrentRow = 4;

	/*----------------------------------------------------------------------
	-- FUNCTION: PrintReceivedData()
	--
	-- DATE: December 1, 2017
	--
	-- DESIGNER: Jeremy Lee, Luke Lee
	--
	-- PROGRAMMER: Li-Yan Tong
	--
	-- INTERFACE: void DrawLetter(paramPrintData* param)
	--				param - This struct holds pointers to the
	--						required 
	--
	-- RETURNS: DWORD WINAPI - 
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

		// Switch out with master switch boolean later
		while (protocolActive)
		{
			// Build Log
			std::ostringstream oss;

			// Print Sent Packets
			oss << "Sent Packets: " << logfile->sent_packet;
			std::string logSent = oss.str();
			PrintLog(hwnd, (const TCHAR*)logSent.c_str(), 0, pX, pY);

			oss.str("");
			oss.clear();

			// Print Lost Packets
			oss << "Lost Packets: " << logfile->lost_packet;
			std::string logLost = oss.str();
			PrintLog(hwnd, (const TCHAR*)logLost.c_str(), 1, pX, pY);

			oss.str("");
			oss.clear();

			// Print Recieved Packets
			oss << "Recieved Packets: " << logfile->received_packet;
			std::string logReceive = oss.str();
			PrintLog(hwnd, (const TCHAR*)logReceive.c_str(), 2, pX, pY);

			oss.str("");
			oss.clear();

			// Print Corrupt Packets
			oss << "Corrupt Packets: : " << logfile->received_corrupted_packet;
			std::string logCorrupt = oss.str();
			PrintLog(hwnd, (const TCHAR*)logCorrupt.c_str(), 3, pX, pY);

			oss.str("");
			oss.clear();

			if (!printQ->empty())
			{
				// Load up payload
				char* payload = new char[512];

				payload = printQ->front();

				// Print Payload

				int payloadLength = strlen(payload);

				for (int i = 0; i < payloadLength; i++)
				{
					char* print = &payload[i];
					PrintPayload(hwnd, (char*)print, mCurrentRow, pX, pY);
				}

				// Remove Data from queue.
				delete payload;					
				printQ->pop();

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
	void PrintData::PrintPayload(HWND* hwnd, char* chars, unsigned int row, int* X, int* Y)
	{
		HDC hdc;
		TEXTMETRIC tm;
		SIZE size;
		RECT rect;

		//const int offsetRightSide = 25;

		//*X = 0; // move to the beginning of line
		//*Y = 0; // move to this row

		hdc = GetDC(*hwnd); // Acquire DC
		GetTextMetrics(hdc, &tm); // get text metrics
		GetTextExtentPoint32(hdc, chars, 1, &size); // compute length of a string 

		//move to this row
		if (mCurrentRow == 4) {
			while (row > 0)
			{
				*Y += tm.tmHeight + tm.tmExternalLeading; // next line
				row--;
			}
		}

		TextOut(hdc, *X, *Y, chars, 1);  // Display string
		*X += size.cx; // advance to end of previous string
		ReleaseDC(*hwnd, hdc); // release device context

		if (GetWindowRect(*hwnd, &rect))
		{
			int width = rect.right - rect.left; // get Window width
			if (*X >= width)//- offsetRightSide)
			{
				*X = 0;
				*Y = *Y + tm.tmHeight + tm.tmExternalLeading; // next line
			}
		}


		mCurrentRow += *Y; // Set Current row to last row printed

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