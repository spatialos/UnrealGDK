// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace Improbable.Unreal.Build.Common
{
    class IniFile
    {
        private string path;
        private List<IniFileSection> sections;
        public IniFile(string path)
        {
            this.path = path;
            string rawIni = File.ReadAllText(path, Encoding.UTF8);

            sections = rawIni.Split('[')
                .Where(rawIniSection => rawIniSection.Contains(']'))
                .Select(rawIniSection => new IniFileSection("[" + rawIniSection)).ToList();
        }

        public void Save()
        {
            File.WriteAllText(path, ToString());
        }

        public void AddSectionValue(string sectionName, string key, string value)
        {
            var section = GetSection(sectionName);
            if (section == null)
            {
                sections.Add(new IniFileSection(sectionName, key, value));
            }
            else
            {
                section.AddValue(key, value);
            }
        }
        public bool HasSectionValue(string sectionName, string key)
        {
            var section = GetSection(sectionName);
            if (section == null)
            {
                return false;
            }
            return section.HasValue(key);
        }
        public override string ToString()
        {
            return String.Join(Environment.NewLine, sections.Select(section => section.ToString()));
        }

        private IniFileSection GetSection(string sectionName)
        {
            return sections.Find(section => section.Name == sectionName);
        }
    }
}
