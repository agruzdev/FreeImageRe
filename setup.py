from __future__ import annotations

import os
import json
import shutil
import subprocess
import sys
from pathlib import Path

from setuptools import Distribution, Extension, setup
from setuptools.command.build_ext import build_ext
from setuptools.command.build_py import build_py
from wheel.bdist_wheel import bdist_wheel


ROOT = Path(__file__).parent.resolve()
PRESETS = {
	"msvc-release": (ROOT / "build-vs18-release", "Release"),
	"msvc-debug": (ROOT / "build-vs18-debug", "Debug"),
	"linux-release": (ROOT / "build-linux-release", "Release"),
}


class CMakeExtension(Extension):
	def __init__(self, name: str) -> None:
		super().__init__(name, sources=[])


class CMakeBuildExt(build_ext):
	user_options = build_ext.user_options + [
		("cmake-preset=", None, "CMake configure preset"),
	]

	def initialize_options(self) -> None:
		super().initialize_options()
		self.cmake_preset = None

	def finalize_options(self) -> None:
		super().finalize_options()
		self.cmake_preset = (self.cmake_preset or os.environ.get("FREEIMAGE_CMAKE_PRESET") or "msvc-release")
		if self.cmake_preset not in PRESETS:
			available = ", ".join(PRESETS)
			raise RuntimeError(f"Unknown CMake preset {self.cmake_preset!r}. Available presets: {available}")

	def run(self) -> None:
		binary_dir, configuration = PRESETS[self.cmake_preset]
		self._configure(binary_dir)
		metadata = self._read_metadata(binary_dir)
		self._build(binary_dir, configuration)
		self._install(binary_dir, configuration, metadata)

	def build_extension(self, ext: Extension) -> None:
		del ext

	def _configure(self, binary_dir: Path) -> None:
		subprocess.check_call(["cmake", "--preset", self.cmake_preset, "-DFREEIMAGE_WITH_PYTHON_BINDINGS=ON"], cwd=ROOT)

	def _build(self, binary_dir: Path, configuration: str) -> None:
		command = ["cmake", "--build", str(binary_dir), "--target", "FreeImagePy"]
		if configuration:
			command.extend(["--config", configuration])
		subprocess.check_call(command, cwd=ROOT)

	def _read_metadata(self, binary_dir: Path) -> dict:
		metadata_path = binary_dir / "freeimagere-build.json"
		if not metadata_path.is_file():
			raise RuntimeError(f"CMake metadata file was not generated: {metadata_path}. Reconfigure the selected CMake preset.")
		with metadata_path.open(encoding="utf-8") as metadata_file:
			return json.load(metadata_file)

	def _install(self, binary_dir: Path, configuration: str, metadata: dict) -> None:
		command = ["cmake", "--install", str(binary_dir)]
		if configuration:
			command.extend(["--config", configuration])
		subprocess.check_call(command, cwd=ROOT)

		output_dir = Path(self.get_ext_fullpath("_freeimage")).parent
		output_dir.mkdir(parents=True, exist_ok=True)

		paths = []
		for target in metadata["targets"].values():
			if "install_path" in target:
				paths.append(target["install_path"])

		copied = set()
		for relative_path in paths:
			source = Path(relative_path)
			if source.is_file():
				shutil.copy2(source, output_dir / source.name)
				copied.add(source.resolve())

		install_root = Path(metadata["targets"]["freeimage"]["install_path"]).parent.parent
		for source in install_root.rglob("*"):
			if source.is_file() and source.resolve() not in copied:
				if source.suffix.lower() in {".pyd", ".dll", ".so"} or ".so." in source.name:
					shutil.copy2(source, output_dir / source.name)

		pyfile_path = ROOT / "Source/Python/freeimage.py"
		if pyfile_path.exists():
			shutil.copy2(pyfile_path, Path(self.build_lib) / pyfile_path.name)


class BuildPy(build_py):
	def run(self) -> None:
		super().run()
		license_path = ROOT / "license-fi.txt"
		if license_path.exists():
			target = Path(self.build_lib) / license_path.name
			target.parent.mkdir(parents=True, exist_ok=True)
			shutil.copy2(license_path, target)


class CMakeBDistWheel(bdist_wheel):
	user_options = bdist_wheel.user_options + [("cmake-preset=", None, "CMake configure preset")]

	def initialize_options(self) -> None:
		super().initialize_options()
		self.cmake_preset = None

	def finalize_options(self) -> None:
		super().finalize_options()
		self.cmake_preset = self.cmake_preset or os.environ.get("FREEIMAGE_CMAKE_PRESET")

	def run(self) -> None:
		build_ext_command = self.distribution.get_command_obj("build_ext")
		if self.cmake_preset:
			build_ext_command.cmake_preset = self.cmake_preset
		super().run()


class FreeImageDistribution(Distribution):
	def has_ext_modules(self) -> bool:
		return True


setup(
	name="FreeImageRe",
	version="4.2.2",
	description="Python bindings for FreeImageRe",
	author="Alexey Gruzdev",
	python_requires=">=3.12",
	install_requires=["numpy>= 2.0.0"],
	ext_modules=[CMakeExtension("_freeimage")],
	cmdclass={
		"build_ext": CMakeBuildExt,
		"build_py": BuildPy,
		"bdist_wheel": CMakeBDistWheel,
	},
	distclass=FreeImageDistribution,
	py_modules=[],
)
