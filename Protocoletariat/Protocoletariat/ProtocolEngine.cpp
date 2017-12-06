/*----------------------------------------------------------------------
-- SOURCE FILE: ProtocolEngine.cpp
--
-- PROGRAM: Protocoleriat
--
-- FUNCTIONS:
--		void ProtocolThread();
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
-- PROGRAMMER: Morgan Ariss and Li-Yan Tong
--
-- NOTES:
-- The program is responsible for ensuring that communications stays
-- within the confines of the Protocol.  It simulates bi-directional
-- communication between two linked devices and manages the data traffic
-- through a wireless connection.
--
-- The program is triggered by events to actively check if there is data
-- set to be uploaded or downloaded and will evenly prioritize data transfer
-- by utilizing event triggers.  This program ensures that the system is only
-- sending or receiving when it is intended to be doing so.
--
-- When a file requires immediate transfer, the program will listen for
-- for a Reverse Interrupt (RVI) to take over the connection and
-- prioritize transfering this file.
----------------------------------------------------------------------*/
#include <iostream>
#include <fstream>
#include <string>

#include "ProtocolEngine.h"

namespace protocoletariat
{
	//Initialize variables from struct
	std::queue<char*>* ProtocolEngine::mUploadQueue = nullptr;
	std::queue<char*>* ProtocolEngine::mDownloadQueue = nullptr;
	std::queue<char*>* ProtocolEngine::mPrintQueue = nullptr;

	// const char* ProtocolEngine::ENQframe = CHAR_SYN + ASCII_ENQ;
	// const char* ProtocolEngine::ACKframe = { ASCII_SYN, ASCII_ACK };
	// const char* ProtocolEngine::EOTframe = { ASCII_SYN, ASCII_EOT };

	char* ProtocolEngine::incFrame;
	char* ProtocolEngine::outFrame;

	LogFile* ProtocolEngine::mLogfile;

	bool* ProtocolEngine::mDownloadReady;
	bool* ProtocolEngine::mRVIflag;

	bool ProtocolEngine::protocolActive;

	HANDLE* ProtocolEngine::mHandle = nullptr;
	// OVERLAPPED& ProtocolEngine::olWrite;
	// DWORD& ProtocolEngine::dwThreadExit;

	DWORD ProtocolEngine::dwEvent, ProtocolEngine::dwError;

	bool ProtocolEngine::linkReceivedENQ;

	paramProtocolEngine* ProtocolEngine::ppe;

	/*
	Protocol Structure (Protocol Thread)
	After the primary thread has initialized the necessary components for the program to function, it moves into the main protocol structure and handles the communication between the devices. It’s job is to ensure that the program is sending data to the serial port only when the paired device is ready to receive, and to ensure that if the other program wants to send data, that it is ready to receive that data.
	*/
	DWORD WINAPI ProtocolEngine::ProtocolThread(paramProtocolEngine* param)
	{
		mDownloadQueue = param->downloadQueue;
		mUploadQueue = param->uploadQueue;
		mPrintQueue = param->printQueue;

		mLogfile = param->logfile;

		mHandle = param->hComm;

		// olWrite = param->olWrite;
		// dwThreadExit = param->dwThreadExit;

		mDownloadReady = param->dlReady;
		mRVIflag = param->RVIflag;

		ppe = param;

		linkReceivedENQ = false;

		protocolActive = true;

		Idle();

		return 0;
	}

	/*
	TRANSMIT Data Side
	This side of the program is for when the system has data to send.
	*/

	/*----------------------------------------------------------------------
	-- FUNCTION: TransmitFrame
	--
	-- DATE: December 4, 2017
	--
	-- DESIGNER: Morgan Ariss
	--
	-- PROGRAMMER: Morgan Ariss
	--
	-- INTERFACE: TransmitFrame()
	--
	-- RETURNS: bool (success condition)
	--
	-- NOTES:
	-- This function is called upon by the other ProtocolEngine functions to
	-- transmit the necessary data through the serial port.
	--
	-- It will send either control frames to signal action from the partnered
	-- device, or data frames when the other device is ready to receive them.
	----------------------------------------------------------------------*/

