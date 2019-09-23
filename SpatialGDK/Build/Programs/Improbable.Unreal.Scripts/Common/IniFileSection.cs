// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace Improbable.Unreal.Build.Common
{
    class IniFileSection
    {
        public string Name { get; private set; }

        private Dictionary<string, List<string>> pairs = new Dictionary<string, List<string>>();
        public IniFileSection(string rawIniSection)
        {
            var section = rawIniSection.Split(Environment.NewLine.ToCharArray()).ToList();
            Name = section[0];

            foreach (var pair in section.Where(pair => pair.Contains("=")))
            {
                var splitted = Regex.Match(pair, @"([^=]+)=(.*)").Groups;
                AddValue(splitted[1].Value, splitted[2].Value);
            }
        }

        public IniFileSection(string name, string key, string value)
        {
            Name = name;
            AddValue(key, value);
        }

        public void AddValue(string key, string value)
        {
            if (!pairs.ContainsKey(key))
            {
                pairs.Add(key, new List<string>());
            }

            if (pairs[key].Count == 1 && pairs[key][0] == "")
            {
                pairs[key][0] = value;
            }
            else
            {
                pairs[key].Add(value);
            }
        }

        public bool HasValue(string key)
        {
            return pairs.ContainsKey(key) && pairs[key][0] != "";
        }

        public override string ToString()
        {
            string str = Name + Environment.NewLine;
            foreach (var pair in pairs)
            {
                foreach (var value in pair.Value)
                {
                    str += pair.Key + '=' + value + Environment.NewLine;
                }
            }
            return str;
        }
    }
}
