#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BlockchainSimulator");

// Fungsi untuk mengatur delay sesuai dengan node ID
std::string GetNodeDelay(uint32_t nodeId) {
  switch (nodeId) {
   /*case 0:
      return "10ms";  // Node 1
    case 1:
      return "50ms";  // Node 2
    case 2:
      return "100ms"; // Node 3
    case 3:
      return "500ms"; // Node 4
    case 4:
      return "1000ms"; // Node 5*/
    default:
      return "10ms";  // Default delay
  }
}

// Membuat jaringan dan menjalankan simulasi
void startSimulator (int N)
{
  NodeContainer nodes;
  nodes.Create (N);

  NetworkHelper networkHelper (N);
  NetDeviceContainer devices;
  PointToPointHelper pointToPoint;

  // Set up Internet stack
  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("1.0.0.0", "255.255.255.0");

  // Menghubungkan node dengan delay yang berbeda
  for (int i = 0; i < N; i++) {
      for (int j = 0; j < N && j != i; j++) {
          Ipv4InterfaceContainer interface;
          Ptr<Node> p1 = nodes.Get (i);
          Ptr<Node> p2 = nodes.Get (j);

          // Set delay yang berbeda untuk setiap node
          std::string delay = GetNodeDelay(i);
          pointToPoint.SetChannelAttribute ("Delay", StringValue (delay));

          // Cetak informasi delay ke konsol
          std::cout << "Setting delay for Node " << i+1 << " -> Node " << j+1 << ": " << delay << std::endl;

          NetDeviceContainer device = pointToPoint.Install(p1, p2);
          
          interface.Add(address.Assign (device.Get(0)));
          interface.Add(address.Assign (device.Get(1)));

          networkHelper.m_nodesConnectionsIps[i].push_back(interface.GetAddress(1));
          networkHelper.m_nodesConnectionsIps[j].push_back(interface.GetAddress(0));

          address.NewNetwork();
      }
  }
  ApplicationContainer nodeApp = networkHelper.Install (nodes);

  nodeApp.Start (Seconds (0.0));
  nodeApp.Stop (Seconds (10.0));

  //AnimationInterface anim ("2.xml");

  Simulator::Run ();
  Simulator::Destroy ();
}

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  // Jumlah node dalam jaringan
  int N = 5;
  
  Time::SetResolution (Time::NS);

  // Aktifkan logging untuk RaftNode (jika diperlukan)
  LogComponentEnable ("PoaNode", LOG_LEVEL_INFO);

  // Memulai simulasi
  startSimulator(N);

  return 0;
}


