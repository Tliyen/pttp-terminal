/*----------------------------------------------------------------------
-- @jeremy:
-- 171127
-- this file is just for testing. Comment out the entire main
-- to run the actual main after testing so that the entry point
-- won't get messed up.
--
----------------------------------------------------------------------*/
#include <iostream>
#include <fstream>
#include "FileUploader.h"
#include "FileDownloader.h"

using namespace protocoletariat;

int main()
{
	paramFileUploader* param = new paramFileUploader();
	std::queue<char*>* queue = new std::queue<char*>();

	param->filePath = "c:\\test.txt";
	param->uploadQueue = queue;

	FileUploader::LoadTextFile(param);

	while (!queue->empty())
	{
		char* queued = queue->front();
		queue->pop();

		for (unsigned i = 0; i < 518; ++i)
		{
			std::cout << queued[i];
		}
	}
	
	return 0;
}