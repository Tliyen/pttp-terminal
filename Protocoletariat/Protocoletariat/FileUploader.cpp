#include <iostream>
#include <fstream>
#include "FileUploader.h"

namespace protocoletariat
{
	std::queue<char*>* FileUploader::mUploadQueue = nullptr;
	std::string FileUploader::mFilePath = "";

	DWORD WINAPI FileUploader::LoadTextFile(paramFileUploader* param)
	{
		mUploadQueue = param->uploadQueue;
		mFilePath = param->filePath;

		std::ifstream fileRead(mFilePath, std::ios::binary | std::ios::ate);
		std::streamsize sizeFile = fileRead.tellg();
		fileRead.seekg(0, std::ios::beg);

		if (sizeFile < 0) // no file found (tellg() returns -1)
		{
			// error msg
			return 0;
		}

		std::vector<char> bufferRead(sizeFile);
		if (fileRead.read(bufferRead.data(), sizeFile))
		{
			if (ConvertFileIntoFrames(bufferRead))
			{
				// trigger ENQ request event for the protocol engine
			}
			else
			{
				// failure msg
			}
		}
		else
		{
			// failure msg
		}

		fileRead.clear();
		fileRead.close();

		return 0;
	}

	bool FileUploader::ConvertFileIntoFrames(const std::vector<char>& bufferRead)
	{
		bool fileConverted = false;

		char* frame;

		unsigned int i = 0;
		while (i < bufferRead.size())
		{
			frame = new char[MAX_FRAME_SIZE];
			unsigned int j = 0;
			frame[j++] = SYN;
			frame[j++] = STX;

			while (i < bufferRead.size() && j < MAX_FRAME_SIZE - 4)
			{
				frame[j++] = bufferRead[i++];
			}

			// CRC
			frame[514] = '*';
			frame[515] = 'C';
			frame[516] = 'R';
			frame[517] = 'C';

			mUploadQueue->push(frame);
		}

		if (i == bufferRead.size())
		{
			return true;
		}

		// failure debug -----------------------------------------------
		unsigned int k = 0;
		while (k < bufferRead.size())
		{
			std::cerr << bufferRead[k];
		}
		std::cerr << std::endl;
		// -------------------------------------------------------------

		return false;
	}
}