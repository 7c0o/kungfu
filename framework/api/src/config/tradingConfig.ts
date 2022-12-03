import {
  InstrumentTypeEnum,
  InstrumentTypes,
  ProcessStatusTypes,
  HedgeFlagEnum,
  PriceTypeEnum,
  VolumeConditionEnum,
  TimeConditionEnum,
  CommissionModeEnum,
  OffsetEnum,
  SideEnum,
  DirectionEnum,
  KfCategoryEnum,
  OrderStatusEnum,
  BrokerStateStatusTypes,
  StrategyStateStatusTypes,
  FutureArbitrageCodeEnum,
  SpaceTabSettingEnum,
  SpaceSizeSettingEnum,
  StrategyExtTypes,
  UnderweightEnum,
  OrderInputKeyEnum,
} from '../typings/enums';

import { Pm2ProcessStatusTypes } from '../utils/processUtils';

import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export const Pm2ProcessStatus: Record<
  Pm2ProcessStatusTypes,
  KungfuApi.KfTradeValueCommonData
> = {
  ['online']: {
    name: t('tradingConfig.running'),
    color: 'kf-color-running',
    level: 2,
  },
  ['stopping']: {
    name: t('tradingConfig.stopping'),
    color: 'kf-color-waiting',
    level: 1,
  },
  ['stopped']: {
    name: t('tradingConfig.stopped'),
    color: 'kf-color-waiting',
    level: 0,
  },
  ['launching']: {
    name: t('tradingConfig.launching'),
    color: 'kf-color-waiting',
    level: 2,
  },
  ['errored']: {
    name: t('tradingConfig.error'),
    color: 'kf-color-error',
    level: -1,
  },
  ['waiting restart']: {
    name: t('tradingConfig.waiting_restart'),
    color: 'kf-color-waiting',
    level: 0,
  },
  ['one-launch-status']: {
    name: t('tradingConfig.launching'),
    color: 'kf-color-waiting',
    level: 2,
  },
};

export const StrategyStateStatus: Record<
  StrategyStateStatusTypes,
  KungfuApi.KfTradeValueCommonData
> = {
  ['Normal']: {
    name: t('正常'),
    color: 'kf-color-running',
    level: 2,
  },
  ['Warn']: {
    name: t('异常'),
    color: 'kf-color-waiting',
    level: 1,
  },
  ['Error']: {
    name: t('错误'),
    color: 'kf-color-error',
    level: -1,
  },
};

export const BrokerStateStatus: Record<
  BrokerStateStatusTypes,
  KungfuApi.KfTradeValueCommonData
> = {
  ['Pending']: {
    name: t('tradingConfig.pending'),
    color: 'kf-color-waiting',
    level: 1,
  },
  ['Idle']: {
    name: t('tradingConfig.Idle'),
    color: 'kf-color-waiting',
    level: 1,
  },
  ['DisConnected']: {
    name: t('tradingConfig.dis_connected'),
    color: 'kf-color-error',
    level: -1,
  },
  ['Connected']: {
    name: t('tradingConfig.connected'),
    color: 'kf-color-waiting',
    level: 1,
  },
  ['LoggedIn']: {
    name: t('tradingConfig.logged_in'),
    color: 'kf-color-waiting',
    level: 1,
  },
  ['LoginFailed']: {
    name: t('tradingConfig.login_failed'),
    color: 'kf-color-error',
    level: -1,
  },
  ['Ready']: {
    name: t('tradingConfig.ready'),
    color: 'kf-color-running',
    level: 2,
  },
};

export const AppStateStatus: Record<
  ProcessStatusTypes,
  KungfuApi.KfTradeValueCommonData
> = {
  ...StrategyStateStatus,
  ...Pm2ProcessStatus,
  ...BrokerStateStatus,
};

