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

ENUM_CLASS(AccDirective, DATA, ENTER_DATA, EXIT_DATA, KERNELS, LOOP, PARALLEL,
    SERIAL, WAIT)

using AccDirectiveSet = common::EnumSet<AccDirective, AccDirective_enumSize>;

ENUM_CLASS(AccClause, AUTO, ASYNC, ATTACH, COLLAPSE, COPY, COPYIN, COPYOUT,
    DEFAULT, DELETE, CREATE, DETACH, DEVICENUM, DEVICEPTR, FINALIZE,
    FIRSTPRIVATE, GANG, INDEPENDENT, NO_CREATE, NUM_GANGS, NUM_WORKERS,
    PRESENT, PRIVATE, VECTOR_LENGTH, SELF, SEQ, VECTOR, WAIT, WORKER)

using AccClauseSet = common::EnumSet<AccClause, AccClause_enumSize>;

class AccStructureChecker : public virtual BaseChecker {
public:
  AccStructureChecker(SemanticsContext &context) : context_{context} {}

  // Construct and directives
  void Enter(const parser::OpenACCConstruct &);
  void Enter(const parser::OpenACCStandaloneConstruct &);
  void Leave(const parser::OpenACCStandaloneConstruct &);
  void Enter(const parser::OpenACCBlockConstruct &);
  void Leave(const parser::OpenACCBlockConstruct &);

  // Clauses
  void Leave(const parser::AccClauseList &);
  void Enter(const parser::AccClause &);
  void Enter(const parser::AccClause::Auto &);
  void Enter(const parser::AccClause::Async &);
  void Enter(const parser::AccClause::Attach &);
  void Enter(const parser::AccClause::Collapse &);
  void Enter(const parser::AccClause::Copy &);
  void Enter(const parser::AccClause::Copyin &);
  void Enter(const parser::AccClause::Copyout &);
  void Enter(const parser::AccClause::Create &);
  void Enter(const parser::AccClause::Default &);
  void Enter(const parser::AccClause::Delete &);
  void Enter(const parser::AccClause::Detach &);
  void Enter(const parser::AccClause::DeviceNum &);
  void Enter(const parser::AccClause::Finalize &);
  void Enter(const parser::AccClause::FirstPrivate &);
  void Enter(const parser::AccClause::Gang &);
  void Enter(const parser::AccClause::NoCreate &);
  void Enter(const parser::AccClause::NumGangs &);
  void Enter(const parser::AccClause::NumWorkers &);
  void Enter(const parser::AccClause::Present &);
  void Enter(const parser::AccClause::Private &);
  void Enter(const parser::AccClause::VectorLength &);
  void Enter(const parser::AccClause::Independent&);
  void Enter(const parser::AccClause::Self &);
  void Enter(const parser::AccClause::Seq &);
  void Enter(const parser::AccClause::Vector &);
  void Enter(const parser::AccClause::Worker &);

private:
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

  SemanticsContext &context_;
  std::vector<AccContext> accContext_;  // used as a stack
};


}

#endif // FORTRAN_SEMANTICS_CHECK_ACC_STRUCTURE_H_
