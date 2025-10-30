extern "C" int sum(int a, int b) {
  return a + b;
}

extern "C" int product(int x, int y) {
  return x * y;
}

extern "C" int main() {
  int s = sum(2, 3);
  int p = product(s, 4);
  return p + 1;
}
