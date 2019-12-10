// Copyright (c) 2019, Oak Ridge National Laboratory.  All rights reserved.
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

#include "check-acc-structure.h"
#include "tools.h"
#include "../parser/parse-tree.h"
#include <unordered_map>

#define CHECK_SIMPLE_CLAUSE(X, Y) void AccStructureChecker::Enter(const parser::AccClause::X &) { \
  CheckAllowed(AccClause::Y); \
}

namespace Fortran::semantics {

void AccStructureChecker::Enter(const parser::OpenACCConstruct &) {
  return;
  // TODO
}

void AccStructureChecker::Enter(const parser::OpenACCBlockConstruct &x) {
  const auto &beginBlockDir{std::get<parser::AccBeginBlockDirective>(x.t)};
  const auto &endBlockDir{std::get<parser::AccEndBlockDirective>(x.t)};
  const auto &beginDir{
      CheckMatching<parser::AccBlockDirective>(beginBlockDir, endBlockDir)};

  switch (beginDir.v) {
    case parser::AccBlockDirective::Directive::Parallel: {
      PushContext(beginDir.source, AccDirective::PARALLEL);
//      SetContextAllowed({}});
//      SetContextAllowedOnce({});
    } break;
    case parser::AccBlockDirective::Directive::Data: {
      PushContext(beginDir.source, AccDirective::DATA);
      SetContextAllowed({AccClause::ATTACH, AccClause::COPY, AccClause::COPYIN,
                         AccClause::COPYOUT, AccClause::CREATE,
                         AccClause::DEVICEPTR, AccClause::NO_CREATE});
      SetContextAllowedOnce({AccClause::DEFAULT});
    } break;
    case parser::AccBlockDirective::Directive::Kernels: {
      PushContext(beginDir.source, AccDirective::KERNELS);
      // TODO set allowed
    } break;
    case parser::AccBlockDirective::Directive::Serial: {
      PushContext(beginDir.source, AccDirective::SERIAL);
      // TODO set allowed
    } break;
    default:
      // TODO others
      break;
  }
}

void AccStructureChecker::Enter(const parser::AccClause &x) {
  SetContextClause(x);
}

void AccStructureChecker::Leave(const parser::AccClauseList &) {
}

void AccStructureChecker::Leave(const parser::OpenACCBlockConstruct &) {
  accContext_.pop_back();
}

void AccStructureChecker::Enter(const parser::OpenACCStandaloneConstruct &x) {
  const auto &dir{std::get<parser::AccStandaloneDirective>(x.t)};
  switch (dir.v) {
    case parser::AccStandaloneDirective::Directive::Loop: {
      PushContext(dir.source, AccDirective::LOOP);
      SetContextAllowed({AccClause::PRIVATE});
      SetContextAllowedOnce({AccClause::COLLAPSE, AccClause::GANG,
                             AccClause::VECTOR, AccClause::WORKER});
      SetContextAllowedExclusive({AccClause::AUTO, AccClause::INDEPENDENT,
                                  AccClause::SEQ});
    } break;
    case parser::AccStandaloneDirective::Directive::Wait: {
      PushContext(dir.source, AccDirective::WAIT);
      SetContextAllowedOnce({AccClause::ASYNC});
    } break;
  }
}

void AccStructureChecker::Leave(const parser::OpenACCStandaloneConstruct &) {
  accContext_.pop_back();
}

// Clause checkers

void AccStructureChecker::Enter(const parser::AccClause::Collapse &c) {
  CheckAllowed(AccClause::COLLAPSE);
  // collapse clause must have a parameter
  RequiresConstantPositiveParameter(AccClause::COLLAPSE, c.v);
}

void AccStructureChecker::Enter(const parser::AccClause::Async &) {
  CheckAllowed(AccClause::ASYNC);
  // TODO specific check for values
}

void AccStructureChecker::Enter(const parser::AccClause::Copy &) {
  CheckAllowed(AccClause::COPY);
  // TODO specific check for values
}

void AccStructureChecker::Enter(const parser::AccClause::Copyin &) {
  CheckAllowed(AccClause::COPYIN);
  // TODO specific check for values
}

void AccStructureChecker::Enter(const parser::AccClause::Copyout &) {
  CheckAllowed(AccClause::COPYOUT);
  // TODO specific check for values
}

void AccStructureChecker::Enter(const parser::AccClause::DeviceNum &) {
  CheckAllowed(AccClause::DEVICENUM);
  // TODO specific check for values
}

void AccStructureChecker::Enter(const parser::AccClause::NumGangs &) {
  CheckAllowed(AccClause::NUM_GANGS);
  // TODO specific check for values
}

void AccStructureChecker::Enter(const parser::AccClause::NumWorkers &) {
  CheckAllowed(AccClause::NUM_WORKERS);
  // TODO specific check for values
}

void AccStructureChecker::Enter(const parser::AccClause::Present &) {
  CheckAllowed(AccClause::PRESENT);
}

void AccStructureChecker::Enter(const parser::AccClause::Private &) {
  CheckAllowed(AccClause::PRIVATE);
  // TODO specific check for values
}

void AccStructureChecker::Enter(const parser::AccClause::VectorLength &) {
  CheckAllowed(AccClause::VECTOR_LENGTH);
  // TODO specific check for values
}

CHECK_SIMPLE_CLAUSE(Auto, AUTO)
CHECK_SIMPLE_CLAUSE(Attach, ATTACH)
CHECK_SIMPLE_CLAUSE(Create, CREATE)
CHECK_SIMPLE_CLAUSE(Default, DEFAULT)
CHECK_SIMPLE_CLAUSE(Detach, DETACH)
CHECK_SIMPLE_CLAUSE(Gang, GANG)
CHECK_SIMPLE_CLAUSE(Independent, INDEPENDENT)
CHECK_SIMPLE_CLAUSE(NoCreate, NO_CREATE)
CHECK_SIMPLE_CLAUSE(Seq, SEQ)
CHECK_SIMPLE_CLAUSE(Vector, VECTOR)
CHECK_SIMPLE_CLAUSE(Worker, WORKER)

void AccStructureChecker::CheckAllowed(AccClause type) {
  if (!GetContext().allowedClauses.test(type) &&
      !GetContext().allowedOnceClauses.test(type) &&
      !GetContext().allowedExclusiveClauses.test(type))
  {
    context_.Say(GetContext().clauseSource,
        "%s clause is not allowed on the %s directive"_err_en_US,
        EnumToString(type),
        parser::ToUpperCaseLetters(
        GetContext().directiveSource.ToString()));
    return;
  }
  if ((GetContext().allowedOnceClauses.test(type) ||
      GetContext().allowedExclusiveClauses.test(type)) &&
      FindClause(type))
  {
    context_.Say(GetContext().clauseSource,
        "At most one %s clause can appear on the %s directive"_err_en_US,
        EnumToString(type),
        parser::ToUpperCaseLetters(
            GetContext().directiveSource.ToString()));
    return;
  }
  if (GetContext().allowedExclusiveClauses.test(type)) {
    std::vector<AccClause> others;
    GetContext().allowedExclusiveClauses.IterateOverMembers([&](AccClause o) {
        if (FindClause(o)) {
          others.emplace_back(o);
        }
    });
    for (const auto &e : others) {
      context_.Say(GetContext().clauseSource,
                   "%s and %s are mutually exclusive and may not appear on the "
                   "same %s directive"_err_en_US,
                   EnumToString(type), EnumToString(e),
                   parser::ToUpperCaseLetters(
                       GetContext().directiveSource.ToString()));
    }
    if (!others.empty()) {
      return;
    }
  }
  SetContextClauseInfo(type);
}

void AccStructureChecker::CheckRequired(AccClause type) {
  if (!FindClause(type)) {
    context_.Say(GetContext().directiveSource,
                 "At least one %s clause must appear on the %s directive"_err_en_US,
                 EnumToString(type), EnumToString(GetContext().directive));
  }
}

void AccStructureChecker::RequiresConstantPositiveParameter(
    const AccClause &clause, const parser::ScalarIntConstantExpr &i) {
  if (const auto v{GetIntValue(i)}) {
    if (*v <= 0) {
      context_.Say(GetContext().clauseSource,
                   "The parameter of the %s clause must be "
                   "a constant positive integer expression"_err_en_US,
                   EnumToString(clause));
    }
  }
}

void AccStructureChecker::SayNotMatching(
    const parser::CharBlock &beginSource, const parser::CharBlock &endSource)
{
  context_
      .Say(endSource, "Unmatched %s directive"_err_en_US,
           parser::ToUpperCaseLetters(endSource.ToString()))
      .Attach(beginSource, "Does not match directive"_en_US);
}

} // namespace Fortran::semantics