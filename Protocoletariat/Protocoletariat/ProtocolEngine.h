#pragma once
#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define ASCII_STX 0x02
#define ASCII_ETX 0x03
#define ASCII_EOT 0x04
#define ASCII_ENQ 0x05
#define ASCII_ACK 0x06

#define ASCII_RVI 0x07

#define ASCII_SYN 0x22

#define TIMEOUT 200
#define INNER_TIMEOUT 30

#include <windows.h>
#include <Winbase.h>
#include "global.h"
#include <queue>
#include <stdio.h>
#include "FileUploader.h"

namespace protocoletariat
{
	struct paramProtocolEngine
	{
		std::queue<char*>* uploadQueue;
		std::queue<char*>* downloadQueue;
		std::queue<char*>* printQueue;
		
		LogFile* logfile;

		bool* dlReady;
		bool* RVIflag;
		
		HANDLE* hComm;
		OVERLAPPED& olWrite = *(new OVERLAPPED());
		DWORD& dwThreadExit = *(new DWORD());
	};

	extern bool globalRVI;
	
	static class ProtocolEngine
	{
	public:
		//ProtocolThread
		static DWORD WINAPI ProtocolThread(paramProtocolEngine* param);
	
		static void Idle();

		// Transmit Data Methods
		static void BidForLine();
		static void SendData();
		static bool ConfirmTransmission();
		static bool Retransmit();
		static void LinkReset();

		// Transmit Frames
		static bool TransmitFrame(bool control, char type);
		
		// Recieve Data Methods
		static void AcknowledgeBid();
		static void ReceiveData();
		static bool ErrorDetection();

	private:
		static std::queue<char*>* mUploadQueue;
		static std::queue<char*>* mDownloadQueue;
		static std::queue<char*>* mPrintQueue;
		
		static LogFile* mLogfile;

		static bool* mDownloadReady;
		static bool* mRVIflag;

		static bool linkReceivedENQ;
		//static bool protocolActive;
		
		static HANDLE* mHandle;
		static OVERLAPPED& olWrite;
		static DWORD& dwThreadExit;

		static DWORD dwEvent;
		static DWORD dwError;
		
		static const char* ENQframe;
		static const char* ACKframe;
		static const char* EOTframe;
		
		static char DATAframe[];
		
		static char* incFrame;
		static char* outFrame;

		static paramProtocolEngine* ppe;
		
		DWORD dwRet;

		static const size_t DATA_FRAME_SIZE = 518;
		static const size_t CONTROL_FRAME_SIZE = 2;

		static const char CHAR_SYN = 22; // Start of Frame Char

		static const char CHAR_STX = 2; // Start of Text Char
		// static const char CHAR_ETX = 3; // End of Text Char
		static const char CHAR_EOT = 4; // End of Transmission Char
		static const char CHAR_ENQ = 5; // Enquiry Char
		static const char CHAR_ACK = 6; // Acknowledge Char

		static const char CHAR_RVI = 7; // RVI originally windows bell
	};
}

#endif // !PROTOCOL_H_