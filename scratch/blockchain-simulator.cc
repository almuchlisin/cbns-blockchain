#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BlockchainSimulator");

// Create network
void startSimulator (int N)
{
  NodeContainer nodes;
  nodes.Create (N);

  NetworkHelper networkHelper (N);
  // By default pointToPoint can only connect two nodes and needs to be connected manually.
  NetDeviceContainer devices;
  PointToPointHelper pointToPoint;

  // The total node bandwidth is 24Mbps, and each point-to-point channel is 3Mbps.
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("3Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("3ms"));
  uint32_t nNodes = nodes.GetN ();

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("1.0.0.0", "255.255.255.0");

  // Network nodes establish connections two by two
  for (int i = 0; i < N; i++) {
      for (int j = 0; j < N && j != i; j++) {
          Ipv4InterfaceContainer interface;
          Ptr<Node> p1 = nodes.Get (i);
          Ptr<Node> p2 = nodes.Get (j);
          NetDeviceContainer device = pointToPoint.Install(p1, p2);
          
          interface.Add(address.Assign (device.Get(0)));
          interface.Add(address.Assign (device.Get(1)));

          networkHelper.m_nodesConnectionsIps[i].push_back(interface.GetAddress(1));
          networkHelper.m_nodesConnectionsIps[j].push_back(interface.GetAddress(0));

           // Create a new network: If you do not add a network, all IPs will be in the same network, and the last device will overwrite the previous settings, making it impossible to access previous neighbor nodes through IPs.
          // Proper settings: The two nodes connected to each device are in the same subnet, so each time an IP is assigned, the address should be increased by one network segment.
          address.NewNetwork();
      }
  }
  ApplicationContainer nodeApp = networkHelper.Install (nodes);

  nodeApp.Start (Seconds (0.0));
  nodeApp.Stop (Seconds (10.0));

  NS_LOG_INFO("Enable pcap tracing");
  pointToPoint.EnablePcapAll ("Raft");

  AnimationInterface anim ("RaftBlockchain.xml");
  
  NS_LOG_INFO("Run Raft Simulation");

  Simulator::Run ();
  Simulator::Destroy ();
}


int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  // numbers of nodes in network
  int N = 16;
  
  Time::SetResolution (Time::NS);

  // 1.need changed to a specific protocol class
  LogComponentEnable ("RaftNode", LOG_LEVEL_INFO);

  // start the simulator
  startSimulator(N);

  return 0;
}
