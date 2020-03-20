import { Observable } from 'rxjs';
import { map } from 'rxjs/operators';

import { 
    dealGatewayStates, 
    transformTradingItemListToData, 
    transformOrderTradeListToData, 
    transformOrderStatListToData, 
    transformAssetItemListToData 
} from '__io/kungfu/watcher';

import { startGetKungfuTradingData, startGetKungfuGlobalData } from '__io/kungfu/watcher';
import moment from 'moment';

export const KUNGFU_TRADING_DATA_OBSERVER = new Observable(subscriber => {
    subscriber.next({})
    startGetKungfuTradingData((state: any) => {
        subscriber.next(state)
    })
})   

export const KUNGFU_GLOBAL_DATA_OBSERVER = new Observable(subscriber => {
    subscriber.next({})
    startGetKungfuGlobalData((state: any) => {
        subscriber.next(state)
    })
})
       
export const buildTradingDataPipe = (type: string) => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map((data: any): any => {
            const ledgerData = data.ledger;
            const orders = Object.values(ledgerData.Order || {});
            const trades = Object.values(ledgerData.Trade || {});
            const positions = Object.values(ledgerData.Position || {});
            const assets = Object.values(ledgerData.Asset || {});
            const pnl = Object.values(ledgerData.AssetSnapshot || {});
            const orderStat = Object.values(ledgerData.OrderStat || {});
            return {
                orders: transformOrderTradeListToData(orders, type),
                trades: transformOrderTradeListToData(trades, type),
                positions: transformTradingItemListToData(positions, type),
                assets: transformAssetItemListToData(assets, type),
                pnl: transformTradingItemListToData(pnl, type),
                orderStat: transformOrderStatListToData(orderStat)
            }
        })
    )
}

export const buildKungfuGlobalDataPipe = () => {
    return KUNGFU_GLOBAL_DATA_OBSERVER.pipe(
        map((data: any): any => {
            return {
                tradingDay: data.tradingDay || moment().format('YYYYMMDD'),
                gatewayStates: dealGatewayStates(data.appStates || {})
            }
        })
    )
}

// insertTradingData()
async function insertTradingData() {
    const moment = require('moment')
    const { kungfu } = require('__gUtils/kungfuUtils');
    const { watcher } = require('__io/kungfu/watcher');
    const { delayMiliSeconds } = require('__gUtils/busiUtils');
    const longfist = kungfu.longfist;
    await delayMiliSeconds(10000)
    watcher.setup();
    await delayMiliSeconds(3000)
    watcher.step();
    await delayMiliSeconds(1000)

    console.log('insert trading data');

    const len = 3001;

    for(let i = 1; i < len; i++) {
       
        await delayMiliSeconds(100)

        let order = longfist.Order();
        order.order_id = BigInt(+moment().format('YYYYMMDD') + +i);
        order.update_time = BigInt(+new Date().getTime());
        order.account_id = (i % 2 === 0) ? '15014990' : '15014991';
        order.source_id = 'xtp';
        order.client_id = (i % 3 === 0) ? 'test' : '';
        order.instrument_id = '60000' + i.toString();
        order.volume = BigInt(+Number(10000 * +Math.random()).toFixed(0));
        order.volume_left = BigInt(+Number(500 * +Math.random()).toFixed(0));
        order.volume_traded = BigInt(order.volume - order.volume_left);
        order.status = i % 8;
        order.trading_day = moment().format('YYYYMMDD')
    
        watcher.publishState(order)

        let trade = longfist.Trade();
        trade.trade_id = BigInt(+moment().format('YYYYMMDD') + +i);
        trade.trade_time = BigInt(+new Date().getTime());
        trade.account_id = (i % 2 === 0) ? '15014990' : '15014991';
        trade.source_id = 'xtp';
        trade.client_id = (i % 3 === 0) ? 'test' : '';
        trade.instrument_id = '60000' + i.toString();
        trade.price = +Number(1000 * +Math.random())
        trade.volume = BigInt(+Number(10000 * +Math.random()).toFixed(0));
        trade.last_price = +Number(10000 * +Math.random()).toFixed(0);
        trade.trading_day = moment().format('YYYYMMDD')

        watcher.publishState(trade)

        let position = longfist.Position();
        position.instrument_id = '60000' + i.toString();
        position.account_id = (i % 2 === 0) ? '15014990' : '15014991';
        position.source_id = 'xtp';
        position.client_id = (i % 3 === 0) ? 'test' : '';
        position.volume = BigInt(+Number(10000 * +Math.random()).toFixed(0));
        position.yesterday_volume = BigInt(+Number(10000 * +Math.random()).toFixed(0));
        position.last_price = +Number(10000 * +Math.random()).toFixed(0);
        position.direction = i % 2;
        position.ledger_category = (i % 3 === 0) ? 1 : 0;
        watcher.publishState(position)
        console.log('insert TradingData', `${i}/${len - 1}`)
    }

    for (var i = 1; i < 4; i++) {

        let asset = longfist.Asset();
        asset.holder_uid = +new Date().getTime() + +i;
        asset.account_id = (i % 2 === 0) ? '15014990' : '15014991';
        asset.source_id = 'xtp';
        asset.client_id = (i % 3 === 0) ? 'test' : '';
        asset.update_time = BigInt(+new Date().getTime());
        asset.initial_equity = +Number(1000 * +Math.random()).toFixed(0);
        asset.static_equity = +Number(1000 * +Math.random()).toFixed(0);
        asset.dynamic_equity = +Number(1000 * +Math.random()).toFixed(0);
        asset.realized_pnl = +Number(1000 * +Math.random()).toFixed(0);
        asset.unrealized_pnl = +Number(1000 * +Math.random()).toFixed(0);
        asset.avail = +Number(1000 * +Math.random()).toFixed(0);
        asset.market_value = +Number(1000 * +Math.random()).toFixed(0);
        asset.margin = +Number(1000 * +Math.random()).toFixed(0);
        asset.accumulated_fee = +Number(1000 * +Math.random()).toFixed(0);
        asset.intraday_fee = +Number(1000 * +Math.random()).toFixed(0);
        asset.position_pnl = +Number(1000 * +Math.random()).toFixed(0);
        asset.close_pnl = +Number(1000 * +Math.random()).toFixed(0);
        asset.accumulated_fee = +Number(1000 * +Math.random()).toFixed(0);
        asset.ledger_category = (i % 3 === 0) ? 1 : 0;
        watcher.publishState(asset)

        console.log('insert assets', `${i}`)
    }

}




