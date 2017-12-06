/*----------------------------------------------------------------------
-- @jeremy:
-- 171127
-- this file is just for testing. Comment out the entire main
-- to run the actual main after testing so that the entry point
-- won't get messed up.
--
----------------------------------------------------------------------*/
//#include <iostream>
//#include <fstream>
//#include "FileUploader.h"
//#include "FileDownloader.h"
//
//using namespace protocoletariat;

//int main()
//{
//	paramFileUploader* param = new paramFileUploader();
//	std::queue<char*>* queue = new std::queue<char*>();
//
//	param->filePath = "c:\\test.txt";
//	param->uploadQueue = queue;
//
//	FileUploader::LoadTextFile(param);
//
//	char* frameReceived = new char[518];
//	char* payload = new char[512];
//	char* strCrc = new char[4];
//
//	unsigned int j = 1;
//	while (!queue->empty())
//	{
//		std::cout << "Frame#" << j++ << std::endl;
//		char* queued = queue->front();
//		queue->pop();
//
//		std::cout << "[";
//		for (unsigned int i = 0; i < 518; ++i)
//		{
//			std::cout << queued[i];
//			if (i > 1 && i < 514)
//			{
//				payload[i - 2] = queued[i];
//			}
//			else if (i >= 514)
//			{
//				strCrc[i - 514] = queued[i];
//			}
//		}
//		std::cout << "]" << std::endl;
//
//		std::cout << "payload:\n[";
//		for (unsigned int i = 0; i < 512; ++i)
//		{
//			std::cout << payload[i];
//		}
//		std::cout << "]" << std::endl;
//
//		std::cout << "crc:\n[";
//		for (unsigned int i = 0; i < 4; ++i)
//		{
//			std::cout << strCrc[i];
//		}
//		std::cout << "]" << std::endl;
//		std::cout << "CRCs match?: " << FileUploader::ValidateCrc(payload, strCrc) << std::endl;
//	}
//
//	//unsigned int k = 0;
//	//while (k < 100)
//	//{
//	//	std::cout << "a";
//	//	k++;
//	//}
//
//	//char* framePayloadOnly = "123456789";
//	//
//	//CRC::Table<std::uint32_t, 32> table(CRC::CRC_32());
//	//std::uint32_t crc = CRC::Calculate(framePayloadOnly, 9, table);
//	//std::cout << std::hex << crc << std::endl;
//
//	while (!queue->empty())
//	{
//		char* queued = queue->front();
//		queue->pop();
//
//		//for (unsigned i = 0; i < 518; ++i)
//		//{
//		//	std::cout << queued[i];
//		//}
//	}
//	
//	return 0;
//}