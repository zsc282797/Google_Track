// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/compiler/bytecode-graph-builder.h"

#include "src/compiler/linkage.h"
#include "src/compiler/operator-properties.h"
#include "src/interpreter/bytecode-array-iterator.h"
#include "src/interpreter/bytecodes.h"

namespace v8 {
namespace internal {
namespace compiler {

// Issues:
// - Need to deal with FrameState / FrameStateBeforeAndAfter / StateValue.
// - Scopes - intimately tied to AST. Need to eval what is needed.
// - Need to resolve closure parameter treatment.
BytecodeGraphBuilder::Environment::Environment(BytecodeGraphBuilder* builder,
                                               int register_count,
                                               int parameter_count,
                                               Node* control_dependency,
                                               Node* context)
    : builder_(builder),
      register_count_(register_count),
      parameter_count_(parameter_count),
      context_(context),
      control_dependency_(control_dependency),
      effect_dependency_(control_dependency),
      values_(builder->local_zone()) {
  // The layout of values_ is:
  //
  // [receiver] [parameters] [registers]
  //
  // parameter[0] is the receiver (this), parameters 1..N are the
  // parameters supplied to the method (arg0..argN-1). The accumulator
  // is stored separately.

  // Parameters including the receiver
  for (int i = 0; i < parameter_count; i++) {
    const char* debug_name = (i == 0) ? "%this" : nullptr;
    const Operator* op = common()->Parameter(i, debug_name);
    Node* parameter = builder->graph()->NewNode(op, graph()->start());
    values()->push_back(parameter);
  }

  // Registers
  register_base_ = static_cast<int>(values()->size());
  Node* undefined_constant = builder->jsgraph()->UndefinedConstant();
  values()->insert(values()->end(), register_count, undefined_constant);

  // Accumulator
  accumulator_ = undefined_constant;
}


int BytecodeGraphBuilder::Environment::RegisterToValuesIndex(
    interpreter::Register the_register) const {
  if (the_register.is_parameter()) {
    return the_register.ToParameterIndex(parameter_count());
  } else {
    return the_register.index() + register_base();
  }
}


void BytecodeGraphBuilder::Environment::BindRegister(
    interpreter::Register the_register, Node* node) {
  int values_index = RegisterToValuesIndex(the_register);
  values()->at(values_index) = node;
}


Node* BytecodeGraphBuilder::Environment::LookupRegister(
    interpreter::Register the_register) const {
  if (the_register.is_function_context()) {
    return builder()->GetFunctionContext();
  } else if (the_register.is_function_closure()) {
    return builder()->GetFunctionClosure();
  } else if (the_register.is_new_target()) {
    return builder()->GetNewTarget();
  } else {
    int values_index = RegisterToValuesIndex(the_register);
    return values()->at(values_index);
  }
}


void BytecodeGraphBuilder::Environment::BindAccumulator(Node* node) {
  accumulator_ = node;
}


Node* BytecodeGraphBuilder::Environment::LookupAccumulator() const {
  return accumulator_;
}


bool BytecodeGraphBuilder::Environment::IsMarkedAsUnreachable() const {
  return GetControlDependency()->opcode() == IrOpcode::kDead;
}


void BytecodeGraphBuilder::Environment::MarkAsUnreachable() {
  UpdateControlDependency(builder()->jsgraph()->Dead());
}


BytecodeGraphBuilder::BytecodeGraphBuilder(Zone* local_zone,
                                           CompilationInfo* compilation_info,
                                           JSGraph* jsgraph)
    : local_zone_(local_zone),
      info_(compilation_info),
      jsgraph_(jsgraph),
      input_buffer_size_(0),
      input_buffer_(nullptr),
      exit_controls_(local_zone) {
  bytecode_array_ = handle(info()->shared_info()->bytecode_array());
}


Node* BytecodeGraphBuilder::GetNewTarget() {
  if (!new_target_.is_set()) {
    int params = bytecode_array()->parameter_count();
    int index = Linkage::GetJSCallNewTargetParamIndex(params);
    const Operator* op = common()->Parameter(index, "%new.target");
    Node* node = NewNode(op, graph()->start());
    new_target_.set(node);
  }
  return new_target_.get();
}


Node* BytecodeGraphBuilder::GetFunctionContext() {
  if (!function_context_.is_set()) {
    int params = bytecode_array()->parameter_count();
    int index = Linkage::GetJSCallContextParamIndex(params);
    const Operator* op = common()->Parameter(index, "%context");
    Node* node = NewNode(op, graph()->start());
    function_context_.set(node);
  }
  return function_context_.get();
}


Node* BytecodeGraphBuilder::GetFunctionClosure() {
  if (!function_closure_.is_set()) {
    int index = Linkage::kJSCallClosureParamIndex;
    const Operator* op = common()->Parameter(index, "%closure");
    Node* node = NewNode(op, graph()->start());
    function_closure_.set(node);
  }
  return function_closure_.get();
}


Node* BytecodeGraphBuilder::BuildLoadObjectField(Node* object, int offset) {
  return NewNode(jsgraph()->machine()->Load(MachineType::AnyTagged()), object,
                 jsgraph()->IntPtrConstant(offset - kHeapObjectTag));
}


Node* BytecodeGraphBuilder::BuildLoadImmutableObjectField(Node* object,
                                                          int offset) {
  return graph()->NewNode(jsgraph()->machine()->Load(MachineType::AnyTagged()),
                          object,
                          jsgraph()->IntPtrConstant(offset - kHeapObjectTag),
                          graph()->start(), graph()->start());
}


Node* BytecodeGraphBuilder::BuildLoadNativeContextField(int index) {
  const Operator* op =
      javascript()->LoadContext(0, Context::NATIVE_CONTEXT_INDEX, true);
  Node* native_context = NewNode(op, environment()->Context());
  return NewNode(javascript()->LoadContext(0, index, true), native_context);
}


Node* BytecodeGraphBuilder::BuildLoadFeedbackVector() {
  if (!feedback_vector_.is_set()) {
    Node* closure = GetFunctionClosure();
    Node* shared = BuildLoadImmutableObjectField(
        closure, JSFunction::kSharedFunctionInfoOffset);
    Node* vector = BuildLoadImmutableObjectField(
        shared, SharedFunctionInfo::kFeedbackVectorOffset);
    feedback_vector_.set(vector);
  }
  return feedback_vector_.get();
}


VectorSlotPair BytecodeGraphBuilder::CreateVectorSlotPair(int slot_id) {
  Handle<TypeFeedbackVector> feedback_vector = info()->feedback_vector();
  FeedbackVectorSlot slot = feedback_vector->ToSlot(slot_id);
  return VectorSlotPair(feedback_vector, slot);
}


// TODO(mythria): Replace this function with one which adds real frame state.
// Also add before and after frame states and checkpointing if required.
void BytecodeGraphBuilder::AddEmptyFrameStateInputs(Node* node) {
  int frame_state_count =
      OperatorProperties::GetFrameStateInputCount(node->op());
  for (int i = 0; i < frame_state_count; i++) {
    NodeProperties::ReplaceFrameStateInput(node, i,
                                           jsgraph()->EmptyFrameState());
  }
}


bool BytecodeGraphBuilder::CreateGraph(bool stack_check) {
  // Set up the basic structure of the graph. Outputs for {Start} are
  // the formal parameters (including the receiver) plus context and
  // closure.

  // Set up the basic structure of the graph. Outputs for {Start} are the formal
  // parameters (including the receiver) plus new target, number of arguments,
  // context and closure.
  int actual_parameter_count = bytecode_array()->parameter_count() + 4;
  graph()->SetStart(graph()->NewNode(common()->Start(actual_parameter_count)));

  Environment env(this, bytecode_array()->register_count(),
                  bytecode_array()->parameter_count(), graph()->start(),
                  GetFunctionContext());
  set_environment(&env);

  CreateGraphBody(stack_check);

  // Finish the basic structure of the graph.
  DCHECK_NE(0u, exit_controls_.size());
  int const input_count = static_cast<int>(exit_controls_.size());
  Node** const inputs = &exit_controls_.front();
  Node* end = graph()->NewNode(common()->End(input_count), input_count, inputs);
  graph()->SetEnd(end);

  return true;
}


void BytecodeGraphBuilder::CreateGraphBody(bool stack_check) {
  // TODO(oth): Review ast-graph-builder equivalent, i.e. arguments
  // object setup, this function variable if used, tracing hooks.
  VisitBytecodes();
}


void BytecodeGraphBuilder::VisitBytecodes() {
  interpreter::BytecodeArrayIterator iterator(bytecode_array());
  while (!iterator.done()) {
    switch (iterator.current_bytecode()) {
#define BYTECODE_CASE(name, ...)       \
  case interpreter::Bytecode::k##name: \
    Visit##name(iterator);             \
    break;
      BYTECODE_LIST(BYTECODE_CASE)
#undef BYTECODE_CODE
    }
    iterator.Advance();
  }
}


void BytecodeGraphBuilder::VisitLdaZero(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* node = jsgraph()->ZeroConstant();
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitLdaSmi8(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* node = jsgraph()->Constant(iterator.GetImmediateOperand(0));
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitLdaConstantWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* node = jsgraph()->Constant(iterator.GetConstantForIndexOperand(0));
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitLdaConstant(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* node = jsgraph()->Constant(iterator.GetConstantForIndexOperand(0));
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitLdaUndefined(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* node = jsgraph()->UndefinedConstant();
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitLdaNull(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* node = jsgraph()->NullConstant();
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitLdaTheHole(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* node = jsgraph()->TheHoleConstant();
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitLdaTrue(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* node = jsgraph()->TrueConstant();
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitLdaFalse(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* node = jsgraph()->FalseConstant();
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitLdar(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* value = environment()->LookupRegister(iterator.GetRegisterOperand(0));
  environment()->BindAccumulator(value);
}


void BytecodeGraphBuilder::VisitStar(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* value = environment()->LookupAccumulator();
  environment()->BindRegister(iterator.GetRegisterOperand(0), value);
}


void BytecodeGraphBuilder::VisitMov(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* value = environment()->LookupRegister(iterator.GetRegisterOperand(0));
  environment()->BindRegister(iterator.GetRegisterOperand(1), value);
}


void BytecodeGraphBuilder::BuildLoadGlobal(
    const interpreter::BytecodeArrayIterator& iterator,
    TypeofMode typeof_mode) {
  Handle<Name> name =
      Handle<Name>::cast(iterator.GetConstantForIndexOperand(0));
  VectorSlotPair feedback = CreateVectorSlotPair(iterator.GetIndexOperand(1));

  const Operator* op = javascript()->LoadGlobal(name, feedback, typeof_mode);
  Node* node = NewNode(op, BuildLoadFeedbackVector());
  AddEmptyFrameStateInputs(node);
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitLdaGlobalSloppy(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_sloppy(language_mode()));
  BuildLoadGlobal(iterator, TypeofMode::NOT_INSIDE_TYPEOF);
}


void BytecodeGraphBuilder::VisitLdaGlobalStrict(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_strict(language_mode()));
  BuildLoadGlobal(iterator, TypeofMode::NOT_INSIDE_TYPEOF);
}


void BytecodeGraphBuilder::VisitLdaGlobalInsideTypeofSloppy(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_sloppy(language_mode()));
  BuildLoadGlobal(iterator, TypeofMode::INSIDE_TYPEOF);
}


void BytecodeGraphBuilder::VisitLdaGlobalInsideTypeofStrict(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_strict(language_mode()));
  BuildLoadGlobal(iterator, TypeofMode::INSIDE_TYPEOF);
}


void BytecodeGraphBuilder::VisitLdaGlobalSloppyWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_sloppy(language_mode()));
  BuildLoadGlobal(iterator, TypeofMode::NOT_INSIDE_TYPEOF);
}


void BytecodeGraphBuilder::VisitLdaGlobalStrictWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_strict(language_mode()));
  BuildLoadGlobal(iterator, TypeofMode::NOT_INSIDE_TYPEOF);
}


void BytecodeGraphBuilder::VisitLdaGlobalInsideTypeofSloppyWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_sloppy(language_mode()));
  BuildLoadGlobal(iterator, TypeofMode::INSIDE_TYPEOF);
}


void BytecodeGraphBuilder::VisitLdaGlobalInsideTypeofStrictWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_strict(language_mode()));
  BuildLoadGlobal(iterator, TypeofMode::INSIDE_TYPEOF);
}


