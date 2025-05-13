using System.Net.Sockets;
using System.Net;
using System.Text;
using System.Runtime.CompilerServices;
using System.IO;

namespace Client
{
    internal class MainApp
    {
        public class Packet
        {
            public Int32 PacketType;
            public Int32 PacketSize;

            public byte[] PacketData;

            public Packet(byte[] packetData) {
                PacketData = new byte[packetData.Length];
                Array.Copy(packetData, 0, PacketData, 0, packetData.Length);

                PacketType = 2;
                PacketSize = PacketData.Length;
            }

            public byte[] GetSerialize() {
                byte[] buffer = new byte[8 + PacketData.Length];
                Array.Copy(BitConverter.GetBytes(PacketType), 0, buffer, 0, 4);
                Array.Copy(BitConverter.GetBytes(PacketSize), 0, buffer, 4, 4);
                Array.Copy(PacketData, 0, buffer, 8, PacketData.Length);

                return buffer;
            }

            public int GetSize() {
                return (sizeof(Int32) * 2) + PacketSize;
            }

            public void GetSerialize(byte[] packetBuffer) {
                PacketType = BitConverter.ToInt32(packetBuffer, 0);
                PacketSize = BitConverter.ToInt32(packetBuffer, 4);
                Array.Copy(PacketData, 0, packetBuffer, 8, PacketSize);
            }
        }

        static void Main(string[] args) {
            string serverIP = "127.0.0.1";
            const int serverPort = 16001;

            IPEndPoint remoteAddress = new IPEndPoint(IPAddress.Parse(serverIP), serverPort);

            try {
                TcpClient client = new TcpClient();

                client.Connect(remoteAddress);
                Console.WriteLine($"Connected to server at {serverIP}:{serverPort}");

                NetworkStream stream = client.GetStream();

                for (int idx = 0; idx < 15; ++idx) {
                    string echoMsg = $"Hello. This message seq {idx}";
                    Packet packet = new(Encoding.Default.GetBytes(echoMsg));

                    stream.Write(packet.GetSerialize(), 0, packet.GetSize());
                    Console.WriteLine($"Data sent to server. {echoMsg}");

                    Thread.Sleep(1500);


                    //byte[] buffer = new byte[1024];
                    //int bytesRead = stream.Read(buffer, 0, buffer.Length);
                    //string message = Encoding.Default.GetString(buffer, 0, bytesRead);
                    //Console.WriteLine($"Received from server: {message}");
                }

                Console.WriteLine("Client close");
                stream.Close();
                client.Close();
            } catch (SocketException ex) {
                Console.WriteLine($"SocketException: {ex.Message}");
            } catch (Exception ex) {
                Console.WriteLine($"Exception: {ex.Message}");
            } finally {
            }

            Console.ReadKey();
        }
    }
}
