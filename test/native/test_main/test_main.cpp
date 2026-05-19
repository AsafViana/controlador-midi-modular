/**
 * Minimal test to verify the native test environment with Rapidcheck.
 *
 * This file confirms that:
 * 1. Unity test framework compiles and runs in the native environment
 * 2. Rapidcheck library is available and functional
 * 3. Minimum 100 iterations per property test are configured
 */

#include <rapidcheck.h>
#include <unity.h>

void setUp() {}
void tearDown() {}

void test_unity_framework_runs() { TEST_ASSERT_TRUE(true); }

void test_rapidcheck_available() {
  // Verify Rapidcheck can generate values and run a simple property
  // with at least 100 iterations (Rapidcheck default is 100)
  int count = 0;
  const auto result =
      rc::check("trivial property - min 100 iterations", [&count](int x) {
        count++;
        RC_ASSERT(x == x);
      });

  TEST_ASSERT_TRUE_MESSAGE(result, "Rapidcheck property check should pass");
  TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(
      100, count, "Rapidcheck should run at least 100 iterations per property");
}

int main() {
  UNITY_BEGIN();

  RUN_TEST(test_unity_framework_runs);
  RUN_TEST(test_rapidcheck_available);

  return UNITY_END();
}
