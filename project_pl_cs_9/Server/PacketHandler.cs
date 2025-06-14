using Server;
using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Net.Http.Headers;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Threading.Tasks.Sources;
using PL_Network;
using Org.BouncyCastle.Bcpg;


namespace Server
{
    public class PacketHandler : IPacketHandler
    {
        private static readonly Lazy<PacketHandler> _instance = new(() => new());
        public static PacketHandler Instance => _instance.Value;

        private readonly Dictionary<UInt32, Func<PlayerSession, PacketBody, Task<bool>>> functionMap = new();

        private PacketHandler(int maxParallelism = 4) : base(maxParallelism) {
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
            PacketCS_Heaetbeat packet = new PacketCS_Heaetbeat(pack.PacketData);
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
