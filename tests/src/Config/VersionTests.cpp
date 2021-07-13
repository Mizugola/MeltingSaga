#include <catch/catch.hpp>

#include <Config/Version.hpp>

using obe::Config::Version;

TEST_CASE("Version objects", "[obe.Config.Version]")
{
    SECTION("Comparisons")
    {
        SECTION("Equality")
        {
            CHECK(Version(0, 0, 0) == Version(0, 0, 0));
            CHECK(Version(1, 0, 0) != Version(0, 0, 0));
        }
        SECTION("Less")
        {
            CHECK(Version(1, 5, 5) < Version(2, 4, 3));
            CHECK(Version(1, 2, 2) < Version(1, 2, 3));
            CHECK(Version(1, 2, 2) <= Version(1, 2, 2));
        }
        SECTION("Greater")
        {
            CHECK(Version(3, 5, 5) > Version(2, 4, 3));
            CHECK(Version(1, 2, 3) > Version(1, 2, 2));
            CHECK(Version(1, 2, 2) >= Version(1, 2, 2));
        }
    }
    SECTION("Conversion from string")
    {
        CHECK(Version(3, 5, 5) == Version("3.5.5"));
        CHECK(Version(3, 5, 5) == Version("003.005.005"));
        CHECK(Version(3, 5, 5) == Version("v3.5.5"));
        CHECK_THROWS(Version("vv3.5.5"));
        CHECK_THROWS(Version("v3.5.5.5"));
        CHECK_THROWS(Version("x3.5.5"));
    }
}
