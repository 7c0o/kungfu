import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export const getColumns = (): AntTableColumns => [
  {
    title: t('mdConfig.counter_name'),
    dataIndex: 'name',
    align: 'left',
    width: 60,
  },
  {
    title: t('mdConfig.state_status'),
    dataIndex: 'stateStatus',
    align: 'left',
    width: 80,
  },
  {
    title: t('mdConfig.process_status'),
    dataIndex: 'processStatus',
    align: 'center',
    width: 60,
  },
  {
    title: t('mdConfig.actions'),
    dataIndex: 'actions',
    align: 'right',
    width: 140,
    fixed: 'right',
  },
];
