#include <kungfu/wingchun/basketorder/basketorderengine.h>

using namespace kungfu::rx;
using namespace kungfu::wingchun;
using namespace kungfu::wingchun::basketorder;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::util;

namespace kungfu::wingchun::basketorder {
BasketOrderEngine::BasketOrderEngine(apprentice &app) : app_(app) {}

void BasketOrderEngine::on_start(const rx::connectable_observable<event_ptr> &events) {
  restore(app_.get_state_bank());

  events | is(BasketOrder::tag) | $$(on_basket_order(event->trigger_time(), event->data<BasketOrder>()));
  events | is(Order::tag) | $$(update_basket_order(event->trigger_time(), event->data<Order>()));
}

void BasketOrderEngine::restore(const cache::bank &state_bank) {
  for (auto &pair : state_bank[boost::hana::type_c<BasketOrder>]) {
    auto basketorder_state = pair.second;
    make_basket_order_state(basketorder_state.update_time, basketorder_state.data);
  }

  for (auto &pair : state_bank[boost::hana::type_c<Order>]) {
    auto order_state = pair.second;
    try_update_basket_order(order_state.update_time, order_state.data);
  }
}

void BasketOrderEngine::on_basket_order(int64_t trigger_time, const longfist::types::BasketOrder &basket_order) {
  make_basket_order_state(trigger_time, basket_order);
}

void BasketOrderEngine::insert_basket_order(int64_t trigger_time, const longfist::types::BasketOrder &basket_order) {
  auto basket_order_state = make_basket_order_state(trigger_time, basket_order);
  app_.get_writer(basket_order.dest)->write(app_.now(), basket_order_state->get_state().data);
}

void BasketOrderEngine::update_basket_order(int64_t trigger_time, const longfist::types::Order &order) {

  if (not try_update_basket_order(trigger_time, order)) {
    return;
  }

  auto &basket_order_state = get_basket_order_state(order.parent_id);
  auto dest = basket_order_state->get_state().dest;
  app_.get_writer(dest)->write(app_.now(), basket_order_state->get_state().data);
}

bool BasketOrderEngine::try_update_basket_order(int64_t trigger_time, const longfist::types::Order &order) {
  if (order.parent_id == (uint64_t)0) {
    SPDLOG_DEBUG("not a basket order");
    return false;
  }

  if (not has_basket_order_state(order.parent_id)) {
    SPDLOG_ERROR(fmt::format("basket order is not exist {} {}", order.parent_id));
    return false;
  }

  auto &basket_order_state = get_basket_order_state(order.parent_id);
  basket_order_state->update(order);
}

bool BasketOrderEngine::has_basket_order_state(uint64_t basket_order_id) {
  return basket_order_states_.find(basket_order_id) != basket_order_states_.end();
}

state<longfist::types::BasketOrder> &BasketOrderEngine::get_basket_order(uint64_t basket_order_id) {
  return basket_order_states_.at(basket_order_id)->get_state();
}

BasketOrderState_ptr BasketOrderEngine::get_basket_order_state(uint64_t basket_order_id) {
  return basket_order_states_.at(basket_order_id);
}

BasketOrderState_ptr BasketOrderEngine::make_basket_order_state(int64_t trigger_time, const BasketOrder &basket_order) {
  auto basket_order_state =
      std::make_shared<BasketOrderState>(basket_order.source, basket_order.dest, trigger_time, basket_order);
  basket_order_states_.insert_or_assign(basket_order.order_id, basket_order_state);
  return basket_order_state;
}

} // namespace kungfu::wingchun::basketorder
