using PL_Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace PL_Network_v2
{
    public class Config
    {
        public bool ListenMode { get; set; } = false;
        public string ListenIp { get; set; } = "";
        public int ListenPort { get; set; } = 0;
        public int ListenWaitCount { get; set; } = 0;
        public int HeartbeatSec { get; set; } = 0;
        public int HeartbeatCount { get; set; } = 0;


        public Config() { }
        public Config(string configPath) { LoadConfig(configPath); }
        public bool LoadConfig(string configPath) {
            try {
                string json = File.ReadAllText(configPath);
                var config = JsonSerializer.Deserialize<Config>(json);

                if( config != null) {
                    this.ListenMode = config.ListenMode;

                    this.ListenIp = config.ListenIp;
                    this.ListenPort = config.ListenPort;
                    this.ListenWaitCount = config.ListenWaitCount;
                    this.HeartbeatSec = config.HeartbeatSec;
                    this.HeartbeatCount = config.HeartbeatCount;

                    return true;
                } else {
                    Console.WriteLine("설정 파일을 읽어올수 없습니다. 기본 설정파일을 생성합니다.");
                    DefaultConfig();
                }
            } catch {
                Console.WriteLine($"LoadConfig failed {configPath}");
            }

            return false;
        }

        public void DefaultConfig() {
            var config = new Config();
            string json = JsonSerializer.Serialize(config, new JsonSerializerOptions { WriteIndented = true });
        }
    }
}
