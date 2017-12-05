#pragma once
#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define ASCII_STX 0x02
#define ASCII_ETX 0x03
#define ASCII_EOT 0x04
#define ASCII_ENQ 0x05
#define ASCII_ACK 0x06

#define TIMEOUT 2000

#include <windows.h>
#include <Winbase.h>
#include <queue>
#include <stdio.h>

namespace protocoletariat
{
	struct paramProtocolEngine
	{
		std::queue<char*>* uploadQueue;
		std::queue<char*>* downloadQueue;
		std::queue<char*>* printQueue;
		
		Struct* logfile;
		
		HANDLE* hComm;
		OVERLAPPED& olWrite = *(new OVERLAPPED());;
		DWORD& dwThreadExit = *(new DWORD());
	};

	extern bool globalRVI;
	extern bool protocolActive;
	
	static class ProtocolEngine
	{
	public:
		//ProtocolThread
		static DWORD WINAPI ProtocolThread(paramProtocolEngine* param);
	
		void Idle();

		// Transmit Data Methods
		void BidForLine();
		void SendData();
		void ConfirmTransmission();
		void Retransmit();
		void LinkReset();

		// Transmit Frames
		bool TransmitFrame(bool control, char type)
		
		// Recieve Data Methods
		void AcknowledgeBid();
		void ReceiveData();
		void ErrorDetection();
		
		bool sendData;
		bool receiveData;
		bool transferConfirmed;

	private:
		static std::queue<char*>* mUploadQueue;
		static std::queue<char*>* mDownloadQueue;
		static std::queue<char*>* mPrintQueue;
		
		static Struct* mLogfile;
		
		static HANDLE* mHandle;
		static OVERLAPPED& olWrite;
		static DWORD& dwThreadExit;
		
		static char ENQframe[];
		static char ACKframe[];
		static char EOTframe[];
		
		static char DATAframe[];
		
		static char* incFrame;
		static char* outFrame;
		
		DWORD dwRet;

		const char CHAR_STX = 0x02; // Start of Text Char
		// const char CHAR_ETX = 0x03; // End of Text Char
		const char CHAR_EOT = 0x04; // End of Transmission Char
		const char CHAR_ENQ = 0x05; // Enquiry Char
		const char CHAR_ACK = 0x06; // Acknowledge Char

		const char CHAR_RVI = 0x07; // RVI originally windows bell
	};
}

#endif // !PROTOCOL_H_