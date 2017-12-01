#pragma once

#include <windows.h>
#include <queue>

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
		static void LoadTextFile(paramFileUploader* param);
		static void ConvertFileIntoFrames();

	private:
		const size_t MAX_FRAME_SIZE = 512;

		static std::queue<char*>* mUploadQueue;
		static std::string mFilePath;
	};
}