void BytecodeGraphBuilder::BuildStoreGlobal(
    const interpreter::BytecodeArrayIterator& iterator) {
  Handle<Name> name =
      Handle<Name>::cast(iterator.GetConstantForIndexOperand(0));
  VectorSlotPair feedback = CreateVectorSlotPair(iterator.GetIndexOperand(1));
  Node* value = environment()->LookupAccumulator();

  const Operator* op =
      javascript()->StoreGlobal(language_mode(), name, feedback);
  Node* node = NewNode(op, value, BuildLoadFeedbackVector());
  AddEmptyFrameStateInputs(node);
}


void BytecodeGraphBuilder::VisitStaGlobalSloppy(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_sloppy(language_mode()));
  BuildStoreGlobal(iterator);
}


void BytecodeGraphBuilder::VisitStaGlobalStrict(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_strict(language_mode()));
  BuildStoreGlobal(iterator);
}

void BytecodeGraphBuilder::VisitStaGlobalSloppyWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_sloppy(language_mode()));
  BuildStoreGlobal(iterator);
}


void BytecodeGraphBuilder::VisitStaGlobalStrictWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_strict(language_mode()));
  BuildStoreGlobal(iterator);
}


void BytecodeGraphBuilder::VisitLdaContextSlot(
    const interpreter::BytecodeArrayIterator& iterator) {
  // TODO(mythria): LoadContextSlots are unrolled by the required depth when
  // generating bytecode. Hence the value of depth is always 0. Update this
  // code, when the implementation changes.
  // TODO(mythria): immutable flag is also set to false. This information is not
  // available in bytecode array. update this code when the implementation
  // changes.
  const Operator* op =
      javascript()->LoadContext(0, iterator.GetIndexOperand(1), false);
  Node* context = environment()->LookupRegister(iterator.GetRegisterOperand(0));
  Node* node = NewNode(op, context);
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitStaContextSlot(
    const interpreter::BytecodeArrayIterator& iterator) {
  // TODO(mythria): LoadContextSlots are unrolled by the required depth when
  // generating bytecode. Hence the value of depth is always 0. Update this
  // code, when the implementation changes.
  const Operator* op =
      javascript()->StoreContext(0, iterator.GetIndexOperand(1));
  Node* context = environment()->LookupRegister(iterator.GetRegisterOperand(0));
  Node* value = environment()->LookupAccumulator();
  Node* node = NewNode(op, context, value);
  CHECK(node != nullptr);
  environment()->BindAccumulator(value);
}


void BytecodeGraphBuilder::BuildNamedLoad(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* object = environment()->LookupRegister(iterator.GetRegisterOperand(0));
  Handle<Name> name =
      Handle<Name>::cast(iterator.GetConstantForIndexOperand(1));
  VectorSlotPair feedback = CreateVectorSlotPair(iterator.GetIndexOperand(2));

  const Operator* op = javascript()->LoadNamed(language_mode(), name, feedback);
  Node* node = NewNode(op, object, BuildLoadFeedbackVector());
  AddEmptyFrameStateInputs(node);
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitLoadICSloppy(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_sloppy(language_mode()));
  BuildNamedLoad(iterator);
}


void BytecodeGraphBuilder::VisitLoadICStrict(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_strict(language_mode()));
  BuildNamedLoad(iterator);
}


