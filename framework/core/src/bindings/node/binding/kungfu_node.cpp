//
// Created by Keren Dong on 2019/12/25.
//

#ifdef _MSC_VER

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <delayimp.h>
#include <regex>
#include <stdio.h>
#include <string.h>
#include <windows.h>

static FARPROC WINAPI load_exe_hook(unsigned int event, DelayLoadInfo *info) {
  HMODULE m;
  if (event != dliNotePreLoadLibrary)
    return NULL;

  if (_stricmp(info->szDll, "NODE.EXE") != 0)
    return NULL;

  char buf[1024];
  auto length = GetModuleFileNameA(NULL, buf, sizeof(buf));
  std::string main_exe_name(buf);
  std::regex kfc_exe("kfc.exe");

  auto name_end = buf + length - strlen("kfc.exe");
  auto libnode_dll = std::regex_replace(main_exe_name, kfc_exe, "libnode.dll");

  m = _stricmp(name_end, "kfc.exe") != 0 ? GetModuleHandle(NULL) : GetModuleHandleA(libnode_dll.c_str());
  return (FARPROC)m;
}

decltype(__pfnDliNotifyHook2) __pfnDliNotifyHook2 = load_exe_hook;

#endif // _MSC_VER

#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/util/util.h>

#include "commission_store.h"
#include "config_store.h"
#include "data_table.h"
#include "history.h"
#include "io.h"
#include "journal.h"
#include "longfist.h"
#include "watcher.h"

using namespace kungfu::longfist;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::node;

namespace kungfu::node {

uint32_t Hash32(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 0, &Napi::Value::IsString)) {
    throw Napi::Error::New(info.Env(), "Invalid argument");
  }
  auto arg = info[0].ToString().Utf8Value();
  return hash_32((const unsigned char *)(arg.c_str()), arg.length());
}

Napi::Value Hash(const Napi::CallbackInfo &info) {
  return Napi::Number::New(info.Env(), Hash32(info));
}

Napi::Value FormatStringToHashHex(const Napi::CallbackInfo &info) {
  return Napi::String::New(info.Env(), fmt::format("{:08x}", Hash32(info)));
}

Napi::Value FormatTime(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 0, &Napi::Value::IsBigInt)) {
    return {};
  }
  auto format = IsValid(info, 1, &Napi::Value::IsString) ? info[1].ToString().Utf8Value() : KUNGFU_DATETIME_FORMAT;
  return Napi::String::New(info.Env(), time::strftime(GetBigInt(info, 0), format));
}

Napi::Value ParseTime(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 0, &Napi::Value::IsString) and IsValid(info, 1, &Napi::Value::IsString)) {
    return Napi::BigInt::New(info.Env(), TryParseTime(info, 0));
  }
  auto time_string = info[0].ToString().Utf8Value();
  auto format = info[1].ToString().Utf8Value();
  return Napi::BigInt::New(info.Env(), time::strptime(time_string, format));
}

void Shutdown(const Napi::CallbackInfo &info) {
  ensure_sqlite_shutdown();
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {

  ensure_sqlite_initilize();
  Longfist::Init(env, exports);
  History::Init(env, exports);
  ConfigStore::Init(env, exports);
  CommissionStore::Init(env, exports);
  Frame::Init(env, exports);
  Reader::Init(env, exports);
  Assemble::Init(env, exports);
  IODevice::Init(env, exports);
  DataTable::Init(env, exports);
  Watcher::Init(env, exports);
  exports.Set("hash", Napi::Function::New(env, Hash));
  exports.Set("formatStringToHashHex", Napi::Function::New(env, FormatStringToHashHex));
  exports.Set("formatTime", Napi::Function::New(env, FormatTime));
  exports.Set("parseTime", Napi::Function::New(env, ParseTime));
  exports.Set("shutdown", Napi::Function::New(env, Shutdown));
  return exports;
}
} // namespace kungfu::node

NODE_API_MODULE(addon, InitAll)