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

void AccStructureChecker::Enter(const parser::OpenACCDeclarativeConstruct &) {
  return;
  // TODO
}

void AccStructureChecker::Enter(const parser::OpenACCBlockConstruct &x) {
  const auto &beginBlockDir{std::get<parser::AccBeginBlockDirective>(x.t)};
  const auto &endBlockDir{std::get<parser::AccEndBlockDirective>(x.t)};
  const auto &beginDir{
      CheckMatching<parser::AccBlockDirective>(beginBlockDir, endBlockDir)};

  switch (beginDir.v) {
    case parser::AccBlockDirective::Directive::Atomic: {
      PushContext(beginDir.source, AccDirective::ATOMIC);
      SetContextAllowedExclusive({AccClause::CAPTURE, AccClause::READ,
                                  AccClause::WRITE});
    } break;
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
                         AccClause::DEVICEPTR, AccClause::NO_CREATE,
                         AccClause::PRESENT});
      SetContextAllowedOnce({AccClause::DEFAULT, AccClause::IF});
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

void AccStructureChecker::Enter(
    const parser::OpenACCStandaloneDeclarativeConstruct &x)
{
  const auto &dir{std::get<parser::AccDeclarativeDirective>(x.t)};
  switch (dir.v) {
    case parser::AccDeclarativeDirective::Directive::Declare: {
      PushContext(dir.source, AccDirective::DECLARE);
      // TODO allowed/required
    } break;
  }
}

void AccStructureChecker::Leave(
    const parser::OpenACCStandaloneDeclarativeConstruct &)
{
  accContext_.pop_back();
}

void AccStructureChecker::Enter(const parser::OpenACCCombinedConstruct &x) {
  const auto &beginBlockDir{std::get<parser::AccBeginCombinedDirective>(x.t)};
  const auto &beginDir{std::get<parser::AccCombinedDirective>(beginBlockDir.t)};
  switch (beginDir.v) {
    case parser::AccCombinedDirective::Directive::KernelsLoop: {
      PushContext(beginDir.source, AccDirective::KERNELS_LOOP);
    } break;
    case parser::AccCombinedDirective::Directive::ParallelLoop: {
      PushContext(beginDir.source, AccDirective::PARALLEL_LOOP);
      SetContextAllowed({AccClause::COPY, AccClause::COPYIN, AccClause::COPYOUT,
                         AccClause::CREATE, AccClause::NO_CREATE,
                         AccClause::PRESENT, AccClause::DEVICEPTR,
                         AccClause::ATTACH, AccClause::PRIVATE,
                         AccClause::FIRSTPRIVATE, AccClause::WAIT});
      SetContextAllowedOnce({AccClause::ASYNC, AccClause::COLLAPSE,
                             AccClause::DEFAULT, AccClause::GANG,
                             AccClause::IF, AccClause::NUM_GANGS,
                             AccClause::NUM_WORKERS, AccClause::SELF,
                             AccClause::VECTOR, AccClause::VECTOR_LENGTH,
                             AccClause::WORKER});
      SetContextAllowedExclusive({AccClause::AUTO, AccClause::INDEPENDENT,
                                  AccClause::SEQ});
      // TODO add REDUCTION, DEVICE_TYPE,
    } break;
    case parser::AccCombinedDirective::Directive::SerialLoop: {
      PushContext(beginDir.source, AccDirective::SERIAL_LOOP);
    } break;
  }
}

void AccStructureChecker::Leave(const parser::OpenACCCombinedConstruct &) {
  accContext_.pop_back();
}

