using System;
using System.Collections.Generic;
using System.Linq;
using System.Security;
using System.Text;
using System.Threading.Tasks;
using System.Text.Json;

namespace Server
{
    internal class Config
    {
        public string Host { get; set; } = "localhost";
        public int Port { get; set; } = 16000;

        public Config() { }
        public Config(string ConfigPath) {
            LoadConfig(ConfigPath);
        }
        public void LoadConfig(string ConfigPath) {
            try {
                string json = File.ReadAllText(ConfigPath);
                var config = JsonSerializer.Deserialize<Config>(json);

                if( config != null) {
                    this.Host = config.Host;
                    this.Port = config.Port;
                }
            } catch {
                Console.WriteLine($"LoadConfig failed {ConfigPath}");
            }
        }
    }
}
