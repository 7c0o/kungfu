//
// Created by Keren Dong on 2020/1/14.
//

#ifndef KUNGFU_NODE_WATCHER_H
#define KUNGFU_NODE_WATCHER_H

#include <napi.h>
#include <uv.h>

#include "common.h"
#include "io.h"
#include "journal.h"
#include "operators.h"
#include <kungfu/wingchun/book/bookkeeper.h>
#include <kungfu/wingchun/broker/client.h>
#include <kungfu/yijinjing/cache/runtime.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::node {
constexpr uint64_t ID_TRANC = 0x00000000FFFFFFFF;
constexpr uint32_t PAGE_ID_MASK = 0x80000000;

class WatcherAutoClient : public wingchun::broker::SilentAutoClient {
public:
  explicit WatcherAutoClient(yijinjing::practice::apprentice &app, bool bypass_trading_data);

  ~WatcherAutoClient() = default;

  void connect(const event_ptr &event, const longfist::types::Register &register_data) override;

private:
  bool bypass_trading_data_;
};

class Watcher : public Napi::ObjectWrap<Watcher>, public yijinjing::practice::apprentice {
  typedef std::unordered_map<uint32_t, longfist::types::InstrumentKey> InstrumentKeyMap;

public:
  explicit Watcher(const Napi::CallbackInfo &info);

  ~Watcher() override;

  void NoSet(const Napi::CallbackInfo &info, const Napi::Value &value);

  Napi::Value HasLocation(const Napi::CallbackInfo &info);

  Napi::Value GetLocation(const Napi::CallbackInfo &info);

  Napi::Value GetLocationUID(const Napi::CallbackInfo &info);

  Napi::Value GetInstrumentUID(const Napi::CallbackInfo &info);

  Napi::Value GetConfig(const Napi::CallbackInfo &info);

  Napi::Value GetHistory(const Napi::CallbackInfo &info);

  Napi::Value GetCommission(const Napi::CallbackInfo &info);

  Napi::Value GetState(const Napi::CallbackInfo &info);

  Napi::Value GetLedger(const Napi::CallbackInfo &info);

  Napi::Value GetAppStates(const Napi::CallbackInfo &info);

  Napi::Value GetStrategyStates(const Napi::CallbackInfo &info);

  Napi::Value GetTradingDay(const Napi::CallbackInfo &info);

  Napi::Value Now(const Napi::CallbackInfo &info);

  Napi::Value IsUsable(const Napi::CallbackInfo &info);

  Napi::Value IsLive(const Napi::CallbackInfo &info);

  Napi::Value IsStarted(const Napi::CallbackInfo &info);

  Napi::Value RequestStop(const Napi::CallbackInfo &info);

  Napi::Value PublishState(const Napi::CallbackInfo &info);

  Napi::Value IsReadyToInteract(const Napi::CallbackInfo &info);

  Napi::Value IssueBlockMessage(const Napi::CallbackInfo &info);

  Napi::Value IssueOrder(const Napi::CallbackInfo &info);

  Napi::Value CancelOrder(const Napi::CallbackInfo &info);

  Napi::Value RequestMarketData(const Napi::CallbackInfo &info);

  Napi::Value Start(const Napi::CallbackInfo &info);

  Napi::Value Sync(const Napi::CallbackInfo &info);

  static void Init(Napi::Env env, Napi::Object exports);

protected:
  const bool bypass_accounting_;
  const bool bypass_trading_data_;

  void on_react() override;

  void on_start() override;

private:
  static Napi::FunctionReference constructor;
  uv_work_t uv_work_ = {};
  bool uv_work_live_ = false;
  WatcherAutoClient broker_client_;
  wingchun::book::Bookkeeper bookkeeper_;
  Napi::ObjectReference state_ref_;
  Napi::ObjectReference ledger_ref_;
  Napi::ObjectReference app_states_ref_;
  Napi::ObjectReference history_ref_;
  Napi::ObjectReference config_ref_;
  Napi::ObjectReference commission_ref_;
  Napi::ObjectReference strategy_states_ref_;
  serialize::JsUpdateState update_state;
  serialize::JsUpdateState update_ledger;
  serialize::JsPublishState publish;
  serialize::JsResetCache reset_cache;
  yijinjing::cache::bank data_bank_;
  yijinjing::cache::trading_bank trading_bank_;
  std::vector<kungfu::state<longfist::types::CacheReset>> reset_cache_states_;
  InstrumentKeyMap subscribed_instruments_ = {};
  std::unordered_map<uint32_t, int> location_uid_states_map_ = {};
  std::unordered_map<uint32_t, longfist::types::StrategyStateUpdate> location_uid_strategy_states_map_ = {};
  std::unordered_set<uint32_t> feeded_instruments_ = {};

  static constexpr auto bypass = [](yijinjing::practice::apprentice *app, bool bypass_quotes) {
    return rx::filter([&](const event_ptr &event) {
      return not(app->get_location(event->source())->category == longfist::enums::category::MD and
                 event->msg_type() != longfist::types::Instrument::tag and bypass_quotes);
    });
  };

