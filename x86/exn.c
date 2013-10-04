/**
 * @file x86/exn.c
 *
 * @author David Matlack
 */
#include <x86/exn.h>

struct x86_exn x86_exceptions[] = {
  { 0, "#DE", "Divide Error Fault", 
              X86_FAULT, 
              "DIV or IDIV instructions", 
              false
  },
  { 1, "#DB", "Debug", 
              X86_FAULT | X86_TRAP, 
              "INT 1 instruction", 
              false 
  },
  { 2, "NMI", "Non-Maskable Interrupt", 
              0, 
              "Nonmaskable external interrupt", 
              false 
  },
  { 3, "#BP", "Breakpoint", 
              X86_TRAP, 
              "INT 3 instruction", 
              false 
  },
  { 4, "#OF", "Overflow", 
              X86_TRAP, 
              "INTO instruction", 
              false 
  },
  { 5, "#BR", "BOUND Range Exceeded", 
              X86_FAULT, 
              "BOUND instruction", 
              false 
  },
  { 6, "#UD", "Invalid Opcode", 
              X86_FAULT, 
              "UD2 instruction or reserved opcode", 
              false 
  },
  { 7, "#NM", "Device Not Available (No Math Coprocessor)", 
              X86_FAULT, 
              "Floating-point or WAIT/FWAIT instruction", 
              false 
  },
  { 8, "#DF", "Double Fault", 
              X86_ABORT, 
              "Any instruction that can generate an exception, an NMI, "
                "or an INTR",
              true 
  },
  { 9, "---", "Coprocessor Segment Overrun (reserved)", 
              X86_FAULT, 
              "Floating-point instruction", 
              false 
  },
  { 10, "#TS", "Invalid TSS", 
               X86_FAULT, 
               "Task switch or TSS access", 
               true 
  },
  { 11, "#NP", "Segment Not Present", 
               X86_FAULT, 
               "Loading segment registers or accessing system segments.",
               true 
  },
  { 12, "#SS", "Stack-Segment Fault", 
               X86_FAULT, 
               "Stack operations and SS register", 
               true 
  },
  { 13, "#GP", "General Protection Fault", 
               X86_FAULT, 
               "Any memory reference and other protection checks", 
               true 
  },
  { 14, "#PF", "Page Fault", 
               X86_FAULT, 
               "Any memory reference", 
               true 
  },
  { 15, "---", "Intel reserved", 
               0, 
               "Do not use", 
               false 
  },
  { 16, "#MF", "x87 FPU Floating-Point Error (Math Fault)", 
               X86_FAULT, 
               "x87 FPU floating-point or WAIT/FWAIT instruction", 
               false 
  },
  { 17, "#AC", "Alignment Check", 
               X86_FAULT, 
               "Any data reference in memory", 
               true 
  },
  { 18, "#MC", "Machine Check", 
               X86_ABORT, 
               "Error codes (if any) and source are model dependent", 
               false 
  },
  { 19, "#XF", "SIMD Floating-Point Exception", 
               X86_FAULT, 
               "SSE and SSE2 floating-point instructions", 
               false 
  }

  /*
   * Intel reserves vectors 20-31
   */

  /*
   * User defined vectors 32-255
   */
};

