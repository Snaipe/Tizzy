/*
 * The MIT License (MIT)
 *
 * Copyright © 2016 Franklin "Snaipe" Mathieu <http://snai.pe/>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "addr2line.h"
#include <windows.h>
#include <dbghelp.h>

int pdb_translate(mod_handle *mod, struct addr2line_entry *ln)
{
    int rc = -1;

    DWORD symopts = SYMOPT_DEFERRED_LOADS | SYMOPT_DEBUG | SYMOPT_LOAD_LINES;
    HANDLE process = GetCurrentProcess();

    if (!SymInitialize(process, NULL, FALSE))
        return -1;

    ULONG64 module_base = 0;

    char filename[PATH_MAX];
    filename[PATH_MAX - 1] = '\0';

    GetModuleFileName(*mod, filename, sizeof (filename) - 1);

    SymSetOptions(symopts);
    module_base = SymLoadModuleEx(process, NULL, filename, NULL, 0, 0, NULL, 0);
    if (!module_base)
        goto fail;

    IMAGEHLP_LINE64 line = { .SizeOfStruct = sizeof (IMAGEHLP_LINE64) };
    DWORD column;
    if (!SymGetLineFromAddr64(process, (DWORD64) ln->addr, &column, &line))
        goto fail;

    ln->line = line.LineNumber;
    ln->column = column;
    ln->filename = line.FileName;

    rc = 0;

fail:
    if (module_base)
        SymUnloadModule64(process, module_base);
    SymCleanup(process);
    return rc;
}
