include "llvm/Target/Target.td"
include "STM8InstrInfo.td"
include "STM8CallingConv.td"
include "STM8Schedule.td"

def : ProcessorModel<"generic", STM8SchedModel, []>;

def STM8InstrInfo : InstrInfo;

def STM8AsmParser : AsmParser {
  let ShouldEmitMatchRegisterName = 1;
}

def STM8AsmParserVariant : AsmParserVariant {
  int Variant = 0;

  // Recognize hard coded registers.
  string RegisterPrefix = "";
}

def STM8AsmWriter : AsmWriter {
  string AsmWriterClassName = "InstPrinter";
  bit isMCAsmWriter = 1;
}

def STM8 : Target {
    let InstructionSet = STM8InstrInfo;
    let AssemblyParsers = [STM8AsmParser];
    let AssemblyParserVariants = [STM8AsmParserVariant];
    let AssemblyWriters = [STM8AsmWriter];
    let AllowRegisterRenaming = 0;
}
