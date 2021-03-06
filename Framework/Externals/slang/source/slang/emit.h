// Emit.h
#ifndef SLANG_EMIT_H_INCLUDED
#define SLANG_EMIT_H_INCLUDED

#include "../core/basic.h"

#include "compiler.h"

namespace Slang
{
    class ProgramSyntaxNode;
    class ProgramLayout;

    String emitProgram(
        ProgramSyntaxNode*  program,
        ProgramLayout*      programLayout,
        CodeGenTarget       target);
}
#endif
