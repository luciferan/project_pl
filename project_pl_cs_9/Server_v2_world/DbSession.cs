using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using PL_Common;
using PL_Network_v2;
using System.ComponentModel.Design;

namespace PL_Server_v2_World
{
    public class DbSession : NetSession
    {
        PacketHead head = new();
        public Int32 HeartbeatCount { get; set; } = 0;
        public DateTime HeartbeatTime { get; set; } = DateTime.UtcNow;

        public DbSession(Socket socket, int recvBufferSize = 1024 * 10) : base(socket, recvBufferSize) { }

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
            if (_recvLength < PacketHead.Length) {
                return true;
            }
            head.Serialize(new Serializer(_recvBuffer, _recvLength));
            if (_recvLength < head.PacketLength) {
                return true;
            }

            Packet packet = Serializer.PacketDeserializer(_recvBuffer, head.PacketLength);
            _readPos = head.PacketLength;
            BufferTrim();

            DbPacketHandler.Instance.Enqueue(this, packet.Body);
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

    public class DbPacketHandler : IPacketHandler
    {
        private static readonly Lazy<DbPacketHandler> _instance = new(() => new());
        public static DbPacketHandler Instance => _instance.Value;

        private readonly Dictionary<Int32, Func<DbSession, PacketBody, Task<bool>>> functionMap = new();

        private DbPacketHandler(int maxParallelism = 4) : base(maxParallelism) => RegistPacketFunction();

        public void Enqueue(DbSession session, PacketBody packet) {
            Task task = Task.Factory.StartNew(async () => await PacketProcess(session, packet),
                CancellationToken.None, TaskCreationOptions.None,
                this
                ).Unwrap();
        }

        async Task<bool> PacketProcess(DbSession session, PacketBody packet) {
            if (functionMap.TryGetValue(packet.PacketType, out var packetFunction)) {
                if (packetFunction != null) {
                    return await packetFunction.Invoke(session, packet);
                }
            }
            return false;
        }

        void Register(PacketTypeDW packetType, Func<DbSession, PacketBody, Task<bool>> func) {
            if (!functionMap.ContainsKey((Int32)packetType)) {
                functionMap.Add((Int32)packetType, func);
            }
        }

        void RegistPacketFunction() {
            Register(PacketTypeDW.join_result, DW_JoinResult);
            Register(PacketTypeDW.heartbeat, DW_Heartbeat);
        }

        private static async Task<bool> DW_JoinResult(DbSession session, PacketBody pack) {
            PacketDW_JoinResult packet = new PacketDW_JoinResult(pack.PacketData);

            if (0 != packet.Result) {
                Console.WriteLine("DbServer 연결 실패");
            } else {
                WorldServerApp.Instance.SetDbSession(packet.DbId, session);
            }

            return await Task.FromResult(true);
        }
        private static async Task<bool> DW_Heartbeat(DbSession session, PacketBody pack) {
            PacketWD_Heartbeat packet = new PacketWD_Heartbeat(pack.PacketData);

            PacketDW_Heartbeat sendPacket = new();
            session.Send(sendPacket);

            return await Task.FromResult(true);
        }
    }
}
