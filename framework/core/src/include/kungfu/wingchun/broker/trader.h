//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_TRADER_H
#define WINGCHUN_TRADER_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/broker/broker.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/log.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::broker {

FORWARD_DECLARE_CLASS_PTR(Trader)

class TraderVendor : public BrokerVendor {
public:
  TraderVendor(locator_ptr locator, const std::string &group, const std::string &name, bool low_latency);

  void set_service(Trader_ptr service);

  void on_trading_day(const event_ptr &event, int64_t daytime) override;

protected:
  void on_start() override;

  BrokerService_ptr get_service() override;

private:
  Trader_ptr service_ = {};

  void clean_orders();
};

class Trader : public BrokerService {
  friend class TraderVendor;

public:
  typedef std::unordered_map<uint64_t, state<longfist::types::Order>> OrderMap;
  typedef std::unordered_map<uint64_t, state<longfist::types::OrderAction>> OrderActionMap;
  typedef std::unordered_map<uint64_t, state<longfist::types::Trade>> TradeMap;

  explicit Trader(BrokerVendor &vendor) : BrokerService(vendor){};

  [[nodiscard]] virtual longfist::enums::AccountType get_account_type() const = 0;

  virtual bool insert_block_message(const event_ptr &event) { return true; }

  virtual bool insert_order(const event_ptr &event) = 0;

  virtual bool cancel_order(const event_ptr &event) = 0;

  virtual bool req_position() = 0;

  virtual bool req_account() = 0;

  virtual bool req_history_order(const event_ptr &event) { return true; }

  virtual bool req_history_trade(const event_ptr &event) { return true; }

  virtual bool on_strategy_exit(const event_ptr &event) { return true; }

  /// 此函数自动发送一个空的AssetMargin数据. 两融柜台需要发送一个存有数据的AssetMargin, 请override此函数取消写入.
  /// 并且在使用writer写入完AssetMargin之后调用enable_asset_margin_sync()函数.
  /// 非两融柜台想要取消日志输出请override此函数.
  virtual bool write_default_asset_margin();

  [[nodiscard]] const std::string &get_account_id() const;

  [[nodiscard]] yijinjing::journal::writer_ptr get_asset_writer() const;

  [[nodiscard]] yijinjing::journal::writer_ptr get_asset_margin_writer() const;

  [[nodiscard]] yijinjing::journal::writer_ptr get_position_writer() const;

  void enable_asset_sync();

  void enable_asset_margin_sync();

  void enable_positions_sync();

protected:
  OrderMap orders_ = {};
  OrderActionMap actions_ = {};
  TradeMap trades_ = {};

private:
  bool sync_asset_ = false;
  bool sync_asset_margin_ = false;
  bool sync_position_ = false;

  void handle_asset_sync();
};
} // namespace kungfu::wingchun::broker

#endif // WINGCHUN_TRADER_H
