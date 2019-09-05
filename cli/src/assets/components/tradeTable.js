import colors from 'colors';
import moment from 'moment';
import Table from './Table';
import { offsetName, orderStatus, sideName, calcuHeaderWidth, parseToString } from "../utils";


function TradeTable(){
    if (!(this instanceof TradeTable)) {
        return new TradeTable();
    }
    Table.call(this);
	this.headers = ['UpdateTime', 'Ticker', 'Side', 'Offset', 'Price', 'Volume', 'Strate']
	this.columnWidth = [0, 0, 0, 0, 0, 6]
}

TradeTable.prototype = new Table();

TradeTable.prototype.getData = function(currentId){
	if(!currentId) return new Promise(resolve => resolve([]))
	return this.getDataMethod(currentId, {}).then(trades => {
        return trades
	})
}
/**
 * @param  {Object} accountData
 * @param  {Object} processStatus
 */
TradeTable.prototype.refresh = function(tradesData, type){
	const tradeListData = tradesData.map(trade => {
		let side = sideName[trade.side] ? sideName[trade.side] : '--';
		if(side === 'buy') side = colors.red(side);
		else if(side === 'sell') side = colors.green(side);
		let offset = offsetName[trade.offset] ? offsetName[trade.offset] : '--';
		if(offset === 'open') offset = colors.red(offset);
		else if(offset.indexOf('close') !== -1) offset = colors.green(offset);
		let last = trade.client_id;
		if(type === 'strategy') last = trade.account_id;
		return parseToString([
			trade.trade_time && moment(trade.trade_time/1000000).format('HH:mm:ss'),
			trade.instrument_id,
			side,
			offset,
			trade.price,
			trade.volume,
			last
		], calcuHeaderWidth(this.headers, this.columnWidth), this.pad)
	})
	this.table.setItems(tradeListData);
	if(!this.table.childList.focused) this.table.childList.setScrollPerc(0);
}

export default TradeTable()