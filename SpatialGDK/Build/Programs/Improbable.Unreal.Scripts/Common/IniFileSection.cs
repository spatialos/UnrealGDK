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

        public bool HasProperty(string property)
        {
            return pairs.ContainsKey(property) && pairs[property][0] != "";
        }

        public List<string> GetPropertyValue(string property)
        {
            return pairs[property];
        }

        public void AddPropertyValue(string property, string value)
        {
            if (!pairs.ContainsKey(property))
            {
                pairs.Add(property, new List<string>());
            }

            if (pairs[property].Count == 1 && pairs[property][0] == "")
            {
                pairs[property][0] = value;
            }
            else
            {
                pairs[property].Add(value);
            }
        }

        public void RemoveProperty(string property, string value)
        {
            pairs.Remove(property);
        }

        public void OverrideProperty(string property, string value)
        {
            // does not use RemoveProperty to maintain setting order
            if (!pairs.ContainsKey(property))
            {
                pairs.Add(property, new List<string>());
            }
            else
            {
                pairs[property] = new List<string>();
            }

            pairs[property].Add(value);
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
