#pragma once

#include <Client.h>

#include <tbb/concurrent_unordered_map.h>

namespace tbb
{
	template<>
	inline size_t tbb_hasher(const net::PeerAddress& addr)
	{
		return std::hash<std::string_view>()(std::string_view{ (const char*)addr.GetSocketAddress(), (size_t)addr.GetSocketAddressLength() });
	}
}

namespace fx
{
	class ClientRegistry : public fwRefCountable
	{
	public:
		void HandleConnectingClient(const std::shared_ptr<Client>& client);

		std::shared_ptr<Client> MakeClient(const std::string& guid);

		inline void RemoveClient(const std::shared_ptr<Client>& client)
		{
			m_clientsByPeer[client->GetPeer()].reset();
			m_clientsByNetId[client->GetNetId()].reset();
			m_clients[client->GetGuid()] = nullptr;
		}

		inline std::shared_ptr<Client> GetClientByGuid(const std::string& guid)
		{
			auto ptr = std::shared_ptr<Client>();
			auto it = m_clients.find(guid);

			if (it != m_clients.end())
			{
				ptr = it->second;
			}

			return ptr;
		}

		inline std::shared_ptr<Client> GetClientByPeer(ENetPeer* peer)
		{
			auto ptr = std::shared_ptr<Client>();
			auto it = m_clientsByPeer.find(peer);

			if (it != m_clientsByPeer.end())
			{
				if (!it->second.expired())
				{
					ptr = it->second.lock();
				}
			}

			return ptr;
		}

		inline std::shared_ptr<Client> GetClientByEndPoint(const net::PeerAddress& address)
		{
			auto ptr = std::shared_ptr<Client>();
			auto it = m_clientsByEndPoint.find(address);

			if (it != m_clientsByEndPoint.end())
			{
				if (!it->second.expired())
				{
					ptr = it->second.lock();
				}
			}

			return ptr;
		}

		inline std::shared_ptr<Client> GetClientByNetID(uint16_t netId)
		{
			auto ptr = std::shared_ptr<Client>();
			auto it = m_clientsByNetId.find(netId);

			if (it != m_clientsByNetId.end())
			{
				if (!it->second.expired())
				{
					ptr = it->second.lock();
				}
			}

			return ptr;
		}

		inline void ForAllClients(const std::function<void(const std::shared_ptr<Client>&)>& cb)
		{
			for (auto& client : m_clients)
			{
				cb(client.second);
			}
		}

	private:
		tbb::concurrent_unordered_map<std::string, std::shared_ptr<Client>> m_clients;

		// aliases for fast lookup
		tbb::concurrent_unordered_map<uint16_t, std::weak_ptr<Client>> m_clientsByNetId;
		tbb::concurrent_unordered_map<net::PeerAddress, std::weak_ptr<Client>> m_clientsByEndPoint;
		tbb::concurrent_unordered_map<ENetPeer*, std::weak_ptr<Client>> m_clientsByPeer;
	};
}

DECLARE_INSTANCE_TYPE(fx::ClientRegistry);