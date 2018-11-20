#!/usr/bin/env python
# -*- coding: utf-8 -*-

from conans import ConanFile, CMake


class NestlConan(ConanFile):
    name = "libnestl"
    version = "0.1.0"
    description = "Strictly noexcept container library"
    topics = ("conan", "libnestl")
    url = "https://github.com/dextero/nestl"
    homepage = "https://github.com/dextero/nestl"
    author = "Marcin Radomski <marcin@mradomski.pl>"
    license = "MIT"
    exports = ["LICENSE.md"]
    exports_sources = ["CMakeLists.txt"]
    generators = "cmake"
    no_copy_source = True
    settings = "os", "arch", "compiler", "build_type"
    requires = "public-conan/doctest:bincrafters@2.0"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.test()

    def package(self):
        self.copy("*.h")

    def package_id(self):
        self.info.header_only()
