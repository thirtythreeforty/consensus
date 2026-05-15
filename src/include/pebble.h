#pragma once

/* Wrapper around the Pebble SDK pebble.h.
 *
 * The SDK includes a file called pebble_warn_unsupported_functions.h which
 * #defines macros like printf, fopen, etc.  Those macros break libc and C++
 * standard library headers when they are included later in the translation
 * unit.
 *
 * This wrapper is placed in the project's include/ directory so the compiler
 * finds it BEFORE the SDK's pebble.h (thanks to -I ordering).  We pull in the
 * real SDK header with #include_next and then clean up the problematic macros
 * so the rest of the compile is safe.
 */

#include_next "pebble.h"

#undef printf
#undef fopen
#undef fclose
#undef fread
#undef fwrite
#undef fseek
#undef ftell
#undef fsetpos
#undef fscanf
#undef fgetc
#undef fgets
#undef fputc
#undef fputs
#undef fprintf
#undef sprintf
#undef vfprintf
#undef vsprintf
#undef vsnprintf
#undef open
#undef close
#undef creat
#undef read
#undef write
#undef stat
#undef alloca
#undef mmap
#undef brk
#undef sbrk
