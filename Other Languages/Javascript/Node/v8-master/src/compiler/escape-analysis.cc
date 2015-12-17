// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/compiler/escape-analysis.h"

#include "src/base/flags.h"
#include "src/bootstrapper.h"
#include "src/compilation-dependencies.h"
#include "src/compiler/common-operator.h"
#include "src/compiler/graph-reducer.h"
#include "src/compiler/js-operator.h"
#include "src/compiler/node.h"
#include "src/compiler/node-matchers.h"
#include "src/compiler/node-properties.h"
#include "src/compiler/simplified-operator.h"
#include "src/objects-inl.h"
#include "src/type-cache.h"

namespace v8 {
namespace internal {
namespace compiler {


// ------------------------------VirtualObject----------------------------------


class VirtualObject : public ZoneObject {
 public:
  enum Status { kUntracked = 0, kTracked = 1 };
  VirtualObject(NodeId id, Zone* zone)
      : id_(id), status_(kUntracked), fields_(zone), phi_(zone) {}

  VirtualObject(const VirtualObject& other)
      : id_(other.id_),
        status_(other.status_),
        fields_(other.fields_),
        phi_(other.phi_) {}

  VirtualObject(NodeId id, Zone* zone, size_t field_number)
      : id_(id), status_(kTracked), fields_(zone), phi_(zone) {
    fields_.resize(field_number);
    phi_.resize(field_number, false);
  }

  Node* GetField(size_t offset) {
    if (offset < fields_.size()) {
      return fields_[offset];
    }
    return nullptr;
  }

  bool IsCreatedPhi(size_t offset) {
    if (offset < phi_.size()) {
      return phi_[offset];
    }
    return false;
  }

  bool SetField(size_t offset, Node* node, bool created_phi = false) {
    bool changed = fields_[offset] != node || phi_[offset] != created_phi;
    fields_[offset] = node;
    phi_[offset] = created_phi;
    if (changed && FLAG_trace_turbo_escape && node) {
      PrintF("Setting field %zu of #%d to #%d (%s)\n", offset, id(), node->id(),
             node->op()->mnemonic());
    }
    return changed;
  }
  bool IsVirtual() const { return status_ == kTracked; }
  bool IsTracked() const { return status_ != kUntracked; }

  Node** fields_array() { return &fields_.front(); }
  size_t field_count() { return fields_.size(); }
  bool ResizeFields(size_t field_count) {
    if (field_count != fields_.size()) {
      fields_.resize(field_count);
      phi_.resize(field_count);
      return true;
    }
    return false;
  }
  bool ClearAllFields() {
    bool changed = false;
    for (size_t i = 0; i < fields_.size(); ++i) {
      if (fields_[i] != nullptr) {
        fields_[i] = nullptr;
        changed = true;
      }
      phi_[i] = false;
    }
    return changed;
  }
  bool UpdateFrom(const VirtualObject& other);

  NodeId id() { return id_; }
  void id(NodeId id) { id_ = id; }

 private:
  NodeId id_;
  Status status_;
  ZoneVector<Node*> fields_;
  ZoneVector<bool> phi_;
};


bool VirtualObject::UpdateFrom(const VirtualObject& other) {
  bool changed = status_ != other.status_;
  status_ = other.status_;
  if (fields_.size() != other.fields_.size()) {
    fields_ = other.fields_;
    return true;
  }
  for (size_t i = 0; i < fields_.size(); ++i) {
    if (fields_[i] != other.fields_[i]) {
      changed = true;
      fields_[i] = other.fields_[i];
    }
  }
  return changed;
}


// ------------------------------VirtualState-----------------------------------


class VirtualState : public ZoneObject {
 public:
  VirtualState(Zone* zone, size_t size);
  VirtualState(const VirtualState& states);

  VirtualObject* GetVirtualObject(Node* node);
  VirtualObject* GetVirtualObject(size_t id);
  VirtualObject* GetOrCreateTrackedVirtualObject(NodeId id, Zone* zone);
  void SetVirtualObject(NodeId id, VirtualObject* state);
  void LastChangedAt(Node* node) { last_changed_ = node; }
  Node* GetLastChanged() { return last_changed_; }
  bool UpdateFrom(NodeId id, VirtualObject* state, Zone* zone);
  bool UpdateFrom(VirtualState* state, Zone* zone);
  bool MergeFrom(MergeCache* cache, Zone* zone, Graph* graph,
                 CommonOperatorBuilder* common, Node* control);

  size_t size() { return info_.size(); }

