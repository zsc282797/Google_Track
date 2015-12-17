// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/bit-vector.h"
#include "src/compiler/escape-analysis.h"
#include "src/compiler/escape-analysis-reducer.h"
#include "src/compiler/graph-visualizer.h"
#include "src/compiler/js-graph.h"
#include "src/compiler/node-properties.h"
#include "src/compiler/simplified-operator.h"
#include "src/types-inl.h"
#include "src/zone-containers.h"
#include "test/unittests/compiler/graph-unittest.h"

namespace v8 {
namespace internal {
namespace compiler {

class EscapeAnalysisTest : public GraphTest {
 public:
  EscapeAnalysisTest()
      : simplified_(zone()),
        jsgraph_(isolate(), graph(), common(), nullptr, nullptr, nullptr),
        escape_analysis_(graph(), common(), zone()),
        effect_(graph()->start()),
        control_(graph()->start()) {}

  ~EscapeAnalysisTest() {}

  EscapeAnalysis* escape_analysis() { return &escape_analysis_; }

 protected:
  void Analysis() { escape_analysis_.Run(); }

  void Transformation() {
    GraphReducer graph_reducer(zone(), graph());
    EscapeAnalysisReducer escape_reducer(&graph_reducer, &jsgraph_,
                                         &escape_analysis_, zone());
    graph_reducer.AddReducer(&escape_reducer);
    graph_reducer.ReduceGraph();
  }

  // ---------------------------------Node Creation Helper----------------------

  Node* BeginRegion(Node* effect = nullptr) {
    if (!effect) {
      effect = effect_;
    }

    return effect_ = graph()->NewNode(common()->BeginRegion(), effect);
  }

  Node* FinishRegion(Node* value, Node* effect = nullptr) {
    if (!effect) {
      effect = effect_;
    }
    return effect_ = graph()->NewNode(common()->FinishRegion(), value, effect);
  }

  Node* Allocate(Node* size, Node* effect = nullptr, Node* control = nullptr) {
    if (!effect) {
      effect = effect_;
    }
    if (!control) {
      control = control_;
    }
    return effect_ = graph()->NewNode(simplified()->Allocate(), size, effect,
                                      control);
  }

  Node* Constant(int num) {
    return graph()->NewNode(common()->NumberConstant(num));
  }

  Node* Store(const FieldAccess& access, Node* allocation, Node* value,
              Node* effect = nullptr, Node* control = nullptr) {
    if (!effect) {
      effect = effect_;
    }
    if (!control) {
      control = control_;
    }
    return effect_ = graph()->NewNode(simplified()->StoreField(access),
                                      allocation, value, effect, control);
  }

  Node* Load(const FieldAccess& access, Node* from, Node* effect = nullptr,
             Node* control = nullptr) {
    if (!effect) {
      effect = effect_;
    }
    if (!control) {
      control = control_;
    }
    return graph()->NewNode(simplified()->LoadField(access), from, effect,
                            control);
  }

  Node* Return(Node* value, Node* effect = nullptr, Node* control = nullptr) {
    if (!effect) {
      effect = effect_;
    }
    if (!control) {
      control = control_;
    }
    return control_ =
               graph()->NewNode(common()->Return(), value, effect, control);
  }

  void EndGraph() {
    for (Edge edge : graph()->end()->input_edges()) {
      if (NodeProperties::IsControlEdge(edge)) {
        edge.UpdateTo(control_);
      }
    }
  }

  Node* Branch() {
    return control_ =
               graph()->NewNode(common()->Branch(), Constant(0), control_);
  }

  Node* IfTrue() {
    return control_ = graph()->NewNode(common()->IfTrue(), control_);
  }

  Node* IfFalse() { return graph()->NewNode(common()->IfFalse(), control_); }

  Node* Merge2(Node* control1, Node* control2) {
    return control_ = graph()->NewNode(common()->Merge(2), control1, control2);
  }

  FieldAccess AccessAtIndex(int offset) {
    FieldAccess access = {kTaggedBase, offset, MaybeHandle<Name>(), Type::Any(),
                          MachineType::AnyTagged()};
    return access;
  }

  // ---------------------------------Assertion Helper--------------------------

  void ExpectReplacement(Node* node, Node* rep) {
    EXPECT_EQ(rep, escape_analysis()->GetReplacement(node));
  }

  void ExpectReplacementPhi(Node* node, Node* left, Node* right) {
    Node* rep = escape_analysis()->GetReplacement(node);
    ASSERT_NE(nullptr, rep);
    ASSERT_EQ(IrOpcode::kPhi, rep->opcode());
    EXPECT_EQ(left, NodeProperties::GetValueInput(rep, 0));
    EXPECT_EQ(right, NodeProperties::GetValueInput(rep, 1));
  }

  void ExpectVirtual(Node* node) {
    EXPECT_TRUE(node->opcode() == IrOpcode::kAllocate ||
                node->opcode() == IrOpcode::kFinishRegion);
    EXPECT_TRUE(escape_analysis()->IsVirtual(node));
  }

