#include "FileDownloader.h"

namespace protocoletariat
{
	//std::queue<char*>* mDownloadQueue = nullptr;
	//HWND* mHandle = nullptr;

	DWORD WINAPI FileDownloader::ReadSerialPort(paramFileDownloader* param)
	{
		//mDownloadQueue = param->downloadQueue;
		//mHandle = param->handle;

		std::queue<char*>* downloadQueue = param->downloadQueue;
		HWND* handle = param->handle;
		OVERLAPPED& olRead = param->olRead;
		DWORD& dwThreadExit = param->dwThreadExit;
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
							frame = new char[MAX_FRAME_SIZE];

							unsigned int i = 0;
							while (i < bufferFrame.size() && i < MAX_FRAME_SIZE)
							{
								frame[i++] = bufferFrame[i++];
							}

							downloadQueue->push(frame); // queue the downloaded frame
							// TODO: call event for protocol engine

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
						frame = new char[MAX_FRAME_SIZE];

						unsigned int i = 0;
						while (i < bufferFrame.size() && i < MAX_FRAME_SIZE)
						{
							frame[i++] = bufferFrame[i++];
						}

						downloadQueue->push(frame); // queue the downloaded frame
													// TODO: call event for protocol engine

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
				|| charRead == ACK || charRead == RVI) // control frame
			{
				bufferFrame.push_back(charRead);
				return true; // frame complete
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