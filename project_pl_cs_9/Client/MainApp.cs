using System.Net.Sockets;
using System.Net;
using System.Text;
using System.Runtime.CompilerServices;
using System.IO;
using PL_Network;

namespace Client
{
    internal class MainApp
    {
        public class Packet
        {
            // head
            public UInt32 CheckHead { get; set; } = 0x00000000;
            public Int32 PacketLength { get; set; }

            // body
            public UInt32 PacketType { get; set; }
            public Int32 EchoMsgLength { get; set; }
            public byte[] EchoMsg { get; set; } = Array.Empty<byte>();

            // tail
            public UInt32 CheckTail { get; set; } = 0x10000000;
            public UInt32 PacketTime { get; set; }

            public Packet() { }
            public Packet(byte[] EchoMsg) {
                PacketType = 2;
                this.EchoMsgLength = EchoMsg.Length;
                this.EchoMsg = new byte[EchoMsg.Length];
                Array.Copy(EchoMsg, 0, this.EchoMsg, 0, EchoMsg.Length);

                PacketLength = (sizeof(Int64) + sizeof(Int32) + EchoMsgLength + (sizeof(Int32) * 5));
                PacketTime = (UInt32)DateTimeOffset.UtcNow.ToUnixTimeSeconds();
            }

            public byte[] Serialize() {
                int offset = 0;
                byte[] buffer = new byte[PacketLength];
                Array.Copy(BitConverter.GetBytes(CheckHead), 0, buffer, offset, sizeof(UInt32)); offset += sizeof(UInt32);
                Array.Copy(BitConverter.GetBytes(PacketLength), 0, buffer, offset, sizeof(Int32)); offset += sizeof(Int32);

                Array.Copy(BitConverter.GetBytes(PacketType), 0, buffer, offset, sizeof(UInt32)); offset += sizeof(UInt32);
                Array.Copy(BitConverter.GetBytes(EchoMsgLength), 0, buffer, offset, sizeof(Int32)); offset += sizeof(Int32);
                Array.Copy(EchoMsg, 0, buffer, offset, EchoMsgLength); offset += EchoMsgLength;

                Array.Copy(BitConverter.GetBytes(CheckTail), 0, buffer, offset, sizeof(UInt32)); offset += sizeof(UInt32);
                Array.Copy(BitConverter.GetBytes(PacketTime), 0, buffer, offset, sizeof(UInt32)); offset += sizeof(UInt32);

                return buffer;
            }
            public void Serialize(byte[] Buffer) {
                int offset = 0;

                CheckHead = BitConverter.ToUInt32(Buffer, offset); offset += sizeof(UInt32);
                PacketLength = BitConverter.ToInt32(Buffer, offset); offset += sizeof(Int32);

                PacketType = BitConverter.ToUInt32(Buffer, offset); offset += sizeof(UInt32);
                EchoMsgLength = BitConverter.ToInt32(Buffer, offset); offset += sizeof(Int32);
                EchoMsg = new byte[EchoMsgLength];
                Array.Copy(Buffer, offset, EchoMsg, 0, EchoMsgLength);

                CheckTail = BitConverter.ToUInt32(Buffer, offset); offset += sizeof(UInt32);
                PacketTime = BitConverter.ToUInt32(Buffer, offset); offset += sizeof(UInt32);
            }

            public Int32 GetSize() {
                return PacketLength;
            }
        }

        static Packet DataParsing(byte[] buffer, int recvByteSize) {
            Packet packet = new();
            packet.Serialize(buffer);
            return packet;
        }

        //static Serializer MakeNetworkPacket(PacketBase body) {
        //    PacketHead head = new PacketHead();
        //    PacketTail tail = new PacketTail();
        //    head.PacketLength = head.GetSize() + body.GetSize() + tail.GetSize();

        //    Serializer ser = new Serializer(head.PacketLength);
        //    head.Serialize(ser);
        //    body.SerializeType(ser);
        //    body.SerializeBody(ser);
        //    tail.Serialize(ser);

        //    return ser;
        //}


        static void Main(string[] args) {
            string serverIP = "127.0.0.1";
            const int serverPort = 16101;

            IPEndPoint remoteAddress = new IPEndPoint(IPAddress.Parse(serverIP), serverPort);

            try {
                TcpClient client = new TcpClient();

                client.Connect(remoteAddress);
                Console.WriteLine($"Connected to server at {serverIP}:{serverPort}");

                NetworkStream stream = client.GetStream();
                byte[] recvBuffer = new byte[1024];

                Random random = new Random();

                //for (int idx = 0; idx < 15; ++idx) {
                int idx = 0;
                while (true) {
                    ++idx;
                    {
                        string echoMsg = $"Hello. This message seq {idx}";
                        //Packet packet = new(Encoding.UTF8.GetBytes(echoMsg));

                        //stream.Write(packet.Serialize(), 0, packet.GetSize());
                        //Console.WriteLine($"Data sent to server: {echoMsg}");
                        PacketCS_Echo sendPacket = new();
                        sendPacket.EchoMsg = echoMsg;
                        //Serializer ser = MakeNetworkPacket(sendPacket);
                        Serializer ser = Serializer.PacketSerializer(sendPacket);
                        stream.Write(ser.Buffer, 0, ser.GetSize());
                        Console.WriteLine($"Data sent to server: {echoMsg}");
                    }

                    {
                        int bytesRead = stream.Read(recvBuffer, 0, recvBuffer.Length);
                        //Packet packet = DataParsing(recvBuffer, bytesRead);
                        //Array.Copy(recvBuffer, packet.GetSize(), recvBuffer, 0, recvBuffer.Length - packet.GetSize());

                        //string message = Encoding.UTF8.GetString(packet.EchoMsg);
                        //Console.WriteLine($"Received from server: {message}");
                        Serializer ser = new Serializer(recvBuffer, bytesRead);
                        PacketHead head = new PacketHead();
                        head.Serialize(ser);
                        PacketSC_Echo recvPacket = new();
                        recvPacket.Serialize(ser);
                        PacketTail tail = new PacketTail();
                        tail.Serialize(ser);

                        string message = recvPacket.EchoMsg;
                        Console.WriteLine($"Received from server: {message}");

                    }

                    //Thread.Sleep(random.Next(1000, 2000));
                    Thread.Sleep(500);
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
