using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection.Metadata;
using System.Text;
using System.Threading.Tasks;

namespace Server
{
    internal class PacketParser
    {
        const int PacketHeaderSize = 8;

        int maxBufferSize = 1024 * 10;
        byte[] packetBuffer = new byte[1024 * 10];
        int packetSize = 0;

        int currPos = 0;
        int readPos = 0;

        public delegate void CompletedPacketRecvCallback(Packet packet);

        PacketHandler _packetHandler = PacketHandler.Instance;

        public async Task<bool> DataParsing(PlayerSession player, byte[] buffer, int offset, int transferred) {
            if (maxBufferSize < currPos + transferred) {
                return false;
            }

            Array.Copy(buffer, offset, packetBuffer, currPos, transferred);
            currPos += transferred;
            packetSize += transferred;
            if (packetSize < PacketHeaderSize) {
                return true;
            }

            PacketHeader packetHeader = new(packetBuffer);
            if (packetSize < packetHeader.PacketSize + PacketHeaderSize) {
                return true;
            }

            Packet packet = new(packetBuffer);
            readPos = packet.GetSize();
            BufferTrim();

            PacketHandler.Instance.Enqueue(player, packet);
            return true;
        }

        public void BufferTrim() {
            Array.Copy(packetBuffer, readPos, packetBuffer, 0, packetBuffer.Length - readPos);
            packetSize -= readPos;
            currPos -= readPos;
            readPos = 0;
        }
    }
}