	bool ProtocolEngine::TransmitFrame(bool control, char type)
	{
		OVERLAPPED& olWrite = ppe->olWrite;
		DWORD& dwThreadExit = ppe->dwThreadExit;

		DWORD  dNoOfBytesWritten; // No of bytes written to the port
		DWORD  dNoOFBytestoWrite; // No of bytes to write into the port

		char* lpBuffer;

		bool status;

		olWrite.hEvent = CreateEvent(NULL, true, false, NULL);
		if (olWrite.hEvent == NULL)
		{
			// Event could not be created
			return false;
		}


		if (control)
		{
			lpBuffer = new char[CONTROL_FRAME_SIZE];
			lpBuffer[0] = CHAR_SYN;
			switch (type)
			{
			case ASCII_ENQ:
				lpBuffer[1] = CHAR_ENQ;
				break;
			case ASCII_ACK:
				lpBuffer[1] = CHAR_ACK;
				break;
			case ASCII_EOT:
				lpBuffer[1] = CHAR_EOT;
				break;
			default:
				//should not get here
				break;
			}
		}
		else
		{
			lpBuffer = new char[DATA_FRAME_SIZE];
			lpBuffer = outFrame;
		}

		dNoOfBytesWritten = 0;
		dNoOFBytestoWrite = sizeof(lpBuffer);  // Calculating the no of bytes to write into the port

		status = WriteFile(mHandle,             // Handle to the Serialport
			lpBuffer,            // Data to be written to the port 
			dNoOFBytestoWrite,   // No of bytes to write into the port
			&dNoOfBytesWritten,  // No of bytes written to the port
			NULL);

		delete lpBuffer;
		return status;
	}

	/*----------------------------------------------------------------------
	-- FUNCTION: Idle
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss
	--
	-- PROGRAMMER: Morgan Ariss
	--
	-- INTERFACE: Idle()
	--
	-- RETURNS: void
	--
	-- NOTES:
	-- This is the default state of the main protocol engine, it is the first
	-- function called by the ProtocolEngine thread, and it has only two
	-- directions:
	-- If there are no frames inside the program’s output queue it simply waits
	-- for an ENQ from the paired system; otherwise, if there are frames inside
	-- the output queue waiting to be sent, the device will transmit an ENQ to
	-- the paired device (hopefully) also in the idle state.
	----------------------------------------------------------------------*/
	void ProtocolEngine::Idle()
	{
		// Loop
		while (protocolActive)
		{
			if (*mRVIflag)
			{
				*mRVIflag = false;
				BidForLine();
			}

			if (linkReceivedENQ)
			{
				linkReceivedENQ = false;
				AcknowledgeBid();
			}

			// A signal has been received
			if (WaitCommEvent(mHandle, &dwEvent, NULL))
			{
				Sleep(20);
				if (*mDownloadReady)
				{
					// read the front frame from the downloadQueue into frame
					incFrame = mDownloadQueue->front();

					// Check if the front of the queue an ENQ
					if (incFrame[1] == CHAR_ENQ)
					{
						incFrame = nullptr;
						AcknowledgeBid();
						*mDownloadReady = false;
						break;
					}
				}
			}

			// If this device wants to take the handle
			if (*mUploadQueue->front() != NULL)
			{
				// Transmit ENQ
				TransmitFrame(true, ASCII_ENQ);

				// Move to BidForLine
				BidForLine();
			}
		}
	}

	/*
	BidForLine
	An ENQ has been TRANSMITTED
	
	*/
	
