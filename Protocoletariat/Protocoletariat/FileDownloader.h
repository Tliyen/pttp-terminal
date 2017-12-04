#pragma once

#include <queue>
#include <Windows.h>

namespace protocoletariat
{
	struct paramFileDownloader
	{
		std::queue<char*>* downloadQueue;
		HWND* handle;
	};

	static class FileDownloader
	{
	public:
		FileDownloader() = delete;
		static DWORD WINAPI ReadSerialPort(paramFileDownloader* param);

	private:
		const size_t MAX_FRAME_SIZE = 518;
		static const char STX = 2;
		static const char SYN = 22;

		//static std::queue<char*>* mDownloadQueue;
		//static HWND* mHandle;
	};
}