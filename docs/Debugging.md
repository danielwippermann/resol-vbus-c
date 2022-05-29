# Debugging

The library includes a minimal debugging functionality that can be enabled by compiling the library with the define `RESOLVBUS_DEBUG=1`.

Every module in the library includes `src/Debug.h`. That file simply checks `RESOLVBUS_DEBUG` and either includes `src/DebugOn.h` (if `RESOLVBUS_DEBUG >= 1`) or `src/DebugOff.h`.

Both `src/Debug{On,Off}.h` provide the same set of macros:

- `__FAIL(ErrorSuffix)`
- `__WRAP(Expression)`
- `__ASSERT_WITH(ErrorSuffix, Expression)`
- `__DLOG(FormatString, ...)`

All of those macros first check, whether the local variable `Result` is still set to `RESOLVBUS_OK`. If it is not, the macro does nothing.


## `__FAIL(ErrorSuffix)`

The `__FAIL` macro:

- checks the local variable `Result` as described above, optionally skipping the other steps below
- sets the local variable `Result` to `RESOLVBUS_ERROR_##ErrorSuffix`
- replaces the contents in the backtrace buffer with information about the failure (if debugging is enabled)


## `__WRAP(Expression)`

The `__WRAP` macro:

- checks the local variable `Result` as described above, optionally skipping the other steps below
- executes the expression
- stores the result of the expression into the local variable `Result`
- if the local variable `Result` now is not `RESOLVBUS_OK` it adds information about the call site to the backtrace buffer (if debugging is enabled)


## `__ASSERT_WITH(ErrorSuffix, Expression)`

The `__ASSERT_WITH` macro:

- checks the local variable `Result` as described above, optionally skipping the other steps below
- executes the expression
- if the result of the expression is false:
    - sets the local variable `Result` to `RESOLVBUS_ERROR_##ErrorSuffix`
    - replaces the contents in the backtrace buffer with information about the failure (if debugging is enabled)


## `__DLOG(FormatString, ...)`

The `__DLOG` macro:

- checks the local variable `Result` as described above, optionally skipping the other steps below
- uses `printf` to print the formatted string to stdout (if debugging is enabled)


## The internal backtrace buffer

Enabling the debugging functionality adds a library-internal backtrace buffer. That buffer is used by the macros `__FAIL`, `__WRAP` and `__ASSERT_WITH` in case an error occurred to track the error's reason and call stack.

Along with that buffer a set of functions is added to work with the backtrace buffer:

```
void ResolVBus_ResetBacktrace(const char *Message, const char *Expression, const char *File, int Line, const char *Func);
void ResolVBus_AddBacktrace(const char *Expression, const char *File, int Line, const char *Func);
const char *ResolVBus_GetBacktrace(void);
void ResolVBus_PrintBacktrace(void);
```
