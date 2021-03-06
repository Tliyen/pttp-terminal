/*----------------------------------------------------------------------
-- SOURCE FILE: FileUploader.cpp		- This file deals with the uploading
--										  file event and assembles the text
--										  characters into fixed-sized frames.
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
-- This thread is responsible for handling the uploading of the text file
-- to be send through the serial port. It gets all characters from the
-- intended file and crams them into a frame, then places the frame into
-- the UploadQueue where it can be grabbed by the protocol engine at the
-- appropriate time.
-- This file also inserts an EOT frame at the end of the file so that the
-- protocol is aware that the entire file has been read.
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
	-- ARGUMENT:	param		- a pointer to the paramFileUploader
	--							  structure containing all the variables
	--							  the uploader functions need.
	--
	-- RETURNS:		DWORD		- 0 if the intended functions run successfully
	--
	-- NOTES:
	-- This function is responsible for initiating the reading of the
	-- file. It is then responsible for calling the necessary functions
	-- to form the text into frames, with the appropriate control
	-- characters so that they can be successfully handled by the protocol
	-- engine.
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
				bool b = mUploadQueue->empty();
				b = mUploadQueue->empty();
			}
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
	-- ARGUMENT:	bufferRead		- a reference to a vector containing
	--								  the characters read from the serial port
	--
	-- RETURNS:		bool			- true if all the characters in text file
	--								  converts to frames successfully; false
	--								  otherwise.
	--
	-- NOTES:
	-- This function is called by the LoadTextFile function when there
	-- is a text file that needs to be read.
	-- It will use the characters taken from the frame in the serial port
	-- and place it into a frame after removing the SYN character. This
	-- readied frame will then be pushed into the uploadQueue, ready for
	-- access by the Protocol Engine.
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

			while (j < MAX_FRAME_SIZE - 4) // 514/518
			{
				frame[j++] = '\0';
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
	-- ARGUMENT:	controlChar		- control character to be put in
	--								  the control frame.
	--
	-- RETURNS:		void
	--
	-- NOTES:
	-- This function is called when the text file has been completely
	-- read. It pushes an EOT control frame into the uploadQueue, which
	-- allows the protocol engine to signify its completion to the pair
	-- device.
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
	-- ARGUMENT:	payload			- char array of data received
	--				strCrcReceived	- CRC received
	--
	-- RETURNS:		bool			- true if the CRC received matches with
	--								  the CRC generated; false otherwise.
	--
	-- NOTES:
	-- This function is called by the protocol engine to Validate the
	-- frame that has been received through the serial port. It will return
	-- true if the frame is validated, otherwise it will return false.
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