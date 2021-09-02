// Copyright 2015-2021 Tier IV, Inc. All rights reserved.
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

#ifndef OPENSCENARIO_INTERPRETER__SYNTAX__USER_DEFINED_VALUE_CONDITION_HPP_
#define OPENSCENARIO_INTERPRETER__SYNTAX__USER_DEFINED_VALUE_CONDITION_HPP_

#include <openscenario_interpreter/error.hpp>
#include <openscenario_interpreter/scope.hpp>
#include <openscenario_interpreter/syntax/rule.hpp>
#include <regex>

namespace openscenario_interpreter
{
inline namespace syntax
{
/* ---- UserDefinedValueCondition ----------------------------------------------
 *
 *  This condition acts as a wrapper for external custom conditions which are
 *  implemented in the user software. This condition is considered true if the
 *  given value verifies the specified relation rule (bigger than, smaller than,
 *  or equal to) relatively to the provided reference.
 *
 *  <xsd:complexType name="UserDefinedValueCondition">
 *    <xsd:attribute name="name" type="String" use="required"/>
 *    <xsd:attribute name="value" type="String" use="required"/>
 *    <xsd:attribute name="rule" type="Rule" use="required"/>
 *  </xsd:complexType>
 *
 * -------------------------------------------------------------------------- */
struct UserDefinedValueCondition
{
  const String name;

  const String value;

  const Rule compare;

  String last_checked_value;

  std::function<std::string()> evaluateValue;

  template <typename Node>
  explicit UserDefinedValueCondition(const Node & node, Scope & scope)
  : name(readAttribute<String>("name", node, scope)),
    value(readAttribute<String>("value", node, scope)),
    compare(readAttribute<Rule>("rule", node, scope))
  {
    static const std::regex pattern{R"(([^\.]+)\.(.+))"};

    if (std::smatch result; std::regex_match(name, result, pattern)) {
      const std::unordered_map<std::string, std::function<std::string()>> dispatch{
        std::make_pair("currentState", [result]() { return evaluateCurrentState(result.str(1)); }),
      };
      evaluateValue = dispatch.at(result.str(2));  // XXX catch
    } else {
      throw SyntaxError(__FILE__, ":", __LINE__);
    }
  }

  auto evaluate() { return asBoolean(compare(last_checked_value = evaluateValue(), value)); }

  auto description() const -> std::string
  {
    std::stringstream description;

    description << "Is the " << name << " (= " << last_checked_value << ") is " << compare << " "
                << value << "?";

    return description.str();
  }
};
}  // namespace syntax
}  // namespace openscenario_interpreter

#endif  // OPENSCENARIO_INTERPRETER__SYNTAX__USER_DEFINED_VALUE_CONDITION_HPP_
