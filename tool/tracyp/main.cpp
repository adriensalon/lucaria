#include "profiler_tracy.hpp"

int main(int argc, char* argv[])
{
	lucaria::detail::run_tracy_profiler_window("127.0.0.1", 8086);
    return 0;
}