export const KfCategory: Record<
  KfCategoryEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [KfCategoryEnum.system]: {
    name: t('tradingConfig.system'),
    color: 'red',
    level: 100,
  },
  [KfCategoryEnum.daemon]: {
    name: t('tradingConfig.daemon'),
    color: 'purple',
    level: 90,
  },
  [KfCategoryEnum.md]: {
    name: t('tradingConfig.md'),
    color: 'orange',
    level: 80,
  },
  [KfCategoryEnum.td]: {
    name: t('tradingConfig.td'),
    color: 'blue',
    level: 70,
  },
  [KfCategoryEnum.strategy]: {
    name: t('tradingConfig.strategy'),
    color: 'cyan',
    level: 60,
  },
};

export const UnknownKfCategory: KungfuApi.KfTradeValueCommonData = {
  name: t('tradingConfig.unknown'),
  color: 'default',
  level: 0,
};

export const Offset: Record<OffsetEnum, KungfuApi.KfTradeValueCommonData> = {
  [OffsetEnum.Open]: { name: t('tradingConfig.open'), color: 'red' },
  [OffsetEnum.Close]: { name: t('tradingConfig.close'), color: 'green' },
  [OffsetEnum.CloseToday]: {
    name: t('tradingConfig.close_today'),
    color: 'green',
  },
  [OffsetEnum.CloseYest]: {
    name: t('tradingConfig.close_yesterday'),
    color: 'green',
  },
};

export const CodeTabSetting: Record<
  SpaceTabSettingEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [SpaceTabSettingEnum.SPACES]: { name: 'Spaces' },
  [SpaceTabSettingEnum.TABS]: { name: 'Tabs' },
};

export const CodeSizeSetting: Record<
  SpaceSizeSettingEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [SpaceSizeSettingEnum.FOURINDENT]: { name: '4' },
  [SpaceSizeSettingEnum.TWOINDENT]: { name: '2' },
};

export const OrderInputKeySetting: Record<
  OrderInputKeyEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [OrderInputKeyEnum.VOLUME]: { name: t('tradingConfig.volume') },
  [OrderInputKeyEnum.PRICE]: { name: t('globalSettingConfig.single_price') },
};

export const Side: Record<SideEnum, KungfuApi.KfTradeValueCommonData> = {
  [SideEnum.Buy]: { name: t('tradingConfig.buy'), color: 'red' },
  [SideEnum.Sell]: { name: t('tradingConfig.sell'), color: 'green' },
  [SideEnum.Lock]: { name: t('tradingConfig.lock'), color: 'orange' },
  [SideEnum.Unlock]: { name: t('tradingConfig.unlock'), color: 'blue' },
  [SideEnum.Exec]: { name: t('tradingConfig.exec'), color: 'blue' },
  [SideEnum.Drop]: { name: t('tradingConfig.drop'), color: 'green' },
  [SideEnum.Purchase]: {
    name: t('tradingConfig.purchase'),
    color: 'red',
  },
  [SideEnum.Redemption]: {
    name: t('tradingConfig.redemption'),
    color: 'green',
  },
  [SideEnum.Split]: { name: t('tradingConfig.split'), color: 'red' },
  [SideEnum.Merge]: { name: t('tradingConfig.merge'), color: 'green' },
  [SideEnum.MarginTrade]: {
    name: t('tradingConfig.margin_trade'),
    color: 'red',
  },
  [SideEnum.ShortSell]: {
    name: t('tradingConfig.short_sell'),
    color: 'green',
  },
  [SideEnum.RepayMargin]: {
    name: t('tradingConfig.repay_margin'),
    color: 'green',
  },
  [SideEnum.RepayStock]: {
    name: t('tradingConfig.repay_short'),
    color: 'red',
  },
  [SideEnum.CashRepayMargin]: {
    name: t('tradingConfig.cash_repay_margin'),
    color: 'orange',
  },
  [SideEnum.StockRepayStock]: {
    name: t('tradingConfig.stock_repay_short'),
    color: 'blue',
  },
  [SideEnum.SurplusStockTransfer]: {
    name: t('tradingConfig.surplus_stock_transfer'),
    color: 'blue',
  },
  [SideEnum.GuaranteeStockTransferIn]: {
    name: t('tradingConfig.guarantee_stock_transfer'),
    color: 'red',
  },
  [SideEnum.GuaranteeStockTransferOut]: {
    name: t('tradingConfig.guarantee_stock_redeem'),
    color: 'green',
  },
};

