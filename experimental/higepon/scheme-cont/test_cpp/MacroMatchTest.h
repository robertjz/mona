#ifndef _MACRO_MATCH_TEST_H_
#define _MACRO_MATCH_TEST_H_

#include <scheme.h>
#include "yaml.h"
#include <cppunit/extensions/HelperMacros.h>

class MacroMatchTest : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(MacroMatchTest);
    CPPUNIT_TEST(testMatch);
    CPPUNIT_TEST(testUnmatch);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp();
    void tearDown();
    void testMatch();
    void testUnmatch();

private:
    void assertMacroMatch(const std::string& macroName, const std::string& macro, const std::string& words, const std::string& target, bool matchOrNot = true);
};

#endif // _MACRO_MATCH_TEST_H_