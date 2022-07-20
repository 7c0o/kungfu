import {
  getUIComponents,
  loadExtScripts,
  loadExtComponents,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { App, defineAsyncComponent } from 'vue';

import { Router } from 'vue-router';
import { useGlobalStore } from './store/global';

import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export const useComponenets = (
  app: App<Element>,
  router: Router,
): Promise<void> => {
  app.component(
    t('Pos'),
    defineAsyncComponent(
      () =>
        import('@kungfu-trader/kungfu-app/src/components/modules/pos/Pos.vue'),
    ),
  );

  app.component(
    t('PosGlobal'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/posGlobal/PosGlobal.vue'
        ),
    ),
  );

  app.component(
    t('Order'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/order/Order.vue'
        ),
    ),
  );

  app.component(
    t('Trade'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/trade/Trade.vue'
        ),
    ),
  );

  app.component(
    t('Td'),
    defineAsyncComponent(
      () =>
        import('@kungfu-trader/kungfu-app/src/components/modules/td/Td.vue'),
    ),
  );

  app.component(
    t('Md'),
    defineAsyncComponent(
      () =>
        import('@kungfu-trader/kungfu-app/src/components/modules/md/Md.vue'),
    ),
  );

  app.component(
    t('Strategy'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/strategy/Strategy.vue'
        ),
    ),
  );

  app.component(
    t('TradingTask'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/tradingTask/TradingTask.vue'
        ),
    ),
  );

  app.component(
    t('MarketData'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/marketdata/MarketData.vue'
        ),
    ),
  );

  app.component(
    t('OrderBook'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/orderBook/OrderBook.vue'
        ),
    ),
  );

  app.component(
    t('MakeOrderDashboard'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/makeOrder/MakeOrderDashboard.vue'
        ),
    ),
  );

  app.component(
    t('FutureArbitrage'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/futureArbitrage/FutureArbitrage.vue'
        ),
    ),
  );

  app.config.globalProperties.$availKfBoards = [
    t('Pos'),
    t('PosGlobal'),
    t('Order'),
    t('Trade'),
    t('Td'),
    t('Md'),
    t('Strategy'),
    t('TradingTask'),
    t('MarketData'),
    t('OrderBook'),
    t('MakeOrderDashboard'),
    t('FutureArbitrage'),
  ];

  return useGlobalStore()
    .setKfUIExtConfigs()
    .then((configs) => getUIComponents(configs))
    .then((components) => loadExtScripts(components, app))
    .then((components) => loadExtComponents(components, app, router))
    .then(() => {
      return useGlobalStore().setKfUIExtConfigs();
    })
    .then(() => {
      useGlobalStore().setKfConfigList();
      useGlobalStore().setKfExtConfigs();
    });
};
