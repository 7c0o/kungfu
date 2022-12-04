#  SPDX-License-Identifier: Apache-2.0

from env import __frozen__
from kungfu import __main__ as origin
from pathlib import Path
from os import environ


environ["KFC_DIR"] = Path("dist/kfc").resolve().as_posix()
environ["KF_CLI_DEV_PATH"] = Path("../cli/dist/cli/index.js").resolve().as_posix()


if __frozen__:
    origin.main()
