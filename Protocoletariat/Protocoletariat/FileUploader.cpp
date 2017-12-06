/*----------------------------------------------------------------------
-- SOURCE FILE: FileUploader.cpp		-
--
--
-- PROGRAM:		Protocoletariat
--
-- FUNCTIONS:
--				DWORD WINAPI LoadTextFile(paramFileUploader* param)
--				bool ConvertFileIntoFrames(const std::vector<char>& bufferRead)
--				void QueueControlFrame(const char controlChar)
--				bool ValidateCrc(char* payload, char* strCrcReceived)
--
--
-- DATE:		December 5, 2017
--
-- DESIGNER:	Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
--
-- PROGRAMMER:	Jeremy Lee
--
-- NOTES:
----------------------------------------------------------------------*/
#include "FileUploader.h"

namespace protocoletariat
{
	std::queue<char*>* FileUploader::mUploadQueue = nullptr;
	std::string FileUploader::mFilePath = "";

	/*------------------------------------------------------------------
	-- FUNCTION:	LoadTextFile
	--
	-- DATE:		December 5, 2017
	--
	-- DESIGNER:	Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
	--
	-- PROGRAMMER:	Jeremy Lee
	--
	-- INTERFACE:	DWORD WINAPI LoadTextFile(paramFileUploader* param)
	--
	-- ARGUMENT:	param			-
	--
	-- RETURNS:		DWORD			-
	--
	-- NOTES:
	-- 
	------------------------------------------------------------------*/
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
				QueueControlFrame(EOT);
				// trigger ENQ request event for the protocol engine
				unsigned int size = mUploadQueue->size();
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

	/*------------------------------------------------------------------
	-- FUNCTION:	ConvertFileIntoFrames
	--
	-- DATE:		December 5, 2017
	--
	-- DESIGNER:	Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
	--
	-- PROGRAMMER:	Jeremy Lee
	--
	-- INTERFACE:	bool ConvertFileIntoFrames(const std::vector<char>& bufferRead)
	--
	-- ARGUMENT:	bufferRead		-
	--
	-- RETURNS:		bool			-
	--
	-- NOTES:
	------------------------------------------------------------------*/
	bool FileUploader::ConvertFileIntoFrames(const std::vector<char>& bufferRead)
	{
		bool fileConverted = false;

		char* frame;

		unsigned int i = 0;
		while (i < bufferRead.size())
		{
			frame = new char[MAX_FRAME_SIZE];
			unsigned int j = 0;
			frame[j++] = SYN; // first char SYN
			frame[j++] = STX; // second char STX

			// until buffer read is empty OR the frame gets filled up
			while (i < bufferRead.size() && j < MAX_FRAME_SIZE - 4) // 514/518
			{
				// frame: from 2 / buffer: from 0
				frame[j++] = bufferRead[i++];
			}

			// CRC_32
			char* framePayloadOnly = new char[MAX_FRAME_SIZE - 6]; // 512/518
			for (unsigned int k = 0; k < MAX_FRAME_SIZE - 6; ++k)
			{
				// payload: from 0 / frame: from 2
				framePayloadOnly[k] = frame[k + 2];
			}

			// generate CRC only with the payload
			CRC::Table<std::uint32_t, 32> table(CRC::CRC_32());
			std::uint32_t crc = CRC::Calculate(framePayloadOnly, 512, table);

			// debug ---------------------------------------------------
			//std::cout << "payload only: " << std::endl;
			//unsigned int count = 0;
			//std::cout << "[";
			//while (count < MAX_FRAME_SIZE - 6)
			//{
			//	std::cout << framePayloadOnly[count++];
			//}
			//std::cout << "]";
			//std::cout << std::endl;
			//std::cout << "char count: " << count << std::endl;
			//std::cout << "crc generated: " << crc << std::endl;
			// debug ---------------------------------------------------

			delete framePayloadOnly;

			char* crcStr = new char[4];

			// second approach
			crcStr[0] = (crc >> 24) & 0xFF;
			crcStr[1] = (crc >> 16) & 0xFF;
			crcStr[2] = (crc >> 8) & 0xFF;
			crcStr[3] = crc & 0xFF;

			for (unsigned int k = 514; k < MAX_FRAME_SIZE; ++k)
			{
				frame[k] = crcStr[k - 514];
			}
			delete crcStr;

			mUploadQueue->push(frame);
		}

		if (i == bufferRead.size())
		{
			return true;
		}

		// failure debug -----------------------------------------------
		unsigned int k = 0;
		std::cerr << "Abnormal file read: ";
		while (k < bufferRead.size())
		{
			std::cerr << bufferRead[k];
		}
		std::cerr << std::endl;
		// -------------------------------------------------------------

		return false;
	}

	/*------------------------------------------------------------------
	-- FUNCTION:	QueueControlFrame
	--
	-- DATE:		December 5, 2017
	--
	-- DESIGNER:	Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
	--
	-- PROGRAMMER:	Jeremy Lee
	--
	-- INTERFACE:	void QueueControlFrame(const char controlChar)
	--
	-- ARGUMENT:	controlChar		-
	--
	-- RETURNS:		void
	--
	-- NOTES:
	------------------------------------------------------------------*/
	void FileUploader::QueueControlFrame(const char controlChar)
	{
		char* frameCtr = new char[2];
		frameCtr[0] = SYN;
		frameCtr[1] = controlChar;

		mUploadQueue->push(frameCtr);
	}

	/*------------------------------------------------------------------
	-- FUNCTION:	ValidateCrc
	--
	-- DATE:		December 5, 2017
	--
	-- DESIGNER:	Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
	--
	-- PROGRAMMER:	Jeremy Lee
	--
	-- INTERFACE:	bool ValidateCrc(char* payload, char* strCrcReceived)
	--
	-- ARGUMENT:	payload			-
	--				strCrcReceived	-
	--
	-- RETURNS:		bool			-
	--
	-- NOTES:
	------------------------------------------------------------------*/
	bool FileUploader::ValidateCrc(char* payload, char* strCrcReceived)
	{
		char* strCrcGenerated = new char[4];

		CRC::Table<std::uint32_t, 32> table(CRC::CRC_32());
		std::uint32_t crcGenerated = CRC::Calculate(payload, 512, table);

		strCrcGenerated[0] = (crcGenerated >> 24) & 0xFF;
		strCrcGenerated[1] = (crcGenerated >> 16) & 0xFF;
		strCrcGenerated[2] = (crcGenerated >> 8) & 0xFF;
		strCrcGenerated[3] = crcGenerated & 0xFF;

		for (unsigned int i = 0; i < 4; ++i)
		{
			if (strCrcReceived[i] != strCrcGenerated[i])
			{
				return false;
			}
		}

		return true;
	}
}