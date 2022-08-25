import path from 'path';
import fse from 'fs-extra';
import { configStore } from '../kungfu';
import { kfLogger, hidePasswordByLogger } from '../utils/busiUtils';
import { BASE_DB_DIR } from '../config/pathConfig';

type AllConfig = Record<string, KungfuApi.KfConfigOrigin>;

export const getKfAllConfig = (): Promise<KungfuApi.KfConfigOrigin[]> => {
  if (fse.pathExistsSync(path.join(BASE_DB_DIR, 'config.db'))) {
    return Promise.resolve(
      Object.values((configStore.getAllConfig() || {}) as AllConfig),
    );
  } else {
    return Promise.resolve([]);
  }
};

export const setKfConfig = (
  kfLocation: KungfuApi.KfLocation,
  configValue: string,
): Promise<boolean> => {
  const configForLog = hidePasswordByLogger(configValue);
  kfLogger.info(
    `Set Kungfu Config ${kfLocation.category} ${kfLocation.group} ${kfLocation.name} ${configForLog}`,
  );
  return Promise.resolve(
    configStore.setConfig(
      kfLocation.category,
      kfLocation.group,
      kfLocation.name,
      kfLocation.mode,
      configValue,
    ),
  );
};

export const removeKfConfig = (
  kfLocation: KungfuApi.KfLocation,
): Promise<boolean> => {
  kfLogger.info(
    `Remove Kungfu Config ${kfLocation.category} ${kfLocation.group} ${kfLocation.name}`,
  );
  return Promise.resolve(
    configStore.removeConfig(
      kfLocation.category,
      kfLocation.group,
      kfLocation.name,
      kfLocation.mode,
    ),
  );
};

export const getKfConfig = (strategyId: string) => {
  const kfLocation: KungfuApi.KfLocation = getStrategyKfLocation(strategyId);
  return configStore.getConfig(
    kfLocation.category,
    kfLocation.group,
    kfLocation.name,
    kfLocation.mode,
  );
};

export const getStrategyKfLocation = (strategyId: string) => {
  return {
    category: 'strategy',
    group: 'default',
    name: strategyId,
    mode: 'live',
  };
};
