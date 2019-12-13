// Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
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

// OpenACC structure validity check list
//    1. invalid clauses on directive
//    2. invalid repeated clauses on directive
//    3. invalid nesting of regions

#ifndef FORTRAN_SEMANTICS_CHECK_ACC_STRUCTURE_H_
#define FORTRAN_SEMANTICS_CHECK_ACC_STRUCTURE_H_

#include "semantics.h"
#include "../common/enum-set.h"
#include "../parser/parse-tree.h"

namespace Fortran::semantics {

ENUM_CLASS(AccDirective, ATOMIC, CACHE, DATA, DECLARE, ENTER_DATA, EXIT_DATA,
    INIT, HOST_DATA, KERNELS, LOOP, PARALLEL, ROUTINE, SERIAL, SHUTDOWN, UPDATE,
    WAIT)

using AccDirectiveSet = common::EnumSet<AccDirective, AccDirective_enumSize>;

ENUM_CLASS(AccClause, AUTO, ASYNC, ATTACH, CAPTURE, BIND, COLLAPSE, COPY,
    COPYIN, COPYOUT, DEFAULT, DELETE, CREATE, DETACH, DEVICE, DEVICENUM,
    DEVICEPTR, DEVICE_TYPE, FINALIZE, FIRSTPRIVATE, GANG, HOST, IF, IF_PRESENT,
    INDEPENDENT, NO_CREATE, NOHOST, NUM_GANGS, NUM_WORKERS, PRESENT, PRIVATE,
    READ, USE_DEVICE, VECTOR_LENGTH, SELF, SEQ, VECTOR, WAIT, WORKER, WRITE)

using AccClauseSet = common::EnumSet<AccClause, AccClause_enumSize>;

class AccStructureChecker : public virtual BaseChecker {
public:
  AccStructureChecker(SemanticsContext &context) : context_{context} {}

  // Construct and directives
  void Enter(const parser::OpenACCConstruct &);
  void Enter(const parser::OpenACCDeclarativeConstruct &);
  void Enter(const parser::OpenACCStandaloneConstruct &);
  void Leave(const parser::OpenACCStandaloneConstruct &);
  void Enter(const parser::OpenACCBlockConstruct &);
  void Leave(const parser::OpenACCBlockConstruct &);
  void Enter(const parser::OpenACCStandaloneDeclarativeConstruct &);
  void Leave(const parser::OpenACCStandaloneDeclarativeConstruct &);

  // Clauses
  void Leave(const parser::AccClauseList &);
  void Enter(const parser::AccClause &);
  void Enter(const parser::AccClause::Auto &);
  void Enter(const parser::AccClause::Async &);
  void Enter(const parser::AccClause::Attach &);
  void Enter(const parser::AccClause::Capture &);
  void Enter(const parser::AccClause::Bind &);
  void Enter(const parser::AccClause::Collapse &);
  void Enter(const parser::AccClause::Copy &);
  void Enter(const parser::AccClause::Copyin &);
  void Enter(const parser::AccClause::Copyout &);
  void Enter(const parser::AccClause::Create &);
  void Enter(const parser::AccClause::Default &);
  void Enter(const parser::AccClause::Delete &);
  void Enter(const parser::AccClause::Detach &);
  void Enter(const parser::AccClause::Device &);
  void Enter(const parser::AccClause::DeviceNum &);
  void Enter(const parser::AccClause::DevicePtr &);
  void Enter(const parser::AccClause::DeviceType &);
  void Enter(const parser::AccClause::Finalize &);
  void Enter(const parser::AccClause::FirstPrivate &);
  void Enter(const parser::AccClause::Gang &);
  void Enter(const parser::AccClause::Host &);
  void Enter(const parser::AccClause::If &);
  void Enter(const parser::AccClause::IfPresent &);
  void Enter(const parser::AccClause::Independent&);
  void Enter(const parser::AccClause::NoCreate &);
  void Enter(const parser::AccClause::NoHost &);
  void Enter(const parser::AccClause::NumGangs &);
  void Enter(const parser::AccClause::NumWorkers &);
  void Enter(const parser::AccClause::Present &);
  void Enter(const parser::AccClause::Private &);
  void Enter(const parser::AccClause::Read &);
  void Enter(const parser::AccClause::Self &);
  void Enter(const parser::AccClause::Seq &);
  void Enter(const parser::AccClause::UseDevice &);
  void Enter(const parser::AccClause::Vector &);
  void Enter(const parser::AccClause::VectorLength &);
  void Enter(const parser::AccClause::Worker &);
  void Enter(const parser::AccClause::Wait &);
  void Enter(const parser::AccClause::Write &);

private:

//  struct AccDirectiveClauses {
//    const AccDirective directive;
//    const AccClauseSet allowed;
//    const AccClauseSet allowedOnce;
//    const AccClauseSet allowedExclusive;
//    const AccClauseSet requiredOneOf;
//  };
//
//  static struct AccDirectiveClauses directiveClausesTable[]{
//    {AccDirective::LOOP,
//         {AccClause::PRIVATE},
//         {AccClause::COLLAPSE, AccClause::GANG, AccClause::VECTOR, AccClause::WORKER},
//         {AccClause::AUTO, AccClause::INDEPENDENT, AccClause::SEQ},
//         {}
//    },
//    {AccDirective::WAIT,
//        {},
//        {AccClause::ASYNC},
//        {},
//        {}
//    },
//    {AccDirective::ENTER_DATA,
//        {AccClause::ATTACH, AccClause::CREATE, AccClause::COPYIN},
//        {AccClause::ASYNC, AccClause::WAIT},
//        {},
//        {}
//     },
//    {AccDirective::EXIT_DATA,
//        {AccClause::COPYOUT, AccClause::DELETE, AccClause::DETACH},
//        {AccClause::ASYNC, AccClause::WAIT, AccClause::FINALIZE},
//        {},
//        {}
//     }
//  };

