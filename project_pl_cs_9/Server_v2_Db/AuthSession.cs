using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using PL_Common;
using PL_Network_v2;

namespace PL_Server_v2_Db
{
    public class AuthSession : NetSession
    {
        const int _maxBufferSize = 1024 * 10;
        byte[] _recvBuffer = new byte[_maxBufferSize];
        int _recvLength = 0;
        int _currPos = 0;
        int _readPos = 0;

        PacketHead head = new();

        public AuthSession(Socket socket, int recvBufferSize = 1024 * 10) : base(socket, recvBufferSize) {
        }
        public void Send(PacketBase sendPacket) {
            lock (_sendLock) {
                Serializer ser = Serializer.PacketSerializer(sendPacket);
                _sendQueue.Enqueue(ser.Buffer);
            }
            DoSend();
        }

        protected override bool DataParsing(NetSession session, byte[] buffer, int offset, int transferred) {
            if (_maxBufferSize < _currPos + transferred) {
                return false;
            }
            Array.Copy(buffer, offset, _recvBuffer, _currPos, transferred);
            _currPos += transferred;
            _recvLength += transferred;
            if (_recvLength > PacketHead.Length) {
                return true;
            }
            head.Serialize(new Serializer(_recvBuffer, _recvLength));
            if (_recvLength > head.PacketLength) {
                return true;
            }

            Packet packet = Serializer.PacketDeserializer(_recvBuffer, head.PacketLength);
            _readPos = head.PacketLength;
            BufferTrim();

            //AuthPacketHandler.Instance.Enqueue(this, packet.Body);
            return true;
        }

        public void BufferTrim() {
            Array.Copy(_recvBuffer, _readPos, _recvBuffer, 0, _recvBuffer.Length - _readPos);
            _recvLength -= _readPos;
            _currPos -= _readPos;
            _readPos = 0;
        }
    }
}
