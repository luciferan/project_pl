using Server;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace Server
{
    public class PacketHandler
    {
        private static readonly Lazy<PacketHandler> _instance = new(() => new PacketHandler());
        public static PacketHandler Instance => _instance.Value;

        private readonly Dictionary<Int32, Func<PlayerSession, Packet, Task<bool>>> functionMap = new();

        private PacketHandler() {
            RegisterPacketFunction(ProtocolCS.ECHO, CS_EchoAsync);
        }

        public void RegisterPacketFunction(Int32 packetType, Func<PlayerSession, Packet, Task<bool>> func) {
            if (!functionMap.ContainsKey(packetType)) {
                functionMap.Add(packetType, func);
            }
        }

        private static async Task<bool> CS_EchoAsync(PlayerSession player, Packet packet) {
            string recvMsg = Encoding.UTF8.GetString(packet.GetPacketData());
            Console.WriteLine($"CS_EchoAsync: {recvMsg}");

            player.Send(packet);
            return await Task.FromResult(true);
        }

        public async Task<bool> PacketProcess(PlayerSession player, Packet packet) {
            if (functionMap.TryGetValue(packet.GetPacketType(), out var packetFunction)) {
                if (packetFunction != null) {
                    return await packetFunction.Invoke(player, packet);
                }
            }
            return false;
        }
    }
}
