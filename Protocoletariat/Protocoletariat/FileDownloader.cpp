/*----------------------------------------------------------------------
-- SOURCE FILE: FileDownloader.cpp		-
--
--
-- PROGRAM:		Protocoletariat
--
-- FUNCTIONS:
--				DWORD WINAPI ReadSerialPort(paramFileDownloader* param)
--				bool combineCharsIntoFrame(std::vector<char>& bufferFrame, const char charRead)
--
--
-- DATE:		December 5, 2017
--
-- DESIGNER:	Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
--
-- PROGRAMMER:	Jeremy Lee
--
-- NOTES:
----------------------------------------------------------------------*/
#include "FileDownloader.h"

namespace protocoletariat
{
	bool* FileDownloader::rviReceived = nullptr;

	/*------------------------------------------------------------------
	-- FUNCTION:	ReadSerialPort
	--
	-- DATE:		December 5, 2017
	--
	-- DESIGNER:	Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
	--
	-- PROGRAMMER:	Jeremy Lee
	--
	-- INTERFACE:	DWORD WINAPI ReadSerialPort(paramFileDownloader* param)
	--
	-- ARGUMENT:	param			-
	--
	-- RETURNS:		DWORD			-
	--
	-- NOTES:
	-- This code is responsible to initializing the read buffer and then
	-- reading bytes from the serial port. First step is to check the 
	-- first byte and ensure that it is a SYN char. If it is not the SYN 
	-- char, the loop continues from the beginning, which means the rest 
	-- of the loop is ignored. If it is a SYN char, the next 1 byte in the 
	-- stream is read and checked if it is a STX character after added to 
	-- the read buffer. If it is a STX character, the next 516 bytes in 
	-- the stream is read and added to the read buffer. Then, the read 
	-- buffer is added to the global download queue so the protocol engine 
	-- can take it, and a comm event is triggered to notify the protocol 
	-- engine.
	------------------------------------------------------------------*/
	DWORD WINAPI FileDownloader::ReadSerialPort(paramFileDownloader* param)
	{
		std::queue<char*>* downloadQueue = param->downloadQueue;
		HANDLE* handle = param->handle;
		OVERLAPPED& olRead = param->olRead;
		DWORD& dwThreadExit = param->dwThreadExit;
		bool* downloadReady = param->dlReady;
		rviReceived = param->RVIflag; // member variable
		DWORD dwRead, dwLrc, dwEndTime;
		char bufferChar[1];
		std::vector<char> bufferFrame(MAX_FRAME_SIZE);
		char* frame;

		// TODO: change to a global flag
		bool bReading = true;

		// create a manual reset event
		olRead.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		while (bReading)
		{
			bufferChar[0] = '\0';
			if (!ReadFile(handle, bufferChar, 1, &dwRead, &olRead))
			{
				dwRead = 0;
				if ((dwLrc = GetLastError()) == ERROR_IO_PENDING)
				{
					dwEndTime = GetTickCount() + 1000;
					while (!GetOverlappedResult(handle, &olRead, &dwRead, FALSE))
					{
						if (GetTickCount() > dwEndTime)
						{
							break;
						}
					}
					if (dwRead)
					{
						// process read char
						if (combineCharsIntoFrame(bufferFrame, bufferChar[0])) // frame complete
						{
							frame = new char[MAX_FRAME_SIZE - 1]; // exclude first char (SYN)

							unsigned int i = 0;
							while (i < bufferFrame.size() && i < MAX_FRAME_SIZE - 1) // exclude first char (SYN)
							{
								frame[i++] = bufferFrame[1 + (i++)];
							}

							downloadQueue->push(frame); // queue the downloaded frame
							*downloadReady = true; // set flag for protocol engine

							bufferFrame.clear(); // clean the frame buffer
						}
					}
				}
				else
				{
					// error msg
				}
			}
			else
			{
				if (dwRead)
				{
					// process read char
					if (combineCharsIntoFrame(bufferFrame, bufferChar[0])) // frame complete
					{
						frame = new char[MAX_FRAME_SIZE - 1]; // exclude first char (SYN)

						unsigned int i = 0;
						while (i < bufferFrame.size() && i < MAX_FRAME_SIZE - 1) // exclude first char (SYN)
						{
							frame[i++] = bufferFrame[1 + (i++)];
						}

						downloadQueue->push(frame); // queue the downloaded frame
						*downloadReady = true; // set flag for protocol engine

						bufferFrame.clear(); // clean the frame buffer
					}
				}
			}
			ResetEvent(olRead.hEvent); // manually reset event
		}

		PurgeComm(handle, PURGE_RXCLEAR); // clean out pending bytes
		ExitThread(dwThreadExit); // exit thread

		return 0;
	}

	/*------------------------------------------------------------------
	-- FUNCTION:	combineCharsIntoFrame
	--
	-- DATE:		December 5, 2017
	--
	-- DESIGNER:	Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
	--
	-- PROGRAMMER:	Jeremy Lee
	--
	-- INTERFACE:	bool combineCharsIntoFrame(std::vector<char>& bufferFrame, const char charRead)
	--
	-- ARGUMENT:	bufferFrame		-
	--				charRead		-
	--
	-- RETURNS:		bool			-
	--
	-- NOTES:
	------------------------------------------------------------------*/
	bool FileDownloader::combineCharsIntoFrame(std::vector<char>& bufferFrame, const char charRead)
	{	// RVI bell char
		if (bufferFrame.size() == 0) // frame empty
		{
			if (charRead != SYN) // first char to put in frame is not SYN
			{
				// do nothing
			}
		}
		else if (bufferFrame.back() == SYN) // only SYN in frame
		{
			if (charRead == STX) // incoming payload
			{
				bufferFrame.push_back(charRead);
			}
			else if (charRead == ENQ || charRead == EOT
				|| charRead == ACK) // control frame excluding RVI
			{
				bufferFrame.push_back(charRead);
				return true; // frame complete
			}
			else if (charRead == RVI) // RVI received
			{
				*rviReceived = true; // set flag for protocol engine
				bufferFrame.clear(); // abandon RVI control frame
				return false; // continue without queuing the frame
			}
			else // invalid char order
			{
				bufferFrame.clear();
			}
		}
		else if (bufferFrame.back() == STX) // incoming payload
		{
			bufferFrame.push_back(charRead);
		}
		else // payload char
		{
			bufferFrame.push_back(charRead);
			if (bufferFrame.size() == MAX_FRAME_SIZE)
			{
				return true; // frame complete
			}
		}

		return false; // frame incomplete
	}
}