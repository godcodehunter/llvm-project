#ifndef LLVM_IR_OBSERVER_H
#define LLVM_IR_OBSERVER_H

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

static IRObserver GLOBAL_OBSERVER = IRObserver();

using Caller = std::string;

class CallerBuilder {

public:
  &CallerBuilder addScope(const std::string &name) { return this; }

  &CallerBuilder addUniq() { return this; }

  operator Caller() {}
}

class IRObserver {
  // TODO: ...
  using ID = uint64_t;

public:
  struct UpdateID {
    std::string ID;
  };

  struct UpdateSourceFile {
    std::string ID;
  };

  using UpdateModulePatches = std::variant<UpdateID, UpdateSourceFile>;

  struct UpdateModule {
    ID ID;
    Caller Caller;
    UpdateModulePatches patches;
  };

  struct CreateModule {
    ID ID;
    Caller Caller;
  };

  struct UpdateBB {
    ID ID;
    Caller Caller;
    // TODO: ...
  };

  struct MoveBB {
    ID ID;
    Caller Caller;
    ID Parent;
  };

  struct DeleteBB {
    ID ID;
    ID Caller;
  };

  struct CreateBB {
    ID ID;
    Caller Caller;
    Twine Name;
    ID Parent;
    ID InsertBefore;
  };

  using Move = std::variant<MoveBB>;
  using Delete = std::variant<DeleteBB>;
  using Update = std::variant<UpdateBB, UpdateModule>;
  using Create = std::variant<CreateBB, CreateModule>;

  using DeltaPatch = std::variant<Delete, Update, Create, Move>;

  std::vector<DeltaPatch> DeltaPatches;

  template <class V1, class V2> struct BiMap {
    std::map<V1, V2> forward;
    std::map<V2, V1> backward;

    template <typename T>
    auto Get(const T &val)
        -> std::optional<std::conditional_t<std::is_same_v<T, V1>, V2, V1>> {
      if constexpr (std::is_same_v<T, V1>) {
        if (auto search = forward.find(val); search != forward.end()) {
          return search->second;
        }
      } else if constexpr (std::is_same_v<T, V2>) {
        if (auto search = backward.find(val); search != backward.end()) {
          return search->second;
        }
      }
      return {};
    }

    void insert(const V1 &lhs, const V2 &rhs) {
      forward[lhs] = rhs;
      backward[rhs] = lhs;
    }
  };

  BiMap<ID, *Value> IdBiVal;

  ID createOrGetID(v *Value) {
    auto id = IdBiVal.Get(v);
    if (id) {
      return *id;
    }
    auto newId = ID();
    IdBiVal.insert(newId, v);
    return newId;
  }

public:
  void createBasicBlock(BasicBlock *This, const Caller *Caller,
                        const Twine &Name, Function *Parent,
                        BasicBlock *InsertBefore) {
    Create create = CreateBB{
        createOrGetID(This),         Caller, Name, createOrGetID(Parent),
        createOrGetID(InsertBefore),
    };
    DeltaPatches.push_back(create);
  }

  void deleteBasicBlock(BasicBlock *This, const Caller *Caller) {
    Delete del = DeleteBB{
        createOrGetID(This),
        Caller,
    };
    DeltaPatches.push_back(del);
  }

  void moveBasicBlock(BasicBlock *This, const Caller *Caller,
                      Function *Parent) {}

public:
  void createModule(Module *This, const Caller *Caller) {}

  void updateModule(Module *This, const Caller *Caller,
                    UpdateModulePatches patches) {
    Create create = CreateModule{
        createOrGetID(This),
        Caller,
    };
    DeltaPatches.push_back(del);
  }
};

#endif // LLVM_IR_OBSERVER_H