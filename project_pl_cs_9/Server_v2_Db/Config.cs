﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;

namespace PL_Server_v2_Db
{
    public class ConnectionConfig
    {
        public string Ip { get; set; } = "";
        public int Port { get; set; } = 0;
    }

    public class InternalConnectionInfo
    {
        public int Id { get; set; } = 0;
        public string Name { get; set; } = "";
        public ConnectionConfig ConnectionInfo { get; set; } = new();
        public IPAddress GetIp() { return ConnectionInfo.Ip == "Any" ? IPAddress.Any : IPAddress.Parse(ConnectionInfo.Ip); }
        public int GetPort() { return ConnectionInfo.Port; }
    }

    //
    public class Config
    {
        public InternalConnectionInfo AuthListener { get; set; } = new();
        public InternalConnectionInfo WorldListener { get; set; } = new();

        public int MaxBufferSize { get; set; } = 1024 * 10;

        public Config() { }
        public Config(string configPath) { LoadConfig(configPath); }
        public bool LoadConfig(string configPath) {
            try {
                string json = File.ReadAllText(configPath);
                var config = JsonSerializer.Deserialize<Config>(json, new JsonSerializerOptions {
                    PropertyNameCaseInsensitive = true
                });

                if (config != null) {
                    this.AuthListener = config.AuthListener;
                    this.WorldListener = config.WorldListener;

                    return true;
                }
            } catch {
                Console.WriteLine($"LoadConfig failed {configPath}");
            }

            return false;
        }

        public void DefaultConfig(string configPath) {
            var config = new Config();
            string json = JsonSerializer.Serialize(config, new JsonSerializerOptions { WriteIndented = true });
            File.WriteAllText(configPath, json);
        }
    }
}
