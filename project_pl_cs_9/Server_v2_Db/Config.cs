using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using PL_Common;

namespace PL_Server_v2_Db
{
    public class Config
    {
        public Int32 DbId { get; set; } = 0;
        public InternalConnectionInfo AuthListener { get; set; } = new();
        public InternalConnectionInfo WorldListener { get; set; } = new();

        public Config() { }
        public Config(string configPath) => LoadConfig(configPath);
        public bool LoadConfig(string configPath) {
            try {
                string json = File.ReadAllText(configPath);
                var config = JsonSerializer.Deserialize<Config>(json, new JsonSerializerOptions {
                    PropertyNameCaseInsensitive = true
                });

                if (config != null) {
                    this.DbId = config.DbId;
                    this.AuthListener = config.AuthListener;
                    this.WorldListener = config.WorldListener;

                    return true;
                }
            } catch {
                Console.WriteLine($"LoadConfig failed {configPath}");
            }
            Console.WriteLine($"{configPath} 파일을 읽어오지 못했습니다. 기본 설정 파일을 생성합니다.");
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
