import { Proc } from 'pm2';

declare global {
  interface Window {
    watcher: Watcher | null;
    kungfu: Kungfu;
    workers: Record<string, WebpackWorker>;
    fileId: number;
    testCase: Record<string, any>;
    pm2: any;
  }

  namespace NodeJS {
    interface ProcessEnv {
      LANG_ENV: 'zh-CN' | 'en-US' | 'zh-HK' | undefined;
      APP_TYPE: 'cli' | 'renderer' | 'component' | 'daemon' | 'main';
      DAEMON_ID: string;
      RENDERER_ID: string;
      EXTENSION_DIRS: string;
      KFC_DIR: string;
      ELECTRON_RUN_AS_NODE: boolean;
      ELECTRON_ENABLE_STACK_DUMPING: boolean;
    }

    interface Process {
      resourcesPath: string;
    }
  }
}

declare module 'tail' {
  export class Tail {
    constructor(
      filePath: string,
      options: {
        follow?: boolean;
        fromBeginning?: boolean;
        nLines?: number;
        useWatchFile?: boolean;
      },
    );
    watch(): void;
    unwatch(): void;
    on(type: 'line', callback: (data: string) => void);
    on(type: 'error', callback: (err: Error) => void);
  }
}

export const switchProcessHookFunc = (
  category: KfCategoryTypes,
  group: string,
  name: string,
) => Promise<Proc | void>;

export interface KfHooks {
  prestart: Record<string, switchProcessHookFunc[]>;
  start: Record<string, switchProcessHookFunc[]>;
}

declare module globalThis {
  var __publicResources: string;
  var __kfResourcesPath: string;
  var pm2: any;
  var hooks: KfHooks;
}