	/*----------------------------------------------------------------------
	-- FUNCTION: BidForLine
	--
	-- DATE: December 4, 2017
	--
	-- DESIGNER: Morgan Ariss
	--
	-- PROGRAMMER: Morgan Ariss
	--
	-- INTERFACE: BidForLine()
	--
	-- RETURNS: void
	--
	-- NOTES:
	-- The program must wait to RECEIVE an ACK from the pair device to ensure 
	-- that it is ready to receive data. If it does, it can proceed down the 
	-- TRANSMIT tree. If the timeout expires before that time, the device must 
	-- move to the link reset delay state.
	----------------------------------------------------------------------*/
	void ProtocolEngine::BidForLine()
	{
		// Start TOS
		int timer = 0;

		// Loop while timeout has not exceeded
		while (timer < TIMEOUT)
		{
			// Check for a CommEventTrigger
			if (WaitCommEvent(mHandle, &dwEvent, NULL))
			{
				int innerTimer = 0;
				while (timer < TIMEOUT && innerTimer < INNER_TIMEOUT)
				{
					Sleep(10);
					if (*mDownloadReady)
					{
						// read the front frame from the downloadQueue into frame
						incFrame = mDownloadQueue->front();

						// Check if the front of the queue an ACK
						if (incFrame[1] == CHAR_ACK)
						{
							incFrame = nullptr;
							// Remove the ACK
							mDownloadQueue->pop();

							// Move to SendData()
							SendData();
							return;
						}
					}
					timer++;
					innerTimer++;
				}
				innerTimer = 0;
			}
			Sleep(10);
			timer++;
		}
		LinkReset();
		return;
	}

	/*
	SendData
	An ACK has been RECEIVED in the BidForLine state OR returning from ConfirmTransmission OR a Retransmit has been acknowledged
	The program’s bid for the line has been acknowledged and the other system is moving into a RECEIVING state. The program will start a loop to wait for the CommEvent triggered by the upload buffer. The program will send the frame,created in the File Upload Thread, at the front of it’s queue. It will then move to the ConfirmTransmission state on a Data Frame transmission. If the EOT Control Frame is submitted or the system has sent 10 frames, it will instead proceed to the LinkReset State.
	*/
	void ProtocolEngine::SendData()
	{
		// Start TOS
		int timer = 0;
		int dfs = 0;

		do
		{
			// Loop while the timeout has no exceeded
			while (timer < TIMEOUT)
			{
				// If CommEvent Triggered
				if (WaitCommEvent(mHandle, &dwEvent, NULL))
				{
					int innerTimer = 0;
					while (timer < TIMEOUT && innerTimer < INNER_TIMEOUT)
					{
						Sleep(10);
						if (*mDownloadReady)
						{
							// read the front frame from the downloadQueue into incFrame
							incFrame = mDownloadQueue->front();

							// read the front of the upload queue into the outFrame
							outFrame = mUploadQueue->front();

							// If front of download queue is RVI
							if (incFrame[1] == CHAR_RVI)
							{
								incFrame = nullptr;
								// Set global RVI variable to false
								globalRVI = true;
								// Clear download buffer
								mDownloadQueue->empty();
								// Transmit EOT control frame through serial port
								TransmitFrame(true, ASCII_EOT);
								// Move to LinkReset
								LinkReset();
								return;
							}
							// If front of upload queue is EOT
							else if (outFrame[1] == CHAR_EOT)
							{
								// Transmit EOT control frame through Serial Port
								TransmitFrame(true, ASCII_EOT);

								delete outFrame;
								mUploadQueue->pop();
								// Move to LinkReset
								LinkReset();
								return;
							}
							// If the frame at the front of the upload queue is a data frame
							else if (outFrame[1] == CHAR_STX)
							{
								// Transmit the data frame through the serial port
								TransmitFrame(false, NULL);

								// Move to ConfirmTransmission
								if (!ConfirmTransmission())
								{
									// Transmit EOT control frame through Serial Port
									TransmitFrame(true, ASCII_EOT);
									// Move to LinkReset
									LinkReset();
									return;
								}
								timer = 0;
								dfs++;
								break;
							}
						}
						innerTimer++;
						timer++;
					}
					innerTimer = 0;
				}
				Sleep(10);
				timer++;
			} // TOS expires
		} while (dfs < 10); // 10 Frames confirmed
		return;
	}

