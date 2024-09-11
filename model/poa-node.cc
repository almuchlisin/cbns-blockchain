#include "ns3/core-module.h"  // Pastikan modul ini diimpor untuk menggunakan Time
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/tcp-socket-factory.h"
#include "poa-node.h"
#include "stdlib.h"
#include "ns3/ipv4.h"
#include <ctime>
#include <map>

int tx_size = 200;                   // the size of tx, in KB
int tx_speed = 2000;                 // the rate of transaction generation, in op/s
int max_blocks = 50;                 // Maximum number of blocks to process

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PoaNode");

NS_OBJECT_ENSURE_REGISTERED (PoaNode);

TypeId
PoaNode::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::PoaNode")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<PoaNode> ()
    ;
    return tid;
}

PoaNode::PoaNode(void) : is_validator(false), blockNum(0) {}

PoaNode::~PoaNode(void) {
    NS_LOG_FUNCTION (this);
}

void 
PoaNode::DetermineValidator()
{
    is_validator = (m_id % 2 == 0);
    if (is_validator) {
        NS_LOG_INFO("Node " << m_id << " is a validator.");
    }
}

static char intToChar(int a) {
    return a + '0';
}

static float getRandomDelay() {
  return (rand() % 3) * 1.0 / 1000;
}

void 
PoaNode::StartApplication ()            
{
    m_startTime = Simulator::Now();  // Catat waktu mulai simulasi

    DetermineValidator();
    m_value = 0;
    proposal = intToChar(m_id);
    heartbeat_timeout = 0.05;
    add_change_value = 0;
    round = 0;

    if (!m_socket)
    {
        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket (GetNode (), tid);

        InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 7071);
        m_socket->Bind (local);
        m_socket->Listen ();
    }
    m_socket->SetRecvCallback (MakeCallback (&PoaNode::HandleRead, this));
    m_socket->SetAllowBroadcast (true);

    std::vector<Ipv4Address>::iterator iter = m_peersAddresses.begin();
    NS_LOG_INFO("Node " << m_id << " start");
    while(iter != m_peersAddresses.end()) {
        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        Ptr<Socket> socketClient = Socket::CreateSocket (GetNode (), tid);
        socketClient->Connect (InetSocketAddress(*iter, 7071));
        m_peersSockets[*iter] = socketClient;
        iter++;
    }
    if (is_validator) {
        m_nextBlock = Simulator::Schedule(Seconds(1.0), &PoaNode::CreateBlock, this);
    }
}

void 
PoaNode::StopApplication ()
{
    Time endTime = Simulator::Now();  // Catat waktu selesai simulasi
    Time duration = endTime - m_startTime;  // Hitung durasi simulasi

    NS_LOG_INFO("Node " << m_id << " has stopped after creating " << blockNum << " blocks.");
    NS_LOG_INFO("Total simulation time: " << duration.GetSeconds() << " seconds.");
    NS_LOG_INFO("At time " << Simulator::Now ().GetSeconds () << "s, simulation stopped.");

    // Log tambahan untuk menunjukkan durasi simulasi
    std::cout << "Simulation completed in " << duration.GetSeconds() << " seconds." << std::endl;
}

void 
PoaNode::HandleRead (Ptr<Socket> socket)
{ 
    Ptr<Packet> packet;
    Address from;
    Address localAddress;

    while ((packet = socket->RecvFrom(from)))
    {
        socket->SendTo(packet, 0, from);
        if (packet->GetSize () == 0) { 
            break;
        }
        if (InetSocketAddress::IsMatchingType (from)) {
            std::string msg = getPacketContent(packet, from);

            uint8_t data[tx_size];
            if (is_validator) {
                packet->CopyData(data, tx_size);
                NS_LOG_INFO("Validator " << m_id << " received block for validation.");
                if (data[0] == '1') {
                    NS_LOG_INFO("Block validated by Validator " << m_id);
                }
            }
        }
        socket->GetSockName (localAddress);
    }
}

std::string 
PoaNode::getPacketContent(Ptr<Packet> packet, Address from) 
{ 
    char *packetInfo = new char[packet->GetSize () + 1];
    std::ostringstream totalStream;
    packet->CopyData (reinterpret_cast<uint8_t*>(packetInfo), packet->GetSize ());
    packetInfo[packet->GetSize ()] = '\0';
    totalStream << m_bufferedData[from] << packetInfo; 
    std::string totalReceivedData(totalStream.str());

    return totalReceivedData;
}  

void 
SendPacket(Ptr<Socket> socketClient, Ptr<Packet> p) {
    socketClient->Send(p);
}

void 
PoaNode::SendBlock(uint8_t data[])
{
    Ptr<Packet> p = Create<Packet>(data, tx_size);
    for (auto iter = m_peersAddresses.begin(); iter != m_peersAddresses.end(); ++iter) {
        Ptr<Socket> socketClient = m_peersSockets[*iter];
        Simulator::Schedule(Seconds(getRandomDelay()), &SendPacket, socketClient, p);
    }
}

void 
PoaNode::CreateBlock()
{
    if (is_validator) {
        NS_LOG_INFO("Validator " << m_id << " is creating block " << blockNum + 1);

        if (blockNum >= max_blocks) {  // Batas jumlah blok
            NS_LOG_INFO("Node " << m_id << " has created " << blockNum << " blocks. Stopping simulation.");

            // Menampilkan waktu simulasi saat simulasi dihentikan
            Time endTime = Simulator::Now();  // Catat waktu selesai simulasi
            Time duration = endTime - m_startTime;  // Hitung durasi simulasi
            std::cout << "Simulation completed in " << duration.GetSeconds() << " seconds." << std::endl;

            Simulator::Stop(); // Hentikan simulasi setelah jumlah blok yang ditentukan tercapai
            return;
        }

        uint8_t data[tx_size];
        for (int i = 0; i < tx_size; i++) {
            data[i] = '1'; 
        }
        SendBlock(data);

        blockNum++;  // Tingkatkan jumlah blok yang telah dibuat

        m_nextBlock = Simulator::Schedule(Seconds(1.0), &PoaNode::CreateBlock, this);
    }
}

} // namespace ns3
