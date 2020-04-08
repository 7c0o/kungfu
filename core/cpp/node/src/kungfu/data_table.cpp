//
// Created by Keren Dong on 2020/4/8.
//

#include "data_table.h"

namespace kungfu::node {
Napi::FunctionReference DataTable::constructor;
DataTable::DataTable(const Napi::CallbackInfo &info) : ObjectWrap(info) {}

Napi::Value DataTable::Filter(const Napi::CallbackInfo &info) {
  if (info.Length() != 2) {
    return Napi::Value();
  }
  if (not info[0].IsString()) {
    return Napi::Value();
  }
  if (info[1].IsEmpty() or info[1].IsUndefined() or info[1].IsNull()) {
    return Napi::Value();
  }
  auto key = info[0].ToString().Utf8Value();
  auto result = constructor.New({info.This()});
  auto names = Value().GetPropertyNames();
  for (int i = 0; i < names.Length(); i++) {
    auto name = names.Get(i);
    auto data = Value().Get(name).ToObject();
    if (data.Get(key) == info[1]) {
      result.Set(name, data);
    }
  }
  return result;
}

Napi::Value DataTable::List(const Napi::CallbackInfo &info) {
  auto names = Value().GetPropertyNames();
  auto result = Napi::Array::New(info.Env(), names.Length());
  for (int i = 0; i < names.Length(); i++) {
    result.Set(i, Value().Get(names.Get(i)).ToObject());
  }
  return result;
}

Napi::Value DataTable::Merge(const Napi::CallbackInfo &info) {
  if (info.Length() == 0) {
    return Napi::Value();
  }
  for (int i = 0; i < info.Length(); i++) {
    if (info[i].IsEmpty() or info[i].IsUndefined() or info[i].IsNull() or not info[i].IsObject()) {
      return Napi::Value();
    }
  }
  auto result = constructor.New({info.This()});
  auto add_all = [&](auto &&target) {
    auto names = target.GetPropertyNames();
    for (int i = 0; i < names.Length(); i++) {
      auto name = names.Get(i);
      result.Set(name, target.Get(name).ToObject());
    }
  };
  add_all(Value().ToObject());
  for (int i = 0; i < info.Length(); i++) {
    add_all(info[i].ToObject());
  }
  return result;
}

Napi::Value DataTable::Range(const Napi::CallbackInfo &info) {
  if (info.Length() < 2) {
    return Napi::Value();
  }
  if (not info[0].IsString()) {
    return Napi::Value();
  }
  if (info[1].IsEmpty() or info[1].IsUndefined() or info[1].IsNull()) {
    return Napi::Value();
  }
  if (info[2].IsEmpty() or info[2].IsUndefined() or info[2].IsNull()) {
    return Napi::Value();
  }
  auto key = info[0].ToString().Utf8Value();
  auto result = constructor.New({info.This()});
  auto names = Value().GetPropertyNames();
  for (int i = 0; i < names.Length(); i++) {
    auto name = names.Get(i);
    auto data = Value().Get(name).ToObject();
    auto value = data.Get(key);
    auto add = [&](auto &val, auto &upper_bound, auto &lower_bound) {
      if (val >= lower_bound and val <= upper_bound) {
        result.Set(name, data);
      }
    };
    if (value.IsNumber()) {
      auto val = value.ToNumber().Int32Value();
      auto lower_bound = info[1].ToNumber().Int32Value();
      auto upper_bound = info[2].ToNumber().Int32Value();
      add(val, upper_bound, lower_bound);
    }
    if (value.IsBigInt()) {
      bool lossless = {};
      auto val = value.template As<Napi::BigInt>().Int64Value(&lossless);
      auto lower_bound = info[1].template As<Napi::BigInt>().Int64Value(&lossless);
      auto upper_bound = info[2].template As<Napi::BigInt>().Int64Value(&lossless);
      add(val, upper_bound, lower_bound);
    }
    if (value.IsString()) {
      auto val = value.ToString().Utf8Value();
      auto lower_bound = info[1].ToString().Utf8Value();
      auto upper_bound = info[2].ToString().Utf8Value();
      add(val, upper_bound, lower_bound);
    }
  }
  return result;
}

Napi::Value DataTable::Sort(const Napi::CallbackInfo &info) {
  if (info.Length() != 1 or not info[0].IsString()) {
    return Napi::Value();
  }

  auto key = info[0].ToString().Utf8Value();
  auto names = Value().GetPropertyNames();
  auto predicate = [&](auto &a, auto &b) {
    auto va = a.Get(key);
    auto vb = b.Get(key);
    if (va.IsNumber()) {
      return va.ToNumber().Int32Value() > vb.ToNumber().Int32Value();
    }
    if (va.IsBigInt()) {
      bool lossless = {};
      return va.template As<Napi::BigInt>().Int64Value(&lossless) >
             vb.template As<Napi::BigInt>().Int64Value(&lossless);
    }
    if (va.IsString()) {
      return va.ToString().Utf8Value() > vb.ToString().Utf8Value();
    }
    return va > vb;
  };

  std::vector<Napi::Object> sorted = {};
  for (int i = 0; i < names.Length(); i++) {
    auto name = names.Get(i);
    auto data = Value().Get(name).ToObject();
    sorted.insert(std::upper_bound(sorted.begin(), sorted.end(), data, predicate), data);
  }

  auto result = Napi::Array::New(info.Env(), sorted.size());
  for (int i = 0; i < sorted.size(); i++) {
    result.Set(i, sorted[i]);
  }
  return result;
}

void DataTable::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "DataTable",
                                    {
                                        InstanceMethod("filter", &DataTable::Filter),
                                        InstanceMethod("list", &DataTable::List),
                                        InstanceMethod("merge", &DataTable::Merge),
                                        InstanceMethod("range", &DataTable::Range),
                                        InstanceMethod("sort", &DataTable::Sort),
                                    });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("DataTable", func);
}

Napi::Value DataTable::NewInstance(const Napi::Value arg) { return constructor.New({arg}); }
} // namespace kungfu::node