	/*
	ConfirmTransmission
	A Data Frame has been TRANSMITTED
	The program must wait for an ACK control frames from the paired device to continue with its transmission. If the program RECEIVES the ACK Control Frame it can pop the TRANSMITTED Data Frame from the queue and return to the SendData state. If the timeout expires, then the program will instead move to the Retransmit state.
	*/
	bool ProtocolEngine::ConfirmTransmission()
	{
		// Start TOR
		int timer = 0;

		//initialize success
		bool success = false;

		// Loop while timeout has not expired
		while (timer < TIMEOUT && success == false)
		{
			// If CommEvent Triggered
			if (WaitCommEvent(mHandle, &dwEvent, NULL))
			{
				int innerTimer = 0;
				while (timer < 200 && innerTimer < 30)
				{
					if (*mDownloadReady)
					{
						// read the front frame from the downloadQueue into frame
						incFrame = mDownloadQueue->front();

						// If download queue front is ACK
						if (incFrame[1] == CHAR_ACK)
						{
							// Pop front of download buffer
							mDownloadQueue->pop();
							delete incFrame;
							// Pop front of upload buffer
							mUploadQueue->pop();
							// Increment logfile successful frames variable
							mLogfile->sent_packet++;
							// Move back to SendData
							return true;
						}
					}
					Sleep(10);
					timer++;
					innerTimer++;
				}
				innerTimer = 0;
			}
			Sleep(10);
			timer++;
		} // TOR expires

		if (success == false)
		{
			// Move to retransmit
			success = Retransmit();
		}

		//return success
		return success;
	}

	/*
	Retransmit
	A Data Frame has failed to TRANSMIT
	The program will attempt three retransmissions of the same frame, and wait to RECEIVE the ACK. If all three transmissions fail, then the program will move to the LinkDelay state. If however, one of the transmission is responded to by an ACK then the program will pop the front frame from the output queue and return to the SendData state.
	*/
	bool ProtocolEngine::Retransmit()
	{
		// Initialize txCounter variable to 1
		int txCounter = 1;

		// Start TOR
		int timer = 0;

		// Retransmit up to 3 times
		do
		{
			TransmitFrame(false, NULL);
			// Loop while timer has not expired
			while (timer < TIMEOUT)
			{
				// If CommEvent Triggered
				if (WaitCommEvent(mHandle, &dwEvent, NULL))
				{
					int innerTimer = 0;
					while (timer < TIMEOUT && innerTimer < INNER_TIMEOUT)
					{
						if (*mDownloadReady)
						{
							// read the front frame from the downloadQueue into frame
							incFrame = mDownloadQueue->front();

							// If download queue front is ACK
							if (incFrame[1] == CHAR_ACK)
							{
								incFrame = nullptr;
								// Pop front of download buffer
								mDownloadQueue->pop();
								// Pop front of upload buffer
								mUploadQueue->pop();
								// Increment logfile successful frames variable
								mLogfile->sent_packet++;
								// Move back to SendData
								return true;
							}
						}
					}
				}
				Sleep(10);
				timer++;
			} // TOR expires
			txCounter++;
		} while (txCounter < 3);

		//return
		return false;
	}

	/*
	LinkReset
	Any State on the TRANSMIT side has experienced a timeout expiry, or failed
	This state exists as a buffer for the post-TRANSMIT states, to stop one device from hogging the line. This state begins a TOR listens to RECEIVE an ENQ from the pair device. If the TOR expires before an ENQ is RECEIVED, the program move to the idle state.
	*/
	void ProtocolEngine::LinkReset()
	{
		// Start TOR
		int timer = 0;

		// Loop while timeout has not expired
		while (timer < TIMEOUT)
		{
			// If CommEvent Triggered
			if (WaitCommEvent(mHandle, &dwEvent, NULL))
			{
				int innerTimer = 0;
				while (timer < TIMEOUT && innerTimer < INNER_TIMEOUT)
				{
					if (*mDownloadReady)
					{
						// read the front frame from the downloadQueue into frame
						incFrame = mDownloadQueue->front();

						// Check if the front of the queue an ENQ
						if (incFrame[1] == CHAR_ENQ)
						{
							linkReceivedENQ = true;
							// Pop download queue front
							mDownloadQueue->pop();
							delete incFrame;
							incFrame = nullptr;
							// Return to Idle
							return;
						}
						else
						{
							return;
						}
					}
				}
			}
			Sleep(10);
			timer++;
		}
	}

	/*
	RECEIVE Data Side
	This side of the protocol handles the receiving of data from the paired device.
	*/

	/*
	AcknowledgeBid
	An ENQ has been RECEIVED
	This state is necessary to acknowledge the other device’s bid for the line.
	*/
	void ProtocolEngine::AcknowledgeBid()
	{
		// Transmit ACK control frame
		TransmitFrame(true, ASCII_ACK);
		// Move to ReceiveData
		ReceiveData();
	}

