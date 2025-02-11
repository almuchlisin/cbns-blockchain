#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BlockchainSimulator");

// Fungsi untuk mengatur delay acak sesuai dengan node ID
std::string GetRandomDelay() {
  // Menggunakan UniformRandomVariable untuk mendapatkan nilai acak
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
  uv->SetAttribute ("Min", DoubleValue (5));   // Minimum 10 ms
  uv->SetAttribute ("Max", DoubleValue (5)); // Maximum 1000 ms
  
  // Menghasilkan delay acak dalam rentang 10 ms hingga 1000 ms
  double randomDelay = uv->GetValue();

  // Mengembalikan nilai delay dalam format string dengan satuan milidetik (ms)
  return std::to_string(randomDelay) + "ms";
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

          // Set delay acak untuk setiap node
          std::string delay = GetRandomDelay();
          pointToPoint.SetChannelAttribute ("Delay", StringValue (delay));

          // Cetak informasi delay ke konsol
          std::cout << "Setting random delay for Node " << i+1 << " -> Node " << j+1 << ": " << delay << std::endl;

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
  int N = 10;
  
  Time::SetResolution (Time::NS);

  // Aktifkan logging untuk RaftNode (jika diperlukan)
  LogComponentEnable ("PoaNode", LOG_LEVEL_INFO);

  // Memulai simulasi
  startSimulator(N);

  return 0;
}

