// ELECTRON_RUN_AS_NODE 应用于通过process.execPath开启任务
process.env.ELECTRON_RUN_AS_NODE = true;
process.env.ELECTRON_ENABLE_STACK_DUMPING = true;
process.env.RENDERER_ID = 'app';
process.env.RELOAD_AFTER_CRASHED = process.argv.includes('reloadAfterCrashed')
  ? 'true'
  : 'false';
console.log('RELOAD_AFTER_CRASHED', process.env.RELOAD_AFTER_CRASHED);
