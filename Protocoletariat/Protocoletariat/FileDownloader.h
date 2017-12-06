#pragma once

#include <queue>
#include <windows.h>
#include "PrintData.h"

namespace protocoletariat
{
	struct paramFileDownloader
	{
		std::queue<char*>* downloadQueue;
		std::queue<char*>* printQueue;
		HANDLE* handle;
		OVERLAPPED* olRead;
		DWORD* dwThreadExit;
		HANDLE* hEvent;
		bool* dlReady;
		bool* RVIflag;
	};

	static class FileDownloader
	{
	public:
		FileDownloader() = delete;
		static DWORD WINAPI ReadSerialPort(paramFileDownloader* param);
		static bool combineCharsIntoFrame(std::vector<char>& bufferFrame, const char charRead);

	private:
		static const size_t MAX_FRAME_SIZE = 518;
		static const char SYN = 22;
		static const char STX = 2;
		static const char EOT = 4;
		static const char ENQ = 5;
		static const char ACK = 6;
		static const char RVI = 7;

		static bool* rviReceived;
		static std::queue<char*>* pq;
	};
}