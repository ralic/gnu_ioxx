#include <adns.h>
#include <sstream>
#include <vector>
#include <cassert>
#include <iostream>

void log(adns_state ads, void * ctx, char const * fmt, va_list args)
{
  static std::ostringstream os;
  static std::vector<char>  buf(1024u);
  int rc;
  for(;;)
  {
    rc = vsnprintf(&buf[0], buf.size(), fmt, args);
    assert(rc >= 0);
    if (static_cast<size_t>(rc) < buf.size()) break;
    buf.resize(rc + 1);
  }
  os.write(&buf[0], rc);
  assert(rc > 0);
  if (buf[rc - 1] == '\n')
  {
    std::cout << "*** " << os.str();
    os.str("");
  }
}

int main(int, char**)
{
  using namespace std;
  adns_state st;
  int const rc( adns_init_logfn(&st, adns_if_debug, NULL, &log, NULL) );
  if (rc != 0)
    cerr << "cannot initialize adns" << endl;
  else
    adns_finish(st);
  return 0;
}