void BytecodeGraphBuilder::VisitLoadICSloppyWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_sloppy(language_mode()));
  BuildNamedLoad(iterator);
}


void BytecodeGraphBuilder::VisitLoadICStrictWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_strict(language_mode()));
  BuildNamedLoad(iterator);
}


void BytecodeGraphBuilder::BuildKeyedLoad(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* key = environment()->LookupAccumulator();
  Node* object = environment()->LookupRegister(iterator.GetRegisterOperand(0));
  VectorSlotPair feedback = CreateVectorSlotPair(iterator.GetIndexOperand(1));

  const Operator* op = javascript()->LoadProperty(language_mode(), feedback);
  Node* node = NewNode(op, object, key, BuildLoadFeedbackVector());
  AddEmptyFrameStateInputs(node);
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitKeyedLoadICSloppy(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_sloppy(language_mode()));
  BuildKeyedLoad(iterator);
}


void BytecodeGraphBuilder::VisitKeyedLoadICStrict(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_strict(language_mode()));
  BuildKeyedLoad(iterator);
}


void BytecodeGraphBuilder::VisitKeyedLoadICSloppyWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_sloppy(language_mode()));
  BuildKeyedLoad(iterator);
}


void BytecodeGraphBuilder::VisitKeyedLoadICStrictWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_strict(language_mode()));
  BuildKeyedLoad(iterator);
}