export const OrderStatus: Record<
  OrderStatusEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [OrderStatusEnum.Unknown]: {
    name: t('tradingConfig.unknown'),
    color: 'default',
  },
  [OrderStatusEnum.Submitted]: {
    name: t('tradingConfig.submitted'),
    color: 'default',
  },
  [OrderStatusEnum.Pending]: {
    name: t('tradingConfig.pending'),
    color: 'default',
  },
  [OrderStatusEnum.Cancelled]: {
    name: t('tradingConfig.cancelled'),
    color: 'default',
  },
  [OrderStatusEnum.Error]: {
    name: t('tradingConfig.error'),
    color: 'red',
  },
  [OrderStatusEnum.Filled]: {
    name: t('tradingConfig.filled'),
    color: 'green',
  },
  [OrderStatusEnum.PartialFilledNotActive]: {
    name: t('tradingConfig.partial_filled_not_active'),
    color: 'green',
  },
  [OrderStatusEnum.PartialFilledActive]: {
    name: t('tradingConfig.partial_filled_active'),
    color: 'default',
  },
  [OrderStatusEnum.Lost]: {
    name: t('tradingConfig.lost'),
    color: 'default',
  },
};

export const UnfinishedOrderStatus = [
  OrderStatusEnum.Pending,
  OrderStatusEnum.Submitted,
  OrderStatusEnum.PartialFilledActive,
];

export const WellFinishedOrderStatus = [
  OrderStatusEnum.Cancelled,
  OrderStatusEnum.Filled,
  OrderStatusEnum.PartialFilledNotActive,
];

export const Direction: Record<
  DirectionEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [DirectionEnum.Long]: { name: t('tradingConfig.long'), color: 'red' },
  [DirectionEnum.Short]: { name: t('tradingConfig.short'), color: 'green' },
};

export const PriceType: Record<
  PriceTypeEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [PriceTypeEnum.Limit]: { name: t('tradingConfig.Limit') },
  [PriceTypeEnum.Market]: { name: t('tradingConfig.Market') },
  [PriceTypeEnum.FakBest5]: {
    name: t('tradingConfig.FakBest5'),
  },
  [PriceTypeEnum.ForwardBest]: {
    name: t('tradingConfig.Forward_best'),
  },
  [PriceTypeEnum.ReverseBest]: {
    name: t('tradingConfig.Reverse_best'),
  },
  [PriceTypeEnum.Fak]: {
    name: t('tradingConfig.Fak'),
  },
  [PriceTypeEnum.Fok]: {
    name: t('tradingConfig.Fok'),
  },
  [PriceTypeEnum.Unknown]: { name: t('tradingConfig.unknown') },
};

export const HedgeFlag: Record<
  HedgeFlagEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [HedgeFlagEnum.Speculation]: { name: t('tradingConfig.speculation') },
  [HedgeFlagEnum.Arbitrage]: { name: t('tradingConfig.arbitrage') },
  [HedgeFlagEnum.Hedge]: { name: t('tradingConfig.hedge') },
  [HedgeFlagEnum.Covered]: { name: t('tradingConfig.covered') },
};

export const VolumeCondition: Record<
  VolumeConditionEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [VolumeConditionEnum.Any]: { name: t('tradingConfig.any') },
  [VolumeConditionEnum.Min]: { name: t('tradingConfig.min') },
  [VolumeConditionEnum.All]: { name: t('tradingConfig.all') },
};

export const TimeCondition: Record<
  TimeConditionEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [TimeConditionEnum.IOC]: { name: t('tradingConfig.IOC') },
  [TimeConditionEnum.GFD]: { name: t('tradingConfig.GFD') },
  [TimeConditionEnum.GTC]: { name: t('tradingConfig.GTC') },
};

