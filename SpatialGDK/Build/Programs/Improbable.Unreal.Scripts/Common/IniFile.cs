// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

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
                .Select(rawIniSection => ParseIniFileSection("[" + rawIniSection)).ToList();
        }

        public static IniFile ParseIniFile(string path)
        {
            IniFile newIniFile = new IniFile(path);
            string rawIni = File.ReadAllText(path, Encoding.UTF8);

            newIniFile.sections = rawIni.Split('[')
                .Where(rawIniSection => rawIniSection.Contains(']'))
                .Select(rawIniSection => new IniFileSection("[" + rawIniSection)).ToList();

            return newIniFile;
        }

        public IniFileSection GetSection(string sectionName)
        {
            return sections.Find(section => section.Name == sectionName);
        }

        public void AddSection(string sectionName)
        {
            var section = GetSection(sectionName);
            if (section == null)
            {
                sections.Add(new IniFileSection(sectionName));
            }
        }

        public override string ToString()
        {
            return String.Join(Environment.NewLine, sections.Select(section => section.ToString()));
        }

        public void Save()
        {
            File.WriteAllText(path, ToString());
        }

        private static IniFileSection ParseIniFileSection(string rawIniSection)
        {
            var section = rawIniSection.Split(Environment.NewLine.ToCharArray()).ToList();
            var newSection = new IniFileSection(section[0]);

            foreach (var pair in section.Where(pair => pair.Contains("=")))
            {
                var splitted = Regex.Match(pair, @"([^=]+)=(.*)").Groups;
                newSection.AddPropertyValue(splitted[1].Value, splitted[2].Value);
            }

            return newSection;
        }
    }
}
