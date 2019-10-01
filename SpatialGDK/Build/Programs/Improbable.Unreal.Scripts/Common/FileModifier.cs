// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.IO;
using System.Text;

namespace Improbable.Unreal.Build.Common
{
    public static class FileModifier
    {
        public static bool SetSpatialNetworkingEnabled(string iniPath, bool enabled)
        {
            Console.WriteLine($"Setting bSpatialNetworking in {iniPath} to {enabled}");
            var iniFile = IniFile.ParseIniFile(iniPath);

            string sectionName = "[/Script/EngineSettings.GeneralProjectSettings]";
            string property = "bSpatialNetworking";
            string value = enabled ? "True" : "False";

            string overriddenValue = iniFile.GetSection(sectionName).GetPropertyValue(property)[0];

            iniFile.GetSection(sectionName).OverrideProperty(property, enabled ? "True" : "False");
            iniFile.Save();

            return overriddenValue == "True" ? true : false;
        }
    }
}
