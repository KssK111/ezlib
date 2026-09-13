#include <cstdio>
#define EZSTR_IMPLEMENTATION
#include "../ezstr.h"

int main(int argc, char *argv[]) {
  ezstr::StringView sv("\t\n     Kocham mysteeeeee    \t     ");
  auto wiadomosc = sv.trim().replace("e", "r");
  auto fmt = "To będą moje ostatnie słowa:\n|%s|";
  printf(fmt, wiadomosc->ptr());
  return 0;
}
