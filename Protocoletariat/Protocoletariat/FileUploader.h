#pragma once

#include <queue>
#include <windows.h>

namespace protocoletariat
{
	struct paramFileUploader
	{
		std::queue<char*>* uploadQueue;
		std::string filePath;
	};

	static class FileUploader
	{
	public:
		FileUploader() = delete;
		static DWORD WINAPI LoadTextFile(paramFileUploader* param);
		static bool FileUploader::ConvertFileIntoFrames(const std::vector<char>& bufferRead);

	private:
		static const size_t MAX_FRAME_SIZE = 518;
		static const char STX = 2;
		static const char SYN = 22;

		static std::queue<char*>* mUploadQueue;
		static std::string mFilePath;
	};
}