void BytecodeGraphBuilder::BuildNamedStore(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* value = environment()->LookupAccumulator();
  Node* object = environment()->LookupRegister(iterator.GetRegisterOperand(0));
  Handle<Name> name =
      Handle<Name>::cast(iterator.GetConstantForIndexOperand(1));
  VectorSlotPair feedback = CreateVectorSlotPair(iterator.GetIndexOperand(2));

  const Operator* op =
      javascript()->StoreNamed(language_mode(), name, feedback);
  Node* node = NewNode(op, object, value, BuildLoadFeedbackVector());
  AddEmptyFrameStateInputs(node);
  environment()->BindAccumulator(value);
}


void BytecodeGraphBuilder::VisitStoreICSloppy(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_sloppy(language_mode()));
  BuildNamedStore(iterator);
}


void BytecodeGraphBuilder::VisitStoreICStrict(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_strict(language_mode()));
  BuildNamedStore(iterator);
}


void BytecodeGraphBuilder::VisitStoreICSloppyWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_sloppy(language_mode()));
  BuildNamedStore(iterator);
}


void BytecodeGraphBuilder::VisitStoreICStrictWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_strict(language_mode()));
  BuildNamedStore(iterator);
}


void BytecodeGraphBuilder::BuildKeyedStore(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* value = environment()->LookupAccumulator();
  Node* object = environment()->LookupRegister(iterator.GetRegisterOperand(0));
  Node* key = environment()->LookupRegister(iterator.GetRegisterOperand(1));
  VectorSlotPair feedback = CreateVectorSlotPair(iterator.GetIndexOperand(2));

  const Operator* op = javascript()->StoreProperty(language_mode(), feedback);
  Node* node = NewNode(op, object, key, value, BuildLoadFeedbackVector());
  AddEmptyFrameStateInputs(node);
  environment()->BindAccumulator(value);
}


void BytecodeGraphBuilder::VisitKeyedStoreICSloppy(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_sloppy(language_mode()));
  BuildKeyedStore(iterator);
}


void BytecodeGraphBuilder::VisitKeyedStoreICStrict(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_strict(language_mode()));
  BuildKeyedStore(iterator);
}


void BytecodeGraphBuilder::VisitKeyedStoreICSloppyWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_sloppy(language_mode()));
  BuildKeyedStore(iterator);
}


void BytecodeGraphBuilder::VisitKeyedStoreICStrictWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_strict(language_mode()));
  BuildKeyedStore(iterator);
}


void BytecodeGraphBuilder::VisitPushContext(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* context = environment()->LookupAccumulator();
  environment()->BindRegister(iterator.GetRegisterOperand(0), context);
  environment()->SetContext(context);
}


void BytecodeGraphBuilder::VisitPopContext(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* context = environment()->LookupRegister(iterator.GetRegisterOperand(0));
  environment()->SetContext(context);
}


void BytecodeGraphBuilder::VisitCreateClosure(
    const interpreter::BytecodeArrayIterator& iterator) {
  Handle<SharedFunctionInfo> shared_info =
      Handle<SharedFunctionInfo>::cast(iterator.GetConstantForIndexOperand(0));
  PretenureFlag tenured =
      iterator.GetImmediateOperand(1) ? TENURED : NOT_TENURED;
  const Operator* op = javascript()->CreateClosure(shared_info, tenured);
  Node* closure = NewNode(op);
  AddEmptyFrameStateInputs(closure);
  environment()->BindAccumulator(closure);
}


void BytecodeGraphBuilder::VisitCreateClosureWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  VisitCreateClosure(iterator);
}


