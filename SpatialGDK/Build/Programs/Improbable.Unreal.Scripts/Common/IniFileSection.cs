// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.Collections.Generic;

namespace Improbable.Unreal.Build.Common
{
    class IniFileSection
    {
        public string Name { get; private set; }
        private Dictionary<string, List<string>> pairs = new Dictionary<string, List<string>>();

        public IniFileSection(string name)
        {
            Name = name;
        }

        public bool HasProperty(string key)
        {
            return pairs.ContainsKey(key) && pairs[key][0] != "";
        }

        public List<string> GetPropertyValue(string property)
        {
            return pairs[property];
        }

        public void AddPropertyValue(string key, string value)
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

        public void RemoveProperty(string key, string value)
        {
            pairs.Remove(key);
        }

        public void OverrideProperty(string key, string value)
        {
            // does not use RemoveProperty to maintain setting order
            if (!pairs.ContainsKey(key))
            {
                pairs.Add(key, new List<string>());
            }
            else
            {
                pairs[key] = new List<string>();
            }

            pairs[key].Add(value);
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
