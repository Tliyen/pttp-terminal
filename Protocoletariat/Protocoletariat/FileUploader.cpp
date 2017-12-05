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

			// CRC_32
			char* framePayloadOnly = new char[MAX_FRAME_SIZE - 6];
			for (unsigned int k = 0; k < MAX_FRAME_SIZE - 6; ++k)
			{
				framePayloadOnly[k] = frame[k + 2];
			}

			std::uint32_t crc = CRC::Calculate(framePayloadOnly, sizeof(framePayloadOnly), CRC::CRC_32());

			// debug ---------------------------------------------------
			//std::cout << "payload only: " << std::endl;
			//for (unsigned int z = 0; z < MAX_FRAME_SIZE - 6; ++z)
			//{
			//	std::cout << framePayloadOnly[z];
			//}
			//std::cout << std::endl;

			//std::cout << "crc generated: " << std::endl << std::hex << crc << std::endl;
			// debug ---------------------------------------------------

			delete framePayloadOnly;

			unsigned char* crcStr = new unsigned char[4];

			// first approach
			//sprintf_s(crcStr, sizeof(crcStr), "%lu", crc);

			// second approach
			crcStr[0] = (crc >> 24) & 0xFF;
			crcStr[1] = (crc >> 16) & 0xFF;
			crcStr[2] = (crc >> 8) & 0xFF;
			crcStr[3] = crc & 0xFF;

			// third approach
			//memcpy(crcStr, &crc, sizeof(crc));

			//std::cout << "CRC first method: " << std::endl;
			for (unsigned int k = 514; k < MAX_FRAME_SIZE; ++k)
			{
				frame[k] = crcStr[k - 514];
				// debug
				//std::cout << crcStr[k - 514] << "(" << (int)crcStr[k - 514] << ")";
			}
			//std::cout << std::endl;
			delete crcStr;

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