void BytecodeGraphBuilder::VisitCreateMappedArguments(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitCreateUnmappedArguments(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::BuildCreateLiteral(const Operator* op) {
  Node* literal = NewNode(op, GetFunctionClosure());
  AddEmptyFrameStateInputs(literal);
  environment()->BindAccumulator(literal);
}


void BytecodeGraphBuilder::BuildCreateRegExpLiteral(
    const interpreter::BytecodeArrayIterator& iterator) {
  Handle<String> constant_pattern =
      Handle<String>::cast(iterator.GetConstantForIndexOperand(0));
  int literal_index = iterator.GetIndexOperand(1);
  int literal_flags = iterator.GetImmediateOperand(2);
  const Operator* op = javascript()->CreateLiteralRegExp(
      constant_pattern, literal_flags, literal_index);
  BuildCreateLiteral(op);
}


void BytecodeGraphBuilder::VisitCreateRegExpLiteral(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCreateRegExpLiteral(iterator);
}


void BytecodeGraphBuilder::VisitCreateRegExpLiteralWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCreateRegExpLiteral(iterator);
}


void BytecodeGraphBuilder::BuildCreateArrayLiteral(
    const interpreter::BytecodeArrayIterator& iterator) {
  Handle<FixedArray> constant_elements =
      Handle<FixedArray>::cast(iterator.GetConstantForIndexOperand(0));
  int literal_index = iterator.GetIndexOperand(1);
  int literal_flags = iterator.GetImmediateOperand(2);
  const Operator* op = javascript()->CreateLiteralArray(
      constant_elements, literal_flags, literal_index);
  BuildCreateLiteral(op);
}


void BytecodeGraphBuilder::VisitCreateArrayLiteral(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCreateArrayLiteral(iterator);
}


void BytecodeGraphBuilder::VisitCreateArrayLiteralWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCreateArrayLiteral(iterator);
}


void BytecodeGraphBuilder::BuildCreateObjectLiteral(
    const interpreter::BytecodeArrayIterator& iterator) {
  Handle<FixedArray> constant_properties =
      Handle<FixedArray>::cast(iterator.GetConstantForIndexOperand(0));
  int literal_index = iterator.GetIndexOperand(1);
  int literal_flags = iterator.GetImmediateOperand(2);
  const Operator* op = javascript()->CreateLiteralObject(
      constant_properties, literal_flags, literal_index);
  BuildCreateLiteral(op);
}


void BytecodeGraphBuilder::VisitCreateObjectLiteral(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCreateObjectLiteral(iterator);
}


void BytecodeGraphBuilder::VisitCreateObjectLiteralWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCreateObjectLiteral(iterator);
}


Node* BytecodeGraphBuilder::ProcessCallArguments(const Operator* call_op,
                                                 Node* callee,
                                                 interpreter::Register receiver,
                                                 size_t arity) {
  Node** all = info()->zone()->NewArray<Node*>(static_cast<int>(arity));
  all[0] = callee;
  all[1] = environment()->LookupRegister(receiver);
  int receiver_index = receiver.index();
  for (int i = 2; i < static_cast<int>(arity); ++i) {
    all[i] = environment()->LookupRegister(
        interpreter::Register(receiver_index + i - 1));
  }
  Node* value = MakeNode(call_op, static_cast<int>(arity), all, false);
  return value;
}


void BytecodeGraphBuilder::BuildCall(
    const interpreter::BytecodeArrayIterator& iterator) {
  // TODO(rmcilroy): Set receiver_hint correctly based on whether the receiver
  // register has been loaded with null / undefined explicitly or we are sure it
  // is not null / undefined.
  ConvertReceiverMode receiver_hint = ConvertReceiverMode::kAny;
  Node* callee = environment()->LookupRegister(iterator.GetRegisterOperand(0));
  interpreter::Register receiver = iterator.GetRegisterOperand(1);
  size_t arg_count = iterator.GetCountOperand(2);
  VectorSlotPair feedback = CreateVectorSlotPair(iterator.GetIndexOperand(3));

  const Operator* call = javascript()->CallFunction(
      arg_count + 2, language_mode(), feedback, receiver_hint);
  Node* value = ProcessCallArguments(call, callee, receiver, arg_count + 2);
  AddEmptyFrameStateInputs(value);
  environment()->BindAccumulator(value);
}


void BytecodeGraphBuilder::VisitCall(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCall(iterator);
}


void BytecodeGraphBuilder::VisitCallWide(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCall(iterator);
}


void BytecodeGraphBuilder::VisitCallJSRuntime(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* callee = BuildLoadNativeContextField(iterator.GetIndexOperand(0));
  interpreter::Register receiver = iterator.GetRegisterOperand(1);
  size_t arg_count = iterator.GetCountOperand(2);

  // Create node to perform the JS runtime call.
  const Operator* call =
      javascript()->CallFunction(arg_count + 2, language_mode());
  Node* value = ProcessCallArguments(call, callee, receiver, arg_count + 2);
  AddEmptyFrameStateInputs(value);
  environment()->BindAccumulator(value);
}


