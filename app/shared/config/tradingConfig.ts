
const isEnglish = process.env.LANG_ENV === 'en';

// open = '0',
// close = '1',
// close_today = '2',
// close_yesterday = '3'
export const offsetName: NumberToStringObject = {
    0: isEnglish ? 'Open' : '开',
    1: isEnglish ? 'Close' : '平',
    2: isEnglish ? 'CloseToday' : '平今',
    3: isEnglish ? 'CloseYest' : '平昨'
}

export const sideName: NumberToStringObject = {
    0: isEnglish ? 'Buy' : '买',
    1: isEnglish ? 'Sell' : '卖'
}

export const posDirection: NumberToStringObject = {
    0: isEnglish ? 'Long' : '多',
    1: isEnglish ? 'Short' : '空' 
}

export const priceType: NumberToStringObject = {
    1: isEnglish ? 'Limit' : '市价',
    0: isEnglish ? 'Market' : '限价'
}

export const statusName: StringToStringObject = {
    all_traded: isEnglish ? 'AllTraded' : '全部成交',
    pending: isEnglish ? 'Pending' : '非最终状态',
    error: isEnglish ? 'Error' : '错误',
    canceled: isEnglish ? 'Canceled' : '全部撤销',
    part_traded_not_queueing: isEnglish ? 'PartialFilledNotActive' : '部分撤销部分成交'
}

// Unknown = '0', // 未知
// Submitted = '1', //已提交 
// Pending = '2', // 等待
// Cancelled = '3', // 已撤销
// Error = '4', // 错误
// Filled = '5', //已成交
// PartialFilledNotActive = '6', //部分撤单
// PartialFilledActive = '7' //正在交易
// 0,3,4,5,6 已完成
export const orderStatus: NumberToStringObject = {
    0: isEnglish ? 'Unknow' : '未知',
    1: isEnglish ? 'Submitted' : '已提交', // ing
    2: isEnglish ? 'Pending' : '等待中', // ing
    3: isEnglish ? 'Cancelled' : '已撤单', 
    4: isEnglish ? 'Error' : '错误', 
    5: isEnglish ? 'Filled' : '已成交',
    6: isEnglish ? 'PartialCancel' : '部分撤单', 
    7: isEnglish ? 'Trading' : '正在交易', // ing
}

export const aliveOrderStatusList = [1, 2, 7]

export const sourceTypeConfig: SourceType = {
    'Stock': {
        name: isEnglish ? 'stock' : '股票',
        kfId: 1,
        color: ''
    },

    'Future': {
        name: isEnglish ? 'future': '期货',
        kfId: 2,
        color: 'danger'
    },

    'Option': {
        name: isEnglish ? 'option' : '期权',
        kfId: 3,
        color: ''
    },

    'Sim': {
        name: isEnglish ? 'simulation' : '模拟',
        kfId: 4,
        color: 'success'
    }
}

export const hedgeFlag: NumberToStringObject = {
    0: isEnglish ? 'Speculation': '投机',
    1: isEnglish ? 'Arbitrage': '套利',
    2: isEnglish ? 'Hedge': '套保',
    3: isEnglish ? 'Covered': '备兑'
}

export const exchangeIds: StringToStringObject = {
    "SSE": "上交所",
    "SZE": "深交所",
    "SHFE": "上期所",
    "DCE": "大商所",
    "DZCE": "郑商所",
    "CFFEX": "中金所",
    "INE": "能源中心"
}