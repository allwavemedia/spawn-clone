#include <gtest/gtest.h>
#include <algorithm>
#include <string>
#include <vector>
#include "plugin_core/InstrumentLibraryManager.h"

using spawnclone::core::InstrumentLibraryManager;

TEST(InstrumentLibraryManagerTest, StartsEmptyAndClearWorks)
{
    InstrumentLibraryManager mgr;
    EXPECT_TRUE(mgr.listInstruments().empty());

    InstrumentLibraryManager::Instrument a{ "piano01", "Piano", {"keys","Acoustic"} };
    EXPECT_TRUE(mgr.registerInstrument(a));
    EXPECT_FALSE(mgr.listInstruments().empty());

    mgr.clear();
    EXPECT_TRUE(mgr.listInstruments().empty());
}

TEST(InstrumentLibraryManagerTest, RegisterInsertAndUpdate)
{
    InstrumentLibraryManager mgr;

    InstrumentLibraryManager::Instrument a{ "piano01", "Piano", {"Keys","Acoustic"} };
    EXPECT_TRUE(mgr.registerInstrument(a));  // insert

    // Update name and tags for same id
    InstrumentLibraryManager::Instrument a2{ "piano01", "Grand Piano", {"keys","Acoustic","acoustic"} };
    EXPECT_FALSE(mgr.registerInstrument(a2)); // update

    auto got = mgr.getInstrument("piano01");
    ASSERT_TRUE(got.has_value());
    EXPECT_EQ(got->name, "Grand Piano");

    // Tags normalized (lower, dedup, trimmed)
    ASSERT_FALSE(got->tags.empty());
    // Expect "acoustic" and "keys" only
    std::sort(got->tags.begin(), got->tags.end());
    EXPECT_EQ(got->tags.size(), 2u);
    EXPECT_EQ(got->tags[0], "acoustic");
    EXPECT_EQ(got->tags[1], "keys");
}

TEST(InstrumentLibraryManagerTest, ListIsSortedByName)
{
    InstrumentLibraryManager mgr;
    EXPECT_TRUE(mgr.registerInstrument({"b","Brass",{}}));
    EXPECT_TRUE(mgr.registerInstrument({"c","Choir",{}}));
    EXPECT_TRUE(mgr.registerInstrument({"a","Accordion",{}}));

    auto list = mgr.listInstruments();
    ASSERT_EQ(list.size(), 3u);
    EXPECT_EQ(list[0].name, "Accordion");
    EXPECT_EQ(list[1].name, "Brass");
    EXPECT_EQ(list[2].name, "Choir");
}

TEST(InstrumentLibraryManagerTest, FindByTagIsCaseInsensitive)
{
    InstrumentLibraryManager mgr;
    EXPECT_TRUE(mgr.registerInstrument({"vln1","Violin",{"Strings","Solo"}}));
    EXPECT_TRUE(mgr.registerInstrument({"vla1","Viola",{"strings","Section"}}));
    EXPECT_TRUE(mgr.registerInstrument({"tpt1","Trumpet",{"Brass"}}));

    auto strings = mgr.findByTag("STRINGS");
    ASSERT_EQ(strings.size(), 2u);

    // Sorted by name
    EXPECT_EQ(strings[0].name, "Viola");
    EXPECT_EQ(strings[1].name, "Violin");
}

TEST(InstrumentLibraryManagerTest, RemoveById)
{
    InstrumentLibraryManager mgr;
    EXPECT_TRUE(mgr.registerInstrument({"vln1","Violin",{"strings"}}));
    EXPECT_TRUE(mgr.hasInstrument("vln1"));
    EXPECT_TRUE(mgr.removeInstrument("vln1"));
    EXPECT_FALSE(mgr.hasInstrument("vln1"));

    // Removing again should return false
    EXPECT_FALSE(mgr.removeInstrument("vln1"));
}
