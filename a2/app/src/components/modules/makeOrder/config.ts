import { ShotableInstrumentTypes } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
    InstrumentTypeEnum,
    KfConfigItem,
} from '@kungfu-trader/kungfu-js-api/typings';

export const getConfigSettings = (
    category?: KfCategoryTypes,
    instrumentTypeEnum?: InstrumentTypeEnum,
): KfConfigItem[] => {
    const shotable = instrumentTypeEnum
        ? false
        : ShotableInstrumentTypes.includes(
              instrumentTypeEnum || InstrumentTypeEnum.unknown,
          );

    const defaultSettings: KfConfigItem[] = [
        category === 'td'
            ? null
            : {
                  key: 'account_id',
                  name: '账户',
                  type: 'td',
                  required: true,
              },
        {
            key: 'instrument',
            name: '标的',
            type: 'instrument',
            required: true,
        },
        {
            key: 'side',
            name: '买卖',
            type: 'side',
            default: 0,
            required: true,
        },
        ...(shotable
            ? [
                  {
                      key: 'offset',
                      name: '开平',
                      type: 'offset',
                      default: 0,
                      required: true,
                  },
                  {
                      key: 'hedge_flag',
                      name: '套保',
                      type: 'hedgeFlag',
                      default: 0,
                      required: true,
                  },
              ]
            : []),
        {
            key: 'price_type',
            name: '方式',
            type: 'priceType',
            default: 0,
            required: true,
        },
        {
            key: 'price',
            name: '价格',
            type: 'float',
            required: true,
        },
        {
            key: 'volume',
            name: '下单量',
            type: 'int',
            required: true,
        },
    ].filter((item) => !!item) as KfConfigItem[];

    return defaultSettings;
};
