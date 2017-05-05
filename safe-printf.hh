#ifndef SAFE_PRINTF_HH
#define SAFE_PRINTF_HH

#include <type_traits>
#include <stdexcept>

namespace safe_printf
{
  // Naturally you should not refer to this stuff directly.
  namespace impl
  {

    // A constexpr string.
    class constExprStr
    {
      unsigned size_;
      const char *str_;

    public:
      template<std::size_t N>
      constexpr constExprStr(const char (&str)[N])
      : size_(N - 1), str_(str)
      {
      }

      // Substring.
      constexpr constExprStr(const constExprStr& str, int offset)
      : size_(str.size_ - offset), str_(str.str_ + offset)
      {
      }

      constexpr bool empty() const { return size_ == 0; }
      constexpr char first() const { return str_[0]; }
      constexpr constExprStr tail() const { return constExprStr(*this, 1); }
    };

    // Helper function to detect if a type is string-like.
    template<typename T>
    constexpr bool isString()
    {
      return std::is_pointer<T>::value
	// Am I the only one who gets tired of sprinkling typename
	// everywhere?
	&& std::is_same<typename std::remove_cv<typename std::remove_pointer<T>::type>::type, char>::value;
    }

    // Check the type T against the format character C.  This has to
    // be a giant constexpr function; but it would be good to split
    // out some of the details into a set of templates, so that users
    // can provide some suitable conversions.
    template<typename T>
    constexpr bool checkOneFormat(char c)
    {
      // Hahahahaha.
      return
	(c == 'd'
	 ? ((std::is_integral<T>::value || std::is_enum<T>::value)
	    ? true
	    : throw std::logic_error("argument to %d not integral"))
	 : (c == 'i'
	    ? ((std::is_integral<T>::value || std::is_enum<T>::value)
	       ? true
	       : throw std::logic_error("argument to %i not integral"))
	    : (c == 'x'
	       ? ((std::is_integral<T>::value || std::is_enum<T>::value)
		  ? true
		  : throw std::logic_error("argument to %x not integral"))
	       : (c == 'X'
		  ? ((std::is_integral<T>::value || std::is_enum<T>::value)
		     ? true
		     : throw std::logic_error("argument to %X not integral"))
		  : (c == 'u'
		     ? ((std::is_integral<T>::value || std::is_enum<T>::value)
			? true
			: throw std::logic_error("argument to %u not integral"))
		     : (c == 'o'
			? ((std::is_integral<T>::value || std::is_enum<T>::value)
			   ? true
			   : throw std::logic_error("argument to %o not integral"))
			: (c == 'c'
			   ? ((std::is_integral<T>::value || std::is_enum<T>::value)
			      ? true
			      : throw std::logic_error("argument to %c not integral"))
			   : (c == 's'
			      ? (isString<T>()
				 ? true
				 : throw std::logic_error("argument to %s not string"))
			      : (c == 'p'
				 ? (std::is_pointer<T>::value
				    ? true
				    : throw std::logic_error("argument to %p not pointer"))
				 : throw std::logic_error("unknown format type"))))))))));
    }

    // Some forward declarations.

    constexpr bool checkNoFormatPercent(constExprStr str);
    template<typename Arg> constexpr bool checkFormat(constExprStr str);

    template<typename Arg, typename Arg2, typename... Args>
    constexpr bool checkFormat(constExprStr str);


    // Check that STR does not contain a format directive.
    constexpr bool
    checkNoFormat(constExprStr str)
    {
      return str.empty() ? true
	: (str.first() == '%'
	   ? checkNoFormatPercent(str.tail())
	   : checkNoFormat(str.tail()));
    }

    // Check that STR does not contain a format directive, after a "%"
    // has been seen.
    constexpr bool
    checkNoFormatPercent(constExprStr str)
    {
      return str.empty() ? throw std::logic_error("dangling '%'")
	: (str.first() == '%'
	   ? checkNoFormat(str.tail())
	   : throw std::logic_error("not enough arguments"));
    }

    // Check the character after a "%".  This is the base case for the
    // recursion.
    template<typename Arg>
    constexpr bool
    checkFormatPercent(constExprStr str)
    {
      return str.empty()
	? throw std::logic_error("dangling '%'")
	: (checkOneFormat<Arg>(str.first())
	   && checkNoFormat(str.tail()));
    }

    // Check the character after a "%" and then proceed to check the
    // remaining arguments.
    template<typename Arg, typename Arg2, typename... Args>
    constexpr bool
    checkFormatPercent(constExprStr str)
    {
      return str.empty()
	? throw std::logic_error("dangling '%'")
	: (str.first() == '%'
	   ? checkFormat<Arg, Arg2, Args...>(str.tail())
	   : (checkOneFormat<Arg>(str.first())
	      && checkFormat<Arg2, Args...>(str.tail())));
    }

    // Check the first character of STR.  This is the base case of the
    // recursion.
    template<typename Arg>
    constexpr bool
    checkFormat(constExprStr str)
    {
      return str.empty() ? throw std::logic_error("too many arguments")
	: (str.first() == '%'
	   ? checkFormatPercent<Arg>(str.tail())
	   : checkFormat<Arg>(str.tail()));
    }

    // Check the first character of STR.
    template<typename Arg, typename Arg2, typename... Args>
    constexpr bool
    checkFormat(constExprStr str)
    {
      return str.empty() ? throw std::logic_error("too many arguments")
	: (str.first() == '%'
	   ? checkFormatPercent<Arg, Arg2, Args...>(str.tail())
	   : checkFormat<Arg, Arg2, Args...>(str.tail()));
    }

    // A wrapper for checkFormat that lets us easily compute the
    // template parameters we need.
    template<typename... T>
    struct checkFormatWrapper
    {
      static constexpr bool checkFormat(constExprStr str)
      {
	return safe_printf::impl::checkFormat<T...>(str);
      }
    };

    // We don't need a definition for this, just a declaration.  We
    // use decltype on this to get the type of the wrapper for the
    // actual arguments.
    template<typename... T> checkFormatWrapper<T...> getWrapper(T... args);
  }
}

// This is what you should call.  This has to be a macro due to
// limitations in C++11.
#define safe_printf(format, ...)					\
  do {									\
    using namespace safe_printf::impl;					\
    static_assert(decltype(getWrapper(__VA_ARGS__))::checkFormat(format), \
		  "safe_printf type-check failed");			\
    /* Because __VA_ARGS__ is still broken, we use the GNU pasting	\
       extension here.  */						\
    printf (format, ## __VA_ARGS__);					\
  } while (false)

#endif // SAFE_PRINTF_HH
