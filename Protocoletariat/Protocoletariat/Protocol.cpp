/*----------------------------------------------------------------------
-- SOURCE FILE: Protocol.cpp
--
-- PROGRAM: Protocoleriat
--
-- FUNCTIONS:
--		void Idle();
--		void BidForLine(char ENQ);
--		void SendData();
--		void ConfirmTransmission();
--		void Retransmit();
--		void LinkReset();
--		void AcknowledgeBid();
--		void ReceiveData();
--		void ErrorDetection();
--
--
-- DATE: December 1, 2017
--
-- DESIGNER: Morgan Ariss
--
-- PROGRAMMER: Li-Yan Tong and Morgan Ariss
--
-- NOTES:
-- The program is responsible for sending and receving data between
-- devices.  It simulates bi-directional communication between
-- devices and manages the data traffic through a wireless connection.
--
-- The program actively checks if there is data being uploaded or
-- downloaded and will evenly prioritize data transfer by utilizing
-- event triggers.
--
-- When a file requires immediate transfer, the program will listen for
-- for a Reverse Interrupt (RVI) to take over the connection and
-- prioritize transfering this file.
----------------------------------------------------------------------*/

#include "Protocol.h"
#include "Main.h"
#include "FileDownloader.h"
#include "FileUploader.h"
#include "PrintData.h"

namespace protocoletariat
{

	/*----------------------------------------------------------------------
	-- FUNCTION: Idle
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss & Jeremy Lee
	--
	-- PROGRAMMER: Li-Yan Tong
	--
	-- INTERFACE: Idle()
	--
	-- RETURNS: void
	--
	-- NOTES:
	-- Entry point of this program. Its main role is to determine which
	-- device will take control of the connection to send or recieve data.
	--
	-- This method first checks if there are frames in a download queue, if
	-- it is empty then it allows for enquiries to take over the connection.
	--
	-- If there are frames in a download queue, an enquiry is then send to
	-- the device that has uploaded these frames.
	----------------------------------------------------------------------*/
	void Protocol::Idle()
	{
		// Switch on when engine starts
		while (protocolActive)
		{
			// Check Character in Serial Port
			if (SetCommMask(hComm, EV_RXCHAR))
			{
				// Continously Keep Reading if there is a byte in Serial Port
				while (bReading)
				{
					if (*downloadQ.front() == CHAR_ENQ)
					{
						downloadQ.pop(); // Popping a frame?
						AcknowledgeBid();
					}

					else if (EnqRequest)
					{
						BidForLine(CHAR_ENQ);
					}
				}
			}
		}
	}

	/*----------------------------------------------------------------------
	-- FUNCTION: BidForLine
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss & Jeremy Lee
	--
	-- PROGRAMMER: Li-Yan Tong
	--
	-- INTERFACE:  BidForLine(char ENQ)
	--
	-- RETURNS: void
	--
	-- NOTES:
	----------------------------------------------------------------------*/
	void Protocol::BidForLine(char ENQ)
	{
		// Start TOR
		dwRet = WaitForSingleObject(hwnd, TIMEOUT);

		if (CommEvent)
		{
			if (*downloadQ.front() == CHAR_ACK)
			{
				sendData = true;
				SendData();
			}
		}

		// Handle TOR
		switch (dwRet)
		{
		case WAIT_ABANDONED:
			wprintf(L"Mutex object was not released by the thread that\n"
				L"owned the mutex object before the owning thread terminates...\n");
			LinkReset();
			break;
		case WAIT_OBJECT_0:
			wprintf(L"The child thread state was signaled!\n");

			break;
		case WAIT_TIMEOUT:
			wprintf(L"Time-out interval elapsed, and the child thread's state is nonsignaled.\n");
			LinkReset();
			break;
		case WAIT_FAILED:
			wprintf(L"WaitForSingleObject() failed, error %u\n", GetLastError());
			LinkReset();
			ExitProcess(0);
		}
	}

