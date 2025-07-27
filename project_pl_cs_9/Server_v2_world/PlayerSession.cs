using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using PL_Common;
using PL_Network_v2;
using System.Net.Mime;

namespace PL_Server_v2_World
{
    public class PlayerSession : NetSession
    {
        int _maxBufferSize = 1024 * 10;

        byte[] _recvBuffer;
        int _recvLength = 0;
        int _currPos = 0;
        int _readPos = 0;

        PacketHead head = new();

        public Int32 HeartbeatCount { get; set; } = 0;
        public DateTime HeartbeatTime { get; set; } = DateTime.UtcNow;

        public PlayerSession(Socket socket, int recvBufferSize = 1024 * 10) : base(socket, recvBufferSize) {
            _maxBufferSize = recvBufferSize;
            _recvBuffer = new byte[_maxBufferSize];
        }

        public override void CheckHeartbeat() {
            DateTime curTime = DateTime.UtcNow;
            TimeSpan diff = curTime - HeartbeatTime;

            double timeSpan = diff.TotalMilliseconds;
            if (timeSpan > 15 * 1000) {
                if (HeartbeatCount > 5) {
                    Disconnect();
                    return;
                }

                HeartbeatTime = curTime;
                IncHeartbeatCount();

                PacketCS_Heaetbeat sendPacket = new();
                Send(sendPacket);
            }
        }
        public void IncHeartbeatCount() {
            HeartbeatCount += 1;
        }
        public void DecHeartbeatCount() {
            HeartbeatCount -= 1;
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

            PlayerPacketHandler.Instance.Enqueue(this, packet.Body);
            return true;
        }

        public void BufferTrim() {
            Array.Copy(_recvBuffer, _readPos, _recvBuffer, 0, _recvBuffer.Length - _readPos);
            _recvLength -= _readPos;
            _currPos -= _readPos;
            _readPos = 0;
        }
    }

    public class PlayerPacketHandler : IPacketHandler
    {
        private static readonly Lazy<PlayerPacketHandler> _instance = new(() => new());
        public static PlayerPacketHandler Instance => _instance.Value;

        private readonly Dictionary<UInt32, Func<PlayerSession, PacketBody, Task<bool>>> functionMap = new();

        private PlayerPacketHandler(int maxParallelism = 4) : base(maxParallelism) {
            RegistPacketFunction();
        }

        public void Enqueue(PlayerSession player, PacketBody packet) {
            Task task = Task.Factory.StartNew(async () => await PacketProcess(player, packet),
                CancellationToken.None, TaskCreationOptions.None,
                this
                ).Unwrap();
        }

        async Task<bool> PacketProcess(PlayerSession player, PacketBody packet) {
            if (functionMap.TryGetValue(packet.PacketType, out var packetFunction)) {
                if (packetFunction != null) {
                    return await packetFunction.Invoke(player, packet);
                }
            }
            return false;
        }

        void Register(PacketTypeCS packetType, Func<PlayerSession, PacketBody, Task<bool>> func) {
            if (!functionMap.ContainsKey((UInt32)packetType)) {
                functionMap.Add((UInt32)packetType, func);
            }
        }

        void RegistPacketFunction() {
            Register(PacketTypeCS.auth, CS_AuthAsync);
            Register(PacketTypeCS.enter, CS_EnterAsync);
            Register(PacketTypeCS.leave, CS_LeaveAsync);
            Register(PacketTypeCS.move, CS_MoveAsync);
            Register(PacketTypeCS.interaction, CS_InteractionAsync);
            Register(PacketTypeCS.chat, CS_ChatAsync);
            Register(PacketTypeCS.heartbeat, CS_HeartbeatAsync);
            Register(PacketTypeCS.echo, CS_EchoAsync);
        }

        private static async Task<bool> CS_AuthAsync(PlayerSession player, PacketBody pack) {
            PacketCS_Auth packet = new PacketCS_Auth(pack.PacketData);
            return await Task.FromResult(true);
        }
        private static async Task<bool> CS_EnterAsync(PlayerSession Player, PacketBody pack) {
            PacketCS_Enter packet = new PacketCS_Enter(pack.PacketData);
            return await Task.FromResult(true);
        }
        private static async Task<bool> CS_LeaveAsync(PlayerSession Player, PacketBody pack) {
            PacketCS_Leave packet = new PacketCS_Leave(pack.PacketData);
            return await Task.FromResult(true);
        }
        private static async Task<bool> CS_MoveAsync(PlayerSession Player, PacketBody pack) {
            PacketCS_Move packet = new PacketCS_Move(pack.PacketData);
            return await Task.FromResult(true);
        }
        private static async Task<bool> CS_InteractionAsync(PlayerSession Player, PacketBody pack) {
            PacketCS_Interaction packet = new PacketCS_Interaction(pack.PacketData);
            return await Task.FromResult(true);
        }
        private static async Task<bool> CS_ChatAsync(PlayerSession Player, PacketBody pack) {
            PacketCS_Chat packet = new PacketCS_Chat(pack.PacketData);
            return await Task.FromResult(true);
        }
        private static async Task<bool> CS_HeartbeatAsync(PlayerSession Player, PacketBody pack) {
            Player.DecHeartbeatCount();
            return await Task.FromResult(true);
        }
        private static async Task<bool> CS_EchoAsync(PlayerSession Player, PacketBody pack) {
            PacketCS_Echo packet = new PacketCS_Echo(pack.PacketData);

            string recvMsg = packet.EchoMsg;
            Console.WriteLine($"CS_EchoAsync: {recvMsg}");

            PacketSC_Echo sendPacket = new();
            sendPacket.SetData(0, recvMsg);
            Player.Send(sendPacket);

            return await Task.FromResult(true);
        }
    }
}
