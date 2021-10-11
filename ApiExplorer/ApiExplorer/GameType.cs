using System;
using System.Collections.Generic;
using System.Text;

namespace ApiExplorer
{
    class GameType
    {
        public SortedDictionary<String, List<GameFunction>> Functions { get; set; }
        public SortedDictionary<String, GameMember> Members { get; set; }
        public String Description { get; set; }
        public String Inherits { get; set; }
    }
}
