#ifndef POA_NODE_H
#define POA_NODE_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/boolean.h"
#include <map>

namespace ns3 {

class Address;
class Socket;
class Packet;

class PoaNode : public Application 
{
  public:
    static TypeId GetTypeId (void);

    void SetPeersAddresses (const std::vector<Ipv4Address> &peers); 

    PoaNode (void);
    virtual ~PoaNode (void);

    uint32_t m_id;
    Ptr<Socket> m_socket;
    Ptr<Socket> m_socketClient;
    std::map<Ipv4Address, Ptr<Socket>> m_peersSockets;
    std::map<Address, std::string> m_bufferedData;
    
    Address m_local;
    std::vector<Ipv4Address> m_peersAddresses;

    int N;
    bool is_validator;
    int m_value;
    char proposal;
    int blockNum;
    int round;
    uint8_t* tx;

    // Deklarasi variabel yang sebelumnya tidak ditemukan
    float heartbeat_timeout;
    int add_change_value;
    EventId m_nextBlock;

    virtual void StartApplication (void);    
    virtual void StopApplication (void); 

    void HandleRead (Ptr<Socket> socket);
    void SendBlock(uint8_t data[]);
    std::string getPacketContent(Ptr<Packet> packet, Address from); 
    void CreateBlock();
    void DetermineValidator(); // Tambahkan deklarasi fungsi ini
};

} // namespace ns3

#endif /* POA_NODE_H */
