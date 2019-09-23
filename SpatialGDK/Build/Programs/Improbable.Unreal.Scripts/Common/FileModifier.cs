// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.IO;
using System.Text;

namespace Improbable.Unreal.Build.Common
{
    public static class FileModifier
    {
        public static void AddTcpPluginListener(string engineIniPath)
        {
            Console.WriteLine("Adding Tcp Plugin Listener for engine ini at path: " + engineIniPath);
            var engineIni = new IniFile(engineIniPath);

            string sectionName = "[/Script/TcpMessaging.TcpMessagingSettings]";
            string key = "ListenEndpoint";
            string value = "0.0.0.0:6667";
            if (!engineIni.HasSectionValue(sectionName, key))
            {
                engineIni.AddSectionValue(sectionName, key, value);
                engineIni.Save();
            }
        }
    }
}