  void ExpectEscaped(Node* node) {
    EXPECT_TRUE(node->opcode() == IrOpcode::kAllocate ||
                node->opcode() == IrOpcode::kFinishRegion);
    EXPECT_TRUE(escape_analysis()->IsEscaped(node));
  }

  SimplifiedOperatorBuilder* simplified() { return &simplified_; }

  Node* effect() { return effect_; }

 private:
  SimplifiedOperatorBuilder simplified_;
  JSGraph jsgraph_;
  EscapeAnalysis escape_analysis_;

  Node* effect_;
  Node* control_;
};


// -----------------------------------------------------------------------------
// Test cases.


TEST_F(EscapeAnalysisTest, StraightNonEscape) {
  Node* object1 = Constant(1);
  BeginRegion();
  Node* allocation = Allocate(Constant(kPointerSize));
  Store(AccessAtIndex(0), allocation, object1);
  Node* finish = FinishRegion(allocation);
  Node* load = Load(AccessAtIndex(0), finish);
  Node* result = Return(load);
  EndGraph();
  Analysis();

  ExpectVirtual(allocation);
  ExpectReplacement(load, object1);

  Transformation();

  ASSERT_EQ(object1, NodeProperties::GetValueInput(result, 0));
}


TEST_F(EscapeAnalysisTest, StraightEscape) {
  Node* object1 = Constant(1);
  BeginRegion();
  Node* allocation = Allocate(Constant(kPointerSize));
  Store(AccessAtIndex(0), allocation, object1);
  Node* finish = FinishRegion(allocation);
  Node* load = Load(AccessAtIndex(0), finish);
  Node* result = Return(allocation);
  EndGraph();
  Analysis();

  ExpectEscaped(allocation);
  ExpectReplacement(load, object1);

  Transformation();

  ASSERT_EQ(allocation, NodeProperties::GetValueInput(result, 0));
}


TEST_F(EscapeAnalysisTest, StoreLoadEscape) {
  Node* object1 = Constant(1);

  BeginRegion();
  Node* allocation1 = Allocate(Constant(kPointerSize));
  Store(AccessAtIndex(0), allocation1, object1);
  Node* finish1 = FinishRegion(allocation1);

  BeginRegion();
  Node* allocation2 = Allocate(Constant(kPointerSize));
  Store(AccessAtIndex(0), allocation2, finish1);
  Node* finish2 = FinishRegion(allocation2);

  Node* load = Load(AccessAtIndex(0), finish2);
  Node* result = Return(load);
  EndGraph();
  Analysis();

  ExpectEscaped(allocation1);
  ExpectVirtual(allocation2);
  ExpectReplacement(load, finish1);

  Transformation();

  ASSERT_EQ(finish1, NodeProperties::GetValueInput(result, 0));
}


TEST_F(EscapeAnalysisTest, BranchNonEscape) {
  Node* object1 = Constant(1);
  Node* object2 = Constant(2);
  BeginRegion();
  Node* allocation = Allocate(Constant(kPointerSize));
  Store(AccessAtIndex(0), allocation, object1);
  Node* finish = FinishRegion(allocation);
  Branch();
  Node* ifFalse = IfFalse();
  Node* ifTrue = IfTrue();
  Node* effect1 = Store(AccessAtIndex(0), allocation, object1, finish, ifFalse);
  Node* effect2 = Store(AccessAtIndex(0), allocation, object2, finish, ifTrue);
  Node* merge = Merge2(ifFalse, ifTrue);
  Node* phi = graph()->NewNode(common()->EffectPhi(2), effect1, effect2, merge);
  Node* load = Load(AccessAtIndex(0), finish, phi, merge);
  Node* result = Return(load, phi);
  EndGraph();
  Analysis();

  ExpectVirtual(allocation);
  ExpectReplacementPhi(load, object1, object2);
  Node* replacement_phi = escape_analysis()->GetReplacement(load);

  Transformation();

  ASSERT_EQ(replacement_phi, NodeProperties::GetValueInput(result, 0));
}


TEST_F(EscapeAnalysisTest, DanglingLoadOrder) {
  Node* object1 = Constant(1);
  Node* object2 = Constant(2);
  Node* allocation = Allocate(Constant(kPointerSize));
  Node* store1 = Store(AccessAtIndex(0), allocation, object1);
  Node* load1 = Load(AccessAtIndex(0), allocation);
  Store(AccessAtIndex(0), allocation, object2);
  Node* load2 = Load(AccessAtIndex(0), allocation, store1);
  Node* result = Return(load2);
  EndGraph();

  Analysis();

  ExpectVirtual(allocation);
  ExpectReplacement(load1, object1);
  ExpectReplacement(load2, object1);

  Transformation();

  ASSERT_EQ(object1, NodeProperties::GetValueInput(result, 0));
}

}  // namespace compiler
}  // namespace internal
}  // namespace v8
