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


namespace Server
{
    public class PacketHandler : TaskScheduler
    {
        private static readonly Lazy<PacketHandler> _instance = new(() => new());
        public static PacketHandler Instance => _instance.Value;

        private readonly int _maxParallelism = 4;
        //private readonly LinkedList<Task> _tasks = new();
        private readonly Queue<Task> _tasks = new();
        private int _runningTasks = 0;
        private readonly object _lock = new();

        private readonly Dictionary<UInt32, Func<PlayerSession, Packet, Task<bool>>> functionMap = new();

        private PacketHandler(int maxParallelism = 4) {
            _maxParallelism = maxParallelism;
            RegisterPacketFunction(ProtocolCS.ECHO, CS_EchoAsync);
        }

        protected override IEnumerable<Task> GetScheduledTasks() {
            lock (_lock) {
                return _tasks.ToArray();
            }
        }

        protected override void QueueTask(Task packetTask) {
            lock (_lock) {
                //_tasks.AddLast(packetTask);
                _tasks.Enqueue(packetTask);
                TryExecuteTasks();
            }
        }

        protected override bool TryExecuteTaskInline(Task task, bool taskWasPreviouslyQueued) => false;

        public void Enqueue(PlayerSession player, Packet packet) {
            Task task = Task.Factory.StartNew(async () => await PacketProcess(player, packet),
                CancellationToken.None, TaskCreationOptions.None,
                this
                ).Unwrap();
        }

        private void TryExecuteTasks() {
            //while (_runningTasks < _maxParallelism && _tasks.First != null) {
            while (_runningTasks < _maxParallelism && _tasks.Count > 0) {
                Task? task = null;
                bool executeTask = false;

                lock (_lock) {
                    //task = _tasks.First.Value;
                    //_tasks.RemoveFirst();
                    //_runningTasks++;
                    executeTask = _tasks.TryDequeue(out task);
                }

                if (executeTask && task != null) {
                    base.TryExecuteTask(task);
                    TaskCompleted();
                }
            }
        }

        private void TaskCompleted() {
            lock (_lock) {
                _runningTasks--;
                TryExecuteTasks();
            }
        }
        public async Task<bool> PacketProcess(PlayerSession player, Packet packet) {
            if (functionMap.TryGetValue(packet.GetPacketType(), out var packetFunction)) {
                if (packetFunction != null) {
                    return await packetFunction.Invoke(player, packet);
                }
            }
            return false;
        }

        public void RegisterPacketFunction(UInt32 packetType, Func<PlayerSession, Packet, Task<bool>> func) {
            if (!functionMap.ContainsKey(packetType)) {
                functionMap.Add(packetType, func);
            }
        }

        private static async Task<bool> CS_Heartbeat(PlayerSession Player, Packet Pack) {
            return await Task.FromResult(true);
        }
        private static async Task<bool> CS_EchoAsync(PlayerSession Player, Packet pack) {
            //packet.Body;
            PacketCS_Echo packet = new PacketCS_Echo(pack.Body);

            string recvMsg = Encoding.UTF8.GetString(packet.EchoMsg);
            Console.WriteLine($"CS_EchoAsync: {recvMsg}");

            Player.Send(pack);
            return await Task.FromResult(true);
        }
    }
}
