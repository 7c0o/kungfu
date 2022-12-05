// SPDX-License-Identifier: Apache-2.0

//
// Created by Keren Dong on 2020/3/16.
//

#ifndef KUNGFU_NODE_COMMISSION_H
#define KUNGFU_NODE_COMMISSION_H

#include "common.h"
#include "operators.h"

#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/practice/profile.h>

namespace kungfu::node {
class CommissionStore : public Napi::ObjectWrap<CommissionStore> {
public:
  explicit CommissionStore(const Napi::CallbackInfo &info);

  ~CommissionStore() override = default;

  Napi::Value SetCommission(const Napi::CallbackInfo &info);

  Napi::Value GetCommission(const Napi::CallbackInfo &info);

  Napi::Value SetAllCommission(const Napi::CallbackInfo &info);

  Napi::Value GetAllCommission(const Napi::CallbackInfo &info);

  Napi::Value RemoveCommission(const Napi::CallbackInfo &info);

  static void Init(Napi::Env env, Napi::Object exports);

  static Napi::Value NewInstance(Napi::Value arg);

private:
  serialize::JsGet get = {};
  serialize::JsSet set = {};
  yijinjing::data::locator_ptr locator_;
  yijinjing::practice::profile profile_;

  static Napi::FunctionReference constructor;

  longfist::types::Commission ExtractCommission(const Napi::CallbackInfo &info);

  friend class Watcher;
};
} // namespace kungfu::node

#endif // KUNGFU_NODE_COMMISSION_H
