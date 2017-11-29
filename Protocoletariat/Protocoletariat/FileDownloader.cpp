#include "FileDownloader.h"

namespace protocoletariat
{
	std::queue<char*>* mDownloadQueue = nullptr;
	HWND* mHandle = nullptr;

	void FileDownloader::ReadSerialPort(paramFileDownloader* param)
	{
		mDownloadQueue = param->downloadQueue;
		mHandle = param->handle;
	}
}