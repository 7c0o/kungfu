import importlib
import json
import os
import sys
import types
import kungfu

from kungfu.console import site
from kungfu.yijinjing import journal as kfj
from kungfu.yijinjing.log import find_logger
from kungfu.yijinjing.practice.master import Master
from kungfu.yijinjing.practice.coloop import KungfuEventLoop
from kungfu.wingchun.strategy import Runner, Strategy

from collections import deque
from importlib.util import module_from_spec, spec_from_file_location
from os import path

lf = kungfu.__binding__.longfist
wc = kungfu.__binding__.wingchun
yjj = kungfu.__binding__.yijinjing


class ExecutorRegistry:
    def __init__(self, ctx):
        self.ctx = ctx
        self.executors = {
            "system": {"master": MasterLoader(ctx), "service": ServiceLoader(ctx)},
            "md": {},
            "td": {},
            "strategy": {"default": ExtensionLoader(self.ctx, None, None)},
        }

    def setup_log(self):
        ctx = self.ctx
        ctx.location = yjj.location(
            kfj.MODES[ctx.mode],
            kfj.CATEGORIES[ctx.category],
            ctx.group,
            ctx.name,
            ctx.runtime_locator,
        )
        ctx.logger = find_logger(ctx.location, ctx.log_level)

    def load_extensions(self):
        self.setup_log()

        ctx = self.ctx
        ctx.logger.debug(f"finding kungfu extension for {ctx.location}")

        if ctx.extension_path:
            deque(map(self.register_extensions, ctx.extension_path.split(path.pathsep)))
        elif ctx.path:
            self.read_config(os.path.dirname(ctx.path))

    def register_extensions(self, root):
        for child in os.listdir(root):
            extension_dir = path.abspath(path.join(root, child))
            self.read_config(extension_dir)

    def read_config(self, extension_dir):
        config_path = os.path.join(extension_dir, "package.json")

        def report(reason):
            self.ctx.logger.info(
                f"kungfu extension not found in {extension_dir}: {reason}"
            )

        if path.exists(config_path):
            with open(config_path, mode="r", encoding="utf8") as config_file:
                config = json.load(config_file)
                if "kungfuConfig" in config:
                    if "config" in config["kungfuConfig"]:
                        group = config["kungfuConfig"]["key"]
                        for category in config["kungfuConfig"]["config"]:
                            if category not in kfj.CATEGORIES:
                                raise RuntimeError(f"Unsupported category {category}")
                            if (
                                self.executors["strategy"]["default"]
                                and self.ctx.category == "strategy"
                                and self.ctx.group == "default"
                            ):
                                self.executors["strategy"]["default"].config = config
                            else:
                                self.executors[category][group] = ExtensionLoader(
                                    self.ctx, extension_dir, config
                                )
                    elif "key" in config["kungfuConfig"]:
                        group = config["kungfuConfig"]["key"]
                        self.executors["strategy"][group] = ExtensionLoader(
                            self.ctx, extension_dir, config
                        )
                    else:
                        report("missing key/config in kungfuConfig")
                else:
                    report("missing kungfuConfig")

    def __getitem__(self, category):
        return self.executors[category]

    def __str__(self):
        return json.dumps(self.executors, indent=2, cls=RegistryJSONEncoder)

    def __repr__(self):
        return json.dumps(self.executors, cls=RegistryJSONEncoder)


class MasterLoader(dict):
    def __init__(self, ctx):
        super().__init__()
        self.ctx = ctx
        self["master"] = self.run

    def run(self, mode: str, low_latency: bool):
        self.ctx.location = yjj.location(
            kfj.MODES[mode],
            lf.enums.category.SYSTEM,
            "master",
            "master",
            self.ctx.runtime_locator,
        )
        self.ctx.logger = find_logger(self.ctx.location, self.ctx.log_level)
        Master(self.ctx).run()


class ServiceLoader(dict):
    def __init__(self, ctx):
        super().__init__()
        self.ctx = ctx
        self["cached"] = self.create_service("cached", yjj.cached)
        self["ledger"] = self.create_service("ledger", wc.Ledger)

    def create_service(self, name, service):
        def run(mode: str, low_latency: bool):
            self.ctx.location = yjj.location(
                kfj.MODES[mode],
                lf.enums.category.SYSTEM,
                "service",
                name,
                self.ctx.runtime_locator,
            )
            self.ctx.logger = find_logger(self.ctx.location, self.ctx.log_level)
            service(
                self.ctx.runtime_locator, kfj.MODES[self.ctx.mode], low_latency
            ).run()

        return run


