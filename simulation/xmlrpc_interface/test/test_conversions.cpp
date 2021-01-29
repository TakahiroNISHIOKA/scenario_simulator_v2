// Copyright 2015-2020 Tier IV, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>

#include <xmlrpc_interface/conversions.hpp>

#include <string>

TEST(Conversion, ConvertInitializeResponse)
{
  simulation_api_schema::InitializeResponse res;
  simulation_api_schema::Result result;
  result.set_description("test");
  result.set_success(true);
  res.set_allocated_result(&result);
  /*
  XmlRpc::XmlRpcValue xml;
  xmlrpc_interfae::fromProto(res, xml);
  std::string description = xml["description"];
  EXPECT_STREQ(description.c_str(), "test");
  */
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
