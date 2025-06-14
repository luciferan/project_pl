using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection.Metadata;
using System.Text;
using System.Threading.Tasks;
using PL_Network;

namespace Server
{
    internal class PacketParser
    {
        const int maxBufferSize = 1024 * 10;

        byte[] recvBuffer = new byte[maxBufferSize];
        int recvLength = 0;

        int currPos = 0;
        int readPos = 0;

        PacketHead head = new();

        public bool DataParsing(PlayerSession player, byte[] buffer, int offset, int transferred) {
            if (maxBufferSize < currPos + transferred) {
                return false;
            }

            Array.Copy(buffer, offset, recvBuffer, currPos, transferred);
            currPos += transferred;
            recvLength += transferred;
            if (recvLength < PacketHead.Length) {
                return true;
            }

            head.Serialize(new Serializer(recvBuffer, recvLength));
            if (recvLength < head.PacketLength) {
                return true;
            }

            Packet packet = Serializer.PacketDeserializer(recvBuffer, head.PacketLength);
            readPos = head.PacketLength;
            BufferTrim();

            PacketHandler.Instance.Enqueue(player, packet.Body);
            return true;
        }

        public void BufferTrim() {
            Array.Copy(recvBuffer, readPos, recvBuffer, 0, recvBuffer.Length - readPos);
            recvLength -= readPos;
            currPos -= readPos;
            readPos = 0;
        }
    }
}
