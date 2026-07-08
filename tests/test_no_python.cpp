#include <cassert>
#include <cstdlib>

int main() {
  const int result = std::system("bash ./scripts/verify-no-" "python.sh");
  assert(result == 0);
  return 0;
}
