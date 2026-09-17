#include "../ezstr.h"

int main(int argc, char *argv[]) {
  ezstr::StringView sv("mr to koks");
  ezstr::String str(sv);
  // ezstr::String str2 = str; // Doesn't work, as expected
  // ezstr::String str2(str); // Same here
  ezstr::String str2(str.clone());
  str2 = str.clone();
  // str2 = str; // Here too
  return 0;
}
