#include "safe-printf.hh"

int main()
{
  int x = 23;
#ifdef OK
  safe_printf("%s -> %d", "hi bob", x);
#else
  // Fail.
  safe_printf("%s -> %d", x, "hi bob");
#endif

  return 0;
}
