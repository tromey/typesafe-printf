# Type-safe printf for C++

This is my attempt at writing a type-safe `printf` for C++.

It is currently minimal.  It handles formats, like `%s`, and
type-checks these.  However it does not handle all the other features
of `printf`, such as width and precision.  There is nothing preventing
this, I just haven't done it yet.

This isn't runtime type-safe.  This is another feature that `printf`
ought to have, to support localization.  That is, it should statically
type-check the canonical string; and then at runtime ensure that the
string taken from the message catalog is also correct.  This is
readily doable by making the real formatting function a variadic
template function; I just haven't done it.

Type safety can eliminate some of the oddities of `printf`.  For
example there's no reason to differentiate between the different
integer widths -- `%d` can just handle all of them.  Likewise `%s`
could handle `char *`, `char16_t *`, `std::string`, and so forth.
With some changes to make the implementation more template-based, this
could be user-extensible as well.

## See It Work

From this source (see `test.cc`):

```
int x;
...
safe_printf("%s -> %d", x, "hi bob");
```

We get this from gcc:

```
In file included from test.cc:1:0:
test.cc: In function ‘int main()’:
safe-printf.hh:199:5: error: non-constant condition for static assertion
     static_assert(decltype(getWrapper(__VA_ARGS__))::checkFormat(format), \
     ^
test.cc:10:3: note: in expansion of macro ‘safe_printf’
   safe_printf("%s -> %d", x, "hi bob");
   ^
test.cc:10:3:   in constexpr expansion of ‘safe_printf::impl::checkFormatWrapper<T>::checkFormat<{int, const char*}>(safe_printf::impl::constExprStr("%s -> %d"))’
safe-printf.hh:183:45:   in constexpr expansion of ‘safe_printf::impl::checkFormat<int, const char*, {}>(str)’
safe-printf.hh:172:45:   in constexpr expansion of ‘safe_printf::impl::checkFormatPercent<int, const char*, {}>(str.safe_printf::impl::constExprStr::tail())’
safe-printf.hh:148:27:   in constexpr expansion of ‘safe_printf::impl::checkOneFormat<int>(((int)str.safe_printf::impl::constExprStr::first()))’
safe-printf.hh:87:58: error: expression ‘<throw-expression>’ is not a constant-expression
      : throw std::logic_error("argument to %s not string"))
                                                          ^
```

## C++ Thoughts

GCC comes with format-checking code.  With C++11, this can be replaced
with metaprogramming.  However, it's pretty difficult.

C++ compilers need to come with a debugger for template and
`constexpr` metaprogramming.  Even logging-style debugging would be
better than what there is now; but of course as programs grow, real
interactive debugging will be needed.

One concrete thing that could be done immediately would be to emit the
argument values to `constexpr` functions when printing an error.

That said, we're also going to need a way for `constexpr` programs to
control the errors that are emitted more directly.

There are weird corners of the language which need to be filled.  See
below for a few specifics.

## Implementation

This mostly uses `constexpr` but there are also some other fun hacks:

* Check out the way that `checkFormatWrapper` works to extract the
  parameter pack.  Maybe this is well-known, I don't know, but I
  thought it was cool.

* If `template<char... Chars> operator "" _mumble` worked for string
  constants, this feature could be implemented using templates.  This
  would be better in many ways, but unfortunately this can't be done
  yet.  Let's hope for C++17.

* A macro is needed because I couldn't think of another way to make
  the format be both statically checked and passed to a
  non-`constexpr` function.  I'd love to hear a solution.

* Note that at the end I had to use a GNU extension, because the
  `__VA_ARGS__` botch still has not been fixed.  I guess I could fix
  this with an indirection, but I couldn't be bothered.
