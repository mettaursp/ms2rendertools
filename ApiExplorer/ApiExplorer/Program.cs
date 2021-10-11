using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Text.Json;
using System.IO;

namespace ApiExplorer
{
    static class Program
    {
        static public ApiData MetaData;

        static String ProcessString(String description)
        {
            return description.Replace("\\r\\n", "\r").Replace("\\n", "\r\n").Replace("\\t", "\t");
        }

        static void Process(SortedDictionary<String, GameType> container, String key)
        {
            GameType type = container[key];

            type.Description = ProcessString(type.Description);

            foreach (KeyValuePair<String, GameMember> entry in type.Members)
                entry.Value.Description = ProcessString(entry.Value.Description);

            foreach (KeyValuePair<String, List<GameFunction>> entry in type.Functions)
            {
                for (int i = 0; i < entry.Value.Count; ++i)
                {
                    entry.Value[i].Description = ProcessString(entry.Value[i].Description);

                    for (int j = 0; j < entry.Value[i].Parameters.Count; ++j)
                        entry.Value[i].Parameters[j].Description = ProcessString(entry.Value[i].Parameters[j].Description);
                }
            }
        }

        /// <summary>
        ///  The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            String jsonData = File.ReadAllText("./apidump.json");

            MetaData = JsonSerializer.Deserialize<ApiData>(jsonData);

            foreach (KeyValuePair<String, List<GameEnum>> entry in MetaData.Enums)
            {
                entry.Value.Sort((x, y) => x.Value.CompareTo(y.Value));
                for (int i = 0; i < entry.Value.Count; ++i)
                    ;
            }

            foreach (KeyValuePair<String, GameType> entry in MetaData.Types)
                Process(MetaData.Types, entry.Key);

            foreach (KeyValuePair<String, GameType> entry in MetaData.Objects)
                Process(MetaData.Objects, entry.Key);

            Application.SetHighDpiMode(HighDpiMode.SystemAware);
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());

            
        }
    }
}
