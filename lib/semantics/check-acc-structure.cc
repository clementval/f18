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

#define CHECK_SIMPLE_CLAUSE(X, Y) \
void AccStructureChecker::Enter(const parser::AccClause::X &) { \
  CheckAllowed(AccClause::Y); \
}

#define CHECK_REQ_SCALAR_INT_CONSTANT_CLAUSE(X, Y) \
void AccStructureChecker::Enter(const parser::AccClause::X &c) { \
  CheckAllowed(AccClause::Y); \
  RequiresConstantPositiveParameter(AccClause::Y, c.v); \
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
      SetContextAllowed({AccClause::COPY, AccClause::COPYIN, AccClause::COPYOUT,
                         AccClause::CREATE, AccClause::NO_CREATE,
                         AccClause::PRESENT, AccClause::DEVICEPTR,
                         AccClause::ATTACH, AccClause::PRIVATE,
                         AccClause::FIRSTPRIVATE, AccClause::WAIT});
      SetContextAllowedOnce({AccClause::ASYNC, AccClause::DEFAULT,
                             AccClause::IF, AccClause::NUM_GANGS,
                             AccClause::NUM_WORKERS, AccClause::SELF,
                             AccClause::VECTOR_LENGTH});
      // TODO add REDUCTION, DEVICE_TYPE,
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
      SetContextAllowed({AccClause::COPY, AccClause::COPYIN, AccClause::COPYOUT,
                         AccClause::CREATE, AccClause::NO_CREATE,
                         AccClause::PRESENT, AccClause::DEVICEPTR,
                         AccClause::ATTACH});
      SetContextAllowedOnce({AccClause::ASYNC, AccClause::DEFAULT,
                             AccClause::IF, AccClause::NUM_GANGS,
                             AccClause::NUM_WORKERS, AccClause::SELF,
                             AccClause::VECTOR_LENGTH});
      // TODO add DEVICE_TYPE
    } break;
    case parser::AccBlockDirective::Directive::Serial: {
      PushContext(beginDir.source, AccDirective::SERIAL);
      SetContextAllowed({AccClause::COPY, AccClause::COPYIN, AccClause::COPYOUT,
                         AccClause::CREATE, AccClause::NO_CREATE,
                         AccClause::PRESENT, AccClause::DEVICEPTR,
                         AccClause::PRIVATE, AccClause::FIRSTPRIVATE,
                         AccClause::ATTACH, AccClause::WAIT});
      SetContextAllowedOnce({AccClause::ASYNC, AccClause::DEFAULT,
                             AccClause::IF, AccClause::SELF});
      // TODO add REDUCTION, DEVICE_TYPE
    } break;
    case parser::AccBlockDirective::Directive::HostData: {
      PushContext(beginDir.source, AccDirective::HOST_DATA);
      SetContextRequired({AccClause::USE_DEVICE});
      SetContextAllowedOnce({AccClause::IF, AccClause::IF_PRESENT});
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
    case parser::AccStandaloneDirective::Directive::EnterData: {
      PushContext(dir.source, AccDirective::ENTER_DATA);
      SetContextAllowed({AccClause::ATTACH, AccClause::CREATE,
                         AccClause::COPYIN});
      SetContextAllowedOnce({AccClause::ASYNC, AccClause::WAIT});
      // TODO add if clause
      // TODO required one of COPYIN CREATE ATTACH
    } break;
    case parser::AccStandaloneDirective::Directive::ExitData: {
      PushContext(dir.source, AccDirective::EXIT_DATA);
      SetContextAllowed({AccClause::COPYOUT, AccClause::DELETE,
          AccClause::DETACH});
      SetContextAllowedOnce({AccClause::ASYNC, AccClause::WAIT,
                             AccClause::FINALIZE});
      // TODO add if clause
      // TODO required one of COPYOUT DELETE DETACH
    } break;
  }
}

void AccStructureChecker::Leave(const parser::OpenACCStandaloneConstruct &) {
  accContext_.pop_back();
}

// Clause checkers
CHECK_REQ_SCALAR_INT_CONSTANT_CLAUSE(Collapse, COLLAPSE)
CHECK_REQ_SCALAR_INT_CONSTANT_CLAUSE(NumGangs, NUM_GANGS)
CHECK_REQ_SCALAR_INT_CONSTANT_CLAUSE(NumWorkers, NUM_WORKERS)
CHECK_REQ_SCALAR_INT_CONSTANT_CLAUSE(VectorLength, VECTOR_LENGTH)

CHECK_SIMPLE_CLAUSE(Auto, AUTO)
CHECK_SIMPLE_CLAUSE(Attach, ATTACH)
CHECK_SIMPLE_CLAUSE(Create, CREATE)
CHECK_SIMPLE_CLAUSE(Default, DEFAULT)
CHECK_SIMPLE_CLAUSE(Delete, DELETE)
CHECK_SIMPLE_CLAUSE(Detach, DETACH)
CHECK_SIMPLE_CLAUSE(Finalize, FINALIZE)
CHECK_SIMPLE_CLAUSE(FirstPrivate, FIRSTPRIVATE)
CHECK_SIMPLE_CLAUSE(Gang, GANG)
CHECK_SIMPLE_CLAUSE(If, IF)
CHECK_SIMPLE_CLAUSE(IfPresent, IF_PRESENT)
CHECK_SIMPLE_CLAUSE(Independent, INDEPENDENT)
CHECK_SIMPLE_CLAUSE(NoCreate, NO_CREATE)
CHECK_SIMPLE_CLAUSE(Self, SELF)
CHECK_SIMPLE_CLAUSE(Seq, SEQ)
CHECK_SIMPLE_CLAUSE(UseDevice, USE_DEVICE)
CHECK_SIMPLE_CLAUSE(Vector, VECTOR)
CHECK_SIMPLE_CLAUSE(Worker, WORKER)

void AccStructureChecker::Enter(const parser::AccClause::Async &c) {
  CheckAllowed(AccClause::ASYNC);
  OptionalConstantPositiveParameter(AccClause::ASYNC, c.v);
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

void AccStructureChecker::Enter(const parser::AccClause::Present &) {
  CheckAllowed(AccClause::PRESENT);
}

void AccStructureChecker::Enter(const parser::AccClause::Private &) {
  CheckAllowed(AccClause::PRIVATE);
  // TODO specific check for values
}

void AccStructureChecker::Enter(const parser::AccClause::Wait &) {
  CheckAllowed(AccClause::WAIT);
  // TODO check arguments
}

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

void AccStructureChecker::OptionalConstantPositiveParameter(
    const AccClause &clause,
    const std::optional<parser::ScalarIntConstantExpr> &o)
{
  if(o != std::nullopt) {
    RequiresConstantPositiveParameter(clause, o.value());
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