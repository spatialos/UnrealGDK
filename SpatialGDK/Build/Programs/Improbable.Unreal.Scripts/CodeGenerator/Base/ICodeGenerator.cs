using Improbable.CodeGen.Base;
using System.Collections.Generic;

namespace Improbable.Codegen.Base
{
    public interface ICodeGenerator
    {
        List<GeneratedFile> GenerateFiles(Bundle bundle);
    }

    public class GeneratedFile
    {
        public GeneratedFile(string RelativeFilePath, string Contents)
        {
            this.RelativeFilePath = RelativeFilePath;
            this.Contents = Contents;
        }
        public string RelativeFilePath { get; set; }
        public string Contents { get; set; }
    }
}
