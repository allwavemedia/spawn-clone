#include <gtest/gtest.h>

// Forward declaration of the minimal init symbol exposed by plugin_core stub.
namespace spawnclone { void plugin_core_init() noexcept; }

TEST(PluginCoreSmokeTest, InitDoesNotThrow)
{
    // Ensure the core can be initialized and the symbol links correctly.
    ASSERT_NO_THROW({ spawnclone::plugin_core_init(); });
}

// Optional sanity test to keep the suite visible even if init becomes a no-op.
TEST(PluginCoreSmokeTest, Sanity)
{
    EXPECT_TRUE(true);
}