  static constexpr auto is_subscribed = [](const InstrumentKeyMap &subscribed_instruments) {
    return rx::filter([&](const event_ptr &event) {
      return subscribed_instruments.find(event->data<longfist::types::Quote>().uid()) != subscribed_instruments.end();
    });
  };

  static constexpr auto is_trading_data = []() {
    return rx::filter([](const event_ptr &event) {
      bool is_target = false;
      boost::hana::for_each(longfist::TradingDataTypes, [&](auto it) {
        using DataType = typename decltype(+boost::hana::second(it))::type;
        is_target |= DataType::tag == event->msg_type();
      });
      return is_target;
    });
  };

  static constexpr auto while_is_trading_data = [](const event_ptr &event) {
    bool is_target = false;
    boost::hana::for_each(longfist::TradingDataTypes, [&](auto it) {
      using DataType = typename decltype(+boost::hana::second(it))::type;
      is_target |= DataType::tag == event->msg_type();
    });
    return is_target;
  };

  void Feed(const event_ptr &event, const longfist::types::Instrument &instrument);

  void RestoreState(const yijinjing::data::location_ptr &state_location, int64_t from, int64_t to, bool sync_schema);

  yijinjing::data::location_ptr FindLocation(const Napi::CallbackInfo &info);

  void InspectChannel(int64_t trigger_time, const longfist::types::Channel &channel);

  void MonitorMarketData(int64_t trigger_time, const yijinjing::data::location_ptr &md_location);

  void OnRegister(int64_t trigger_time, const longfist::types::Register &register_data);

  void OnDeregister(int64_t trigger_time, const longfist::types::Deregister &deregister_data);

  void UpdateBrokerState(uint32_t source_id, uint32_t dest_id, const longfist::types::BrokerStateUpdate &state);

  void UpdateStrategyState(uint32_t strategy_uid, const longfist::types::StrategyStateUpdate &state);

  void UpdateAsset(const event_ptr &event, uint32_t book_uid);

  void UpdateBook(const event_ptr &event, const longfist::types::Quote &quote);

  void UpdateBook(const event_ptr &event, const longfist::types::Position &position);

  void SyncLedger();

  void SyncOrder();

  void SyncAppStates();

  void SyncStrategyStates();

  void UpdateEventCache(const event_ptr &event);

  void SyncEventCache();

  void StartWorker();

  void CancelWorker();

  void AfterMasterDown();

  template <typename DataType>
  void feed_state_data_bank(const state<DataType> &state, yijinjing::cache::bank &receiver) {
    boost::hana::for_each(longfist::StateDataTypes, [&](auto it) {
      using DataTypeItem = typename decltype(+boost::hana::second(it))::type;
      if (std::is_same<DataType, DataTypeItem>::value) {
        receiver << state;
      }
    });
  };

  template <typename TradingData> void UpdateBook(const event_ptr &event, const TradingData &data) {
    auto update = [&](uint32_t source, uint32_t dest) {
      if (source == yijinjing::data::location::PUBLIC) {
        return;
      }
      auto location = get_location(source);
      auto book = bookkeeper_.get_book(source);
      auto &position = book->get_position_for(data);
      state<kungfu::longfist::types::Position> cache_state_position(source, dest, event->gen_time(), position);
      feed_state_data_bank(cache_state_position, data_bank_);
      state<kungfu::longfist::types::Asset> cache_state_asset(source, dest, event->gen_time(), book->asset);
      feed_state_data_bank(cache_state_asset, data_bank_);
      state<kungfu::longfist::types::AssetMargin> cache_state_asset_margin(source, dest, event->gen_time(),
                                                                           book->asset_margin);
      feed_state_data_bank(cache_state_asset_margin, data_bank_);
    };
    update(event->source(), event->dest());
    update(event->dest(), event->source());
  }

  template <typename TradingData>
  std::enable_if_t<std::is_same_v<TradingData, longfist::types::OrderInput>> UpdateBook(uint32_t source, uint32_t dest,
                                                                                        const TradingData &data) {
    bookkeeper_.on_order_input(now(), source, dest, data);
    state<kungfu::longfist::types::OrderInput> cache_state_order_input(source, dest, now(), data);
    trading_bank_ << cache_state_order_input;
  }

  template <typename TradingData>
  std::enable_if_t<not std::is_same_v<TradingData, longfist::types::OrderInput>>
  UpdateBook(uint32_t source, uint32_t dest, const TradingData &data) {}

  template <typename Instruction, typename IdPtrType = uint64_t Instruction::*>
  uint64_t WriteInstruction(int64_t trigger_time, Instruction instruction, IdPtrType id_ptr,
                            const yijinjing::data::location_ptr &account_location,
                            const yijinjing::data::location_ptr &strategy_location) {
    auto account_writer = get_writer(account_location->uid);
    uint64_t id_left = (uint64_t)(strategy_location->uid xor account_location->uid) << 32u;
    uint64_t id_right = (ID_TRANC & account_writer->current_frame_uid()) | PAGE_ID_MASK;
    instruction.*id_ptr = id_left | id_right;
    account_writer->write_as(trigger_time, instruction, strategy_location->uid, account_location->uid);
    UpdateBook(strategy_location->uid, account_location->uid, instruction);
    return instruction.*id_ptr;
  }

