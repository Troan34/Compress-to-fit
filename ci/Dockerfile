FROM archlinux:latest

#Install dependencies
RUN pacman -Syu --noconfirm \
    clang lld libc++ clang-tools-extra\
    cmake ninja git \
    python python-pip \
    libgl mesa \
    freetype2 fontconfig libxkbcommon \
    && pacman -Scc --noconfirm

#Verify clang-scan-deps exists (fail fast, avoid silent CI failures)
RUN clang-scan-deps --version

#Install Qt with aqt into /opt/Qt
RUN pip install --break-system-packages aqtinstall && \
    aqt install-qt linux desktop 6.12.0 linux_gcc_64 --outputdir /opt/Qt

ENV CMAKE_PREFIX_PATH=/opt/Qt/6.12.0/gcc_64