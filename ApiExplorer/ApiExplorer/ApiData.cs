using System;
using System.Collections.Generic;
using System.Text;

namespace ApiExplorer
{
    class ApiData
    {
        public SortedDictionary<String, List<GameEnum>> Enums { get; set; }
        public SortedDictionary<String, GameType> Types { get; set; }
        public SortedDictionary<String, GameType> Objects { get; set; }
    }
}
