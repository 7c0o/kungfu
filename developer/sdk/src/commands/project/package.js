const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'package',
  desc: 'Package kungfu prebuilt',
  run: () => {
    sdk.lib.project.package();
  },
};