export const CommissionMode: Record<
  CommissionModeEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [CommissionModeEnum.ByAmount]: { name: t('tradingConfig.by_amount') },
  [CommissionModeEnum.ByVolume]: { name: t('tradingConfig.by_volume') },
};

export const InstrumentType: Record<
  InstrumentTypeEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [InstrumentTypeEnum.unknown]: {
    name: t('tradingConfig.unknown'),
    color: 'default',
  },
  [InstrumentTypeEnum.stock]: {
    name: t('tradingConfig.stock'),
    color: 'orange',
    level: 10,
  },
  [InstrumentTypeEnum.future]: {
    name: t('tradingConfig.future'),
    color: 'red',
    level: 10,
  },
  [InstrumentTypeEnum.bond]: {
    name: t('tradingConfig.bond'),
    color: 'pink',
  },
  [InstrumentTypeEnum.stockoption]: {
    name: t('tradingConfig.stock_option'),
    color: 'blue',
    level: 10,
  },
  [InstrumentTypeEnum.fund]: {
    name: t('tradingConfig.fund'),
    color: 'purple',
  },
  [InstrumentTypeEnum.techstock]: {
    name: t('tradingConfig.tech_stock'),
    color: 'blue',
  },
  [InstrumentTypeEnum.index]: {
    name: t('tradingConfig.index'),
    color: 'purple',
  },
  [InstrumentTypeEnum.repo]: {
    name: t('tradingConfig.repo'),
    color: 'red',
  },
  [InstrumentTypeEnum.crypto]: {
    name: t('tradingConfig.crypto'),
    color: 'blue',
    level: 10,
  },
  [InstrumentTypeEnum.cryptofuture]: {
    name: t('tradingConfig.crypto_future'),
    color: 'blue',
    level: 10,
  },
  [InstrumentTypeEnum.multi]: {
    name: t('tradingConfig.multi'),
    color: 'green',
    level: 10,
  },
};

export const UnderweightType: Record<
  UnderweightEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [UnderweightEnum.UnrestrictedShares]: {
    name: t('blockTradeConfig.unrestricted_shares'),
    color: 'default',
  },
  [UnderweightEnum.RestrictedShares]: {
    name: t('blockTradeConfig.restricted_shares'),
    color: 'default',
  },
};

export const StrategyExtType: Record<
  StrategyExtTypes,
  KungfuApi.KfTradeValueCommonData
> = {
  unknown: {
    name: t('tradingConfig.unknown'),
    color: 'default',
  },
  default: {
    name: t('tradingConfig.default'),
    color: 'cyan',
    level: 9,
  },
  trade: {
    name: t('tradingConfig.order_task'),
    color: 'blue',
    level: 10,
  },
};

export const ShotableInstrumentTypes = [
  InstrumentTypeEnum.future,
  InstrumentTypeEnum.stockoption,
  InstrumentTypeEnum.cryptofuture,
  InstrumentTypeEnum.multi,
];

export const T0InstrumentTypes = [
  InstrumentTypeEnum.future,
  InstrumentTypeEnum.bond,
];

export const AbleSubscribeInstrumentTypesBySourceType: Record<
  InstrumentTypes,
  InstrumentTypeEnum[]
