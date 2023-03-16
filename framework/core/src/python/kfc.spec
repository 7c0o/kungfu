# SPDX-License-Identifier: Apache-2.0

# PyInstaller Settings
###############################################################################
import glob
import pathlib
import platform
import os
import shutil

from collections import deque
from distutils import sysconfig
from os.path import (
    abspath,
    basename,
    dirname,
    curdir as cwd,
    join as make_path,
)
from PyInstaller.building.api import COLLECT, EXE, PYZ, MERGE
from PyInstaller.building.build_main import Analysis
from PyInstaller.utils.hooks import collect_data_files
from PyInstaller.utils.hooks import collect_submodules
from PyInstaller.utils.hooks import logger

###############################################################################
# python dir
python_inc = sysconfig.get_python_inc(plat_specific=True)
python_dir = (
    dirname(python_inc)
    if basename(python_inc) == "include"
    else dirname(dirname(python_inc))
)

# cmake includes
cmake_dir = abspath(make_path(cwd, ".cmake"))

# cpp dependencies
deps_dir = abspath(make_path(cwd, ".deps"))
dep_hana_dir = abspath(make_path(deps_dir, "hana-*"))
dep_sqlite_orm_dir = abspath(make_path(deps_dir, "sqlite_orm-*"))
dep_pybind11_dir = abspath(make_path(deps_dir, "pybind11*"))

# kungfu source files
src_dir = abspath(make_path(cwd, "src"))

# kungfu build files
build_dir = abspath(make_path(cwd, "build"))
build_cpp_dir = abspath(make_path(build_dir, "src"))
build_output_dir = make_path(build_dir, os.environ["CMAKE_BUILD_TYPE"])

path_env = "PYI_PYTHONPATH"
extra_python_paths = (
    os.environ[path_env].split(os.pathsep) if path_env in os.environ else []
)

###############################################################################


def extend_datas(datas, src_dirs, build_dirs, packages):
    def add_include(path):
        deque(
            map(
                lambda include: datas.append((include, "include")),
                glob.glob(make_path(path, "**", "include"), recursive=True),
            )
        )

    def add_lib(path):
        deque(
            map(
                lambda lib: datas.append((lib, ".")),
                glob.glob(make_path(path, "**", "*.lib"), recursive=True),
            )
        )

    deque(map(add_include, src_dirs))
    deque(map(add_lib, build_dirs))
    deque(map(lambda pkg: datas.extend(collect_data_files(pkg)), packages))

    if platform.system() == "Windows":
        datas.append((make_path(python_dir, "libs"), "libs"))
    return datas


def extend_hiddenimports(modules, executable_modules):
    hiddenimports = list(modules)

    def is_valid(submodule):
        return submodule not in hiddenimports and "test" not in submodule

    for pkg_name in modules:
        hiddenimports.extend(filter(is_valid, collect_submodules(pkg_name)))
    for pkg_name in executable_modules:
        hiddenimports.append(f"{pkg_name}.__main__")

    return hiddenimports


def get_hookspath():
    key = "KFC_PYI_HOOKS_PATH"
    return [] if key not in os.environ else os.environ[key].split(os.pathsep)


def get_runtimehooks():
    key = "KFC_PYI_RUNTIME_HOOKS"
    return None if key not in os.environ else os.environ[key].split(",")


###############################################################################
block_cipher = None

kfc_name = "kfc"
kfs_name = "kfs"

kfc_a = Analysis(
    scripts=["kfc.py"],
    pathex=extra_python_paths,
    binaries=[],
    datas=extend_datas(
        [
            (cmake_dir, "cmake"),
            (make_path(dep_hana_dir, "include"), "include"),
            (make_path(dep_sqlite_orm_dir, "include"), "include"),
            (dep_pybind11_dir, "pybind11"),
            (make_path(build_output_dir, "*"), "."),
            (make_path(build_dir, "include"), "include"),
        ],
        src_dirs=[
            src_dir,
        ],
        build_dirs=[
            build_cpp_dir,
        ],
        packages=[],
    ),
    hiddenimports=extend_hiddenimports(
        modules=[
            "black",
            "chardet",
            "pip._internal",
            "pip._vendor",
            "pkg_resources",
            "pdm",
            "pep517",
            "pyproject_hooks",
            "shellingham",
            "nuitka",
            "ordered_set",
            "SCons",
            "setuptools",
            "numpy",
            "pandas",
            "scipy",
            "statsmodels",
        ],
        executable_modules=[
            "kungfu",
            "pip",
        ],
    ),
    excludes=[
        "matplotlib",
    ],
    hookspath=get_hookspath(),
    runtime_hooks=get_runtimehooks(),
    cipher=block_cipher,
)
kfs_a = Analysis(scripts=["kfs.py"], pathex=extra_python_paths, cipher=block_cipher)

MERGE((kfc_a, kfc_name, kfc_name), (kfs_a, kfs_name, kfs_name))

kfc_pyz = PYZ(kfc_a.pure, kfc_a.zipped_data, cipher=block_cipher)
kfc_exe = EXE(
    kfc_pyz,
    kfc_a.scripts,
    name=kfc_name,
    console=True,
    debug=False,
    exclude_binaries=True,
    strip=False,
)
kfc_coll = COLLECT(
    kfc_exe, kfc_a.binaries, kfc_a.zipfiles, kfc_a.datas, name=kfc_name, strip=False
)

kfs_pyz = PYZ(kfs_a.pure, kfs_a.zipped_data, cipher=block_cipher)
kfs_exe = EXE(
    kfs_pyz,
    kfs_a.scripts,
    name=kfs_name,
    console=True,
    debug=False,
    exclude_binaries=True,
    strip=False,
)
kfs_coll = COLLECT(
    kfs_exe, kfs_a.binaries, kfs_a.zipfiles, kfs_a.datas, name=kfs_name, strip=False
)

###############################################################################


def copy_dll_x64(dll_pattern):
    """
    Some libs require '-x64' suffix for kfc bundled dlls like libcrypto and libssl
    :param dll_pattern:
    :return:
    """
    from wcmatch import glob

    kfc_dir = kfc_coll.name

    def locate(file):
        return abspath(make_path(kfc_dir, file))

    for dll_file in glob.glob(dll_pattern, flags=glob.EXTGLOB, root_dir=kfc_dir):
        dll_path = pathlib.Path(dll_file)
        x64_dll = make_path(dll_path.parent, dll_path.stem + "-x64" + ".dll")
        logger.info(f"Copying {locate(dll_path)} to {locate(x64_dll)}")
        shutil.copyfile(locate(dll_path), locate(x64_dll))


copy_dll_x64("libssl-*.dll")
copy_dll_x64("libcrypto-*.dll")
