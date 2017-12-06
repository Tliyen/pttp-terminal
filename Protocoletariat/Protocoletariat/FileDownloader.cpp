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
-- This file is responsible for handling the downloading of frames from
-- the serial port. These frames will be taken, run through basic
-- validation to ensure that the SYN char is in place. The SYN char will
-- be stripped from the frame, and it will be placed in the download queue
-- ready for processing by the Protocol Engine.
----------------------------------------------------------------------*/
#include "FileDownloader.h"

namespace protocoletariat
{
	bool* FileDownloader::rviReceived = nullptr;
	std::queue<char*>* FileDownloader::pq = nullptr;

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
	-- ARGUMENT:	param		- represents a pointer to the paramFileDownload
	--							  structure including all the variables required
	--							  for reading data
	--
	-- RETURNS:		DWORD		- 0 if no error is encountered
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
		OVERLAPPED* olRead = param->olRead;
		DWORD* dwThreadExit = param->dwThreadExit;
		bool* downloadReady = param->dlReady;

		LogFile* mLogFile = param->logfile;

		rviReceived = param->RVIflag; // member variable
		HANDLE* hEvent = param->hEvent;
		DWORD dwRead, dwLrc, dwEndTime;
		pq = param->printQueue;
		char bufferChar[1];
		std::vector<char> bufferFrame;
		char* frame;

		// TODO: change to a global flag
		//bool bReading = true;

		// create a manual reset event
		*hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		olRead->hEvent = *hEvent;
		ResetEvent(olRead->hEvent); // manually reset event

		while (protocolActive)
		{
			bufferChar[0];
			if (!ReadFile(*handle, bufferChar, 1, &dwRead, olRead))
			{
				std::cout << bufferChar;
				dwRead = 0;
				if ((dwLrc = GetLastError()) == ERROR_IO_PENDING)
				{
					dwEndTime = GetTickCount() + 1000;
					while (!GetOverlappedResult(handle, olRead, &dwRead, FALSE))
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
							mLogFile->received_packet++;
							frame = new char[MAX_FRAME_SIZE]; // exclude first char (SYN)

							unsigned int i = 0;
							while (i < bufferFrame.size() && i < MAX_FRAME_SIZE) // exclude first char (SYN)
							{
								frame[i] = bufferFrame[i];
								i++;
							}

							std::cout << frame[0];
							downloadQueue->push(frame); // queue the downloaded frame
							*downloadReady = true; // set flag for protocol engine

							bufferFrame.resize(0); // clean the frame buffer
						}
					}
				}
			}
			else
			{
				if (dwRead)
				{
					// process read char
					if (combineCharsIntoFrame(bufferFrame, bufferChar[0])) // frame complete
					{
						mLogFile->received_packet++;
						frame = new char[MAX_FRAME_SIZE]; // exclude first char (SYN)

						unsigned int i = 0;
						while (i < bufferFrame.size() && i < MAX_FRAME_SIZE) // exclude first char (SYN)
						{
							frame[i] = bufferFrame[i];
							i++;
						}

						std::cout << frame[0];
						downloadQueue->push(frame); // queue the downloaded frame
						*downloadReady = true; // set flag for protocol engine

						bufferFrame.resize(0); // clean the frame buffer
					}
				}
			}
			ResetEvent(olRead->hEvent); // manually reset event
		}

		PurgeComm(handle, PURGE_RXCLEAR); // clean out pending bytes
		ExitThread(*dwThreadExit); // exit thread

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
	-- ARGUMENT:	bufferFrame		- reference to a vector to stored read
	--								  characters
	--				charRead		- character read from the serial port
	--
	-- RETURNS:		bool			- true if frame is successfully assembled;
	--								  false otherwise
	--
	-- NOTES:
	-- This function is called to combine all the chars read from the port
	-- into a frame that can be placed in the upload queue. The frame is
	-- loaded into the already created char pointer, which is then called
	-- used by the calling function if the function returns true.
	------------------------------------------------------------------*/
	bool FileDownloader::combineCharsIntoFrame(std::vector<char>& bufferFrame, const char charRead)
	{
		if (bufferFrame.size() == 0) // frame empty
		{
			if (charRead != SYN) // first char to put in frame is not SYN
			{
				// do nothing
			}
			else
			{
				bufferFrame.push_back(charRead);
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
				bufferFrame.resize(0); // abandon RVI control frame
				return false; // continue without queuing the frame
			}
			else // invalid char order
			{
				bufferFrame.resize(0);
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