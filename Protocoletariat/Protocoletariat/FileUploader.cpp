#include <iostream>
#include "FileUploader.h"

namespace protocoletariat
{
	FileUploader::FileUploader(std::queue<char*>* uploadQueue, std::string filePath)
		: mUploadQueue(uploadQueue)
		, mFilePath(filePath)
	{
		
	}

	void FileUploader::LoadTextFile()
	{
		
	}

	void FileUploader::ConvertFileIntoFrames()
	{

	}
}