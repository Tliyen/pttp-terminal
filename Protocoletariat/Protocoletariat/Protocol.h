#pragma once
#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <windows.h>

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
		bool RVI;

		char CHAR_STX = 0x02; // Start of Text Char
		char CHAR_EOT = 0x04; // End of Transmission Char
		char CHAR_ENQ = 0x05; // Enquiry Char


	private:


	};
}

#endif // !PROTOCOL_H_