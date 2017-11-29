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
		void ReadSerialPort(paramFileDownloader* param);

	private:
		const size_t MAX_FRAME_SIZE = 512;

		static std::queue<char*>* mDownloadQueue;
		static HWND* mHandle;
	};
}