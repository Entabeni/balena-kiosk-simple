// -*- C++ -*-
// $Id: testsMain.cpp 4759 2008-06-19 19:02:27Z vbuzuev $

// DYMO LabelWriter Drivers
// Copyright (C) 2008 Sanford L.P.

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>


int main(int argc, char* argv[])
{
  // Get the top level suite from the registry
  CPPUNIT_NS::Test *suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  // Adds the test to the list of test to run

  CPPUNIT_NS::TextUi::TestRunner runner;

  runner.addTest(suite);



  // Change the default outputter to a compiler error format outputter

  runner.setOutputter(
    new CPPUNIT_NS::CompilerOutputter(
      &runner.result(),
      std::cerr ));

  // Run the test.

  bool wasSucessful = runner.run();



  // Return error code 1 if the one of test failed.

  return wasSucessful ? 0 : 1;

}

/*
 * End of "$Id: testsMain.cpp 4759 2008-06-19 19:02:27Z vbuzuev $".
 */