Node* BytecodeGraphBuilder::ProcessCallRuntimeArguments(
    const Operator* call_runtime_op, interpreter::Register first_arg,
    size_t arity) {
  Node** all = info()->zone()->NewArray<Node*>(arity);
  int first_arg_index = first_arg.index();
  for (int i = 0; i < static_cast<int>(arity); ++i) {
    all[i] = environment()->LookupRegister(
        interpreter::Register(first_arg_index + i));
  }
  Node* value = MakeNode(call_runtime_op, static_cast<int>(arity), all, false);
  return value;
}


void BytecodeGraphBuilder::VisitCallRuntime(
    const interpreter::BytecodeArrayIterator& iterator) {
  Runtime::FunctionId functionId =
      static_cast<Runtime::FunctionId>(iterator.GetIndexOperand(0));
  interpreter::Register first_arg = iterator.GetRegisterOperand(1);
  size_t arg_count = iterator.GetCountOperand(2);

  // Create node to perform the runtime call.
  const Operator* call = javascript()->CallRuntime(functionId, arg_count);
  Node* value = ProcessCallRuntimeArguments(call, first_arg, arg_count);
  AddEmptyFrameStateInputs(value);
  environment()->BindAccumulator(value);
}


Node* BytecodeGraphBuilder::ProcessCallNewArguments(
    const Operator* call_new_op, interpreter::Register callee,
    interpreter::Register first_arg, size_t arity) {
  Node** all = info()->zone()->NewArray<Node*>(arity);
  all[0] = environment()->LookupRegister(callee);
  int first_arg_index = first_arg.index();
  for (int i = 1; i < static_cast<int>(arity) - 1; ++i) {
    all[i] = environment()->LookupRegister(
        interpreter::Register(first_arg_index + i - 1));
  }
  // Original constructor is the same as the callee.
  all[arity - 1] = environment()->LookupRegister(callee);
  Node* value = MakeNode(call_new_op, static_cast<int>(arity), all, false);
  return value;
}


void BytecodeGraphBuilder::VisitNew(
    const interpreter::BytecodeArrayIterator& iterator) {
  interpreter::Register callee = iterator.GetRegisterOperand(0);
  interpreter::Register first_arg = iterator.GetRegisterOperand(1);
  size_t arg_count = iterator.GetCountOperand(2);

  // TODO(turbofan): Pass the feedback here.
  const Operator* call = javascript()->CallConstruct(
      static_cast<int>(arg_count) + 2, VectorSlotPair());
  Node* value = ProcessCallNewArguments(call, callee, first_arg, arg_count + 2);
  AddEmptyFrameStateInputs(value);
  environment()->BindAccumulator(value);
}


void BytecodeGraphBuilder::VisitThrow(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* value = environment()->LookupAccumulator();
  // TODO(mythria): Change to Runtime::kThrow when we have deoptimization
  // information support in the interpreter.
  NewNode(javascript()->CallRuntime(Runtime::kReThrow, 1), value);
  Node* control = NewNode(common()->Throw(), value);
  UpdateControlDependencyToLeaveFunction(control);
  environment()->BindAccumulator(value);
}


