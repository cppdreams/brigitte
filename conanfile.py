# https://docs.conan.io/en/latest/reference/conanfile.html?highlight=conanfile

from conans import ConanFile, tools, Meson

class Brigitte(ConanFile):
    name       = "Brigitte"
    version    = "0.1.0"
    requires   = ('yaml-cpp/0.6.2@bincrafters/stable')
    generators = 'cmake'
    settings   = "os", "compiler", "build_type", 'arch'

    def configure(self):
        self.settings.compiler.cppstd = 17

    def build(self):
        # https://docs.conan.io/en/latest/reference/build_helpers/meson.html#meson-build-reference
        meson = Meson(self)
        meson.configure(build_folder='.')
        meson.build()
