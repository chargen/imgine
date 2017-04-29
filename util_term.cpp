
#include "util_term.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>

extern "C" {
#include <unistd.h>
}

using std::getenv;
using std::string;

namespace util_term {

/** Check if the current stdout writes to an ANSI-compatible terminal.
 */
bool check_ansi()
{
#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE
    if (isatty(fileno(stdout))) {
        const char *term_p = getenv("TERM");
        if (term_p) {
            string term(term_p);
            if (term != "" && term != "dumb") { // not a dumb terminal
                return true;
            }
        }
    }
#endif
    return false;
}

} // namespace util_term
