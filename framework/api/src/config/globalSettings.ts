import fse from 'fs-extra';
import { SpaceSizeSettingEnum, SpaceTabSettingEnum } from '../typings/enums';
import { KF_CONFIG_PATH, PY_WHL_DIR } from './pathConfig';
import { CodeSizeSetting, CodeTabSetting } from './tradingConfig';
import {
  languageList,
  langDefault,
} from '@kungfu-trader/kungfu-js-api/language';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export interface KfSystemConfig {
  key: string;
  name: string;
  config: KungfuApi.KfConfigItem[];
}

export const getKfGlobalSettings = (): KfSystemConfig[] => [
  {
    key: 'system',
    name: t('globalSettingConfig.system'),
    config: [
      {
        key: 'logLevel',
        name: t('globalSettingConfig.log_level'),
        tip: t('globalSettingConfig.for_all_log'),
        type: 'select',
        options: [
          { value: '-l trace', label: 'TRACE' },
          { value: '-l debug', label: 'DEBUG' },
          { value: '-l info', label: 'INFO' },
          { value: '-l warning', label: 'WARN' },
          { value: '-l error', label: 'ERROR' },
          { value: '-l critical', label: 'CRITICAL' },
        ],
        default: '-l info',
      },
      {
        key: 'language',
        name: t('globalSettingConfig.language'),
        tip: t('globalSettingConfig.select_language'),
        type: 'select',
        options: languageList,
        default: langDefault,
      },
    ],
  },
  {
    key: 'performance',
    name: t('globalSettingConfig.porformance'),
    config: [
      {
        key: 'rocket',
        name: t('globalSettingConfig.rocket_model'),
        tip: t('globalSettingConfig.rocket_model_desc'),
        default: false,
        type: 'bool',
      },
      {
        key: 'bypassAccounting',
        name: t('globalSettingConfig.bypass_accounting'),
        tip: t('globalSettingConfig.bypass_accounting_desc'),
        default: false,
        type: 'bool',
      },
      {
        key: 'bypassTradingData',
        name: t('globalSettingConfig.bypass_trading_data'),
        tip: t('globalSettingConfig.bypass_trading_data_desc'),
        default: false,
        type: 'bool',
      },
    ],
  },
  {
    key: 'strategy',
    name: t('globalSettingConfig.strategy'),
    config: [
      {
        key: 'python',
        name: t('globalSettingConfig.use_local_python'),
        tip: t('globalSettingConfig.local_python_desc', {
          py_version: __python_version,
          whl_dir_path: PY_WHL_DIR,
        }),
        default: false,
        type: 'bool',
      },
      {
        key: 'pythonPath',
        name: t('globalSettingConfig.python_path'),
        tip: t('globalSettingConfig.python_path_desc'),
        default: '',
        type: 'file',
      },
    ],
  },
  {
    key: 'trade',
    name: t('globalSettingConfig.trade'),
    config: [
      {
        key: 'sound',
        name: t('globalSettingConfig.sound'),
        tip: t('globalSettingConfig.use_sound'),
        default: false,
        type: 'bool',
      },
      {
        key: 'fatFinger',
        name: t('globalSettingConfig.fat_finger_threshold'),
        tip: t('globalSettingConfig.set_fat_finger'),
        default: '',
        type: 'percent',
      },
      {
        key: 'close',
        name: t('globalSettingConfig.close_threshold'),
        tip: t('globalSettingConfig.set_close_threshold'),
        default: '',
        type: 'percent',
      },
    ],
  },
  {
    key: 'code',
    name: t('globalSettingConfig.code_editor'),
    config: [
      {
        key: 'tabSpaceType',
        name: t('globalSettingConfig.tab_space_type'),
        tip: t('globalSettingConfig.set_tab_space'),
        default: CodeTabSetting[SpaceTabSettingEnum.SPACES].name,
        type: 'select',
        options: [
          {
            value: SpaceTabSettingEnum.SPACES,
            label: CodeTabSetting[SpaceTabSettingEnum.SPACES].name,
          },
          {
            value: SpaceTabSettingEnum.TABS,
            label: CodeTabSetting[SpaceTabSettingEnum.TABS].name,
          },
        ],
      },
      {
        key: 'tabSpaceSize',
        name: t('globalSettingConfig.tab_space_size'),
        tip: t('globalSettingConfig.set_tab_space_size'),
        default: CodeSizeSetting[SpaceSizeSettingEnum.FOURINDENT].name,
        type: 'select',
        options: [
          {
            value: SpaceSizeSettingEnum.TWOINDENT,
            label: CodeSizeSetting[SpaceSizeSettingEnum.TWOINDENT].name,
          },
          {
            value: SpaceSizeSettingEnum.FOURINDENT,
            label: CodeSizeSetting[SpaceSizeSettingEnum.FOURINDENT].name,
          },
        ],
      },
    ],
  },
];

export const getKfGlobalSettingsValue = (): Record<
  string,
  Record<string, KungfuApi.KfConfigValue>
> => {
  return fse.readJSONSync(KF_CONFIG_PATH) as Record<
    string,
    Record<string, KungfuApi.KfConfigValue>
  >;
};

export const setKfGlobalSettingsValue = (
  value: Record<string, Record<string, KungfuApi.KfConfigValue>>,
) => {
  return Promise.resolve(fse.writeJSONSync(KF_CONFIG_PATH, value));
};

export const riskSettingConfig: KfSystemConfig = {
  key: 'riskSetting',
  name: '风控',
  config: [
    {
      key: 'riskSetting',
      name: t('风控'),
      tip: t('风控描述'),
      default: [],
      type: 'table',
      columns: [
        {
          key: 'account_id',
          name: t('账户'),
          type: 'td',
        },
        {
          key: 'max_order_volume',
          name: t('单比最大量'),
          type: 'int',
        },
        {
          key: 'max_daily_volume',
          name: t('每日最大成交量'),
          type: 'int',
        },
        {
          key: 'self_filled_check',
          name: t('防止自成交'),
          type: 'bool',
          default: false,
        },
        {
          key: 'max_cancel_ratio',
          name: t('最大回撤率'),
          type: 'int',
        },
        {
          key: 'white_list',
          name: t('标的白名单'),
          type: 'instruments',
        },
      ],
    },
  ],
};