class ExtensionLoader:
    def __init__(self, ctx, extension_dir, config):
        self.ctx = ctx
        self.extension_dir = extension_dir
        self.config = config

    def __getitem__(self, name):
        return ExtensionExecutor(self.ctx, self)

    def __str__(self):
        return self.config["kungfuConfig"]["name"]

    def __repr__(self):
        return self.__str__()


class ExtensionExecutor:
    def __init__(self, ctx, loader):
        self.ctx = ctx
        self.loader = loader
        self.runners = {
            "md": self.run_market_data,
            "td": self.run_trader,
            "strategy": self.run_strategy,
        }

    def __call__(self, mode, low_latency):
        self.runners[self.ctx.category]()

    def run_broker_vendor(self, vendor_builder):
        ctx = self.ctx
        loader = self.loader
        location = yjj.location(
            kfj.MODES[ctx.mode],
            kfj.CATEGORIES[ctx.category],
            ctx.group,
            ctx.name,
            ctx.runtime_locator,
        )

        if loader.extension_dir:
            site.setup(loader.extension_dir)
            sys.path.insert(0, loader.extension_dir)
        module = importlib.import_module(ctx.group)
        self.ctx.logger.info(f"loading {ctx.group} from {loader.extension_dir}")
        vendor = vendor_builder(
            ctx.runtime_locator, ctx.group, ctx.name, ctx.low_latency
        )
        service_builder = getattr(module, ctx.category)
        self.ctx.logger.debug(f"loaded service builder")
        service = service_builder(vendor)
        self.ctx.logger.debug("set service for vendor")
        vendor.set_service(service)
        self.ctx.logger.info(f"vendor {location.uname} ready to run")
        vendor.run()

    def run_market_data(self):
        self.run_broker_vendor(wc.MarketDataVendor)

    def run_trader(self):
        self.run_broker_vendor(wc.TraderVendor)

    def run_strategy(self):
        loader = self.loader
        if loader.extension_dir:
            site.setup(loader.extension_dir)
            sys.path.insert(0, loader.extension_dir)
        else:
            dirname = os.path.dirname(self.ctx.path)
            site.setup(dirname)
            sys.path.insert(0, dirname)

        ctx = self.ctx
        ctx.location = yjj.location(
            kfj.MODES[ctx.mode],
            lf.enums.category.STRATEGY,
            ctx.group,
            ctx.name,
            ctx.runtime_locator,
        )
        os.environ["KF_STG_GROUP"] = ctx.group
        os.environ["KF_STG_NAME"] = ctx.name
        if loader.config is None:
            load = False
            json_config = os.path.join(os.path.dirname(ctx.path), "package.json")
            if path.exists(json_config):
                with open(json_config, mode="r", encoding="utf8") as json_config_out:
                    config = json.load(json_config_out)
                    if "kungfuConfig" in config and "key" in config["kungfuConfig"]:
                        key = config["kungfuConfig"]["key"]
                        load = True
                        ctx.strategy = load_strategy(ctx, ctx.path, key)
            if not load:
                ctx.strategy = load_strategy(ctx, ctx.path, loader.config)
        else:
            ctx.strategy = load_strategy(
                ctx, ctx.path, loader.config["kungfuConfig"]["key"]
            )
        ctx.runner = Runner(ctx, kfj.MODES[ctx.mode])
        ctx.runner.add_strategy(ctx.strategy)
        ctx.loop = KungfuEventLoop(ctx, ctx.runner)
        ctx.loop.run_forever()


class RegistryJSONEncoder(json.JSONEncoder):
    def default(self, obj):
        test = isinstance(obj, ExtensionLoader) or isinstance(obj, types.FunctionType)
        return str(obj) if test else obj.__dict__


def load_strategy(ctx, path, key):
    if path.endswith(".py"):
        return Strategy(ctx)  # keep strategy alive for pybind11
    elif key is not None and (path.endswith(".so") or path.endswith(".pyd")):
        return try_load_cpp_strategy(ctx, path, key)
    elif key is not None and path.endswith(key):
        return Strategy(ctx)
    else:
        ctx.path = os.path.join(os.path.dirname(path), key)
        return Strategy(ctx)


def try_load_cpp_strategy(ctx, path, key):
    try:
        module = importlib.import_module(key)
        return module.strategy()
    except Exception as e:
        ctx.logger.info(f"fallback to python loader due to: {e}")
        ctx.path = os.path.join(os.path.dirname(path), key)
        return Strategy(ctx)
