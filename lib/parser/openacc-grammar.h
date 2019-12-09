// Copyright (c) 2018-2019, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef FORTRAN_PARSER_OPENACC_GRAMMAR_H_
#define FORTRAN_PARSER_OPENACC_GRAMMAR_H_

#include "basic-parsers.h"
#include "characters.h"
#include "debug-parser.h"
#include "grammar.h"
#include "parse-tree.h"
#include "stmt-parser.h"
#include "token-parsers.h"
#include "type-parsers.h"
#include "user-state.h"
#include <cinttypes>
#include <cstdio>
#include <functional>
#include <list>
#include <optional>
#include <string>
#include <tuple>
#include <utility>

// OpenACC Directives and Clauses
namespace Fortran::parser {

constexpr auto startAccLine = skipStuffBeforeStatement >> "!$ACC "_sptok;
constexpr auto endAccLine = space >> endOfLine;

TYPE_PARSER(construct<AccBlockDirective>(
    first("DATA" >> pure(AccBlockDirective::Directive::Data),
          "KERNELS" >> pure(AccBlockDirective::Directive::Kernels),
          "PARALLEL" >> pure(AccBlockDirective::Directive::Parallel),
          "SERIAL" >> pure(AccBlockDirective::Directive::Serial))))

TYPE_PARSER(construct<AccBeginBlockDirective>(
    sourced(Parser<AccBlockDirective>{}), Parser<AccClauseList>{}))

TYPE_PARSER(
    "COLLAPSE" >> construct<AccClause>(construct<AccClause::Collapse>(
                        parenthesized(scalarIntConstantExpr))) ||
    "COPYIN" >> construct<AccClause>(construct<AccClause::Copyin>(
                        parenthesized(Parser<AccObjectList>{}))))

TYPE_PARSER(construct<AccObject>(designator) || construct<AccObject>("/" >> name / "/"))
TYPE_PARSER(construct<AccObjectList>(nonemptyList(Parser<AccObject>{})))


// [Clause, [Clause], ...]
TYPE_PARSER(sourced(construct<AccClauseList>(
    many(maybe(","_tok) >> sourced(Parser<AccClause>{})))))

// Block Construct
TYPE_PARSER(construct<OpenACCBlockConstruct>(
    Parser<AccBeginBlockDirective>{} / endAccLine, block,
    Parser<AccEndBlockDirective>{} / endAccLine))

TYPE_PARSER(
     sourced(construct<OpenACCStandaloneConstruct>(
             Parser<OpenACCStandaloneConstruct>{})) / endOfLine)


TYPE_CONTEXT_PARSER("OpenACC construct"_en_US,
    startAccLine >>
        first(construct<OpenACCConstruct>(Parser<OpenACCBlockConstruct>{}),
              construct<OpenACCConstruct>(Parser<OpenACCStandaloneConstruct>{})))


// END ACC Block directives
TYPE_PARSER(
    startAccLine >> construct<AccEndBlockDirective>(
                        sourced("END"_tok >> Parser<AccBlockDirective>{}),
                        Parser<AccClauseList>{}))

}



#endif // FORTRAN_PARSER_OPENACC_GRAMMAR_H_
