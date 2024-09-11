#ifndef POA_NODE_H
#define POA_NODE_H

#include <algorithm>
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

    void SetPeersAddresses (const std::vector<Ipv4Address> &peers);         // Set the addresses of all neighbor nodes

    PoaNode (void);
    virtual ~PoaNode (void);

    // Node information
    uint32_t        m_id;                               // Node ID
    Ptr<Socket>     m_socket;                           // Listening socket
    Ptr<Socket>     m_socketClient;                     // Client socket
    std::map<Ipv4Address, Ptr<Socket>>      m_peersSockets; // List of neighbor node sockets
    std::map<Address, std::string>          m_bufferedData; // Map holding the buffered data from previous handleRead events
    Address         m_local;                            // Local address
    std::vector<Ipv4Address>  m_peersAddresses;         // List of neighbor nodes

    int             N;                                  // Total number of nodes
    bool            is_validator;                       // Is this node a validator (Proof of Authority)
    int             m_value;                            // Value modified through consensus
    char            proposal;                           // Command to be sent
    int             blockNum;                           // Number of blocks processed
    int             round;                              // Consensus round
    float           heartbeat_timeout;                  // Heartbeat timeout interval
    int             add_change_value;                   // Whether to include proposal in heartbeat
    EventId         m_nextBlock;                        // Next block creation event (specific to PoA)

    // Inherited Application class methods
    virtual void StartApplication (void);    
    virtual void StopApplication (void); 

    // Message handling methods
    void HandleRead (Ptr<Socket> socket);
    void Send (uint8_t data[]);
    void Send (uint8_t data[], Address from);
    void SendTX (uint8_t data[], int num);
    void SendBlock (uint8_t data[]);                     // PoA specific: Send block

    // Proposal setting method
    void setProposal (void);

    // PoA Specific Methods
    void DetermineValidator (void);                      // PoA specific: Determine if node is a validator
    void CreateBlock (void);                             // PoA specific: Create block
    std::string getPacketContent (Ptr<Packet> packet, Address from); 
};

enum Message
{
    CLIENT_REQ,        // 0       Client request
    CLIENT_RES,        // 1       Response to client
    VOTE_REQ,          // 2       Vote request (Not used in PoA)
    VOTE_RES,          // 3       Vote response (Not used in PoA)
    HEARTBEAT,         // 4       Heartbeat
    HEARTBEAT_RES,     // 5       Heartbeat response
};

enum HeartBeatType
{
    HEART_BEAT,       // 0        Regular heartbeat
    PROPOSAL,         // 1        Heartbeat with proposal
};

enum State
{
    POA_SUCCESS,      // 0      Success
    POA_FAILED,       // 1      Failed
};

} // namespace ns3

#endif // POA_NODE_H