 private:
  ZoneVector<VirtualObject*> info_;
  Node* last_changed_;
};


VirtualState::VirtualState(Zone* zone, size_t size)
    : info_(zone), last_changed_(nullptr) {
  info_.resize(size);
}


VirtualState::VirtualState(const VirtualState& state)
    : info_(state.info_.get_allocator().zone()),
      last_changed_(state.last_changed_) {
  info_.resize(state.info_.size());
  for (size_t i = 0; i < state.info_.size(); ++i) {
    if (state.info_[i] && state.info_[i]->id() == i) {
      info_[i] = new (state.info_.get_allocator().zone())
          VirtualObject(*state.info_[i]);
    }
  }
  for (size_t i = 0; i < state.info_.size(); ++i) {
    if (state.info_[i] && state.info_[i]->id() != i) {
      info_[i] = info_[state.info_[i]->id()];
    }
  }
}


VirtualObject* VirtualState::GetVirtualObject(size_t id) {
  if (id >= info_.size()) return nullptr;
  return info_[id];
}


VirtualObject* VirtualState::GetVirtualObject(Node* node) {
  return GetVirtualObject(node->id());
}


VirtualObject* VirtualState::GetOrCreateTrackedVirtualObject(NodeId id,
                                                             Zone* zone) {
  if (VirtualObject* obj = GetVirtualObject(id)) {
    return obj;
  }
  VirtualObject* obj = new (zone) VirtualObject(id, zone, 0);
  SetVirtualObject(id, obj);
  return obj;
}


void VirtualState::SetVirtualObject(NodeId id, VirtualObject* obj) {
  info_[id] = obj;
}


bool VirtualState::UpdateFrom(NodeId id, VirtualObject* fromObj, Zone* zone) {
  VirtualObject* obj = GetVirtualObject(id);
  if (!obj) {
    obj = new (zone) VirtualObject(*fromObj);
    SetVirtualObject(id, obj);
    if (FLAG_trace_turbo_escape) {
      PrintF("  Taking field for #%d from %p\n", id,
             static_cast<void*>(fromObj));
    }
    return true;
  }

  if (obj->UpdateFrom(*fromObj)) {
    if (FLAG_trace_turbo_escape) {
      PrintF("  Updating field for #%d from %p\n", id,
             static_cast<void*>(fromObj));
    }
    return true;
  }

  return false;
}


bool VirtualState::UpdateFrom(VirtualState* from, Zone* zone) {
  bool changed = false;
  for (NodeId id = 0; id < size(); ++id) {
    VirtualObject* ls = GetVirtualObject(id);
    VirtualObject* rs = from->GetVirtualObject(id);

    if (rs == nullptr) {
      continue;
    }

    if (ls == nullptr) {
      ls = new (zone) VirtualObject(*rs);
      SetVirtualObject(id, ls);
      changed = true;
      continue;
    }

    if (FLAG_trace_turbo_escape) {
      PrintF("  Updating fields of #%d\n", id);
    }

    changed = ls->UpdateFrom(*rs) || changed;
  }
  return false;
}


namespace {

size_t min_size(ZoneVector<VirtualState*>& states) {
  size_t min = SIZE_MAX;
  for (VirtualState* state : states) {
    min = std::min(state->size(), min);
  }
  return min;
}


size_t min_field_count(ZoneVector<VirtualObject*>& objs) {
  size_t min = SIZE_MAX;
  for (VirtualObject* obj : objs) {
    min = std::min(obj->field_count(), min);
  }
  return min;
}


void GetVirtualObjects(ZoneVector<VirtualState*> states,
                       ZoneVector<VirtualObject*>& objs, NodeId id) {
  objs.clear();
  for (VirtualState* state : states) {
    if (VirtualObject* obj = state->GetVirtualObject(id)) {
      objs.push_back(obj);
    }
  }
}


void GetVirtualObjects(VirtualState* state, ZoneVector<Node*> nodes,
                       ZoneVector<VirtualObject*>& objs) {
  objs.clear();
  for (Node* node : nodes) {
    if (VirtualObject* obj = state->GetVirtualObject(node)) {
      objs.push_back(obj);
    }
  }
}


Node* GetFieldIfSame(size_t pos, ZoneVector<VirtualObject*>& objs) {
  Node* rep = objs.front()->GetField(pos);
  for (VirtualObject* obj : objs) {
    if (obj->GetField(pos) != rep) {
      return nullptr;
    }
  }
  return rep;
}


void GetFields(ZoneVector<VirtualObject*>& objs, ZoneVector<Node*>& fields,
               size_t pos) {
  fields.clear();
  for (VirtualObject* obj : objs) {
    if (Node* field = obj->GetField(pos)) {
      fields.push_back(field);
    }
  }
}


bool IsEquivalentPhi(Node* node1, Node* node2) {
  if (node1 == node2) return true;
  if (node1->opcode() != IrOpcode::kPhi || node2->opcode() != IrOpcode::kPhi ||
      node1->op()->ValueInputCount() != node2->op()->ValueInputCount()) {
    return false;
  }
  for (int i = 0; i < node1->op()->ValueInputCount(); ++i) {
    Node* input1 = NodeProperties::GetValueInput(node1, i);
    Node* input2 = NodeProperties::GetValueInput(node2, i);
    if (!IsEquivalentPhi(input1, input2)) {
      return false;
    }
  }
  return true;
}


bool IsEquivalentPhi(Node* phi, ZoneVector<Node*>& inputs) {
  if (phi->opcode() != IrOpcode::kPhi) return false;
  if (phi->op()->ValueInputCount() != inputs.size()) {
    return false;
  }
  for (size_t i = 0; i < inputs.size(); ++i) {
    Node* input = NodeProperties::GetValueInput(phi, static_cast<int>(i));
    if (!IsEquivalentPhi(input, inputs[i])) {
      return false;
    }
  }
  return true;
}

}  // namespace


Node* EscapeAnalysis::GetReplacementIfSame(ZoneVector<VirtualObject*>& objs) {
  Node* rep = GetReplacement(objs.front()->id());
  for (VirtualObject* obj : objs) {
    if (GetReplacement(obj->id()) != rep) {
      return nullptr;
    }
  }
  return rep;
}


bool VirtualState::MergeFrom(MergeCache* cache, Zone* zone, Graph* graph,
                             CommonOperatorBuilder* common, Node* control) {
  DCHECK_GT(cache->states().size(), 0u);
  bool changed = false;
  for (NodeId id = 0; id < min_size(cache->states()); ++id) {
    GetVirtualObjects(cache->states(), cache->objects(), id);
    if (cache->objects().size() == cache->states().size()) {
      // Don't process linked objects.
      if (cache->objects()[0]->id() != id) continue;
      if (FLAG_trace_turbo_escape) {
        PrintF("  Merging virtual objects of #%d\n", id);
      }
      VirtualObject* mergeObject = GetOrCreateTrackedVirtualObject(id, zone);
      size_t fields = min_field_count(cache->objects());
      changed = mergeObject->ResizeFields(fields) || changed;
      for (size_t i = 0; i < fields; ++i) {
        if (Node* field = GetFieldIfSame(i, cache->objects())) {
          changed = mergeObject->SetField(i, field) || changed;
          if (FLAG_trace_turbo_escape) {
            PrintF("    Field %zu agree on rep #%d\n", i, field->id());
          }
        } else {
          GetFields(cache->objects(), cache->fields(), i);
          if (cache->fields().size() == cache->objects().size()) {
            Node* rep = mergeObject->GetField(i);
            if (!rep || !mergeObject->IsCreatedPhi(i)) {
              cache->fields().push_back(control);
              Node* phi =
                  graph->NewNode(common->Phi(MachineRepresentation::kTagged, 2),
                                 static_cast<int>(cache->fields().size()),
                                 &cache->fields().front());
              if (mergeObject->SetField(i, phi, true)) {
                if (FLAG_trace_turbo_escape) {
                  PrintF("    Creating Phi #%d as merge of", phi->id());
                  for (size_t i = 0; i + 1 < cache->fields().size(); i++) {
                    PrintF(" #%d (%s)", cache->fields()[i]->id(),
                           cache->fields()[i]->op()->mnemonic());
                  }
                  PrintF("\n");
                }
                changed = true;
              }
            } else {
              DCHECK(rep->opcode() == IrOpcode::kPhi);
              for (size_t n = 0; n < cache->fields().size(); ++n) {
                Node* old =
                    NodeProperties::GetValueInput(rep, static_cast<int>(n));
                if (old != cache->fields()[n]) {
                  changed = true;
                  NodeProperties::ReplaceValueInput(rep, cache->fields()[n],
                                                    static_cast<int>(n));
                }
              }
            }
          } else {
            changed = mergeObject->SetField(i, nullptr) || changed;
          }
        }
      }

    } else {
      SetVirtualObject(id, nullptr);
    }
  }
  // Update linked objects.
  for (NodeId id = 0; id < min_size(cache->states()); ++id) {
    GetVirtualObjects(cache->states(), cache->objects(), id);
    if (cache->objects().size() == cache->states().size()) {
      if (cache->objects()[0]->id() != id) {
        SetVirtualObject(id, GetVirtualObject(cache->objects()[0]->id()));
      }
    }
  }
  return changed;
}


// ------------------------------EscapeStatusAnalysis---------------------------


EscapeStatusAnalysis::EscapeStatusAnalysis(EscapeAnalysis* object_analysis,
                                           Graph* graph, Zone* zone)
    : object_analysis_(object_analysis),
      graph_(graph),
      zone_(zone),
      info_(zone),
      queue_(zone) {
  info_.resize(graph->NodeCount());
}


EscapeStatusAnalysis::~EscapeStatusAnalysis() {}


bool EscapeStatusAnalysis::HasEntry(Node* node) {
  return info_[node->id()] != kUnknown;
}


bool EscapeStatusAnalysis::IsVirtual(Node* node) {
  if (node->id() >= info_.size()) {
    return false;
  }
  return info_[node->id()] == kVirtual;
}


bool EscapeStatusAnalysis::IsEscaped(Node* node) {
  return info_[node->id()] == kEscaped;
}


bool EscapeStatusAnalysis::IsAllocation(Node* node) {
  return node->opcode() == IrOpcode::kAllocate ||
         node->opcode() == IrOpcode::kFinishRegion;
}


bool EscapeStatusAnalysis::SetEscaped(Node* node) {
  bool changed = info_[node->id()] != kEscaped;
  info_[node->id()] = kEscaped;
  return changed;
}


void EscapeStatusAnalysis::Run() {
  info_.resize(graph()->NodeCount());
  ZoneVector<bool> visited(zone());
  visited.resize(graph()->NodeCount());
  queue_.push_back(graph()->end());
  while (!queue_.empty()) {
    Node* node = queue_.front();
    queue_.pop_front();
    Process(node);
    if (!visited[node->id()]) {
      RevisitInputs(node);
    }
    visited[node->id()] = true;
  }
  if (FLAG_trace_turbo_escape) {
    DebugPrint();
  }
}


void EscapeStatusAnalysis::RevisitInputs(Node* node) {
  for (Edge edge : node->input_edges()) {
    Node* input = edge.to();
    queue_.push_back(input);
  }
}


void EscapeStatusAnalysis::RevisitUses(Node* node) {
  for (Edge edge : node->use_edges()) {
    Node* use = edge.from();
    queue_.push_back(use);
  }
}


void EscapeStatusAnalysis::Process(Node* node) {
  switch (node->opcode()) {
    case IrOpcode::kAllocate:
      ProcessAllocate(node);
      break;
    case IrOpcode::kFinishRegion:
      ProcessFinishRegion(node);
      break;
    case IrOpcode::kStoreField:
      ProcessStoreField(node);
      break;
    case IrOpcode::kStoreElement:
      ProcessStoreElement(node);
      break;
    case IrOpcode::kLoadField:
    case IrOpcode::kLoadElement: {
      if (Node* rep = object_analysis_->GetReplacement(node)) {
        if (IsAllocation(rep) && CheckUsesForEscape(node, rep)) {
          RevisitInputs(rep);
          RevisitUses(rep);
        }
      }
      break;
    }
    case IrOpcode::kPhi:
      if (!HasEntry(node)) {
        info_[node->id()] = kVirtual;
      }
      CheckUsesForEscape(node);
    default:
      break;
  }
}


void EscapeStatusAnalysis::ProcessStoreField(Node* node) {
  DCHECK_EQ(node->opcode(), IrOpcode::kStoreField);
  Node* to = NodeProperties::GetValueInput(node, 0);
  Node* val = NodeProperties::GetValueInput(node, 1);
  if ((IsEscaped(to) || !IsAllocation(to)) && SetEscaped(val)) {
    RevisitUses(val);
    RevisitInputs(val);
    if (FLAG_trace_turbo_escape) {
      PrintF("Setting #%d (%s) to escaped because of store to field of #%d\n",
             val->id(), val->op()->mnemonic(), to->id());
    }
  }
}


void EscapeStatusAnalysis::ProcessStoreElement(Node* node) {
  DCHECK_EQ(node->opcode(), IrOpcode::kStoreElement);
  Node* to = NodeProperties::GetValueInput(node, 0);
  Node* val = NodeProperties::GetValueInput(node, 2);
  if ((IsEscaped(to) || !IsAllocation(to)) && SetEscaped(val)) {
    RevisitUses(val);
    RevisitInputs(val);
    if (FLAG_trace_turbo_escape) {
      PrintF("Setting #%d (%s) to escaped because of store to field of #%d\n",
             val->id(), val->op()->mnemonic(), to->id());
    }
  }
}


void EscapeStatusAnalysis::ProcessAllocate(Node* node) {
  DCHECK_EQ(node->opcode(), IrOpcode::kAllocate);
  if (!HasEntry(node)) {
    info_[node->id()] = kVirtual;
    if (FLAG_trace_turbo_escape) {
      PrintF("Created status entry for node #%d (%s)\n", node->id(),
             node->op()->mnemonic());
    }
    NumberMatcher size(node->InputAt(0));
    if (!size.HasValue() && SetEscaped(node)) {
      RevisitUses(node);
      if (FLAG_trace_turbo_escape) {
        PrintF("Setting #%d to escaped because of non-const alloc\n",
               node->id());
      }
      // This node is known to escape, uses do not have to be checked.
      return;
    }
  }
  if (CheckUsesForEscape(node, true)) {
    RevisitUses(node);
  }
}


bool EscapeStatusAnalysis::CheckUsesForEscape(Node* uses, Node* rep,
                                              bool phi_escaping) {
  for (Edge edge : uses->use_edges()) {
    Node* use = edge.from();
    if (!NodeProperties::IsValueEdge(edge) &&
        !NodeProperties::IsContextEdge(edge))
      continue;
    switch (use->opcode()) {
      case IrOpcode::kStoreField:
      case IrOpcode::kLoadField:
      case IrOpcode::kStoreElement:
      case IrOpcode::kLoadElement:
      case IrOpcode::kFrameState:
      case IrOpcode::kStateValues:
      case IrOpcode::kReferenceEqual:
      case IrOpcode::kFinishRegion:
      case IrOpcode::kPhi:
        if (HasEntry(use) && IsEscaped(use) && SetEscaped(rep)) {
          if (FLAG_trace_turbo_escape) {
            PrintF(
                "Setting #%d (%s) to escaped because of use by escaping node "
                "#%d (%s)\n",
                rep->id(), rep->op()->mnemonic(), use->id(),
                use->op()->mnemonic());
          }
          return true;
        }
        if (phi_escaping && use->opcode() == IrOpcode::kPhi &&
            SetEscaped(rep)) {
          if (FLAG_trace_turbo_escape) {
            PrintF(
                "Setting #%d (%s) to escaped because of use by phi node "
                "#%d (%s)\n",
                rep->id(), rep->op()->mnemonic(), use->id(),
                use->op()->mnemonic());
          }
          return true;
        }
        break;
      case IrOpcode::kObjectIsSmi:
        if (!IsAllocation(rep) && SetEscaped(rep)) {
          PrintF("Setting #%d (%s) to escaped because of use by #%d (%s)\n",
                 rep->id(), rep->op()->mnemonic(), use->id(),
                 use->op()->mnemonic());
          return true;
        }
        break;
      default:
        if (use->op()->EffectInputCount() == 0 &&
            uses->op()->EffectInputCount() > 0) {
          PrintF("Encountered unaccounted use by #%d (%s)\n", use->id(),
                 use->op()->mnemonic());
          UNREACHABLE();
        }
        if (SetEscaped(rep)) {
          if (FLAG_trace_turbo_escape) {
            PrintF("Setting #%d (%s) to escaped because of use by #%d (%s)\n",
                   rep->id(), rep->op()->mnemonic(), use->id(),
                   use->op()->mnemonic());
          }
          return true;
        }
    }
  }
  return false;
}


void EscapeStatusAnalysis::ProcessFinishRegion(Node* node) {
  DCHECK_EQ(node->opcode(), IrOpcode::kFinishRegion);
  if (!HasEntry(node)) {
    info_[node->id()] = kVirtual;
    RevisitUses(node);
  }
  if (CheckUsesForEscape(node, true)) {
    RevisitInputs(node);
  }
}


void EscapeStatusAnalysis::DebugPrint() {
  for (NodeId id = 0; id < info_.size(); id++) {
    if (info_[id] != kUnknown) {
      PrintF("Node #%d is %s\n", id,
             info_[id] == kEscaped ? "escaping" : "virtual");
    }
  }
}


// -----------------------------EscapeAnalysis----------------------------------


EscapeAnalysis::EscapeAnalysis(Graph* graph, CommonOperatorBuilder* common,
                               Zone* zone)
    : graph_(graph),
      common_(common),
      zone_(zone),
      virtual_states_(zone),
      replacements_(zone),
      escape_status_(this, graph, zone),
      cache_(zone) {}


EscapeAnalysis::~EscapeAnalysis() {}


void EscapeAnalysis::Run() {
  replacements_.resize(graph()->NodeCount());
  RunObjectAnalysis();
  escape_status_.Run();
}


void EscapeAnalysis::RunObjectAnalysis() {
  virtual_states_.resize(graph()->NodeCount());
  ZoneVector<Node*> stack(zone());
  stack.push_back(graph()->start());
  while (!stack.empty()) {
    Node* node = stack.back();
    stack.pop_back();
    if (Process(node)) {
      for (Edge edge : node->use_edges()) {
        if (NodeProperties::IsEffectEdge(edge)) {
          Node* use = edge.from();
          if ((use->opcode() != IrOpcode::kLoadField &&
               use->opcode() != IrOpcode::kLoadElement) ||
              !IsDanglingEffectNode(use)) {
            stack.push_back(use);
          }
        }
      }
      // First process loads: dangling loads are a problem otherwise.
      for (Edge edge : node->use_edges()) {
        if (NodeProperties::IsEffectEdge(edge)) {
          Node* use = edge.from();
          if ((use->opcode() == IrOpcode::kLoadField ||
               use->opcode() == IrOpcode::kLoadElement) &&


              IsDanglingEffectNode(use)) {
            stack.push_back(use);
          }
        }
      }
    }
  }
  if (FLAG_trace_turbo_escape) {
    DebugPrint();
  }
}


bool EscapeAnalysis::IsDanglingEffectNode(Node* node) {
  if (node->op()->EffectInputCount() == 0) return false;
  if (node->op()->EffectOutputCount() == 0) return false;
  if (node->op()->EffectInputCount() == 1 &&
      NodeProperties::GetEffectInput(node)->opcode() == IrOpcode::kStart) {
    // The start node is used as sentinel for nodes that are in general
    // effectful, but of which an analysis has determined that they do not
    // produce effects in this instance. We don't consider these nodes dangling.
    return false;
  }
  for (Edge edge : node->use_edges()) {
    if (NodeProperties::IsEffectEdge(edge)) {
      return false;
    }
  }
  return true;
}


bool EscapeAnalysis::Process(Node* node) {
  switch (node->opcode()) {
    case IrOpcode::kAllocate:
      ProcessAllocation(node);
      break;
    case IrOpcode::kBeginRegion:
      ForwardVirtualState(node);
      break;
    case IrOpcode::kFinishRegion:
      ProcessFinishRegion(node);
      break;
    case IrOpcode::kStoreField:
      ProcessStoreField(node);
      break;
    case IrOpcode::kLoadField:
      ProcessLoadField(node);
      break;
    case IrOpcode::kStoreElement:
      ProcessStoreElement(node);
      break;
    case IrOpcode::kLoadElement:
      ProcessLoadElement(node);
      break;
    case IrOpcode::kStart:
      ProcessStart(node);
      break;
    case IrOpcode::kEffectPhi:
      return ProcessEffectPhi(node);
      break;
    default:
      if (node->op()->EffectInputCount() > 0) {
        ForwardVirtualState(node);
      }
      ProcessAllocationUsers(node);
      break;
  }
  return true;
}


void EscapeAnalysis::ProcessAllocationUsers(Node* node) {
  for (Edge edge : node->input_edges()) {
    Node* input = edge.to();
    if (!NodeProperties::IsValueEdge(edge) &&
        !NodeProperties::IsContextEdge(edge))
      continue;
    switch (node->opcode()) {
      case IrOpcode::kStoreField:
      case IrOpcode::kLoadField:
      case IrOpcode::kStoreElement:
      case IrOpcode::kLoadElement:
      case IrOpcode::kFrameState:
      case IrOpcode::kStateValues:
      case IrOpcode::kReferenceEqual:
      case IrOpcode::kFinishRegion:
      case IrOpcode::kPhi:
        break;
      default:
        VirtualState* state = virtual_states_[node->id()];
        if (VirtualObject* obj = ResolveVirtualObject(state, input)) {
          if (obj->ClearAllFields()) {
            state->LastChangedAt(node);
          }
        }
        break;
    }
  }
}


bool EscapeAnalysis::IsEffectBranchPoint(Node* node) {
  int count = 0;
  for (Edge edge : node->use_edges()) {
    if (NodeProperties::IsEffectEdge(edge)) {
      if (++count > 1) {
        return true;
      }
    }
  }
  return false;
}


void EscapeAnalysis::ForwardVirtualState(Node* node) {
  DCHECK_EQ(node->op()->EffectInputCount(), 1);
  if (node->opcode() != IrOpcode::kLoadField &&
      node->opcode() != IrOpcode::kLoadElement &&
      node->opcode() != IrOpcode::kLoad && IsDanglingEffectNode(node)) {
    PrintF("Dangeling effect node: #%d (%s)\n", node->id(),
           node->op()->mnemonic());
    UNREACHABLE();
  }
  Node* effect = NodeProperties::GetEffectInput(node);
  // Break the cycle for effect phis.
  if (effect->opcode() == IrOpcode::kEffectPhi) {
    if (virtual_states_[effect->id()] == nullptr) {
      virtual_states_[effect->id()] =
          new (zone()) VirtualState(zone(), graph()->NodeCount());
    }
  }
  DCHECK_NOT_NULL(virtual_states_[effect->id()]);
  if (IsEffectBranchPoint(effect)) {
    if (FLAG_trace_turbo_escape) {
      PrintF("Copying object state %p from #%d (%s) to #%d (%s)\n",
             static_cast<void*>(virtual_states_[effect->id()]), effect->id(),
             effect->op()->mnemonic(), node->id(), node->op()->mnemonic());
    }
    if (!virtual_states_[node->id()]) {
      virtual_states_[node->id()] =
          new (zone()) VirtualState(*virtual_states_[effect->id()]);
    } else {
      virtual_states_[node->id()]->UpdateFrom(virtual_states_[effect->id()],
                                              zone());
    }
  } else {
    virtual_states_[node->id()] = virtual_states_[effect->id()];
    if (FLAG_trace_turbo_escape) {
      PrintF("Forwarding object state %p from #%d (%s) to #%d (%s)\n",
             static_cast<void*>(virtual_states_[effect->id()]), effect->id(),
             effect->op()->mnemonic(), node->id(), node->op()->mnemonic());
    }
  }
}


void EscapeAnalysis::ProcessStart(Node* node) {
  DCHECK_EQ(node->opcode(), IrOpcode::kStart);
  virtual_states_[node->id()] =
      new (zone()) VirtualState(zone(), graph()->NodeCount());
}


bool EscapeAnalysis::ProcessEffectPhi(Node* node) {
  DCHECK_EQ(node->opcode(), IrOpcode::kEffectPhi);
  bool changed = false;

  VirtualState* mergeState = virtual_states_[node->id()];
  if (!mergeState) {
    mergeState = new (zone()) VirtualState(zone(), graph()->NodeCount());
    virtual_states_[node->id()] = mergeState;
    changed = true;
    if (FLAG_trace_turbo_escape) {
      PrintF("Effect Phi #%d got new states map %p.\n", node->id(),
             static_cast<void*>(mergeState));
    }
  } else if (mergeState->GetLastChanged() != node) {
    changed = true;
  }

  cache_.Clear();

  if (FLAG_trace_turbo_escape) {
    PrintF("At Effect Phi #%d, merging states into %p:", node->id(),
           static_cast<void*>(mergeState));
  }

  for (int i = 0; i < node->op()->EffectInputCount(); ++i) {
    Node* input = NodeProperties::GetEffectInput(node, i);
    VirtualState* state = virtual_states_[input->id()];
    if (state) {
      cache_.states().push_back(state);
    }
    if (FLAG_trace_turbo_escape) {
      PrintF(" %p (from %d %s)", static_cast<void*>(state), input->id(),
             input->op()->mnemonic());
    }
  }
  if (FLAG_trace_turbo_escape) {
    PrintF("\n");
  }

  if (cache_.states().size() == 0) {
    return changed;
  }

  changed = mergeState->MergeFrom(&cache_, zone(), graph(), common(),
                                  NodeProperties::GetControlInput(node)) ||
            changed;

  if (FLAG_trace_turbo_escape) {
    PrintF("Merge %s the node.\n", changed ? "changed" : "did not change");
  }

  if (changed) {
    mergeState->LastChangedAt(node);
  }
  return changed;
}


void EscapeAnalysis::ProcessAllocation(Node* node) {
  DCHECK_EQ(node->opcode(), IrOpcode::kAllocate);
  ForwardVirtualState(node);

  // Check if we have already processed this node.
  if (virtual_states_[node->id()]->GetVirtualObject(node)) return;

  NumberMatcher size(node->InputAt(0));
  if (size.HasValue()) {
    virtual_states_[node->id()]->SetVirtualObject(
        node->id(), new (zone()) VirtualObject(node->id(), zone(),
                                               size.Value() / kPointerSize));
  } else {
    virtual_states_[node->id()]->SetVirtualObject(
        node->id(), new (zone()) VirtualObject(node->id(), zone()));
  }
  virtual_states_[node->id()]->LastChangedAt(node);
}


void EscapeAnalysis::ProcessFinishRegion(Node* node) {
  DCHECK_EQ(node->opcode(), IrOpcode::kFinishRegion);
  ForwardVirtualState(node);
  Node* allocation = NodeProperties::GetValueInput(node, 0);
  if (allocation->opcode() == IrOpcode::kAllocate) {
    VirtualState* states = virtual_states_[node->id()];
    DCHECK_NOT_NULL(states->GetVirtualObject(allocation));
    if (!states->GetVirtualObject(node->id())) {
      states->SetVirtualObject(node->id(),
                               states->GetVirtualObject(allocation));
      if (FLAG_trace_turbo_escape) {
        PrintF("Linked finish region node #%d to node #%d\n", node->id(),
               allocation->id());
      }
      states->LastChangedAt(node);
    }
  }
}


Node* EscapeAnalysis::replacement(NodeId id) {
  if (id >= replacements_.size()) return nullptr;
  return replacements_[id];
}


Node* EscapeAnalysis::replacement(Node* node) {
  return replacement(node->id());
}


bool EscapeAnalysis::SetReplacement(Node* node, Node* rep) {
  bool changed = replacements_[node->id()] != rep;
  replacements_[node->id()] = rep;
  return changed;
}


bool EscapeAnalysis::UpdateReplacement(VirtualState* state, Node* node,
                                       Node* rep) {
  if (SetReplacement(node, rep)) {
    state->LastChangedAt(node);
    if (FLAG_trace_turbo_escape) {
      if (rep) {
        PrintF("Replacement of #%d is #%d (%s)\n", node->id(), rep->id(),
               rep->op()->mnemonic());
      } else {
        PrintF("Replacement of #%d cleared\n", node->id());
      }
    }
    return true;
  }
  return false;
}


Node* EscapeAnalysis::ResolveReplacement(Node* node) {
  while (replacement(node)) {
    node = replacement(node);
  }
  return node;
}


Node* EscapeAnalysis::GetReplacement(Node* node) {
  return GetReplacement(node->id());
}


Node* EscapeAnalysis::GetReplacement(NodeId id) {
  Node* node = nullptr;
  while (replacement(id)) {
    node = replacement(id);
    id = node->id();
  }
  return node;
}


bool EscapeAnalysis::IsVirtual(Node* node) {
  return escape_status_.IsVirtual(node);
}


bool EscapeAnalysis::IsEscaped(Node* node) {
  return escape_status_.IsEscaped(node);
}


bool EscapeAnalysis::SetEscaped(Node* node) {
  return escape_status_.SetEscaped(node);
}


VirtualObject* EscapeAnalysis::GetVirtualObject(Node* at, NodeId id) {
  if (VirtualState* states = virtual_states_[at->id()]) {
    return states->GetVirtualObject(id);
  }
  return nullptr;
}


VirtualObject* EscapeAnalysis::ResolveVirtualObject(VirtualState* state,
                                                    Node* node) {
  VirtualObject* obj = state->GetVirtualObject(ResolveReplacement(node));
  while (obj && replacement(obj->id()) &&
         state->GetVirtualObject(replacement(obj->id()))) {
    obj = state->GetVirtualObject(replacement(obj->id()));
  }
  return obj;
}


int EscapeAnalysis::OffsetFromAccess(Node* node) {
  DCHECK(OpParameter<FieldAccess>(node).offset % kPointerSize == 0);
  return OpParameter<FieldAccess>(node).offset / kPointerSize;
}


void EscapeAnalysis::ProcessLoadFromPhi(int offset, Node* from, Node* node,
                                        VirtualState* state) {
  if (FLAG_trace_turbo_escape) {
    PrintF("Load #%d from phi #%d", node->id(), from->id());
  }

  ZoneVector<Node*> inputs(zone());
  for (int i = 0; i < node->op()->ValueInputCount(); ++i) {
    Node* input = NodeProperties::GetValueInput(node, i);
    inputs.push_back(input);
  }

  GetVirtualObjects(state, inputs, cache_.objects());
  if (cache_.objects().size() == inputs.size()) {
    GetFields(cache_.objects(), cache_.fields(), offset);
    if (cache_.fields().size() == cache_.objects().size()) {
      Node* rep = replacement(node);
      if (!rep || !IsEquivalentPhi(rep, cache_.fields())) {
        cache_.fields().push_back(NodeProperties::GetControlInput(from));
        Node* phi = graph()->NewNode(
            common()->Phi(MachineRepresentation::kTagged, 2),
            static_cast<int>(cache_.fields().size()), &cache_.fields().front());
        SetReplacement(node, phi);
        state->LastChangedAt(node);
        if (FLAG_trace_turbo_escape) {
          PrintF(" got phi created.\n");
        }
      } else if (FLAG_trace_turbo_escape) {
        PrintF(" has already phi #%d.\n", rep->id());
      }
    } else if (FLAG_trace_turbo_escape) {
      PrintF(" has incomplete field info.\n");
    }
  } else if (FLAG_trace_turbo_escape) {
    PrintF(" has incomplete virtual object info.\n");
  }
}


void EscapeAnalysis::ProcessLoadField(Node* node) {
  DCHECK_EQ(node->opcode(), IrOpcode::kLoadField);
  ForwardVirtualState(node);
  Node* from = NodeProperties::GetValueInput(node, 0);
  VirtualState* state = virtual_states_[node->id()];
  if (VirtualObject* object = ResolveVirtualObject(state, from)) {
    int offset = OffsetFromAccess(node);
    if (!object->IsTracked()) return;
    Node* value = object->GetField(offset);
    if (value) {
      value = ResolveReplacement(value);
    }
    // Record that the load has this alias.
    UpdateReplacement(state, node, value);
  } else {
    if (from->opcode() == IrOpcode::kPhi &&
        OpParameter<FieldAccess>(node).offset % kPointerSize == 0) {
      int offset = OffsetFromAccess(node);
      // Only binary phis are supported for now.
      ProcessLoadFromPhi(offset, from, node, state);
    }
  }
}


void EscapeAnalysis::ProcessLoadElement(Node* node) {
  DCHECK_EQ(node->opcode(), IrOpcode::kLoadElement);
  ForwardVirtualState(node);
  Node* from = NodeProperties::GetValueInput(node, 0);
  VirtualState* state = virtual_states_[node->id()];
  Node* index_node = node->InputAt(1);
  NumberMatcher index(index_node);
  ElementAccess access = OpParameter<ElementAccess>(node);
  if (index.HasValue()) {
    int offset = index.Value() + access.header_size / kPointerSize;
    if (VirtualObject* object = ResolveVirtualObject(state, from)) {
      CHECK_GE(ElementSizeLog2Of(access.machine_type.representation()),
               kPointerSizeLog2);
      CHECK_EQ(access.header_size % kPointerSize, 0);

      if (!object->IsTracked()) return;
      Node* value = object->GetField(offset);
      if (value) {
        value = ResolveReplacement(value);
      }
      // Record that the load has this alias.
      UpdateReplacement(state, node, value);
    } else if (from->opcode() == IrOpcode::kPhi) {
      ElementAccess access = OpParameter<ElementAccess>(node);
      int offset = index.Value() + access.header_size / kPointerSize;
      ProcessLoadFromPhi(offset, from, node, state);
    }
  } else {
    // We have a load from a non-const index, cannot eliminate object.
    if (SetEscaped(from)) {
      if (FLAG_trace_turbo_escape) {
        PrintF(
            "Setting #%d (%s) to escaped because store element #%d to "
            "non-const "
            "index #%d (%s)\n",
            from->id(), from->op()->mnemonic(), node->id(), index_node->id(),
            index_node->op()->mnemonic());
      }
    }
  }
}


void EscapeAnalysis::ProcessStoreField(Node* node) {
  DCHECK_EQ(node->opcode(), IrOpcode::kStoreField);
  ForwardVirtualState(node);
  Node* to = NodeProperties::GetValueInput(node, 0);
  Node* val = NodeProperties::GetValueInput(node, 1);
  VirtualState* state = virtual_states_[node->id()];
  if (VirtualObject* obj = ResolveVirtualObject(state, to)) {
    if (!obj->IsTracked()) return;
    int offset = OffsetFromAccess(node);
    if (obj->SetField(offset, ResolveReplacement(val))) {
      state->LastChangedAt(node);
    }
  }
}


void EscapeAnalysis::ProcessStoreElement(Node* node) {
  DCHECK_EQ(node->opcode(), IrOpcode::kStoreElement);
  ForwardVirtualState(node);
  Node* to = NodeProperties::GetValueInput(node, 0);
  Node* index_node = node->InputAt(1);
  NumberMatcher index(index_node);
  ElementAccess access = OpParameter<ElementAccess>(node);
  Node* val = NodeProperties::GetValueInput(node, 2);
  if (index.HasValue()) {
    int offset = index.Value() + access.header_size / kPointerSize;
    VirtualState* states = virtual_states_[node->id()];
    if (VirtualObject* obj = ResolveVirtualObject(states, to)) {
      if (!obj->IsTracked()) return;
      CHECK_GE(ElementSizeLog2Of(access.machine_type.representation()),
               kPointerSizeLog2);
      CHECK_EQ(access.header_size % kPointerSize, 0);
      if (obj->SetField(offset, ResolveReplacement(val))) {
        states->LastChangedAt(node);
      }
    }
  } else {
    // We have a store to a non-const index, cannot eliminate object.
    if (SetEscaped(to)) {
      if (FLAG_trace_turbo_escape) {
        PrintF(
            "Setting #%d (%s) to escaped because store element #%d to "
            "non-const "
            "index #%d (%s)\n",
            to->id(), to->op()->mnemonic(), node->id(), index_node->id(),
            index_node->op()->mnemonic());
      }
    }
  }
}


void EscapeAnalysis::DebugPrintObject(VirtualObject* object, NodeId id) {
  PrintF("  Object #%d with %zu fields\n", id, object->field_count());
  for (size_t i = 0; i < object->field_count(); ++i) {
    if (Node* f = object->GetField(i)) {
      PrintF("    Field %zu = #%d (%s)\n", i, f->id(), f->op()->mnemonic());
    }
  }
}


void EscapeAnalysis::DebugPrintState(VirtualState* state) {
  PrintF("Dumping object state %p\n", static_cast<void*>(state));
  for (size_t id = 0; id < state->size(); id++) {
    if (VirtualObject* object = state->GetVirtualObject(id)) {
      if (object->id() == id) {
        DebugPrintObject(object, static_cast<int>(id));
      } else {
        PrintF("  Object #%zu links to object #%d\n", id, object->id());
      }
    }
  }
}


void EscapeAnalysis::DebugPrint() {
  ZoneVector<VirtualState*> object_states(zone());
  for (NodeId id = 0; id < virtual_states_.size(); id++) {
    if (VirtualState* states = virtual_states_[id]) {
      if (std::find(object_states.begin(), object_states.end(), states) ==
          object_states.end()) {
        object_states.push_back(states);
      }
    }
  }
  for (size_t n = 0; n < object_states.size(); n++) {
    DebugPrintState(object_states[n]);
  }
}

}  // namespace compiler
}  // namespace internal
}  // namespace v8
