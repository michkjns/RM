
#pragma once

#include "time.h"
#include "networkinterface.h"
#include "bitstream.h"

namespace network
{

	class Networker
	{
	public:
		Networker();
		virtual ~Networker();
	
		virtual bool initialize();
		virtual void tick(uint64_t dt);
	
		bool isInitialized() const;
		bool isConnected() const;
	
	
	private:
		unsigned int m_numClients;
		bool m_isInitialized;
		bool m_isConnected;
	
	};
	
}; // namespace network