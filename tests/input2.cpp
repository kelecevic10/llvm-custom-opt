extern "C" int helper(int a, int b) {
  // b je mrtav argument
  return a * 2;
}

extern "C" int compute(int x, int y, int z) {
  // y je mrtav argument
  int t = helper(x, 0);
  return t + z;
}

extern "C" int main() {
  return compute(3, 4, 5);
}