> = {
  unknown: [InstrumentTypeEnum.unknown],

  stock: [
    InstrumentTypeEnum.stock,
    InstrumentTypeEnum.bond,
    InstrumentTypeEnum.fund,
    InstrumentTypeEnum.techstock,
    InstrumentTypeEnum.index,
    InstrumentTypeEnum.repo,
  ],

  future: [InstrumentTypeEnum.future],

  bond: [InstrumentTypeEnum.bond],

  stockoption: [InstrumentTypeEnum.stockoption],

  fund: [InstrumentTypeEnum.fund],

  techstock: [InstrumentTypeEnum.techstock],

  index: [InstrumentTypeEnum.index],

  repo: [InstrumentTypeEnum.repo],

  crypto: [InstrumentTypeEnum.crypto],

  cryptofuture: [InstrumentTypeEnum.cryptofuture],

  multi: [
    InstrumentTypeEnum.stock,
    InstrumentTypeEnum.future,
    InstrumentTypeEnum.bond,
    InstrumentTypeEnum.stockoption,
    InstrumentTypeEnum.fund,
    InstrumentTypeEnum.techstock,
    InstrumentTypeEnum.index,
    InstrumentTypeEnum.repo,
    InstrumentTypeEnum.crypto,
    InstrumentTypeEnum.cryptofuture,
  ],
};

export const ExchangeIds: Record<string, KungfuApi.KfTradeValueCommonData> = {
  SSE: {
    name: t('tradingConfig.SSE'),
    color: InstrumentType[InstrumentTypeEnum.stock].color,
  },
  SZE: {
    name: t('tradingConfig.SZE'),
    color: InstrumentType[InstrumentTypeEnum.stock].color,
  },
  BSE: {
    name: t('tradingConfig.BSE'),
    color: InstrumentType[InstrumentTypeEnum.stock].color,
  },
  SHFE: {
    name: t('tradingConfig.SHFE'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  DCE: {
    name: t('tradingConfig.DCE'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  CZCE: {
    name: t('tradingConfig.CZCE'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  CFFEX: {
    name: t('tradingConfig.CFFEX'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  INE: {
    name: t('tradingConfig.INE'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  HK: {
    name: t('tradingConfig.HK'),
    color: InstrumentType[InstrumentTypeEnum.stock].color,
  },
  HKFUT: {
    name: t('tradingConfig.HKFUT'),
    color: InstrumentType[InstrumentTypeEnum.stockoption].color,
  },
  US: {
    name: t('tradingConfig.US'),
    color: InstrumentType[InstrumentTypeEnum.stock].color,
  },
  GLFX: {
    name: t('tradingConfig.GLFX'),
    color: InstrumentType[InstrumentTypeEnum.stock].color,
  },
  IPE: {
    name: t('tradingConfig.IPE'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  CBOT: {
    name: t('tradingConfig.CBOT'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  CEC: {
    name: t('tradingConfig.CEC'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  LIFE: {
    name: t('tradingConfig.LIFE'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  MTIF: {
    name: t('tradingConfig.MTIF'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  NYCE: {
    name: t('tradingConfig.NYCE'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  CMX: {
    name: t('tradingConfig.CMX'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  NYME: {
    name: t('tradingConfig.NYME'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  SIME: {
    name: t('tradingConfig.SIME'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  CME: {
    name: t('tradingConfig.CME'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  IMM: {
    name: t('tradingConfig.IMM'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  WIDX: {
    name: t('tradingConfig.WIDX'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  FREX: {
    name: t('tradingConfig.FREX'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  METL: {
    name: t('tradingConfig.METL'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  IPM: {
    name: t('tradingConfig.IPM'),
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
};

export const FutureArbitrageCodes: Record<
  FutureArbitrageCodeEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [FutureArbitrageCodeEnum.SP]: {
    name: t('tradingConfig.SP'),
  },
  [FutureArbitrageCodeEnum.SPC]: {
    name: t('tradingConfig.SPC'),
  },
  [FutureArbitrageCodeEnum.SPD]: {
    name: t('tradingConfig.SPD'),
  },
  [FutureArbitrageCodeEnum.IPS]: {
    name: t('tradingConfig.IPS'),
  },
};

export const SystemProcessName: Record<
  string,
  KungfuApi.KfTradeValueCommonData
> = {
  master: { name: t('tradingConfig.master') },
  ledger: { name: t('tradingConfig.ledger') },
  cached: { name: t('tradingConfig.cached') },
  archive: { name: t('tradingConfig.archive') },
};
