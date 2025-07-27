using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using PL_Network_v2;

namespace PL_Server_v2_World
{
    public class DbSession : NetSession
    {
        public DbSession(Socket socket, int recvBufferSize = 1024 * 10) : base(socket, recvBufferSize) {
        }
        protected override bool DataParsing(NetSession session, byte[] buffer, int offset, int transferred) {
            return true;
        }
    }
}