	/*----------------------------------------------------------------------
	-- FUNCTION: SendData
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss & Jeremy Lee
	--
	-- PROGRAMMER: Li-Yan Tong
	--
	-- INTERFACE: SendData()
	--
	-- RETURNS: void
	--
	-- NOTES:
	----------------------------------------------------------------------*/
	void Protocol::SendData()
	{
		BYTE inbuff[512];
		DWORD nBytesRead, dwEvent, dwError;
		COMSTAT cs;

		while (sendData)
		{
			if (*uploadQ.front() == CHAR_RVI)
			{
				RVI = false;
				// Clear download buffer
				// EOT control frame
				LinkReset();
			}
			else if (*uploadQ.front() == CHAR_EOT)
			{
				// EOT control frame
				LinkReset();
			}
			else if (*uploadQ.front() == CHAR_STX)
			{
				/* read all available bytes */
				ClearCommError(hComm, &dwError, &cs);

				if ((dwEvent & EV_RXCHAR) && cs.cbInQue)
				{
					if (!ReadFile(hComm, inbuff, cs.cbInQue,
						&nBytesRead, NULL)) {


						for (int i = 0; i < 512; i++)
						{
							if (inbuff[i] != CHAR_ETX)
							{

							}
						}
						transferConfirmed = true;
						ConfirmTransmission();
						sendData = false;
						/* handle error */
						//locProcessCommError(GetLastError());
					}
				}
			}
		}
	}


	/*----------------------------------------------------------------------
	-- FUNCTION: ConfirmTransmission
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss & Jeremy Lee
	--
	-- PROGRAMMER: Li-Yan Tong
	--
	-- INTERFACE: ConfirmTransmission()
	--
	-- RETURNS: void
	--
	-- NOTES:
	----------------------------------------------------------------------*/
	void Protocol::ConfirmTransmission()
	{
		while (transferConfirmed)
		{
			// Download Queue front is ACK

		}
	}

	/*----------------------------------------------------------------------
	-- FUNCTION: Retransmit
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss & Jeremy Lee
	--
	-- PROGRAMMER: Li-Yan Tong
	--
	-- INTERFACE: Retransmit()
	--
	-- RETURNS: void
	--
	-- NOTES:
	----------------------------------------------------------------------*/
	void Protocol::Retransmit()
	{

	}

	/*----------------------------------------------------------------------
	-- FUNCTION: LinkReset
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss & Jeremy Lee
	--
	-- PROGRAMMER: Li-Yan Tong
	--
	-- INTERFACE: LinkReset()
	--
	-- RETURNS: void
	--
	-- NOTES:
	----------------------------------------------------------------------*/
	void Protocol::LinkReset()
	{

	}

	/*----------------------------------------------------------------------
	-- FUNCTION: AcknowledgeBid
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss & Jeremy Lee
	--
	-- PROGRAMMER: Li-Yan Tong
	--
	-- INTERFACE: AcknowledgeBid()
	--
	-- RETURNS: void
	--
	-- NOTES:
	----------------------------------------------------------------------*/
	void Protocol::AcknowledgeBid()
	{

	}

	/*----------------------------------------------------------------------
	-- FUNCTION: ReceiveData
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss & Jeremy Lee
	--
	-- PROGRAMMER: Li-Yan Tong
	--
	-- INTERFACE: ReceiveData()
	--
	-- RETURNS: void
	--
	-- NOTES:
	----------------------------------------------------------------------*/
	void Protocol::ReceiveData()
	{

	}

	/*----------------------------------------------------------------------
	-- FUNCTION: ErrorDetection
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss & Jeremy Lee
	--
	-- PROGRAMMER: Li-Yan Tong
	--
	-- INTERFACE: ErrorDetection()
	--
	-- RETURNS: void
	--
	-- NOTES:
	----------------------------------------------------------------------*/
	void Protocol::ErrorDetection()
	{

	}
}