	/*
	ReceiveData
	An ACK has been TRANSMITTED in response to the RECEIVED ENQ
	This state will prepare the system to RECEIVE Data Frames from the serial port. This state will wait for a Data Frame from the serial port, until the timeout expires. This timeout is required to be 3 times longer than the corresponding wait on the TRANSMIT Data Side since it must account for 3 failed transmissions.
	*/
	void ProtocolEngine::ReceiveData()
	{
		// Start TOR
		int timer = 0;

		// Initialize error detection success
		bool errorDetection = false;

		// Initialize recieved/failed frames counter
		int RxCounter = 0;
		int FailedFrames = 0;

		// Loop until 3 consecutive failures or 10 successful frames
		do
		{
			// Loop while timeout not exceeded
			while (timer < TIMEOUT)
			{
				// If RVIevent triggered
				if (*mRVIflag)
				{
					// Move to Idle
					return;
				}
				// If CommEvent is triggered
				if (WaitCommEvent(mHandle, &dwEvent, NULL))
				{
					int innerTimer = 0;
					while (timer < TIMEOUT && innerTimer < INNER_TIMEOUT)
					{
						Sleep(10);
						if (*mDownloadReady)
						{
							// read the front frame from the downloadQueue into frame
							incFrame = mDownloadQueue->front();

							// Check if the front of the queue an EOT frame
							if (incFrame[1] == CHAR_EOT)
							{
								// Remove the EOT frame
								mDownloadQueue->pop();
								// Return to idle
								return;
							}
							// Else if download queue front is STX
							if (incFrame[1] = CHAR_STX)
							{
								// Remove STX char
								// mDownloadQueue->pop();

								// Send the frame for error detection
								if (ErrorDetection())
								{
									// Increment the frames received counter
									RxCounter++;
									continue;
								}
								else
								{
									// Increment the failed frames counter
									mLogfile->lost_packet++;
									// Reset the timer
									timer = 0;
									continue;
								}
							}
						}
						innerTimer++;
						timer++;
					}
					innerTimer = 0;
				}
				// Increment the timer
				Sleep(10);
				timer++;
			}
		} while (RxCounter < 10 && FailedFrames < 3); // 10 Frames have been successfully received or 3 concurrent fails have occurred
	}

	/*
	ErrorDetection
	A Data Frame has been RECEIVED
	This state will handle the detection of errors using the CRC at the end of the of the Data Frame. All remaining bytes will be read from the download queue; the first 512 will be the data bytes, the next 4 will be the CRC bytes. If no error is detected the system will TRANSMIT an ACK, send the received data to be printed, and return to the Receive state. Otherwise the system will timeout and return to the Receive state without TRANSMITTING an ACK. This timeout must be very short.
	*/
	bool ProtocolEngine::ErrorDetection()
	{
		// Start TOS
		int timer = 0;

		// Initialize frame and CRC holder
		char frame[512];
		char CRC[4];

		bool errorDetected = false;

		while (timer < TIMEOUT)
		{
			// Get the 512 data characters from the download queue
			for (int i = 1; i <= 513; i++)
			{
				// Read all chars from the front of the queue and remove it
				frame[i - 1] = incFrame[i];

			}

			for (int i = 514; (i - 514) < 4; i++)
			{
				// Read the remaining chars from the front of the queue as CRC
				CRC[i - 514] = incFrame[i];
			}
			delete incFrame;
			mDownloadQueue->pop();

			// Implement CRC error detection -- use available source code
			errorDetected = FileUploader::ValidateCrc(frame, CRC);

			// If an error is detected
			if (errorDetected)
			{
				// Increment logfile corrupt frame counter
				mLogfile->received_corrupted_packet++;
				// Move back to ReceiveData
				return false;
			}
			else
			{
				// Send data to print
				mPrintQueue->push(frame);
				// Increment the logfile successful frames counter
				mLogfile->sent_packet++;
				// Transmit ACK control frame
				TransmitFrame(true, ASCII_ACK);
				// Move back to ReceiveData
				return true;
			}
			Sleep(20);
			timer++;
		} // TOS Ends
		// Move back to ReceiveData
		return false;
	}
}