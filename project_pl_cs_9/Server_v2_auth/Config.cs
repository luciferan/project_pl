using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using System.Net;
using PL_Common;

namespace PL_Server_v2_Auth
{
    public class Config
    {
        public InternalConnectionInfo ClientListener { get; set; } = new();
        public InternalConnectionInfo DbServerConnect { get; set; } = new();

        public Config() { }
        public Config(string configPath) { LoadConfig(configPath); }
        public bool LoadConfig(string configPath) {
            try {
                string json = File.ReadAllText(configPath);
                var config = JsonSerializer.Deserialize<Config>(json, new JsonSerializerOptions {
                    PropertyNameCaseInsensitive = true
                });

                if (config != null) {
                    this.ClientListener = config.ClientListener;
                    this.DbServerConnect = config.DbServerConnect;

                    return true;
                }
            } catch (Exception e) {
                Console.WriteLine($"LoadConfig failed {configPath}. Exception: {e.Message}");
            }

            Console.WriteLine("server_config_v2.json 파일을 읽어오지 못했습니다. 기본 설정 파일을 생성합니다.");
            DefaultConfig(configPath);
            return false;
        }
        public void DefaultConfig(string configPath) {
            var config = new Config();
            string json = JsonSerializer.Serialize(config, new JsonSerializerOptions { WriteIndented = true });
            File.WriteAllText(configPath, json);
        }
    }
}
