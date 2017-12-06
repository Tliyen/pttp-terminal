/*----------------------------------------------------------------------
-- SOURCE FILE: ProtocolEngine.cpp
--
-- PROGRAM: Protocoletariat
--
-- FUNCTIONS:
--		DWORD WINAPI ProtocolThread();
--		bool TransmitFrame();
--		void Idle();
--		void BidForLine();
--		void SendData();
--		bool ConfirmTransmission();
--		bool Retransmit();
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

	char* ProtocolEngine::incFrame;
	char* ProtocolEngine::outFrame;

	LogFile* ProtocolEngine::mLogfile;

	bool* ProtocolEngine::mDownloadReady;
	bool* ProtocolEngine::mRVIflag;

	//bool ProtocolEngine::protocolActive;

	HANDLE* ProtocolEngine::mHandle = nullptr;

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
	-- RETURNS: bool	- true if frame is successfully transmitted to the
	--					  serial port; false otherwise.
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

		OVERLAPPED osWrite = { 0 };

		DWORD  dNoOfBytesWritten; // No of bytes written to the port
		DWORD  dNoOFBytestoWrite; // No of bytes to write into the port

		DWORD dwRes;
		DWORD dwWritten;

		char* lpBuffer;

		bool status;

		osWrite.hEvent = CreateEvent(NULL, true, false, NULL);
		if (osWrite.hEvent == NULL)
		{
			// Event could not be created
			return false;
		}

		if (control)
		{
			lpBuffer = new char[CONTROL_FRAME_SIZE];
			lpBuffer[0] = CHAR_SYN;
			dNoOFBytestoWrite = 2;  // Calculating the no of bytes to write into the port
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
			for (size_t i = 0; i < 518; i++)
			{
				lpBuffer[i] = outFrame[i];
			}
			dNoOFBytestoWrite = 518;  // Calculating the no of bytes to write into the port
		}

		dNoOFBytestoWrite = sizeof(lpBuffer);  // Calculating the no of bytes to write into the port

		status = WriteFile(*mHandle,            // Handle to the Serialport
			lpBuffer,							// Data to be written to the port 
			dNoOFBytestoWrite,					// No of bytes to write into the port
			&dwRes,								// No of bytes written to the port
			&osWrite);

		delete lpBuffer;
		return status;
	}

	/*----------------------------------------------------------------------
	-- FUNCTION: Idle
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
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
			//if (WaitCommEvent(*mHandle, &dwEvent, NULL))
			//{
			Sleep(20);
			if (*mDownloadReady)
			{
				// read the front frame from the downloadQueue into frame
				if (!mDownloadQueue->empty())
				{
					incFrame = mDownloadQueue->front();
					// Check if the front of the queue an ENQ
					if (incFrame[1] == CHAR_ENQ)
					{
						delete incFrame;
						incFrame = nullptr;
						if (!mDownloadQueue->empty())
							mDownloadQueue->pop();
						*mDownloadReady = false;
						AcknowledgeBid();
						break;
					}
				}
			}
			//}

			// If this device wants to take the handle
			if (!(mUploadQueue->empty()))
			{
				// Transmit ENQ
				std::cout << "Sending ENQ" << std::endl;
				if (TransmitFrame(true, ASCII_ENQ))
				{
					std::cout << "Sent ENQ" << std::endl;
					mLogfile->sent_packet++;
				}
				else
				{
					std::cout << "Failed Sending ENQ" << std::endl;
				}

				// Move to BidForLine
				BidForLine();
			}
		}
	}

	/*----------------------------------------------------------------------
	-- FUNCTION: BidForLine
	--
	-- DATE: December 4, 2017
	--
	-- DESIGNER: Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
	--
	-- PROGRAMMER: Morgan Ariss
	--
	-- INTERFACE: void BidForLine()
	--
	-- RETURNS: void
	--
	-- NOTES:
	-- This function will be called when an ENQ has been TRANSMITTED.
	-- The program must wait to RECEIVE an ACK from the paired device to ensure
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
			//if (WaitCommEvent(mHandle, &dwEvent, NULL))
			//{
			int innerTimer = 0;
			while (timer < TIMEOUT && innerTimer < INNER_TIMEOUT)
			{
				Sleep(10);
				if (*mDownloadReady)
				{
					// read the front frame from the downloadQueue into frame
					if (!mDownloadQueue->empty())
					{
						incFrame = mDownloadQueue->front();

						// Check if the front of the queue an ACK
						if (incFrame[1] == CHAR_ACK)
						{
							// Remove the ACK

							delete incFrame;
							incFrame = nullptr;
							if (!mDownloadQueue->empty())
								mDownloadQueue->pop();

							// Move to SendData()
							SendData();
							return;
						}
					}
				}
				timer++;
				innerTimer++;
			}
			innerTimer = 0;
			//}
			Sleep(10);
			timer++;
		}
		LinkReset();
		return;
	}

	/*----------------------------------------------------------------------
	-- FUNCTION: SendData
	--
	-- DATE: December 4, 2017
	--
	-- DESIGNER: Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
	--
	-- PROGRAMMER: Morgan Ariss
	--
	-- INTERFACE: void SendData()
	--
	-- RETURNS: void
	--
	-- NOTES:
	-- This method will be called if ACK has been RECEIVED in the BidForLine
	-- state OR returning from ConfirmTransmission OR a Retransmit has been
	-- acknowledged.
	-- The program’s bid for the line has been acknowledged and the other
	-- system is moving into a RECEIVING state. The program will start a
	-- loop to wait for the CommEvent triggered by the upload buffer.
	-- The program will send the frame,created in the File Upload Thread,
	-- at the front of it’s queue. It will then move to the ConfirmTransmission
	-- state on a Data Frame transmission. If the EOT Control Frame is submitted
	-- or the system has sent 10 frames, it will instead proceed to the LinkReset
	-- State.
	----------------------------------------------------------------------*/
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
				// read the front of the upload queue into the outFrame
				if (!mUploadQueue->empty())
				{
					outFrame = mUploadQueue->front();

					// If CommEvent Triggered
					//if (WaitCommEvent(mHandle, &dwEvent, NULL))
					//{
					int innerTimer = 0;
					while (timer < TIMEOUT && innerTimer < INNER_TIMEOUT)
					{
						Sleep(10);
						if (*mDownloadReady)
						{
							// read the front frame from the downloadQueue into incFrame
							if (!mDownloadQueue->empty())
							{
								incFrame = mDownloadQueue->front();

								// If front of download queue is RVI
								if (incFrame[1] == CHAR_RVI)
								{
									delete incFrame;
									incFrame = nullptr;
									// Set global RVI variable to false
									globalRVI = true;
									// Clear download buffer
									while (!mDownloadQueue->empty())
									{
										mDownloadQueue->pop();
									}
									// Transmit EOT control frame through serial port
									std::cout << "Sending EOT" << std::endl;
									if (TransmitFrame(true, ASCII_EOT))
									{
										std::cout << "Sent EOT" << std::endl;
										mLogfile->sent_packet++;
									}
									else
									{
										std::cout << "Failed Sending EOT" << std::endl;
									}
									// Move to LinkReset
									LinkReset();
									return;
								}
							}
						}
						innerTimer++;
						timer++;
						//}
						innerTimer = 0;
					}
					// If front of upload queue is EOT
					if (outFrame[1] == CHAR_EOT)
					{
						// Transmit EOT control frame through Serial Port
						std::cout << "Sending EOT" << std::endl;
						if (TransmitFrame(true, ASCII_EOT))
						{
							std::cout << "Sent EOT" << std::endl;
							mLogfile->sent_packet++;
						}
						else
						{
							std::cout << "Failed Sending EOT" << std::endl;
						}
						delete outFrame;
						outFrame = nullptr;
						if (!mUploadQueue->empty())
							mUploadQueue->pop();
						// Move to LinkReset
						LinkReset();
						return;
					}
					// If the frame at the front of the upload queue is a data frame
					else if (outFrame[1] == CHAR_STX)
					{
						// Transmit the data frame through the serial port
						if (TransmitFrame(false, NULL))
						{
							mLogfile->sent_packet++;
						}
						// Move to ConfirmTransmission
						if (!ConfirmTransmission())
						{
							// Transmit EOT control frame through Serial Port
							std::cout << "Sending EOT" << std::endl;
							if (TransmitFrame(true, ASCII_EOT))
							{
								std::cout << "Sent EOT" << std::endl;
								mLogfile->sent_packet++;
							}
							else
							{
								std::cout << "Failed Sending EOT" << std::endl;
							}
							// Move to LinkReset
							LinkReset();
							return;
						}
						timer = 0;
						dfs++;
						break;
					}
				}
				Sleep(10);
				timer++;
			} // TOS expires
		} while (dfs < 10); // 10 Frames confirmed
		return;
	}

	/*----------------------------------------------------------------------
	-- FUNCTION: ConfirmTransmission
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
	--
	-- PROGRAMMER: Morgan Ariss
	--
	-- INTERFACE: bool ConfirmTransmission()
	--
	-- RETURNS: bool	- true if an ACK gets back, indicating that the
	--					  receiver receives the frame successfully; false
	--					  if retransmission fails 3 times.
	--
	-- NOTES:
	-- This function will be called when a Data Frame has been TRANSMITTED.
	-- The program must wait for an ACK control frames from the paired device
	-- to continue with its transmission. If the program RECEIVES the ACK
	-- Control Frame it can pop the TRANSMITTED Data Frame from the queue
	-- and return to the SendData state. If the timeout expires, then the
	-- program will instead move to the Retransmit state.
	----------------------------------------------------------------------*/
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
			//if (WaitCommEvent(mHandle, &dwEvent, NULL))
			//{
			int innerTimer = 0;
			while (timer < TIMEOUT && innerTimer < INNER_TIMEOUT)
			{
				if (*mDownloadReady)
				{
					// read the front frame from the downloadQueue into frame
					if (!mDownloadQueue->empty())
					{
						incFrame = mDownloadQueue->front();

						// If download queue front is ACK
						if (incFrame[1] == CHAR_ACK)
						{
							// Pop front of download buffer
							delete incFrame;
							incFrame = nullptr;
							if (!mDownloadQueue->empty())
								mDownloadQueue->pop();
							// Pop front of upload buffer
							delete outFrame;
							outFrame = nullptr;
							if (!mUploadQueue->empty())
								mUploadQueue->pop();
							// Increment logfile successful frames variable
							mLogfile->sent_packet++;
							// Move back to SendData
							return true;
						}
					}
				}
				Sleep(10);
				timer++;
				innerTimer++;
				//}
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

	/*----------------------------------------------------------------------
	-- FUNCTION: Retransmit
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
	--
	-- PROGRAMMER: Morgan Ariss
	--
	-- INTERFACE: bool Retransmit()
	--
	-- RETURNS: bool	- true if the retransmission is successful; false
	--					  if it fails 3 times.
	--
	-- NOTES:
	-- This function will be called when a Data Frame has been TRANSMITTED
	-- but the system has timed out before an ACK has been recieved.
	-- The program will attempt three retransmissions of the same frame,
	-- and wait to RECEIVE the ACK. If all three transmissions fail, then
	-- the program will move to the LinkDelay state. If however, one of the
	-- transmission is responded to by an ACK then the program will pop the
	-- front frame from the output queue and return to the SendData state.
	----------------------------------------------------------------------*/
	bool ProtocolEngine::Retransmit()
	{
		// Initialize txCounter variable to 1
		int txCounter = 1;

		// Start TOR
		int timer = 0;

		// Retransmit up to 3 times
		do
		{
			std::cout << "Sending DATA Frame" << std::endl;
			if (TransmitFrame(false, NULL))
			{
				std::cout << "Sent DATA Frame" << std::endl;
			}
			else
			{
				std::cout << "Failed Sending DATA Frame" << std::endl;
			}
			
			// Loop while timer has not expired
			while (timer < TIMEOUT)
			{
				// If CommEvent Triggered
				//if (WaitCommEvent(mHandle, &dwEvent, NULL))
				//{
				int innerTimer = 0;
				while (timer < TIMEOUT && innerTimer < INNER_TIMEOUT)
				{
					if (*mDownloadReady)
					{
						// read the front frame from the downloadQueue into frame
						if (!mDownloadQueue->empty())
						{
							incFrame = mDownloadQueue->front();

							// If download queue front is ACK
							if (incFrame[1] == CHAR_ACK)
							{
								incFrame = nullptr;
								delete incFrame;
								// Pop front of download buffer
								if (!mDownloadQueue->empty())
									mDownloadQueue->pop();
								// Pop front of upload buffer
								outFrame = nullptr;
								delete outFrame;
								if (!mUploadQueue->empty())
									mUploadQueue->pop();
								// Increment logfile successful frames variable
								mLogfile->sent_packet++;
								// Move back to SendData
								return true;
							}
						}
					}
				}
				//}
				Sleep(10);
				timer++;
			} // TOR expires
			txCounter++;
		} while (txCounter < 3);

		//return
		return false;
	}

	/*----------------------------------------------------------------------
	-- FUNCTION: LinkReset
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
	--
	-- PROGRAMMER: Morgan Ariss
	--
	-- INTERFACE: void LinkReset()
	--
	-- RETURNS: void
	--
	-- NOTES:
	-- This function will be called when any previous state on the TRANSMIT
	-- side has experienced a timeout expiry, or failed.
	-- This state exists as a buffer for the post-TRANSMIT states, to stop one
	-- device from hogging the line. This state begins a TOR, and listens to
	-- RECEIVE an ENQ from the pair device. If the TOR expires before an ENQ
	-- is RECEIVED, the program move to the idle state.
	----------------------------------------------------------------------*/
	void ProtocolEngine::LinkReset()
	{
		// Start TOR
		int timer = 0;

		// Loop while timeout has not expired
		while (timer < TIMEOUT)
		{
			// If CommEvent Triggered
			//if (WaitCommEvent(mHandle, &dwEvent, NULL))
			//{
			int innerTimer = 0;
			while (timer < TIMEOUT && innerTimer < INNER_TIMEOUT)
			{
				if (*mDownloadReady)
				{
					// read the front frame from the downloadQueue into frame
					if (!mDownloadQueue->empty())
					{
						incFrame = mDownloadQueue->front();

						// Check if the front of the queue an ENQ
						if (incFrame[1] == CHAR_ENQ)
						{
							linkReceivedENQ = true;
							// Pop download queue front
							if (!mDownloadQueue->empty())
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
			//}
			Sleep(10);
			timer++;
		}
	}

	/*----------------------------------------------------------------------
	-- FUNCTION: AcknowledgeBid
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
	--
	-- PROGRAMMER: Morgan Ariss
	--
	-- INTERFACE: void AcknowledgeBid()
	--
	-- RETURNS: void
	--
	-- NOTES:
	-- This function will be called when an ENQ has been RECEIVED in the
	-- Idle state.
	-- This is the first state of the Protocol on the Receive side; this state
	-- is necessary to acknowledge the other device’s bid for the line. It's
	-- only responsibility is to send an ACK control frame to the paired device.
	----------------------------------------------------------------------*/
	void ProtocolEngine::AcknowledgeBid()
	{
		while (!mDownloadQueue->empty())
		{
			mDownloadQueue->pop();
		}

		// Transmit ACK control frame
		std::cout << "Sending ACK" << std::endl;
		if (TransmitFrame(true, ASCII_ACK))
		{
			std::cout << "Sent ACK" << std::endl;
			mLogfile->sent_packet++;
		}
		else
		{
			std::cout << "Failed Sending ACK" << std::endl;
		}
		// Move to ReceiveData
		ReceiveData();
	}

	/*----------------------------------------------------------------------
	-- FUNCTION: ReceiveData
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
	--
	-- PROGRAMMER: Morgan Ariss
	--
	-- INTERFACE: void ReceiveData()
	--
	-- RETURNS: void
	--
	-- NOTES:
	-- This function will be called when An ACK has been TRANSMITTED from the
	-- AcknowledgeBid state in response to the RECEIVED ENQ.
	-- This state will prepare the system to RECEIVE Data Frames from the serial
	-- port. This state will wait for a Data Frame from the serial port, until
	-- the timeout expires. This timeout is required to be 3 times longer than
	-- the corresponding wait on the TRANSMIT Data Side since it must account
	-- for 3 failed transmissions.
	----------------------------------------------------------------------*/
	void ProtocolEngine::ReceiveData()
	{
		// Start TOR
		int timer;

		// Initialize error detection success
		bool errorDetection = false;

		// Initialize recieved/failed frames counter
		int RxCounter = 0;
		int FailedFrames = 0;

		// Loop until 3 consecutive failures or 10 successful frames
		do
		{
			timer = 0;
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
				//if (WaitCommEvent(*mHandle, &dwEvent, NULL))
				//{
				int innerTimer = 0;
				while (timer < TIMEOUT && innerTimer < INNER_TIMEOUT)
				{
					Sleep(10);
					if (*mDownloadReady)
					{
						// read the front frame from the downloadQueue into frame
						if (!mDownloadQueue->empty())
						{
							incFrame = mDownloadQueue->front();

							// Check if the front of the queue an EOT frame
							if (incFrame[1] == CHAR_EOT)
							{
								// Remove the EOT frame

								delete incFrame;
								incFrame = nullptr;
								if (!mDownloadQueue->empty())
									mDownloadQueue->pop();
								// Return to idle
								return;
							}
							// Else if download queue front is STX
							if (incFrame[1] = CHAR_STX)
							{
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
					}
					innerTimer++;
					timer++;
				}
				innerTimer = 0;
				//}
				// Increment the timer
				Sleep(10);
				timer++;
			}
		} while (RxCounter < 10 && FailedFrames < 3); // 10 Frames have been successfully received or 3 concurrent fails have occurred
	}

	/*
	ErrorDetection
	*/

	/*----------------------------------------------------------------------
	-- FUNCTION: ErrorDetection
	--
	-- DATE: December 2, 2017
	--
	-- DESIGNER: Morgan Ariss, Jeremy Lee, Luke Lee, Li-Yan Tong
	--
	-- PROGRAMMER: Morgan Ariss & Jeremy Lee
	--
	-- INTERFACE: bool ErrorDetection()
	--
	-- RETURNS: bool	- true if no error is detected; false if an error is
	--					  detected or reaches timeout.
	--
	-- NOTES:
	-- This function will be called when a Data Frame has been RECEIVED and
	-- needs to be checked for errors.
	-- This state will handle the detection of errors using the CRC at the
	-- end of the of the Data Frame. All remaining bytes will be read from
	-- the download queue; the first 512 will be the data bytes, the next 4
	-- will be the CRC bytes. If no error is detected the system will TRANSMIT
	-- an ACK, send the received data to be printed, and return to the Receive
	-- state. Otherwise the system will timeout and return to the Receive state
	-- without TRANSMITTING an ACK. This timeout must be very short.
	----------------------------------------------------------------------*/
	bool ProtocolEngine::ErrorDetection()
	{
		// Start TOS
		int timer = 0;

		// Initialize frame and CRC holder
		char* frame = new char[512];
		char* CRC = new char[4];

		bool errorDetected = false;

		while (timer < TIMEOUT)
		{
			// Get the 512 data characters from the download queue
			for (int i = 2; i <= 514; i++)
			{
				// Read all chars from the front of the queue and remove it
				frame[i - 2] = incFrame[i];
			}

			for (int i = 515; (i - 515) < 4; i++)
			{
				// Read the remaining chars from the front of the queue as CRC
				CRC[i - 515] = incFrame[i];
			}

			delete incFrame;
			incFrame = nullptr;
			if (!mDownloadQueue->empty())
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
				//mPrintQueue->push(frame);
				// Increment the logfile successful frames counter
				mLogfile->sent_packet++;
				// Transmit ACK control frame
				std::cout << "Sending ACK" << std::endl;
				if (TransmitFrame(true, ASCII_ACK))
				{
					std::cout << "Sent ACK" << std::endl;
					mLogfile->sent_packet++;
				}
				else
				{
					std::cout << "Failed Sending ACK" << std::endl;
				}
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