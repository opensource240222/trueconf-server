using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;
using System.IO;

using SourceSafeTypeLib;


namespace AutoIncr
{
    
    class Program
    {
        const string using_util = "use AutoIncr $(SolutionDir) $(ProjectDir) <VSS_ini_file>";
        static void Main(string[] args)
        {
            if (args.Length < 3)
            {
                Console.Write(using_util);
                return;
            }
            string project_path = args[1].Substring(args[0].Length - 1);
            string vss_ver_file = project_path  + "version.h";
            string local_ver_file = args[1] + "version.h";
            string vss_ini_path = args[2];
            string vss_root = "";

            if (!File.Exists(args[0] + "mssccprj.scc"))
            {
                Console.Write("mssccprj.scc is absent\n");
                return;
            }
            string[] vss_conf = File.ReadAllLines(args[0] + "mssccprj.scc");
            foreach (string str in vss_conf)
            {
                if (str.IndexOf("SCC_Project_Name") != -1)
                {
                    vss_root = Regex.Match(str, "\"(.+)\"").Groups[1].ToString();
                }
            }
            if (vss_root.Length == 0)
            {
                Console.Write("SCC_Project_Name was not found\n");
                return;
            }

            VSSDatabase vssDatabase = new VSSDatabaseClass();
            VSSItem ver_file_Item = null;
            try
            {
                vssDatabase.Open(vss_ini_path, "", "");                
            }
            catch
            {
                Console.Write("Open VSS DB failed\n");
                return;
            }
            vssDatabase.CurrentProject = vss_root;//+ "/" + project_path; 

            try
            {
                ver_file_Item = vssDatabase.get_VSSItem(vssDatabase.CurrentProject + vss_ver_file, false);                
            }
            catch
            {
                Console.Write("Getting Version.h failed\n");
                return;
            }
            try
            {
                if(2 == ver_file_Item.IsCheckedOut)
                {
                    try
                    {
                        ver_file_Item.UndoCheckout(null, 0);
                    }
                    catch
                    {
                        ;
                    }
                }
                ver_file_Item.Checkout(null,local_ver_file, 0);
            }
            catch
            {
                
                Console.Write("CheckOut failed\n");
                return;
            }
            int curVer = 0;
            if (File.Exists(local_ver_file))
            {
                string[] sLines = File.ReadAllLines(local_ver_file);
                string ksFV = "#define FILEVER";
                string ksSFV = "#define STRFILEVER";

                string[] newVerFileCont = new string[sLines.Length];

                for (int i = 0; i < sLines.Length; i++)
                {
                    string str = sLines[i];
                    if (str.IndexOf(ksFV) != -1)
                    {
                        newVerFileCont[i] = str.Substring(0, str.LastIndexOf(",") + 1) +
                            Convert.ToString(curVer = Convert.ToInt32(str.Substring(str.LastIndexOf(",") + 1)) + 1);

                    }
                    else if (str.IndexOf(ksSFV) != -1)
                    {
                        Int32 m = Convert.ToInt32(Regex.Match(str, @"(\d+)\\").Groups[1].ToString()) + 1;
                        newVerFileCont[i] = Regex.Replace(str, @"(\d+)(\\)", m + "$2");
                    }
                    else
                        newVerFileCont[i] = str;

                }
                File.WriteAllLines(local_ver_file, newVerFileCont);                
                try
                {
                    ver_file_Item.Checkin("Ver. " + curVer, null, 0);
                }
                catch
                {
                    Console.Write("CheckIn failed\n");
                    return;
                }
                Console.Write("Version was incremented. Current Build = " + curVer);

            }
        }
        private static void AddText(FileStream fs, string value)
        {
            byte[] info = new UTF8Encoding(true).GetBytes(value);
            fs.Write(info, 0, info.Length);
        }
    }
}