void BytecodeGraphBuilder::BuildBinaryOp(
    const Operator* js_op, const interpreter::BytecodeArrayIterator& iterator) {
  Node* left = environment()->LookupRegister(iterator.GetRegisterOperand(0));
  Node* right = environment()->LookupAccumulator();
  Node* node = NewNode(js_op, left, right);

  AddEmptyFrameStateInputs(node);
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitAdd(
    const interpreter::BytecodeArrayIterator& iterator) {
  BinaryOperationHints hints = BinaryOperationHints::Any();
  BuildBinaryOp(javascript()->Add(language_mode(), hints), iterator);
}


void BytecodeGraphBuilder::VisitSub(
    const interpreter::BytecodeArrayIterator& iterator) {
  BinaryOperationHints hints = BinaryOperationHints::Any();
  BuildBinaryOp(javascript()->Subtract(language_mode(), hints), iterator);
}


void BytecodeGraphBuilder::VisitMul(
    const interpreter::BytecodeArrayIterator& iterator) {
  BinaryOperationHints hints = BinaryOperationHints::Any();
  BuildBinaryOp(javascript()->Multiply(language_mode(), hints), iterator);
}


void BytecodeGraphBuilder::VisitDiv(
    const interpreter::BytecodeArrayIterator& iterator) {
  BinaryOperationHints hints = BinaryOperationHints::Any();
  BuildBinaryOp(javascript()->Divide(language_mode(), hints), iterator);
}


void BytecodeGraphBuilder::VisitMod(
    const interpreter::BytecodeArrayIterator& iterator) {
  BinaryOperationHints hints = BinaryOperationHints::Any();
  BuildBinaryOp(javascript()->Modulus(language_mode(), hints), iterator);
}


void BytecodeGraphBuilder::VisitBitwiseOr(
    const interpreter::BytecodeArrayIterator& iterator) {
  BinaryOperationHints hints = BinaryOperationHints::Any();
  BuildBinaryOp(javascript()->BitwiseOr(language_mode(), hints), iterator);
}


void BytecodeGraphBuilder::VisitBitwiseXor(
    const interpreter::BytecodeArrayIterator& iterator) {
  BinaryOperationHints hints = BinaryOperationHints::Any();
  BuildBinaryOp(javascript()->BitwiseXor(language_mode(), hints), iterator);
}


void BytecodeGraphBuilder::VisitBitwiseAnd(
    const interpreter::BytecodeArrayIterator& iterator) {
  BinaryOperationHints hints = BinaryOperationHints::Any();
  BuildBinaryOp(javascript()->BitwiseAnd(language_mode(), hints), iterator);
}


void BytecodeGraphBuilder::VisitShiftLeft(
    const interpreter::BytecodeArrayIterator& iterator) {
  BinaryOperationHints hints = BinaryOperationHints::Any();
  BuildBinaryOp(javascript()->ShiftLeft(language_mode(), hints), iterator);
}


void BytecodeGraphBuilder::VisitShiftRight(
    const interpreter::BytecodeArrayIterator& iterator) {
  BinaryOperationHints hints = BinaryOperationHints::Any();
  BuildBinaryOp(javascript()->ShiftRight(language_mode(), hints), iterator);
}


void BytecodeGraphBuilder::VisitShiftRightLogical(
    const interpreter::BytecodeArrayIterator& iterator) {
  BinaryOperationHints hints = BinaryOperationHints::Any();
  BuildBinaryOp(javascript()->ShiftRightLogical(language_mode(), hints),
                iterator);
}


void BytecodeGraphBuilder::VisitInc(
    const interpreter::BytecodeArrayIterator& iterator) {
  const Operator* js_op =
      javascript()->Add(language_mode(), BinaryOperationHints::Any());
  Node* node = NewNode(js_op, environment()->LookupAccumulator(),
                       jsgraph()->OneConstant());

  AddEmptyFrameStateInputs(node);
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitDec(
    const interpreter::BytecodeArrayIterator& iterator) {
  const Operator* js_op =
      javascript()->Subtract(language_mode(), BinaryOperationHints::Any());
  Node* node = NewNode(js_op, environment()->LookupAccumulator(),
                       jsgraph()->OneConstant());

  AddEmptyFrameStateInputs(node);
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitLogicalNot(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* value = NewNode(javascript()->ToBoolean(ToBooleanHint::kAny),
                        environment()->LookupAccumulator());
  Node* node = NewNode(common()->Select(MachineRepresentation::kTagged), value,
                       jsgraph()->FalseConstant(), jsgraph()->TrueConstant());
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitTypeOf(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* node =
      NewNode(javascript()->TypeOf(), environment()->LookupAccumulator());
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::BuildDelete(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* key = environment()->LookupAccumulator();
  Node* object = environment()->LookupRegister(iterator.GetRegisterOperand(0));

  Node* node =
      NewNode(javascript()->DeleteProperty(language_mode()), object, key);
  AddEmptyFrameStateInputs(node);
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitDeletePropertyStrict(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_strict(language_mode()));
  BuildDelete(iterator);
}


void BytecodeGraphBuilder::VisitDeletePropertySloppy(
    const interpreter::BytecodeArrayIterator& iterator) {
  DCHECK(is_sloppy(language_mode()));
  BuildDelete(iterator);
}


void BytecodeGraphBuilder::BuildCompareOp(
    const Operator* js_op, const interpreter::BytecodeArrayIterator& iterator) {
  Node* left = environment()->LookupRegister(iterator.GetRegisterOperand(0));
  Node* right = environment()->LookupAccumulator();
  Node* node = NewNode(js_op, left, right);

  AddEmptyFrameStateInputs(node);
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitTestEqual(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCompareOp(javascript()->Equal(), iterator);
}


void BytecodeGraphBuilder::VisitTestNotEqual(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCompareOp(javascript()->NotEqual(), iterator);
}


void BytecodeGraphBuilder::VisitTestEqualStrict(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCompareOp(javascript()->StrictEqual(), iterator);
}


void BytecodeGraphBuilder::VisitTestNotEqualStrict(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCompareOp(javascript()->StrictNotEqual(), iterator);
}


void BytecodeGraphBuilder::VisitTestLessThan(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCompareOp(javascript()->LessThan(language_mode()), iterator);
}


void BytecodeGraphBuilder::VisitTestGreaterThan(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCompareOp(javascript()->GreaterThan(language_mode()), iterator);
}


void BytecodeGraphBuilder::VisitTestLessThanOrEqual(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCompareOp(javascript()->LessThanOrEqual(language_mode()), iterator);
}


void BytecodeGraphBuilder::VisitTestGreaterThanOrEqual(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCompareOp(javascript()->GreaterThanOrEqual(language_mode()), iterator);
}


void BytecodeGraphBuilder::VisitTestIn(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCompareOp(javascript()->HasProperty(), iterator);
}


void BytecodeGraphBuilder::VisitTestInstanceOf(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCompareOp(javascript()->InstanceOf(), iterator);
}


void BytecodeGraphBuilder::BuildCastOperator(
    const Operator* js_op, const interpreter::BytecodeArrayIterator& iterator) {
  Node* node = NewNode(js_op, environment()->LookupAccumulator());
  AddEmptyFrameStateInputs(node);
  environment()->BindAccumulator(node);
}


void BytecodeGraphBuilder::VisitToName(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCastOperator(javascript()->ToName(), iterator);
}


void BytecodeGraphBuilder::VisitToNumber(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCastOperator(javascript()->ToNumber(), iterator);
}


void BytecodeGraphBuilder::VisitToObject(
    const interpreter::BytecodeArrayIterator& iterator) {
  BuildCastOperator(javascript()->ToObject(), iterator);
}


void BytecodeGraphBuilder::VisitJump(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitJumpConstant(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitJumpIfTrue(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitJumpIfTrueConstant(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitJumpIfFalse(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitJumpIfFalseConstant(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitJumpIfToBooleanTrue(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitJumpIfToBooleanTrueConstant(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitJumpIfToBooleanFalse(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitJumpIfToBooleanFalseConstant(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitJumpIfNull(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitJumpIfNullConstant(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitJumpIfUndefined(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitJumpIfUndefinedConstant(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitReturn(
    const interpreter::BytecodeArrayIterator& iterator) {
  Node* control =
      NewNode(common()->Return(), environment()->LookupAccumulator());
  UpdateControlDependencyToLeaveFunction(control);
}


void BytecodeGraphBuilder::VisitForInPrepare(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitForInNext(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


void BytecodeGraphBuilder::VisitForInDone(
    const interpreter::BytecodeArrayIterator& iterator) {
  UNIMPLEMENTED();
}


Node** BytecodeGraphBuilder::EnsureInputBufferSize(int size) {
  if (size > input_buffer_size_) {
    size = size + kInputBufferSizeIncrement + input_buffer_size_;
    input_buffer_ = local_zone()->NewArray<Node*>(size);
    input_buffer_size_ = size;
  }
  return input_buffer_;
}


Node* BytecodeGraphBuilder::MakeNode(const Operator* op, int value_input_count,
                                     Node** value_inputs, bool incomplete) {
  DCHECK_EQ(op->ValueInputCount(), value_input_count);

  bool has_context = OperatorProperties::HasContextInput(op);
  int frame_state_count = OperatorProperties::GetFrameStateInputCount(op);
  bool has_control = op->ControlInputCount() == 1;
  bool has_effect = op->EffectInputCount() == 1;

  DCHECK_LT(op->ControlInputCount(), 2);
  DCHECK_LT(op->EffectInputCount(), 2);

  Node* result = NULL;
  if (!has_context && frame_state_count == 0 && !has_control && !has_effect) {
    result = graph()->NewNode(op, value_input_count, value_inputs, incomplete);
  } else {
    int input_count_with_deps = value_input_count;
    if (has_context) ++input_count_with_deps;
    input_count_with_deps += frame_state_count;
    if (has_control) ++input_count_with_deps;
    if (has_effect) ++input_count_with_deps;
    Node** buffer = EnsureInputBufferSize(input_count_with_deps);
    memcpy(buffer, value_inputs, kPointerSize * value_input_count);
    Node** current_input = buffer + value_input_count;
    if (has_context) {
      *current_input++ = environment()->Context();
    }
    for (int i = 0; i < frame_state_count; i++) {
      // The frame state will be inserted later. Here we misuse
      // the {Dead} node as a sentinel to be later overwritten
      // with the real frame state.
      *current_input++ = jsgraph()->Dead();
    }
    if (has_effect) {
      *current_input++ = environment()->GetEffectDependency();
    }
    if (has_control) {
      *current_input++ = environment()->GetControlDependency();
    }
    result = graph()->NewNode(op, input_count_with_deps, buffer, incomplete);
    if (!environment()->IsMarkedAsUnreachable()) {
      // Update the current control dependency for control-producing nodes.
      if (NodeProperties::IsControl(result)) {
        environment()->UpdateControlDependency(result);
      }
      // Update the current effect dependency for effect-producing nodes.
      if (result->op()->EffectOutputCount() > 0) {
        environment()->UpdateEffectDependency(result);
      }
      // Add implicit success continuation for throwing nodes.
      if (!result->op()->HasProperty(Operator::kNoThrow)) {
        const Operator* if_success = common()->IfSuccess();
        Node* on_success = graph()->NewNode(if_success, result);
        environment_->UpdateControlDependency(on_success);
      }
    }
  }

  return result;
}


Node* BytecodeGraphBuilder::MergeControl(Node* control, Node* other) {
  int inputs = control->op()->ControlInputCount() + 1;
  if (control->opcode() == IrOpcode::kLoop) {
    // Control node for loop exists, add input.
    const Operator* op = common()->Loop(inputs);
    control->AppendInput(graph_zone(), other);
    NodeProperties::ChangeOp(control, op);
  } else if (control->opcode() == IrOpcode::kMerge) {
    // Control node for merge exists, add input.
    const Operator* op = common()->Merge(inputs);
    control->AppendInput(graph_zone(), other);
    NodeProperties::ChangeOp(control, op);
  } else {
    // Control node is a singleton, introduce a merge.
    const Operator* op = common()->Merge(inputs);
    Node* merge_inputs[] = {control, other};
    control = graph()->NewNode(op, arraysize(merge_inputs), merge_inputs, true);
  }
  return control;
}


void BytecodeGraphBuilder::UpdateControlDependencyToLeaveFunction(Node* exit) {
  if (environment()->IsMarkedAsUnreachable()) return;
  environment()->MarkAsUnreachable();
  exit_controls_.push_back(exit);
}

}  // namespace compiler
}  // namespace internal
}  // namespace v8
