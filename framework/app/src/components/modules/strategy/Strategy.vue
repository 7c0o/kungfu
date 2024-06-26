<script setup lang="ts">
import { ref, computed, toRefs, Ref } from 'vue';

import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfSetByConfigModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSetByConfigModal.vue';
import Icon, {
  FileTextOutlined,
  SettingOutlined,
  DeleteOutlined,
  FormOutlined,
} from '@ant-design/icons-vue';

import {
  useTableSearchKeyword,
  useDashboardBodySize,
  handleOpenLogview,
  handleOpenCodeView,
  messagePrompt,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { getColumns } from './config';
import {
  handleSwitchProcessStatusGenerator,
  useAddUpdateRemoveKfConfig,
  useAllKfConfigData,
  useAssets,
  useCurrentGlobalKfLocation,
  useProcessStatusDetailData,
  useSwitchAllConfig,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import {
  dealAssetPrice,
  getConfigValue,
  getIfProcessRunning,
  getIfProcessStopping,
  getProcessIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import path from 'path';
import KfBlinkNum from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfBlinkNum.vue';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';

const { t } = VueI18n.global;
const { success, error } = messagePrompt();

const handleSwitchProcessStatus = handleSwitchProcessStatusGenerator();
const { dashboardBodyHeight, handleBodySizeChange } = useDashboardBodySize();

const setStrategyModalVisible = ref<boolean>(false);
const setStrategyConfigPayload = ref<KungfuApi.SetKfConfigPayload>({
  type: 'add',
  title: t('strategyConfig.strategy'),
  config: {} as KungfuApi.KfExtConfig,
});

const { strategy } = toRefs(useAllKfConfigData());
const strategyIdList = computed(() => {
  return strategy.value.map((item: KungfuApi.KfLocation): string => item.name);
});

const { dealRowClassName, customRow } = useCurrentGlobalKfLocation(
  window.watcher,
);

const { processStatusData } = useProcessStatusDetailData();
const { allProcessOnline, handleSwitchAllProcessStatus } = useSwitchAllConfig(
  strategy,
  processStatusData,
);
const { searchKeyword, tableData } = useTableSearchKeyword<KungfuApi.KfConfig>(
  strategy as Ref<KungfuApi.KfConfig[]>,
  ['name'],
);

const tableDataResolved = computed(() => {
  return [...tableData.value].sort((a, b) => {
    const aAddTime = getConfigValue(a).add_time || 0;
    const bAddTime = getConfigValue(b).add_time || 0;
    return bAddTime - aAddTime;
  });
});
const { getAssetsByKfConfig } = useAssets();

const { handleConfirmAddUpdateKfConfig, handleRemoveKfConfig } =
  useAddUpdateRemoveKfConfig();

const columns = getColumns((dataIndex) => {
  return (a: KungfuApi.KfConfig, b: KungfuApi.KfConfig) => {
    return (
      (getAssetsByKfConfig(a)[dataIndex as keyof KungfuApi.Asset] || 0) -
      (getAssetsByKfConfig(b)[dataIndex as keyof KungfuApi.Asset] || 0)
    );
  };
});

const getPrefixByLocation = (kfLocation: KungfuApi.KfLocation) =>
  globalThis.HookKeeper.getHooks().prefix.trigger(kfLocation);

function handleOpenSetStrategyDialog(
  type: KungfuApi.ModalChangeType,
  strategyConfig?: KungfuApi.KfConfig,
) {
  setStrategyConfigPayload.value.type = type;
  setStrategyConfigPayload.value.config = {
    type: [],
    name: t('strategyConfig.strategy'),
    category: 'strategy',
    key: 'default',
    extPath: '',
    settings: [
      {
        key: 'strategy_id',
        name: t('strategyConfig.strategy_id'),
        type: 'str',
        primary: true,
        required: true,
        tip: t('strategyConfig.strategy_tip'),
      },
      {
        key: 'strategy_path',
        name: t('strategyConfig.strategy_path'),
        type: 'file',
        tip: t('strategyConfig.strategy_path_tip'),
        required: true,
      },
    ],
  };
  setStrategyConfigPayload.value.initValue = undefined;

  if (type === 'update') {
    if (strategyConfig) {
      setStrategyConfigPayload.value.initValue = JSON.parse(
        strategyConfig.value,
      );
    }
  }

  setStrategyModalVisible.value = true;
}

function getStrategyPathShowName(kfConfig: KungfuApi.KfConfig): string {
  const strategyPath = getConfigValue(kfConfig).strategy_path || '';
  return path.basename(strategyPath);
}

function handleRemoveStrategy(record: KungfuApi.KfConfig) {
  return handleRemoveKfConfig(window.watcher, record, processStatusData.value)
    .then(() => {
      success();
    })
    .catch((err) => {
      error(err.message || t('operation_failed'));
    });
}
</script>

<template>
  <div class="kf-strategy__warp kf-translateZ">
    <KfDashboard @boardSizeChange="handleBodySizeChange">
      <template v-slot:header>
        <KfDashboardItem>
          <a-input-search
            v-model:value="searchKeyword"
            :placeholder="$t('keyword_input')"
            style="width: 120px"
          />
        </KfDashboardItem>
        <KfDashboardItem>
          <a-switch
            :checked="allProcessOnline"
            @click="handleSwitchAllProcessStatus"
          ></a-switch>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button
            size="small"
            type="primary"
            @click="handleOpenSetStrategyDialog('add')"
          >
            {{ $t('strategyConfig.add_strategy') }}
          </a-button>
        </KfDashboardItem>
      </template>
      <a-table
        class="kf-ant-table"
        :columns="columns"
        :data-source="tableDataResolved"
        size="small"
        :pagination="false"
        :scroll="{ y: dashboardBodyHeight - 4 }"
        :rowClassName="dealRowClassName"
        :customRow="customRow"
        :defaultExpandAllRows="true"
        :emptyText="$t('empty_text')"
      >
        <template
          #bodyCell="{
            column,
            record,
          }: {
            column: AntTableColumn,
            record: KungfuApi.KfConfig,
          }"
        >
          <template v-if="column.dataIndex === 'name'">
            <span>{{ record[column.dataIndex] }}</span>
            <Icon
              v-if="getPrefixByLocation(record).prefixType === 'icon'"
              :component="getPrefixByLocation(record).prefix"
              style="font-size: 12px; margin-left: 7px"
            />
          </template>
          <template v-else-if="column.dataIndex === 'strategyFile'">
            {{ getStrategyPathShowName(record) }}
          </template>
          <template v-else-if="column.dataIndex === 'processStatus'">
            <a-switch
              size="small"
              :checked="
                getIfProcessRunning(
                  processStatusData,
                  getProcessIdByKfLocation(record),
                )
              "
              :loading="
                getIfProcessStopping(
                  processStatusData,
                  getProcessIdByKfLocation(record),
                )
              "
              @click="(checked: boolean, Event: MouseEvent) => handleSwitchProcessStatus(checked, Event, record)"
            ></a-switch>
          </template>
          <template v-else-if="column.dataIndex === 'unrealizedPnl'">
            <KfBlinkNum
              mode="compare-zero"
              :num="dealAssetPrice(getAssetsByKfConfig(record).unrealized_pnl)"
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'marketValue'">
            <KfBlinkNum
              :num="dealAssetPrice(getAssetsByKfConfig(record).market_value)"
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'actions'">
            <div class="kf-actions__warp">
              <FileTextOutlined
                style="font-size: 12px"
                @click.stop="handleOpenLogview(record)"
              />
              <FormOutlined
                style="font-size: 12px"
                @click.stop="handleOpenCodeView(record)"
              ></FormOutlined>
              <SettingOutlined
                style="font-size: 12px"
                @click.stop="handleOpenSetStrategyDialog('update', record)"
              />
              <DeleteOutlined
                style="font-size: 12px"
                @click.stop="handleRemoveStrategy(record)"
              />
            </div>
          </template>
        </template>
      </a-table>
    </KfDashboard>
    <KfSetByConfigModal
      v-if="setStrategyModalVisible"
      :width="420"
      v-model:visible="setStrategyModalVisible"
      :payload="setStrategyConfigPayload"
      :primaryKeyAvoidRepeatCompareTarget="strategyIdList"
      @confirm="handleConfirmAddUpdateKfConfig($event, 'strategy', 'default')"
    ></KfSetByConfigModal>
  </div>
</template>
<style lang="less">
.kf-strategy__warp {
  height: 100%;
}
</style>
