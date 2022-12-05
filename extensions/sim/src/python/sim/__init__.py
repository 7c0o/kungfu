#  SPDX-License-Identifier: Apache-2.0

def td(vendor):
    from . import trader

    return trader.TraderSim(vendor)


def md(vendor):
    from . import marketdata

    return marketdata.MarketDataSim(vendor)
