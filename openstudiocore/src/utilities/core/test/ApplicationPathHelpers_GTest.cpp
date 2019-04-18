/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) 2008-2019, Alliance for Sustainable Energy, LLC, and other contributors. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
*  following conditions are met:
*
*  (1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
*  disclaimer.
*
*  (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
*  disclaimer in the documentation and/or other materials provided with the distribution.
*
*  (3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote products
*  derived from this software without specific prior written permission from the respective party.
*
*  (4) Other than as required in clauses (1) and (2), distributions in any form of modifications or other derivative works
*  may not use the "OpenStudio" trademark, "OS", "os", or any other confusingly similar designation without specific prior
*  written permission from Alliance for Sustainable Energy, LLC.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) AND ANY CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER(S), ANY CONTRIBUTORS, THE UNITED STATES GOVERNMENT, OR THE UNITED
*  STATES DEPARTMENT OF ENERGY, NOR ANY OF THEIR EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
*  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
*  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************************************************************/

#include <gtest/gtest.h>

#include "../ApplicationPathHelpers.hpp"
#include "../PathHelpers.hpp"
#include "../Logger.hpp"
#include <stdlib.h>
#include <thread>

using namespace openstudio;

TEST(ApplicationPathHelpers, Strings)
{
  path applicationPath = getApplicationPath();
  path applicationDir = getApplicationDirectory();
  path openstudioModule = getOpenStudioModule();
  path openstudioModuleDirectory = getOpenStudioModuleDirectory();
  path openstudioCLI = getOpenStudioCLI();
  path energyplusDirectory = getEnergyPlusDirectory();
  path energyplusExecutable = getEnergyPlusExecutable();

  EXPECT_TRUE(exists(applicationPath));
  EXPECT_TRUE(exists(applicationDir));
  EXPECT_TRUE(exists(openstudioModule));
  EXPECT_TRUE(exists(openstudioModuleDirectory));
  EXPECT_TRUE(exists(openstudioCLI));
  EXPECT_TRUE(exists(energyplusDirectory));
  EXPECT_TRUE(exists(energyplusExecutable));
}



// This is really in PathHelpers but used by ApplicationPathHelpers
TEST(ApplicationPathHelpers, findInSystemPath) {

  openstudio::path dummy_dir = getApplicationBuildDirectory() / toPath("Testing/Temporary");
  openstudio::filesystem::create_directories(dummy_dir);
  openstudio::path dummy_file_path = dummy_dir / "dummy_file";

  boost::filesystem::ofstream outfile(dummy_file_path);
  outfile.close();

  // This should fail
  openstudio::path absolute_path_to_dummy = findInSystemPath(dummy_file_path.filename());
  EXPECT_FALSE(exists(absolute_path_to_dummy));

  // Append the containing dir to the PATH
  auto current_path = std::getenv("PATH");
  std::string new_path = toString(dummy_dir) + ":" + current_path;
  setenv("PATH", new_path.c_str(), 1);
  setenv("PathWhenSymlinkInPathUnixOnly_Setup", "true", 1);

  // Locate the file with only its name
  absolute_path_to_dummy = findInSystemPath(dummy_file_path.filename());
  EXPECT_TRUE(exists(absolute_path_to_dummy));
  EXPECT_EQ(dummy_file_path, absolute_path_to_dummy);

  // Put it back
  setenv("PATH", current_path, 1);
  boost::filesystem::remove(dummy_file_path);

}

#if !defined(_WIN32)
// From PathHelpers too. Ensure that completeAndNormalize keeps resolving symlinks until found
TEST(ApplicationPathHelpers, completeAndNormalizeMultipleSymlinks) {

  std::vector<openstudio::path> toClean;
  openstudio::path path_dir = getApplicationBuildDirectory() / toPath("Testing/Temporary");
  openstudio::path path_subdir = path_dir / toPath("SubDir");

  openstudio::filesystem::create_directories(path_subdir);

  openstudio::path ori_path = path_dir / "completeAndNormalize_oriFile";
  toClean.push_back(ori_path);

  // 'touch'
  boost::filesystem::ofstream outfile(ori_path);
  outfile.close();

  openstudio::path prev_path = ori_path;

  // First one, same directory, absolute
  openstudio::path symlink_path = path_dir / toPath("1.symlink_absolute_same_dir");
  openstudio::path pointsTo = prev_path;
  boost::filesystem::create_symlink(pointsTo, symlink_path);
  prev_path = symlink_path;
  toClean.push_back(symlink_path);

  // Subdir, absolute
  symlink_path = path_subdir / toPath("2.symlink_absolute_subdir");
  pointsTo = prev_path;
  boost::filesystem::create_symlink(pointsTo, symlink_path);
  prev_path = symlink_path;
  toClean.push_back(symlink_path);

  // Same subdir, relative
  symlink_path = path_subdir / toPath("3.symlink_relative_same_subdir");
  pointsTo = prev_path.filename();
  boost::filesystem::create_symlink(pointsTo, symlink_path);
  prev_path = symlink_path;
  toClean.push_back(symlink_path);

  // Parent dir, relative
  symlink_path = path_dir / toPath("4.symlink_relative_pardir");
  // Just for fun, adding an uncessary "dir/../dir/" portion
  pointsTo = path_subdir.filename() / toPath("..") / path_subdir.filename() / prev_path.filename();
  boost::filesystem::create_symlink(pointsTo, symlink_path);
  prev_path = symlink_path;
  toClean.push_back(symlink_path);

  // Subdir, relative
  symlink_path = path_subdir / toPath("5.symlink_relative_pardir");
  pointsTo = toPath("..") / prev_path.filename();
  boost::filesystem::create_symlink(pointsTo, symlink_path);
  prev_path = symlink_path;
  toClean.push_back(symlink_path);


  openstudio::path foundPath = completeAndNormalize(symlink_path);
  EXPECT_TRUE(exists(foundPath));
  EXPECT_EQ(toString(ori_path), toString(foundPath));

  for (const auto& p: toClean) {
    // boost::filesystem::remove(p);
  }


}

void launch_another_instance_from_symlink(const path& symlink_path) {
  std::string cmd = toString(symlink_path.filename()) + " --gtest_filter=*PathWhenSymlinkInPathUnixOnly_Run*";
  std::system(cmd.c_str());
}

TEST(ApplicationPathHelpers, PathWhenSymlinkInPathUnixOnly_Setup) {

  openstudio::path symlink_path_dir = getApplicationBuildDirectory() / toPath("Testing/Temporary");
  openstudio::path symlink_path = symlink_path_dir / "openstudio_utilities_tests_symlink";
  openstudio::filesystem::create_directories(symlink_path_dir);

  openstudio::path openstudioUtilitiesTestPath = getApplicationBuildDirectory() / toPath("Products/openstudio_utilities_tests");
  if (exists(symlink_path)) {
    boost::filesystem::remove(symlink_path);
  }
  boost::filesystem::create_symlink(openstudioUtilitiesTestPath, symlink_path);

  auto current_path = std::getenv("PATH");
  std::string new_path = toString(symlink_path_dir) + ":" + current_path;

  setenv("PATH", new_path.c_str(), 1);
  setenv("PathWhenSymlinkInPathUnixOnly_Setup", "true", 1);

  // We run a system call to the symlink name (not its path), with a gtest_filter set to PathWhenSymlinkInPathUnixOnly_Run
  // and we ensure we actually can get the right executable
  std::thread t(launch_another_instance_from_symlink, symlink_path);
  t.join();

  unsetenv("PathWhenSymlinkInPathUnixOnly_Setup");
  setenv("PATH", current_path, 1);
}

TEST(ApplicationPathHelpers, Dumb_test) {
    path openstudioModulePath = getOpenStudioModule();
    EXPECT_TRUE(exists(openstudioModulePath));
    // The expected path is the utilities one, but resolved for symlinks (we don't want to hardcode the version eg openstudio_utilities_tests-2.8.0)
    openstudio::path expectedOpenstudioModulePath = getApplicationBuildDirectory() / toPath("Products/openstudio_utilities_tests");
    expectedOpenstudioModulePath = completeAndNormalize(expectedOpenstudioModulePath);
    EXPECT_EQ(toString(expectedOpenstudioModulePath), toString(openstudioModulePath));
}

TEST(ApplicationPathHelpers, PathWhenSymlinkInPathUnixOnly_Run) {

  openstudio::Logger::instance().standardOutLogger().enable();
  openstudio::Logger::instance().standardOutLogger().setLogLevel(Trace);

  //EXPECT_TRUE(false) << std::getenv("PATH");
  //EXPECT_TRUE(false) << std::getenv("PathWhenSymlinkInPathUnixOnly_Setup");
  if (std::getenv("PathWhenSymlinkInPathUnixOnly_Setup")) {
    path openstudioModulePath = getOpenStudioModule();
    EXPECT_TRUE(exists(openstudioModulePath));
     // The expected path is the utilities one, but resolved for symlinks (we don't want to hardcode the version eg openstudio_utilities_tests-2.8.0)
    openstudio::path expectedOpenstudioModulePath = getApplicationBuildDirectory() / toPath("Products/openstudio_utilities_tests");
    expectedOpenstudioModulePath = completeAndNormalize(expectedOpenstudioModulePath);
    EXPECT_EQ(toString(expectedOpenstudioModulePath), toString(openstudioModulePath));
  }

  openstudio::Logger::instance().standardOutLogger().disable();

}
#endif
