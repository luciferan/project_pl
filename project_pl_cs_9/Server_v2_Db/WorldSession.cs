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
    public class WorldSession : NetSession
    {
        PacketHead head = new();
        public Int32 HeartbeatCount { get; set; } = 0;
        public DateTime HeartbeatTime { get; set; } = DateTime.UtcNow;

        public WorldSession(Socket socket, int recvBufferSize = 1024 * 10) : base(socket, recvBufferSize) { }
        public void Send(PacketBase sendPacket) {
            lock (_sendLock) {
                Serializer ser = Serializer.PacketSerializer(sendPacket);
                _sendQueue.Enqueue(ser.Buffer);
            }
            DoSend();
        }

        protected override bool DataParsing(NetSession session, byte[] buffer, int offset, int transferred) {
            if (null == _recvBuffer) {
                return false;
            }
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

            WorldPacketHandler.Instance.Enqueue(this, packet.Body);
            return true;
        }

        public void BufferTrim() {
            if (null == _recvBuffer) {
                return;
            }
            Array.Copy(_recvBuffer, _readPos, _recvBuffer, 0, _recvBuffer.Length - _readPos);
            _recvLength -= _readPos;
            _currPos -= _readPos;
            _readPos = 0;
        }
    }

    public class WorldPacketHandler : IPacketHandler
    {
        private static readonly Lazy<WorldPacketHandler> _instance = new(() => new());
        public static WorldPacketHandler Instance => _instance.Value;

        private readonly Dictionary<Int32, Func<WorldSession, PacketBody, Task<bool>>> functionMap = new();

        private WorldPacketHandler(int maxParallelism = 4) : base(maxParallelism) => RegistPacketFunction();

        public void Enqueue(WorldSession session, PacketBody packet) {
            Task task = Task.Factory.StartNew(async () => await PacketProcess(session, packet),
                CancellationToken.None, TaskCreationOptions.None,
                this
                ).Unwrap();
        }

        async Task<bool> PacketProcess(WorldSession session, PacketBody packet) {
            if (functionMap.TryGetValue(packet.PacketType, out var packetFunction)) {
                if (packetFunction != null) {
                    return await packetFunction.Invoke(session, packet);
                }
            }
            return false;
        }

        void Register(PacketTypeWD packetType, Func<WorldSession, PacketBody, Task<bool>> func) {
            if (!functionMap.ContainsKey((Int32)packetType)) {
                functionMap.Add((Int32)packetType, func);
            }
        }

        void RegistPacketFunction() {
            Register(PacketTypeWD.join_request, WD_JoinRequest);
            Register(PacketTypeWD.heartbeat, WD_Heartbeat);
        }

        private static async Task<bool> WD_JoinRequest(WorldSession session, PacketBody pack) {
            PacketWD_JoinRequest packet = new PacketWD_JoinRequest(pack.PacketData);

            Int32 result = 0;
            var dbSession = DbServerApp.Instance.GetWorldSession(packet.WorldId);

            if (null != dbSession) {
                Console.WriteLine("already connected");
                result = 1;
            }

            PacketDW_JoinResult sendPacket = new();
            sendPacket.Result = result;
            sendPacket.DbId = DbServerApp.Instance.GetServerId();
            session.Send(sendPacket);

            return await Task.FromResult(true);
        }

        private static async Task<bool> WD_Heartbeat(WorldSession session, PacketBody pack) {
            PacketWD_Heartbeat packet = new PacketWD_Heartbeat(pack.PacketData);
            return await Task.FromResult(true);
        }
    }
}