  struct AccContext {
    AccContext(parser::CharBlock source, AccDirective d)
        : directiveSource{source}, directive{d} {}

    parser::CharBlock directiveSource{nullptr};
    parser::CharBlock clauseSource{nullptr};
    AccDirective directive;
    AccClauseSet allowedClauses{};
    AccClauseSet allowedOnceClauses{};
    AccClauseSet allowedExclusiveClauses{};
    AccClauseSet requiredClauses{};

    const parser::AccClause *clause{nullptr};
    std::multimap<AccClause, const parser::AccClause *> clauseInfo;
  };

  // back() is the top of the stack
  AccContext &GetContext() {
    CHECK(!accContext_.empty());
    return accContext_.back();
  }

  void SetContextAllowed(const AccClauseSet &allowed) {
    GetContext().allowedClauses = allowed;
  }

  void SetContextAllowedOnce(const AccClauseSet &allowedOnce) {
    GetContext().allowedOnceClauses = allowedOnce;
  }

  void SetContextAllowedExclusive(const AccClauseSet &allowedExclusive) {
    GetContext().allowedExclusiveClauses = allowedExclusive;
  }

  void SetContextRequired(const AccClauseSet &required) {
    GetContext().requiredClauses = required;
  }

  void SetContextClause(const parser::AccClause &clause) {
    GetContext().clauseSource = clause.source;
    GetContext().clause = &clause;
  }

  void SetContextClauseInfo(AccClause type) {
    GetContext().clauseInfo.emplace(type, GetContext().clause);
  }

  const parser::AccClause *FindClause(AccClause type) {
    auto it{GetContext().clauseInfo.find(type)};
    if (it != GetContext().clauseInfo.end()) {
      return it->second;
    }
    return nullptr;
  }

  void PushContext(const parser::CharBlock &source, AccDirective dir) {
    accContext_.emplace_back(source, dir);
  }

  void SayNotMatching(const parser::CharBlock &, const parser::CharBlock &);
  template<typename A, typename B, typename C>
  const A &CheckMatching(const B &beginDir, const C &endDir) {
    const A &begin{std::get<A>(beginDir.t)};
    const A &end{std::get<A>(endDir.t)};
    if (begin.v != end.v) {
      SayNotMatching(begin.source, end.source);
    }
    return begin;
  }

  void CheckAllowed(AccClause);

  void CheckRequired(AccClause);

  void RequiresConstantPositiveParameter(
      const AccClause &clause, const parser::ScalarIntConstantExpr &i);
  void OptionalConstantPositiveParameter(
      const AccClause &clause, const std::optional<parser::ScalarIntConstantExpr> &o);

  SemanticsContext &context_;
  std::vector<AccContext> accContext_;  // used as a stack
};


}

#endif // FORTRAN_SEMANTICS_CHECK_ACC_STRUCTURE_H_
