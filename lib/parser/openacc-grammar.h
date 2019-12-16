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
    first("ATOMIC" >> pure(AccBlockDirective::Directive::Atomic),
          "DATA" >> pure(AccBlockDirective::Directive::Data),
          "HOST_DATA" >> pure(AccBlockDirective::Directive::HostData),
          "KERNELS" >> pure(AccBlockDirective::Directive::Kernels),
          "PARALLEL" >> pure(AccBlockDirective::Directive::Parallel),
          "SERIAL" >> pure(AccBlockDirective::Directive::Serial))))

TYPE_PARSER(construct<AccBeginBlockDirective>(
    sourced(Parser<AccBlockDirective>{}), Parser<AccClauseList>{}))

TYPE_PARSER(
    "AUTO" >> construct<AccClause>(construct<AccClause::Auto>()) ||
    "ASYNC" >> construct<AccClause>(construct<AccClause::Async>(maybe(
        parenthesized(scalarIntConstantExpr)))) || // TODO optional int-expr
    "ATTACH" >> construct<AccClause>(construct<AccClause::Attach>(
        parenthesized(Parser<AccObjectList>{}))) ||
    "CAPTURE" >> construct<AccClause>(construct<AccClause::Capture>()) ||
    "BIND" >> construct<AccClause>(construct<AccClause::Bind>(
        parenthesized(name))) ||
    "COLLAPSE" >> construct<AccClause>(construct<AccClause::Collapse>(
        parenthesized(scalarIntConstantExpr))) ||
    "COPY" >> construct<AccClause>(construct<AccClause::Copy>(
        parenthesized(Parser<AccObjectList>{}))) ||
    "COPYIN" >> construct<AccClause>(construct<AccClause::Copyin>(
        parenthesized(Parser<AccObjectList>{}))) ||
    "COPYOUT" >> construct<AccClause>(construct<AccClause::Copyout>(
        parenthesized(Parser<AccModifierClause>{}))) ||
    "CREATE" >> construct<AccClause>(construct<AccClause::Create>(
        parenthesized(Parser<AccModifierClause>{}))) ||
    "DEFAULT" >> construct<AccClause>(construct<AccClause::Default>(first(
        "NONE" >> pure(AccDefaultClause::Arg::None),
        "PRESENT" >> pure(AccDefaultClause::Arg::Present)))) ||
    "DELETE" >> construct<AccClause>(construct<AccClause::Delete>(
        parenthesized(Parser<AccObjectList>{}))) ||
    "DETACH" >> construct<AccClause>(construct<AccClause::Detach>(
        parenthesized(Parser<AccObjectList>{}))) ||
    "DEVICE" >> construct<AccClause>(construct<AccClause::Device>(
        parenthesized(Parser<AccObjectList>{}))) ||
    "DEVICEPTR" >> construct<AccClause>(construct<AccClause::DevicePtr>(
        parenthesized(Parser<AccObjectList>{}))) ||
    "DEVICENUM" >> construct<AccClause>(construct<AccClause::DeviceNum>(
        parenthesized(scalarIntConstantExpr))) ||
    "DEVICE_TYPE" >> construct<AccClause>(construct<AccClause::DeviceType>(
        /*parenthesized(TODO)*/)) ||
    "FINALIZE" >> construct<AccClause>(construct<AccClause::Finalize>()) ||
    "FIRSTPRIVATE" >> construct<AccClause>(construct<AccClause::FirstPrivate>(
        parenthesized(Parser<AccObjectList>{}))) ||
    "GANG" >> construct<AccClause>(construct<AccClause::Gang>()) ||
    "HOST" >> construct<AccClause>(construct<AccClause::Host>(
        parenthesized(Parser<AccObjectList>{}))) ||
    "IF" >> construct<AccClause>(
        construct<AccClause::If>(parenthesized(scalarLogicalExpr))) ||
    "IF_PRESENT" >> construct<AccClause>(construct<AccClause::IfPresent>()) ||
    "INDEPENDENT" >> construct<AccClause>(
        construct<AccClause::Independent>()) ||
    "NO_CREATE" >> construct<AccClause>(construct<AccClause::NoCreate>(
        parenthesized(Parser<AccObjectList>{}))) ||
    "NOHOST" >> construct<AccClause>(construct<AccClause::NoHost>()) ||
    "NUM_GANGS" >> construct<AccClause>(construct<AccClause::NumGangs>(
        parenthesized(scalarIntConstantExpr))) ||
    "NUM_WORKERS" >> construct<AccClause>(construct<AccClause::NumWorkers>(
        parenthesized(scalarIntConstantExpr))) ||
    "PRESENT" >> construct<AccClause>(construct<AccClause::Present>(
        parenthesized(Parser<AccObjectList>{}))) ||
    "READ" >> construct<AccClause>(construct<AccClause::Read>()) ||
    "VECTOR_LENGTH" >> construct<AccClause>(construct<AccClause::VectorLength>(
        parenthesized(scalarIntConstantExpr))) ||
    "SELF" >> construct<AccClause>(construct<AccClause::Self>(
        maybe(parenthesized(scalarLogicalExpr)))) ||
    "SEQ" >> construct<AccClause>(construct<AccClause::Seq>()) ||
    "USE_DEVICE" >> construct<AccClause>(construct<AccClause::UseDevice>(
        parenthesized(Parser<AccObjectList>{}))) ||
    "VECTOR" >> construct<AccClause>(construct<AccClause::Vector>()) ||
    "WAIT" >> construct<AccClause>(construct<AccClause::Wait>(
        maybe(parenthesized(scalarIntExpr)))) || // TODO optional int-expr-list
    "WORKER" >> construct<AccClause>(construct<AccClause::Worker>()) ||
    "WRITE" >>  construct<AccClause>(construct<AccClause::Auto>()))

