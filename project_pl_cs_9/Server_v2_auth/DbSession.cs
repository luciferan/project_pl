using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using PL_Network_v2;
using PL_Common;

namespace PL_Server_v2_Auth
{
    public class DbSession : NetSession
    {
        public DbSession(Socket socket, int recvBufferSize = 1024 * 10) : base(socket, recvBufferSize) { }
        protected override bool DataParsing(NetSession session, byte[] buffer, int offset, int transferred) {
            return true;
        }
    }

    public class DbPacketHandler : IPacketHandler
    {
        private static readonly Lazy<DbPacketHandler> _instance = new(() => new());
        public static DbPacketHandler Instance => _instance.Value;

        private readonly Dictionary<Int32, Func<DbSession, PacketBody, Task<bool>>> functionMap = new();

        private DbPacketHandler(int maxParallelism = 4) : base(maxParallelism) {
            RegistPacketFunction();
        }

        public void Enqueue(DbSession player, PacketBody packet) {
            Task task = Task.Factory.StartNew(async () => await PacketProcess(player, packet),
                CancellationToken.None, TaskCreationOptions.None,
                this
                ).Unwrap();
        }

        async Task<bool> PacketProcess(DbSession player, PacketBody packet) {
            if (functionMap.TryGetValue(packet.PacketType, out var packetFunction)) {
                if (packetFunction != null) {
                    return await packetFunction.Invoke(player, packet);
                }
            }
            return false;
        }

        void Register(PacketTypeDA packetType, Func<DbSession, PacketBody, Task<bool>> func) {
            if (!functionMap.ContainsKey((Int32)packetType)) {
                functionMap.Add((Int32)packetType, func);
            }
        }

        void RegistPacketFunction() {
            Register(PacketTypeDA.heartbeat, DA_Heartbeat);
        }

        private static async Task<bool> DA_Heartbeat(DbSession dbSession, PacketBody pack) {
            PacketDA_Heartbeat packet = new(pack.PacketData);

            return await Task.FromResult(true);
        }

    }
}
