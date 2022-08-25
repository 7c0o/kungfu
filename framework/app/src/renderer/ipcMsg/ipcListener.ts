import { ipcRenderer } from 'electron';
import {
  getStrategyById,
  updateStrategyPath,
} from '@kungfu-trader/kungfu-js-api/kungfu/strategy';
import { BrowserWindow } from '@electron/remote';
import { messagePrompt } from '../assets/methods/uiUtils';
const { success, error } = messagePrompt();

export function bindIPCListener(store) {
  ipcRenderer.removeAllListeners('ipc-emit-strategyById');
  ipcRenderer.on('ipc-emit-strategyById', (event, { childWinId, params }) => {
    const childWin = BrowserWindow.fromId(childWinId);
    const { strategyId } = params;
    return getStrategyById(strategyId)
      .then((strategies) => {
        if (childWin) {
          childWin.webContents.send('ipc-res-strategyById', strategies);
        }
      })
      .catch((err) => {
        error(err.message);
      });
  });

  ipcRenderer.removeAllListeners('ipc-emit-updateStrategyPath');
  ipcRenderer.on(
    'ipc-emit-updateStrategyPath',
    (event, { childWinId, params }) => {
      const childWin = BrowserWindow.fromId(childWinId);
      const { strategyId, strategyPath } = params;
      return updateStrategyPath(strategyId, strategyPath).then(() => {
        store.setKfConfigList();
        success();
        if (childWin) {
          childWin.webContents.send('ipc-res-updateStrategyPath');
        }
      });
    },
  );

  ipcRenderer.removeAllListeners('ipc-emit-strategyList');
  ipcRenderer.on('ipc-emit-strategyList', (event, { childWinId }) => {
    const childWin = BrowserWindow.fromId(childWinId);
    return new Promise(() => {
      if (childWin) {
        childWin.webContents.send('ipc-res-strategyList', store.strategyList);
      }
    });
  });

  ipcRenderer.removeAllListeners('ipc-emit-strategyStates');
  ipcRenderer.on('ipc-emit-strategyStates', (event, { childWinId }) => {
    const childWin = BrowserWindow.fromId(childWinId);
    return new Promise(() => {
      if (childWin) {
        childWin.webContents.send(
          'ipc-res-strategyStates',
          store.strategyStates,
        );
      }
    });
  });
}
