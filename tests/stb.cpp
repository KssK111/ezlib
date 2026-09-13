#include "../ezstr.h"

int main(int argc, char *argv[]) {
  ezstr::StringView x("pozdro mr");
  return 0;
}

#define EZSTR_IMPLEMENTATION
#include "../ezstr.h"
