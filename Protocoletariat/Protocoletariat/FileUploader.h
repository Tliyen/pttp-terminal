#pragma once

#include <queue>
#include <windows.h>
#include <iomanip>  // Includes ::std::hex
#include <iostream>
#include <fstream>
#include <cstdint>  // Includes ::std::uint32_t
#include "CRC.h"

namespace protocoletariat
{
	struct paramFileUploader
	{
		std::queue<char*>* uploadQueue;
		std::string filePath;
	};

	union mix_t
	{
		std::uint32_t theDWord;
		std::uint8_t theBytes[4];
	};

	static class FileUploader
	{
	public:
		FileUploader() = delete;
		static DWORD WINAPI LoadTextFile(paramFileUploader* param);
		static bool ConvertFileIntoFrames(const std::vector<char>& bufferRead);
		static void QueueControlFrame(const char controlChar);
		static bool ValidateCrc(char* payload, char* strCrcReceived);

	private:
		static const size_t MAX_FRAME_SIZE = 518;
		static const char SYN = 22;
		static const char STX = 2;
		static const char EOT = 4;

		static std::queue<char*>* mUploadQueue;
		static std::string mFilePath;
	};
}