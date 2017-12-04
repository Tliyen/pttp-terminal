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
#include <stdio.h>

namespace protocoletariat
{
	static class Protocol
	{
	public:
		void Idle();

		// Transmit Data Methods
		void BidForLine(char ENQ);
		void SendData();
		void ConfirmTransmission();
		void Retransmit();
		void LinkReset();

		// Recieve Data Methods
		void AcknowledgeBid();
		void ReceiveData();
		void ErrorDetection();

		bool protocolActive;
		bool sendData;
		bool receiveData;
		bool RVI;

		DWORD dwRet;

		const char CHAR_STX = 0x02; // Start of Text Char
		const char CHAR_ETX = 0x03; // End of Text Char
		const char CHAR_EOT = 0x04; // End of Transmission Char
		const char CHAR_ENQ = 0x05; // Enquiry Char
		const char CHAR_ACK = 0x06; // Acknowledge Char

		const char CHAR_RVI = 0x07; // RVI originally windows bell

	private:


	};
}

#endif // !PROTOCOL_H_