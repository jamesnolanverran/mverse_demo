# Patterns

Mverse does not decide whether a macro is an expression or a statement. It
expands text, then the C compiler decides whether the result is valid.

That sounds small, but it is the rule that explains most macro-writing habits.

## Expression-Shaped Macros

If you want to use a macro where C expects an expression, keep the expansion
expression-shaped:

```c
@def(add(a, b)) {
    (($a) + ($b))
}

int result = @add(1, 2);
```

This works because the expansion is just:

```c
((1) + (2))
```

No local variables. No extra statements. No typed parameters.

## Statement-Shaped Macros

If the macro emits statements, call it where statements are allowed:

```c
@def(log_value(value)) {
    printf("value = %d\n", $value);
}

@log_value(42);
```

That is direct and usually enough.

## Typed Parameters Make A Block

Typed parameters are useful when an argument should be evaluated once:

```c
@def(show(value:int)) {
    printf("%d\n", $value);
}
```

The definition still references a typed parameter as `$value`. Mverse creates
a local named `_value` and substitutes that generated name:

```c
{
    int _value = argument;
    printf("%d\n", _value);
}
```

That automatic block is great for statement-like macros. It also means a typed
macro is not expression-shaped.

## The Extra-Braces Rule

Untyped macros do not automatically get a C scope.

If an untyped macro declares locals or emits several statements, give it a C
block yourself. The common pattern is `{{ ... }}`:

```c
@def(foreach(type, arr, item_name=item, index_name=__mv_i)){{
    size_t __mv_len = ($arr) ? ($arr)->len : 0;
    $type *__mv_items = ($arr) ? ($arr)->items : NULL;

    if (!__mv_items) __mv_len = 0;

    for (size_t $index_name = 0; $index_name < __mv_len; ++$index_name) {
        $type *$item_name = &__mv_items[$index_name];
        $body
    }
}}
```

The first `{` starts the Mverse definition body. The second `{` is literal C
that gets emitted into every expansion.

That block matters when you call the macro twice:

```c
@foreach(int, &numbers) {
    printf("%d\n", *item);
}

@foreach(int, &numbers) {
    printf("again: %d\n", *item);
}
```

Without the emitted C block, both expansions would declare `__mv_len` and
`__mv_items` in the same C scope.

## A Useful Rule Of Thumb

Use these shapes:

- Expression macro: untyped parameters, one expression-shaped body.
- Simple statement macro: statements are fine, especially if no locals collide.
- Local-declaring statement macro: use `{{ ... }}`.
- Typed-argument macro: expect an automatic block and generated `_name` locals.

Mverse is intentionally not hygienic yet. Choose internal names that are
unlikely to collide, and add a C block when a macro owns local variables.
