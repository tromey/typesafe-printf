#include <type_traits>
#include <stdexcept>
#include <iostream>
#include <tuple>

struct constExprStr
{
  unsigned size_;
  const char *str_;

  template<std::size_t N>
  constexpr constExprStr(const char (&str)[N]) : size_(N - 1), str_(str)
  {
  }

  // Substring.
  constexpr constExprStr(const constExprStr& str, int offset)
  : size_(str.size_ - offset), str_(str.str_ + offset)
  {
  }

  constexpr bool empty() const
  {
    return size_ == 0;
  }

  constexpr char first() const
  {
    return str_[0];
  }

  constexpr constExprStr tail() const
  {
    return constExprStr(*this, 1);
  }
};

template<typename T>
constexpr bool isString()
{
  return std::is_pointer<T>::value
    && std::is_same<typename std::remove_cv<typename std::remove_pointer<T>::type>::type, char>::value;
}

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

constexpr bool checkNoFormatPercent(constExprStr str);

template<typename Arg>
constexpr bool checkFormat(constExprStr str);

template<typename Arg, typename Arg2, typename... Args>
constexpr bool checkFormat(constExprStr str);

constexpr bool
checkNoFormat(constExprStr str)
{
  return str.empty() ? true
    : (str.first() == '%'
       ? checkNoFormatPercent(str.tail())
       : checkNoFormat(str.tail()));
}

constexpr bool
checkNoFormatPercent(constExprStr str)
{
  return str.empty() ? throw std::logic_error("dangling '%'")
    : (str.first() == '%'
       ? checkNoFormat(str.tail())
       : throw std::logic_error("not enough arguments"));
}

template<typename Arg>
constexpr bool
checkFormatPercent(constExprStr str)
{
  return str.empty()
    ? throw std::logic_error("dangling '%'")
    : (checkOneFormat<Arg>(str.first())
       ? checkOneFormat<Arg>(str.first())
       : checkNoFormat(str.tail()));
}

template<typename Arg, typename Arg2, typename... Args>
constexpr bool
checkFormatPercent(constExprStr str)
{
  return str.empty()
    ? throw std::logic_error("dangling '%'")
    : (str.first() == '%'
       ? checkFormat<Arg, Arg2, Args...>(str.tail())
       : (checkOneFormat<Arg>(str.first())
	  ? checkOneFormat<Arg>(str.first())
	  : checkFormat<Arg2, Args...>(str.tail())));
}

template<typename Arg>
constexpr bool
checkFormat(constExprStr str)
{
  return str.empty() ? throw new std::logic_error("too many arguments")
    : (str.first() == '%'
       ? checkFormatPercent<Arg>(str.tail())
       : checkFormat<Arg>(str.tail()));
}

template<typename Arg, typename Arg2, typename... Args>
constexpr bool
checkFormat(constExprStr str)
{
  return str.empty() ? throw new std::logic_error("too many arguments")
    : (str.first() == '%'
       ? checkFormatPercent<Arg, Arg2, Args...>(str.tail())
       : checkFormat<Arg, Arg2, Args...>(str.tail()));
}

template<bool B>
struct DoCheckPrintf
{
};

// template<constExprStr theFormat, typename Arg, typename... Args>
// int checkedPrintf(constExprStr format, Arg arg, Args... args)
// {
//   // std::cerr << checkFormat<Arg, Args...>(format);

//   DoCheckPrintf<checkFormat<Arg, Args...>(theFormat)> checker;

//   return 0;
// }


// #define MyPrintf(fmt, ...) checkedPrintf<fmt>(fmt, __VA_ARGS__)

// struct Formatter
// {
//   constExprStr format_;

//   constexpr Formatter(const constExprStr &f) : format_(f)
//   {
//   }

//   template<typename... T>
//   doFormat(T... args)
//   {
//     DoCheckPrintf<checkFormat<T...>(format_)> checker;

//     // Other stuff.
//   }
// };


// template<typename... T>
// constexpr bool checkFormatz(constExprStr fmt, T... args)
// {
//   return checkFormat<T...>(fmt);
// }

// template<typename R, typename... T>
// struct wat<R (*)(const char *, T...)>
// {
//   static constexpr bool
//   checkFormaty(constExprStr fmt)
//   {
//     return checkFormat<T...>(fmt);
//   }
// };


template<typename... T>
struct helper
{
  static constexpr bool checkFormat(constExprStr str)
  {
    return ::checkFormat<T...>(str);
  }
};

template<typename... T> helper<T...> getUsTheTypeEh(T... args);

// Let's pretend this is the real formatter.
template<typename... T>
void doit(const char *fmt, T... args)
{
}



    // constexpr wat<decltype(doit(format, __VA_ARGS__))> wtf ;		
    // static_assert(wtf.checkFormaty(format, __VA_ARGS__), "die");	


#define PRINTF(format, ...)			\
  do {						\
      static_assert(decltype(getUsTheTypeEh(__VA_ARGS__))::checkFormat(format), "die"); \
    doit(format, __VA_ARGS__);				\
  } while (false)

int main()
{
  // DoCheckPrintf<checkFormat<const char*, int>("%s -> %d")> checkre; // , 23, "hi bob"

  // Formatter f("%s -> %d");
  // f.doFormat(23, "hi bob");

  int x = 23;
  //  PRINTF("%s -> %d", x, "hi bob");
  PRINTF("%s -> %d", "hi bob", x);


  return 0;
}
