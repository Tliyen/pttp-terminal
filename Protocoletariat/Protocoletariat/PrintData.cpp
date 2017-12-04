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
#include "Main.h"

namespace protocoletariat
{
	/*----------------------------------------------------------------------
	-- FUNCTION: PrintReceivedData()
	--
	-- DATE: October 4, 2017
	--
	-- DESIGNER: Jeremy Lee
	--
	-- PROGRAMMER: Jeremy Lee
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
	void PrintData::PrintReceivedData()
	{
		//needs to be globally set
		printActive = TRUE;

		while (printActive) {
			if (!printQueue.empty()) {

				// Load up
				char* payload = new char[512];

				payload = printQueue.front();

				wchar_t printPayload[513];

				size_t outSize;

//				_tcscpy_s(printPayload, payload);

				mbstowcs_s(&outSize, printPayload, 513, payload, 512);

				// Get currentRow
				char* printWindow = new char[20000];
				//GetWindowText()

				DrawCharsByRow((TCHAR*)printPayload, mCurrentRow);
				printQueue.pop();
			}

			std::string logPrint;

			logPrint.append("Sent Packets: ");
			logPrint.append(logfile->sent_packet + " ");

			logPrint.append("Lost/Corrupt Packets: ");
			logPrint.append(logfile->lost_packet + " ");

			logPrint.append("Recieved Packets: ");
			logPrint.append(logfile->received_packet + " ");

			logPrint.append("Lost/Corrupt Packets: ");
			logPrint.append(logfile->received_corrupted_packet + "");

			UpdateLog((TCHAR*)logPrint.c_str(), 0);
			Sleep(1000);
		}
	}

	/*----------------------------------------------------------------------
	-- FUNCTION:	DrawCharsByRow
	--
	-- DATE:		October 15, 2017
	--
	-- DESIGNER:	Jeremy Lee, Luke Lee
	--
	-- PROGRAMMER:	Jeremy Lee, Luke Lee
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
	-- string from. Erases the previously drawn character string on the line
	-- specified by the row input, and then draws the input character string
	-- on the same line.
	----------------------------------------------------------------------*/
	void PrintData::DrawCharsByRow(const TCHAR* chars, unsigned int row)
	{
		HDC hdc;
		TEXTMETRIC tm;

		hdc = GetDC(hwnd); // Acquire DC
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
		ReleaseDC(hwnd, hdc); // release device context
	}

	/*----------------------------------------------------------------------
	-- FUNCTION:	DrawCharsByRow
	--
	-- DATE:		October 15, 2017
	--
	-- DESIGNER:	Jeremy Lee, Luke Lee
	--
	-- PROGRAMMER:	Jeremy Lee, Luke Lee
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
	-- string from. Erases the previously drawn character string on the line
	-- specified by the row input, and then draws the input character string
	-- on the same line.
	----------------------------------------------------------------------*/
	void UpdateLog(const TCHAR* chars, unsigned int row)
	{
		HDC hdc;
		TEXTMETRIC tm;

		hdc = GetDC(hwnd); // Acquire DC
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
		ReleaseDC(hwnd, hdc); // release device context
	}
}