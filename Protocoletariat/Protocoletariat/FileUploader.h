#pragma once

#include <memory>
#include <queue>

namespace protocoletariat
{
	class FileUploader
	{
	public:
		FileUploader(std::queue<char*>* uploadQueue, std::string filePath);
		void LoadTextFile();
		void ConvertFileIntoFrames();

	private:
		const size_t MAX_FRAME_SIZE = 512;

		std::queue<char*>* mUploadQueue;
		std::string mFilePath;

	};
}