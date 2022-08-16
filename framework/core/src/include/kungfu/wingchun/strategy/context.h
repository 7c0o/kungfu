//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_CONTEXT_H
#define WINGCHUN_CONTEXT_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/book/bookkeeper.h>
#include <kungfu/wingchun/broker/client.h>
#include <kungfu/wingchun/strategy/strategy.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::strategy {
class Context : public std::enable_shared_from_this<Context> {
public:
  Context() = default;

  virtual ~Context() = default;

  /**
   * Get current time in nano seconds.
   * @return current time in nano seconds
   */
  virtual int64_t now() const = 0;

  /**
   * Add one shot timer callback.
   * @param nanotime when to call in nano seconds
   * @param callback callback function
   */
  virtual void add_timer(int64_t nanotime, const std::function<void(event_ptr)> &callback) = 0;

  /**
   * Add periodically callback.
   * @param duration duration in nano seconds
   * @param callback callback function
   */
  virtual void add_time_interval(int64_t duration, const std::function<void(event_ptr)> &callback) = 0;

  /**
   * Add account for strategy.
   * @param source TD group
   * @param account TD account ID
   */
  virtual void add_account(const std::string &source, const std::string &account) = 0;

  /**
   * Subscribe market data.
   * @param source MD group
   * @param instrument_ids instrument IDs
   * @param exchange_ids exchange IDs
   */
  virtual void subscribe(const std::string &source, const std::vector<std::string> &instrument_ids,
                         const std::string &exchange_ids) = 0;

  /**
   * Subscribe all from given MD
   * @param source MD group
   */
  virtual void subscribe_all(const std::string &source, uint8_t market_type = 0, uint64_t instrument_type = 0,
                             uint64_t data_type = 0) = 0;

  /**
   * Insert Block Message
   * @param opponent_seat
   * @param match_number
   * @param opponent_account
   * @param value
   * @return
   */
  virtual uint64_t insert_block_message(const std::string &source, const std::string &account, uint32_t opponent_seat,
                                        uint64_t match_number, const std::string &opponent_account,
                                        const std::string &value = "") = 0;

  /**
   * Insert order.
   * @param instrument_id instrument ID
   * @param exchange_id exchange ID
   * @param source source ID
   * @param account account ID
   * @param limit_price limit price
   * @param volume trade volume
   * @param type price type
   * @param side side
   * @param offset offset, defaults to longfist::enums::Offset::Open
   * @param hedge_flag hedge_flag, defaults to longfist::enums::HedgeFlag::Speculation
   * @param block_id BlockMessage id
   * @return
   */
  virtual uint64_t insert_order(const std::string &instrument_id, const std::string &exchange_id,
                                const std::string &source, const std::string &account, double limit_price,
                                int64_t volume, longfist::enums::PriceType type, longfist::enums::Side side,
                                longfist::enums::Offset offset,
                                longfist::enums::HedgeFlag hedge_flag = HedgeFlag::Speculation, bool is_swap = false,
                                uint64_t block_id = 0) = 0;

  /**
   * query history order
   */
  virtual void req_history_order(const std::string &source, const std::string &account, uint32_t query_num = 0) = 0;

  /**
   * query history trade
   */
  virtual void req_history_trade(const std::string &source, const std::string &account, uint32_t query_num = 0) = 0;

  /**
   * Cancel order.
   * @param order_id order ID
   * @return order action ID
   */
  virtual uint64_t cancel_order(uint64_t order_id) = 0;

  /**
   * Get current trading day.
   * @return current trading day
   */
  virtual int64_t get_trading_day() const = 0;

  /**
   * Tells whether the book is held.
   * If book is held, all traded positions will be recovered from ledger.
   * If book is not held, ledger will use the information collected in pre_start to build a fresh book.
   * @return true if book is held, false otherwise. Defaults to false.
   */
  [[nodiscard]] bool is_book_held() const;

  /**
   * Tells whether to mirror positions from enrolled accounts.
   * If positions are mirrored, will always have positions equal to the corresponding positions under accounts.
   * If positions are not mirrored, will set positions to 0 for newly added strategies, or the history values recorded
   * by kungfu.
   * @return true if positions are mirrored, false otherwise. Defaults to true.
   */
  [[nodiscard]] bool is_positions_mirrored() const;

  /**
   * Call to hold book.
   */
  void hold_book();

  /**
   * Call to hold positions, i.e. do not mirror positions.
   */
  void hold_positions();

  /**
   * request deregister.
   * @return void
   */
  virtual void req_deregister() {}

  /**
   * Update Strategy State
   * @param state StrategyState
   * @param infos vector<string>, info_a, info_b, info_c.
   */
  virtual void update_strategy_state(longfist::types::StrategyStateUpdate &state_update) {}

private:
  bool book_held_ = false;
  bool positions_mirrored_ = true;
};
} // namespace kungfu::wingchun::strategy

#endif // WINGCHUN_CONTEXT_H
