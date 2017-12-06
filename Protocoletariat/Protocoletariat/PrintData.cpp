/*----------------------------------------------------------------------
-- SOURCE FILE: PrintData.cpp -	An Application that will monitor and display
--								LogFile containing packet transfer data and 
--								print transfered text data.
--									
--
-- PROGRAM:		Protocoletariat
--
-- FUNCTIONS:
--				DWORD WINAPI  PrintReceivedData(paramPrintData* param);
--				void PrintChar(HWND* hwnd, char* letter, unsigned int row,
--									int* X, int* Y)
--				void PrintLog(HWND* hwnd, const TCHAR* chars, unsigned int row)
--
-- DATE: December 1, 2017
--		 Initialize variable and methods
--		 December 5, 2017
--		 Change to print by character & utilize pointers
--		 December 6, 2017
--		 Code Clean up
--
-- DESIGNER: Jeremy Lee & Luke Lee
--
-- PROGRAMMER: Li-Yan Tong
--
-- NOTES:
-- This part of the program is responsible for displaying transfered data
-- and updating log file statistics on the terminal Windows screen.
--
-- It gathers packet transfer information from a LogFile
-- struct containing a count of sent, recieved, lost and corrupted
-- packets.  The log statistics are shown before and after a
-- transmission.
--
-- It assigns a pointer to the print data queue and displays the
-- transferred data character by character onto the screen and updates
-- the current x and y coordinates on screen accordingly.  After printing
-- it removes the latest print information from a data queue.
----------------------------------------------------------------------*/

#include "PrintData.h"

namespace protocoletariat
{
	int mCurrentRow = 1;

	/*----------------------------------------------------------------------
	-- FUNCTION: PrintReceivedData
	--
	-- DATE: December 1, 2017
	--
	-- DESIGNER: Jeremy Lee, Luke Lee
	--
	-- PROGRAMMER: Li-Yan Tong
	--
	-- INTERFACE: DWORD WINAPI PrintReceivedData(paramPrintData* param)
	--
	-- ARGUMENT: param			- a pointer to the paramPrintData structure
	--							  containing all the variables (windows GUI and print
	--							  data) required in PrintData functions.
	--
	-- RETURNS: DWORD WINAPI	- 0 if the intended functions run successfully
	--
	-- NOTES:
	-- This function thread takes a paraPrintData struct to gather device
	-- context and properly print characters in the terminal.  The struct
	-- also contains text data in printQueue and packet transfer information
	-- in a logfile that this function will print out.
	--
	-- This function continously loops and prints transfer information and
	-- text data until this thread is terminated.
	----------------------------------------------------------------------*/
	DWORD WINAPI PrintData::PrintReceivedData(paramPrintData* param)
	{
		std::queue<char*>* printQ = param->printQueue;
		LogFile* logfile = param->logfile;
		HWND* hwnd = param->hwnd;

		int* pX = param->X;
		int* pY = param->Y;

		// Active switch while engine is running
		while (protocolActive)
		{
			// Build Log
			std::ostringstream oss;

			// Print Sent Packets
			oss << "Sent: " << logfile->sent_packet << " | Lost: " << logfile->lost_packet
				<< " | Recieved: " << logfile->received_packet << " | Corrupted: " << logfile->received_corrupted_packet;
			std::string logSent = oss.str();
			PrintLog(hwnd, (const TCHAR*)logSent.c_str(), 0);

			if (!printQ->empty())
			{
				// Load up payload
				char* payload = new char[512];
				unsigned int payloadLength = 512;
				for (unsigned int i = 0; i < payloadLength; ++i)
				{
					payload[i] = printQ->front()[i];
				}

				// Print Payload
				for (unsigned int i = 0; i < payloadLength; i++)
				{
					PrintChar(hwnd, &payload[i], mCurrentRow, pX, pY);
				}

				// Remove Data from queue.
				delete payload;
				payload = nullptr;
				printQ->pop();
			}

			// Save it
			Sleep(2000);
		}
		return 0;
	}

	/*----------------------------------------------------------------------
	-- FUNCTION:	PrintLog
	--
	-- DATE:		December 5, 2017
	--
	-- DESIGNER:	Jeremy Lee, Luke Lee
	--
	-- PROGRAMMER:	Li-Yan Tong
	--
	-- INTERFACE:	void PrintLog(HWND* hwnd, const TCHAR* chars, unsigned int row)
	--
	-- ARGUMENT:	hwnd		- a pointer to the main Windows HWND handle.
	--				chars		- pointer to the beginning of a character
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
	void PrintData::PrintLog(HWND* hwnd, const TCHAR* chars, unsigned int row)
	{
		HDC hdc;
		TEXTMETRIC tm;

		hdc = GetDC(*hwnd); // Acquire DC
		GetTextMetrics(hdc, &tm); // get text metrics

		unsigned int X = 0; // move to the beginning of line
		unsigned int Y = 0; // move to the first line

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

	/*----------------------------------------------------------------------
	-- FUNCTION:	PrintChar
	--
	-- DATE:		December 2, 2017
	--
	-- DESIGNER:	Jeremy Lee, Luke Lee
	--
	-- PROGRAMMER:	Li-Yan Tong
	--
	-- INTERFACE:	void PrintChar(HWND* hwnd, char* letter, unsigned int row,
	--									int* X, int* Y)
	--
	-- ARGUMENT:	hwnd		- Windows handle access GUI information
	--				letter		- char Pointer a character to draw on the Window.
	--				row			- Line number to draw a character string on.
	--							  Starts from 0.
	--				X			- int Pointer to x-coordinate of a caret to print
	--							  characters
	--				Y			- int Pointer to y-coordinate of a caret to print
	--							  characters
	--
	-- RETURNS: void
	--
	-- NOTES:
	-- This function takes a string and displays it (one character) on a
	-- device context. One major benefit of this function is that it
	-- calculates the position of the last character written on screen, and
	-- determines if it is off the Window by calculating the Window's width.
	----------------------------------------------------------------------*/
	void PrintData::PrintChar(HWND* hwnd, char* letter, unsigned int row, int* X, int* Y)
	{
		HDC hdc;
		TEXTMETRIC tm;
		SIZE size;
		RECT rect;

		hdc = GetDC(*hwnd); // Acquire DC
		GetTextMetrics(hdc, &tm); // get text metrics
		GetTextExtentPoint32(hdc, letter, 1, &size); // compute length of a string 

		//move to this row
		if (mCurrentRow == 1) {
			while (row > 0)
			{
				*Y += tm.tmHeight + tm.tmExternalLeading; // next line
				row--;
			}
		}

		TextOut(hdc, *X, *Y, letter, 1);  // Display string
		*X += size.cx; // advance to end of previous string
		ReleaseDC(*hwnd, hdc); // release device context

		if (GetWindowRect(*hwnd, &rect))
		{
			int width = rect.right - rect.left; // get Window width
			if (*X >= width)
			{
				*X = 0;
				*Y = *Y + tm.tmHeight + tm.tmExternalLeading; // next line
			}
		}
		mCurrentRow += *Y; // Set Current row to last row printed
	}
}