# Maintainer: you
pkgname=algoviz
pkgver=1.1
pkgrel=1
pkgdesc="Qt6 algorithm visualizer (sorting + pathfinding)"
arch=('x86_64')
url=""
license=('MIT')
depends=('qt6-base')
makedepends=('cmake' 'gcc')

build() {
    cd "$startdir"
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build build
}

package() {
    cd "$startdir"
    DESTDIR="$pkgdir" cmake --install build
}
