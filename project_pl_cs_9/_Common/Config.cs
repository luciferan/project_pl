using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace PL_Common
{
    public class ConnectionConfig
    {
        public string Ip { get; set; } = "";
        public Int32 Port { get; set; } = 0;
    }
    
    public class InternalConnectionInfo
    {
        public Int32 Id { get; set; } = 0;
        public string Name { get; set; } = "";
        public ConnectionConfig ConnectionInfo { get; set; } = new();
        public Int32 MaxBufferSize { get; set; } = 0;
        public IPAddress GetIp() { return ConnectionInfo.Ip == "Any" ? IPAddress.Any : IPAddress.Parse(ConnectionInfo.Ip); }
        public int GetPort() { return ConnectionInfo.Port; }
    }
}