  template <typename DataType> void UpdateLedger(const boost::hana::basic_type<DataType> &type) {
    using DataTypeMap = std::unordered_map<uint64_t, state<DataType>>;
    auto &target_map = const_cast<DataTypeMap &>(data_bank_[type]);
    auto iter = target_map.begin();

    while (iter != target_map.end() and target_map.size() > 0) {
      auto &state = iter->second;
      update_ledger(state.update_time, state.source, state.dest, state.data);
      iter = target_map.erase(iter);
    }
  }

  template <typename DataType> void UpdateOrder(const boost::hana::basic_type<DataType> &type) {
    auto &order_queue = trading_bank_[type];
    int i = 0;
    kungfu::state<DataType> *pstate = nullptr;
    while (i < longfist::TRADING_MAP_RING_SIZE && order_queue.pop(pstate) && pstate != nullptr) {
      update_ledger(pstate->update_time, pstate->source, pstate->dest, pstate->data);
      i++;
    }
  }

  template <typename Instruction, typename IdPtrType = uint64_t Instruction::*>
  Napi::Value InteractWithTD(const Napi::CallbackInfo &info, IdPtrType id_ptr) {
    try {
      using namespace kungfu::rx;
      using namespace kungfu::longfist::types;
      using namespace kungfu::yijinjing;
      using namespace kungfu::yijinjing::data;

      auto account_location = ExtractLocation(info, 1, get_locator());
      if (not is_location_live(account_location->uid) or not has_writer(account_location->uid)) {
        return Napi::BigInt::New(info.Env(), std::uint64_t(0));
      }

      auto trigger_time = time::now_in_nano();
      auto account_writer = get_writer(account_location->uid);
      auto master_cmd_writer = get_writer(get_master_commands_uid());
      auto instruction_object = info[0].ToObject();
      Instruction instruction = {};
      serialize::JsGet{}(instruction_object, instruction);

      if (info.Length() == 2) {
        instruction.*id_ptr = account_writer->current_frame_uid();
        account_writer->write(trigger_time, instruction);
        UpdateBook(get_home_uid(), account_location->uid, instruction);
        return Napi::BigInt::New(info.Env(), instruction.*id_ptr);
      }

      auto strategy_location = ExtractLocation(info, 2, get_locator());

      if (not strategy_location) {
        return Napi::BigInt::New(info.Env(), std::uint64_t(0));
      }

      if (not has_location(strategy_location->uid)) {
        add_location(trigger_time, strategy_location);
        master_cmd_writer->write(trigger_time, *std::dynamic_pointer_cast<Location>(strategy_location));
      }

      if (has_channel(account_location->uid, strategy_location->uid)) {
        uint64_t id = WriteInstruction(trigger_time, instruction, id_ptr, account_location, strategy_location);
        return Napi::BigInt::New(info.Env(), id);
      }

      ChannelRequest request = {};
      request.dest_id = strategy_location->uid;
      request.source_id = ledger_home_location_->uid;
      master_cmd_writer->write(trigger_time, request);
      request.source_id = account_location->uid;
      master_cmd_writer->write(trigger_time, request);

      events_ | is(Channel::tag) | filter([account_location, strategy_location](const event_ptr &event) {
        const Channel &channel = event->data<Channel>();
        return channel.source_id == account_location->uid and channel.dest_id == strategy_location->uid;
      }) | first() |
          $([this, trigger_time, instruction, id_ptr, account_location, strategy_location](auto event) {
            // TODO: async make order / order action
            WriteInstruction(trigger_time, instruction, id_ptr, account_location, strategy_location);
          });

      return Napi::BigInt::New(info.Env(), std::uint64_t(0));
    } catch (const std::exception &ex) {
      throw Napi::Error::New(info.Env(), fmt::format("invalid order arguments: {}", ex.what()));
    } catch (...) {
      throw Napi::Error::New(info.Env(), "invalid order arguments");
    }
  };

  class BookListener : public wingchun::book::BookListener {
  public:
    explicit BookListener(Watcher &watcher);

    ~BookListener() = default;

    void on_position_sync_reset(const wingchun::book::Book &old_book, const wingchun::book::Book &new_book) override;

    void on_asset_sync_reset(const longfist::types::Asset &old_asset, const longfist::types::Asset &new_asset) override;

    void on_asset_margin_sync_reset(const longfist::types::AssetMargin &old_asset_margin,
                                    const longfist::types::AssetMargin &new_asset_margin) override;

  private:
    Watcher &watcher_;
  };

  DECLARE_PTR(BookListener);
};

} // namespace kungfu::node

#endif // KUNGFU_NODE_WATCHER_H