void AccStructureChecker::Enter(const parser::OpenACCStandaloneConstruct &x) {
  const auto &dir{std::get<parser::AccStandaloneDirective>(x.t)};
  switch (dir.v) {
    case parser::AccStandaloneDirective::Directive::Init: {
      PushContext(dir.source, AccDirective::INIT);
      SetContextAllowedOnce({AccClause::DEVICENUM, AccClause::DEVICE_TYPE,
                            AccClause::IF});
      // TODO maybe multiple times
    } break;
    case parser::AccStandaloneDirective::Directive::Cache: {
      PushContext(dir.source, AccDirective::CACHE);
    } break;
    case parser::AccStandaloneDirective::Directive::Loop: {
      PushContext(dir.source, AccDirective::LOOP);
      SetContextAllowed({AccClause::PRIVATE});
      SetContextAllowedOnce({AccClause::COLLAPSE, AccClause::GANG,
                             AccClause::VECTOR, AccClause::WORKER});
      SetContextAllowedExclusive({AccClause::AUTO, AccClause::INDEPENDENT,
                                  AccClause::SEQ});
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
    case parser::AccStandaloneDirective::Directive::Routine: {
      PushContext(dir.source, AccDirective::ROUTINE);
      SetContextAllowedOnce({AccClause::DEVICE_TYPE});
      SetContextAllowedOnce({AccClause::GANG, AccClause::WORKER,
                             AccClause::VECTOR, AccClause::SEQ, AccClause::BIND,
                             AccClause::NOHOST});
      // TODO require_one_of: GANG, VECTOR, WORKER, SEQ
      // TODO only GANG, WORKER, VECTOR, CLAUSE can follow DEVICE_TYPE
      // TODO If DEVICE_TYPE, must have one DEFAULT parrallelism level before
      // or one after each
    } break;
    case parser::AccStandaloneDirective::Directive::Shutdown: {
      PushContext(dir.source, AccDirective::SHUTDOWN);
      SetContextAllowedOnce({AccClause::DEVICENUM, AccClause::DEVICE_TYPE,
          AccClause::IF});
      // TODO maybe allowed multiple times
    } break;
    case parser::AccStandaloneDirective::Directive::Update: {
      PushContext(dir.source, AccDirective::UPDATE);
      SetContextAllowed({AccClause::DEVICE, AccClause::HOST, AccClause::SELF});
      SetContextAllowedOnce({AccClause::ASYNC, AccClause::IF,
                             AccClause::IF_PRESENT});
      // TODO DEVICE, HOST, SELF requires at least one of them
      // TODO other clauses WAIT, DEVICE_TYPE
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
CHECK_SIMPLE_CLAUSE(Async, ASYNC)
CHECK_SIMPLE_CLAUSE(Attach, ATTACH)
CHECK_SIMPLE_CLAUSE(Capture, CAPTURE)
CHECK_SIMPLE_CLAUSE(Bind, BIND)
CHECK_SIMPLE_CLAUSE(Default, DEFAULT)
CHECK_SIMPLE_CLAUSE(Delete, DELETE)
CHECK_SIMPLE_CLAUSE(Detach, DETACH)
CHECK_SIMPLE_CLAUSE(Device, DEVICE) // TODO more ?
CHECK_SIMPLE_CLAUSE(Finalize, FINALIZE)
CHECK_SIMPLE_CLAUSE(FirstPrivate, FIRSTPRIVATE)
CHECK_SIMPLE_CLAUSE(Gang, GANG)
CHECK_SIMPLE_CLAUSE(Host, HOST) // TODO more ?
CHECK_SIMPLE_CLAUSE(If, IF)
CHECK_SIMPLE_CLAUSE(IfPresent, IF_PRESENT)
CHECK_SIMPLE_CLAUSE(Independent, INDEPENDENT)
CHECK_SIMPLE_CLAUSE(NoCreate, NO_CREATE)
CHECK_SIMPLE_CLAUSE(NoHost, NOHOST)
CHECK_SIMPLE_CLAUSE(Read, READ)
CHECK_SIMPLE_CLAUSE(Self, SELF)
CHECK_SIMPLE_CLAUSE(Seq, SEQ)
CHECK_SIMPLE_CLAUSE(UseDevice, USE_DEVICE)
CHECK_SIMPLE_CLAUSE(Vector, VECTOR)
CHECK_SIMPLE_CLAUSE(Worker, WORKER)
CHECK_SIMPLE_CLAUSE(Write, WRITE)
//
void AccStructureChecker::Enter(const parser::AccClause::Create &c) {
  CheckAllowed(AccClause::CREATE);

  const auto &modifierClause{c.v};
  if (const auto &modifier{std::get<std::optional<parser::AccDataModifier>>(modifierClause.t)}) {
    if(modifier->v != parser::AccDataModifier::Modifier::Zero) {
      context_.Say(GetContext().clauseSource,
                   "Only the ZERO modifier is permitted for the CREATE clause "
                   "on the %s directive "_err_en_US,
                   EnumToString(GetContext().directive));
    }
  }
}

void AccStructureChecker::Enter(const parser::AccClause::Copy &) {
  CheckAllowed(AccClause::COPY);
  // TODO specific check for values
}

void AccStructureChecker::Enter(const parser::AccClause::Copyin &c) {
  CheckAllowed(AccClause::COPYIN);
  const auto &modifierClause{c.v};
  if (const auto &modifier{std::get<std::optional<parser::AccDataModifier>>(modifierClause.t)}) {
    if(modifier->v != parser::AccDataModifier::Modifier::ReadOnly) {
      context_.Say(GetContext().clauseSource,
                   "Only the READONLY modifier is permitted for the %s clause "
                   "on the %s directive"_err_en_US,
                   EnumToString(AccClause::COPYIN),
                   EnumToString(GetContext().directive));
    }
  }
  // TODO specific check for values
}

void AccStructureChecker::Enter(const parser::AccClause::Copyout &c) {
  CheckAllowed(AccClause::COPYOUT);

  const auto &modifierClause{c.v};
  if (const auto &modifier{std::get<std::optional<parser::AccDataModifier>>(modifierClause.t)}) {
    if(modifier->v != parser::AccDataModifier::Modifier::Zero) {
      context_.Say(GetContext().clauseSource,
                   "Only the ZERO modifier is permitted for the COPYOUT clause "
                   "on the %s directive"_err_en_US,
                   EnumToString(GetContext().directive));
    }
  }
  // TODO specific check for values
}

void AccStructureChecker::Enter(const parser::AccClause::DeviceNum &) {
  CheckAllowed(AccClause::DEVICENUM);
  // TODO specific check for values
}

void AccStructureChecker::Enter(const parser::AccClause::DevicePtr &) {
  CheckAllowed(AccClause::DEVICEPTR);
  // TODO specific check for values
}

void AccStructureChecker::Enter(const parser::AccClause::DeviceType &) {
  CheckAllowed(AccClause::DEVICE_TYPE);
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