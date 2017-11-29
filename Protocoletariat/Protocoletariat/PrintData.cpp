#include <stdio.h>
#include <windows.h>
#include "PrintData.h"

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
void PrintReceivedData()
{	
	//needs to be globally set
	printActive = TRUE;

	while (printActive) {
		if (!printQueue.empty()) {
			
		}
	}
}

/*----------------------------------------------------------------------
-- FUNCTION: DrawLetter
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
void DrawLetter(HWND hwnd, char* letter)
{
	const int offsetRightSide = 25;

	HDC hdc;
	TEXTMETRIC tm;
	SIZE size;
	RECT rect;

	hdc = GetDC(hwnd); // Acquire DC
	GetTextMetrics(hdc, &tm); // get text metrics
	GetTextExtentPoint32(hdc, letter, 1, &size); // compute length of a string 
	TextOut(hdc, X, Y, letter, 1);
	X += size.cx; // advance to end of previous string
	ReleaseDC(hwnd, hdc); // release device context
	if (GetWindowRect(hwnd, &rect))
	{
		int width = rect.right - rect.left; // get Window width
		if (X >= width - offsetRightSide)
		{
			X = 0;
			Y = Y + tm.tmHeight + tm.tmExternalLeading; // next line
		}
	}
}