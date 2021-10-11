using System;
using System.Collections.Generic;
using System.Text;

namespace ApiExplorer
{
    class GameFunction
    {
        public class Parameter
        {
            public String Description { get; set; }
            public String Name { get; set; }
            public String Type { get; set; }
            public String DefaultValue { get; set; }
        }

        public String ReturnType { get; set; }
        public String ReturnValue { get; set; }
        public String InheritedFrom { get; set; }
        public String Description { get; set; }
        public int IsStatic { get; set; }
        public List<Parameter> Parameters { get; set; }
    }
}
