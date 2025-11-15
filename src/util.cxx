#include "util.hxx"
#include <csignal>
#include <cstdlib>

void udisks_xplr::util::redrawXplr() {
    // TODO: we don't have a pipe to write to, because those only exist while a process is run by
    // xplr, and communication via socket isn't implemented yet. We also can't call a Lua API
    // because we're on another thread. Since we can't communicate with xplr, we have to hack and
    // kill ourselves with SIGWINCH to make xplr think the window was rezised and then redraw.
    kill(getpid(), SIGWINCH);
}
