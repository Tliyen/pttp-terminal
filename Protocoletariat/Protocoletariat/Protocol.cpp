/*----------------------------------------------------------------------
-- SOURCE FILE: Protocol.cpp
--
-- PROGRAM: Protocoleriat
--
-- FUNCTIONS:
--		void Idle();
--		void BidForLine();
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
		while (protocolActive)
		{
			if (CommEvent)
			{
				if (downloadQ.peek() == CHAR_ENQ)
				{
					downloadQ.pop(); // Popping a frame?
					AcknowledgeBid();
				}
			}
			else if (EnqRequest)
			{
				BidForLine(CHAR_ENQ);
			}
		}

	}


	void Protocol::BidForLine(char ENQ)
	{

	}

	void Protocol::SendData()
	{

	}

	void Protocol::ConfirmTransmission()
	{

	}

	void Protocol::Retransmit()
	{

	}

	void Protocol::LinkReset()
	{

	}

	void Protocol::AcknowledgeBid()
	{

	}

	void Protocol::ReceiveData()
	{

	}

	void Protocol::ErrorDetection()
	{

	}
}