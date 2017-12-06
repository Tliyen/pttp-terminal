#pragma once

namespace protocoletariat
{
	struct LogFile {
		int sent_packet;
		int lost_packet;
		int received_packet;
		int received_corrupted_packet;

		LogFile()
			: sent_packet(0)
			, lost_packet(0)
			, received_packet(0)
			, received_corrupted_packet(0)
		{
		}
	};

	// bCommOn to be used as global flag
	extern bool bCommOn;
}


