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



		return 0;
	}
}