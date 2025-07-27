using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace PL_Common
{
    public class IPacketHandler : TaskScheduler
    {
        protected readonly int _maxParallelism = 4;
        protected readonly Queue<Task> _tasks = new();
        private int _runningTasks = 0;
        protected readonly object _lock = new();

        protected IPacketHandler(int maxParallelism = 4) {
            _maxParallelism = maxParallelism;
        }

        protected override IEnumerable<Task> GetScheduledTasks() {
            lock (_lock) {
                return _tasks.ToArray();
            }
        }

        protected override void QueueTask(Task packetTask) {
            lock (_lock) {
                _tasks.Enqueue(packetTask);
                TryExecuteTasks();
            }
        }

        protected override bool TryExecuteTaskInline(Task task, bool taskWasPreviouslyQueued) => false;

        void TryExecuteTasks() {
            while (_runningTasks < _maxParallelism && _tasks.Count > 0) {
                Task? task = null;
                bool executeTask = false;

                lock (_lock) {
                    executeTask = _tasks.TryDequeue(out task);
                }

                if (executeTask && task != null) {
                    base.TryExecuteTask(task);
                    TaskCompleted();
                }
            }
        }

        void TaskCompleted() {
            lock (_lock) {
                _runningTasks--;
                TryExecuteTasks();
            }
        }
    }
}