TYPE_PARSER(construct<AccObject>(designator)
        || construct<AccObject>("/" >> name / "/"))
TYPE_PARSER(construct<AccObjectList>(nonemptyList(Parser<AccObject>{})))

TYPE_PARSER(
    construct<AccModifierClause>(
        Parser<AccDataModifier>{}, Parser<AccObjectList>{}))

TYPE_PARSER(construct<AccDataModifier>(
     first(
        "ZERO:" >> pure(AccDataModifier::Modifier::Zero),
        "READONLY:" >> pure(AccDataModifier::Modifier::ReadOnly))))

TYPE_PARSER(construct<AccStandaloneDirective>(first(
    "ENTER DATA" >> pure(AccStandaloneDirective::Directive::EnterData),
    "EXIT DATA" >> pure(AccStandaloneDirective::Directive::ExitData),
    "INIT" >> pure(AccStandaloneDirective::Directive::Init),
    "LOOP" >> pure(AccStandaloneDirective::Directive::Loop),
    "ROUTINE" >> pure(AccStandaloneDirective::Directive::Routine),
    "SHUTDOWN" >> pure(AccStandaloneDirective::Directive::Shutdown),
    "UPDATE" >> pure(AccStandaloneDirective::Directive::Update),
    "WAIT" >> pure(AccStandaloneDirective::Directive::Wait))))

TYPE_PARSER(construct<AccDeclarativeDirective>(first(
    "DECLARE" >> pure(AccDeclarativeDirective::Directive::Declare)
    )))

// [Clause, [Clause], ...]
TYPE_PARSER(sourced(construct<AccClauseList>(
    many(maybe(","_tok) >> sourced(Parser<AccClause>{})))))

// Block Construct
TYPE_PARSER(construct<OpenACCBlockConstruct>(
    Parser<AccBeginBlockDirective>{} / endAccLine, block,
    Parser<AccEndBlockDirective>{} / endAccLine))

TYPE_PARSER(
        construct<OpenACCStandaloneConstruct>(
            sourced(Parser<AccStandaloneDirective>{}), Parser<AccClauseList>{}))

TYPE_PARSER(
    construct<OpenACCStandaloneDeclarativeConstruct>(
        sourced(Parser<AccDeclarativeDirective>{}), Parser<AccClauseList>{}))

TYPE_PARSER( // TODO correct parser ?
    startAccLine >> sourced(construct<OpenACCDeclarativeConstruct>(
                Parser<OpenACCStandaloneDeclarativeConstruct>{})))

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