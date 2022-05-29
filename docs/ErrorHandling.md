# Error handling

Most of the functions in this library return a `RESOLVBUS_RESULT` which is an integer type under the hood. If the function did not encounter any error, the result will be `RESOLVBUS_OK` which is guaranteed to be `0`. All other values are positive integers greater than zero. See `include/ResolVBus.h` for a list of possible values.

Error propagation could look like this:

```
RESOLVBUS_RESULT SomeEventHandler(...)
{
    RESOLVBUS_RESULT Result = 0;

    if (Result == 0) {
        Result = DoFirstThing();
    }

    if (Result == 0) {
        Result = DoSecondThing();
    }

    // ...

    return Result;
}
```

Internally the library uses a macro for error propagation: `__WRAP(Expression)`, defined in the respective `src/Debug{On,Off}.h`. It:

- does nothing if the `Result` local variable already is not equal to `RESOLVBUS_OK`
- otherwise:
    - executes the expression and assigns its result to the `Result` local variable
    - if debugging is enabled and executing the expression resulted in an error, adds a backtrace info

See [Debugging](./Debugging.md) for details.

Inside the library the example above would look like this:

```
RESOLVBUS_RESULT SomeEventHandler(...)
{
    RESOLVBUS_RESULT Result = 0;

    __WRAP(DoFirstThing());

    __WRAP(DoSecondThing());

    // ...

    return Result;
}
```
